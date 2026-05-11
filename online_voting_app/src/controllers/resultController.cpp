#include "controllers/resultController.h"
#include "services/crypto/cryptoengine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

ResultController::ResultController() : m_resultRepo(nullptr), m_electionRepo(nullptr) {}
ResultController::~ResultController() {}

ResultController& ResultController::getInstance() {
    static ResultController instance;
    return instance;
}

void ResultController::injectRepositories(IPollResultRepository* resultRepo, IElectionRepository* electionRepo) {
    m_resultRepo = resultRepo;
    m_electionRepo = electionRepo;
}

// ==========================================
// STEP 1: DECRYPT & PARSE (For Frontend Preview)
// ==========================================
std::optional<Result> ResultController::loadAndPreviewResultFile(const QString& filePath, const QByteArray& publicKey, const QByteArray& privateKey) 
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    QByteArray encryptedData = file.readAll();
    file.close();

    qDebug() << "--- CRYPTO DEBUG ---";
    qDebug() << "Expected PK Size: 32 | Actual PK Size:" << publicKey.size();
    qDebug() << "Expected SK Size: 64 | Actual SK Size:" << privateKey.size();

    // 1. Decrypt using the System Keys
    auto decryptedOpt = CryptoEngine::getInstance().decryptMessage(encryptedData, publicKey, privateKey);
    if (!decryptedOpt.has_value()) return std::nullopt;

    // 2. Parse JSON Structure
    QJsonDocument doc = QJsonDocument::fromJson(decryptedOpt.value());
    if (doc.isNull() || !doc.isObject()) return std::nullopt;
    
    QJsonObject root = doc.object();
    QJsonObject crypto = root["cryptographic_proof"].toObject();
    QJsonObject meta = root["metadata"].toObject();
    QJsonObject stats = root["statistics"].toObject();
    QJsonArray tallyArray = root["tally"].toArray();

    // 3. Map to Flattened Model
    Result result;
    result.setElectionId(meta["election_id"].toString());
    result.setPollOpenedAt(QDateTime::fromString(meta["poll_opened_at"].toString(), Qt::ISODate));
    result.setPollClosedAt(QDateTime::fromString(meta["poll_closed_at"].toString(), Qt::ISODate));
    
    result.setTotalVotesCast(stats["total_votes_cast"].toInt());
    
    result.setAuditStatus(crypto["audit_status"].toString());
    result.setFinalLedgerHash(crypto["final_ledger_hash"].toString());

    // 4. Handle Dynamic Array for Tally
    int count = tallyArray.size();
    if (count > 0) {
        CandidateTally* tempTally = new CandidateTally[count];
        for (int i = 0; i < count; ++i) {
            QJsonObject item = tallyArray[i].toObject();
            tempTally[i].candidateCnic = item["candidate_cnic"].toString();
            tempTally[i].totalVotes = item["total_votes"].toInt();
        }
        result.setTallyData(tempTally, count);
        delete[] tempTally; // Clean up temp buffer, Result made its own copy
    }

    return result;
}

// ==========================================
// STEP 2: COMMIT TO DATABASE
// ==========================================
bool ResultController::commitFinalResults(const Result& result) {
    if (!m_resultRepo || !m_electionRepo) return false;

    // 1. Check if results for this election already exist to prevent duplicates
    if (m_resultRepo->getResultByElection(result.getElectionId()).has_value()) {
        return false; 
    }

    // 2. Save to MongoDB Results Collection
    if (!m_resultRepo->insertPollResult(result)) {
        return false;
    }

    // 3. Update Election State to "ResultsAnnounced"
    return m_electionRepo->updateElectionState(result.getElectionId(), ElectionState::ResultsAnnounced);
}

// ==========================================
// UTILITY: FETCH RESULTS
// ==========================================
std::optional<Result> ResultController::getElectionResults(const QString& electionId) {
    if (!m_resultRepo) return std::nullopt;
    return m_resultRepo->getResultByElection(electionId);
}