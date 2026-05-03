#include "audit_controller.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "auth_manager.h"
#include "models/entities/voting_record.h"
#include "services/crypto/crypto_engine.h"
#include <sodium.h>

AuditController::AuditController()
    : m_voteRepo(nullptr),
      m_configRepo(nullptr), m_tokenRepo(nullptr)
{
}

AuditController &AuditController::getInstance()
{
    static AuditController instance;
    return instance;
}

void AuditController::injectDependencies(IVoteRepository *vote, IConfigRepository *config, ITokenRepository *token)
{
    m_voteRepo = vote;
    m_configRepo = config;
    m_tokenRepo = token;
}

bool AuditController::runForensicAudit(QString &out_failureDetails)
{
    if (!AuthManager::getInstance().isMasterUnlocked())
    {
        out_failureDetails = "Cryptographic authority required to run audit.";
        return false;
    }

    int votesSize = 0;
    // The DB Repo MUST return records strictly ORDERED BY id ASC
    VoteRecord *ledger = m_voteRepo->getAllVotesForAudit(votesSize);

    if (votesSize == 0)
    {
        out_failureDetails = "SUCCESS: Ledger is completely empty. No votes cast.";
        delete[] ledger; // Always clean up memory!
        return true;
    }

    QByteArray expectedPrevHash; // Genesis block starts with empty previous hash

    for (int i = 0; i < votesSize; ++i)
    {
        // MUST perfectly match the string used in ElectionController::castVote
        QString dataToHash = ledger[i].getCandidateCnic() + ledger[i].getTimestamp();

        auto calculatedHashOpt = CryptoEngine::getInstance().generateBlockHash(dataToHash, expectedPrevHash);

        if (!calculatedHashOpt)
        {
            out_failureDetails = "Cryptographic engine failure at row " + QString::number(ledger[i].getId());
            delete[] ledger;
            return false;
        }

        QByteArray calculatedHash = calculatedHashOpt.value();
        QByteArray storedHash = ledger[i].getCurrentHash();

        // Constant-time memory comparison to prevent side-channel leaks
        if (sodium_memcmp(calculatedHash.constData(), storedHash.constData(), storedHash.size()) != 0)
        {
            out_failureDetails = "CRITICAL BREACH: Hash mismatch detected at row " + QString::number(ledger[i].getId());
            delete[] ledger;
            return false;
        }

        // The current hash becomes the expected previous hash for the next row
        expectedPrevHash = storedHash;
    }

    delete[] ledger;
    out_failureDetails = "SUCCESS: Cryptographic ledger verified. 0 anomalies detected.";
    return true;
}

bool AuditController::exportFinalResults(const QString &outputFilePath)
{
    QString auditDetails;
    if (!runForensicAudit(auditDetails))
    {
        return false; // NEVER export if audit fails!
    }

    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;
    SystemConfig config = configOpt.value();

    // --- 1. METADATA ---
    QJsonObject metadata;
    metadata["election_id"] = config.getElectionId();
    metadata["station_id"] = config.getStationId();
    metadata["poll_opened_at"] = config.getPollOpenedAt();
    metadata["poll_closed_at"] = config.getPollClosedAt();
    metadata["exported_by_admin"] = AuthManager::getInstance().getCurrentWorker()->getUsername();

    // --- 2. CRYPTOGRAPHIC PROOF (OPTIMIZED) ---
    int totalVotes = m_voteRepo->getTotalVotesCount();
    QByteArray finalHash = m_voteRepo->getLatestVoteHash();

    QJsonObject cryptoProof;
    cryptoProof["audit_status"] = "PASSED";
    cryptoProof["final_ledger_hash"] = QString(finalHash.toHex());
    cryptoProof["total_records_audited"] = totalVotes;

    // --- 3. STATISTICS ---
    QJsonObject stats;
    stats["total_tokens_consumed"] = m_tokenRepo->getTotalTokensUsedCount();
    stats["total_votes_cast"] = totalVotes;

    // --- 4. TALLY ---
    QJsonArray tallyArray;
    int tallySize = 0;
    candidateVotes *tallyResults = m_voteRepo->getElectionTally(tallySize);

    for (int i = 0; i < tallySize; ++i)
    {
        QJsonObject candResult;
        candResult["candidate_cnic"] = tallyResults[i].candidateCnic;
        candResult["total_votes"] = tallyResults[i].totalVotes;
        tallyArray.append(candResult);
    }
    delete[] tallyResults; // Clean up memory!

    // --- ASSEMBLE PAYLOAD ---
    QJsonObject payload;
    payload["metadata"] = metadata;
    payload["cryptographic_proof"] = cryptoProof;
    payload["statistics"] = stats;
    payload["tally"] = tallyArray;

    QJsonDocument doc(payload);
    QFile file(outputFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    return false;
}