#include "election_controller.h"
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "auth_manager.h"
#include "services//crypto/crypto_engine.h"

ElectionController::ElectionController()
    : m_configRepo(nullptr)
    , m_candidateRepo(nullptr)
    , m_voteRepo(nullptr)
    , m_auditRepo(nullptr)
    , m_tokenRepo(nullptr)
{}

ElectionController &ElectionController::getInstance()
{
    static ElectionController instance;
    return instance;
}

void ElectionController::injectDependencies(IConfigRepository *config,
                                            ICandidateRepository *candidate,
                                            IVoteRepository *vote,
                                            IAuditLogRepository *audit,
                                            IUsedTokenRepository *token)
{
    m_configRepo = config;
    m_candidateRepo = candidate;
    m_voteRepo = vote;
    m_auditRepo = audit;
    m_tokenRepo = token;
}

void ElectionController::logEvent(const QString &eventType, const QString &description)
{
    if (!m_auditRepo)
        return;
    AuditLog log;
    log.setTimestamp(QDateTime::currentDateTime().toString(Qt::ISODate));
    log.setEventType(eventType);
    log.setDescription(description);
    m_auditRepo->insertLog(log);
}

QString ElectionController::getCurrentState()
{
    auto config = m_configRepo->getConfig();
    return config.has_value() ? config->getCurrentState() : "SETUP";
}

// ==========================================
// ELECTION LIFECYCLE
// ==========================================
bool ElectionController::loadElectionDataFromUSB(const QString &jsonFilePath)
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;

    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull() || !doc.isObject())
        return false;

    QJsonObject root = doc.object();
    QJsonObject electionData = root["election"].toObject();

    // 1. Wipe old election data (but keep admins/logs)
    m_candidateRepo->clearAllCandidates();

    // 2. Load System Config
    SystemConfig config = m_configRepo->getConfig().value_or(SystemConfig());
    config.setElectionId(electionData["id"].toString());
    config.setStationId("LOCAL-STATION-1"); // Should be assigned during setup
    config.setCurrentState("LOADED");
    m_configRepo->saveConfig(config);

    // 3. Load Public Key into AuthManager
    QByteArray pubKey = QByteArray::fromBase64(root["public_key"].toString().toUtf8());
    AuthManager::getInstance().setSystemPublicKey(pubKey);

    // 4. Load Candidates
    QJsonArray candidatesArray = root["candidates"].toArray();
    for (const QJsonValue &val : candidatesArray) {
        QJsonObject candObj = val.toObject();
        Candidate c;
        c.setCnic(candObj["cnic"].toString());
        c.setPartyName(candObj["party"].toString());
        c.setSymbolName(candObj["symbolName"].toString());
        c.setSymbolBase64(candObj["symbolData"].toString());
        c.setName(candObj["name"].toString());
        c.setProfileImageBase64(candObj["imageData"].toString());
        m_candidateRepo->insertCandidate(c);
    }

    logEvent("USB_LOAD",
             "Election configuration loaded from USB by "
                 + AuthManager::getInstance().getCurrentWorker()->getUsername() + ".");
    return true;
}

bool ElectionController::startElection()
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;

    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;

    SystemConfig config = configOpt.value();
    config.setCurrentState("OPEN");
    config.setPollOpenedAt(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_configRepo->saveConfig(config);

    logEvent("POLLS_OPENED",
             "Voting has officially commenced under the supervision of "
                 + AuthManager::getInstance().getCurrentWorker()->getUsername() + ".");
    return true;
}

bool ElectionController::pauseElection()
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;

    SystemConfig config = configOpt.value();
    config.setCurrentState("PAUSED");
    m_configRepo->saveConfig(config);

    logEvent("POLLS_PAUSED",
             "Voting temporarily suspended by Admin "
                 + AuthManager::getInstance().getCurrentWorker()->getUsername() + ".");
    return true;
}

bool ElectionController::closeElection(const QString &masterPassword)
{
    // Requires Cryptographic Master Password
    if (!AuthManager::getInstance().unlockMasterAuthority(masterPassword)) {
        logEvent("UNAUTHORIZED_CLOSE_ATTEMPT",
                 "Failed attempt to close polls with invalid master password.");
        return false;
    }

    auto configOpt = m_configRepo->getConfig();
    SystemConfig config = configOpt.value();
    config.setCurrentState("CLOSED");
    config.setPollClosedAt(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_configRepo->saveConfig(config);

    logEvent("POLLS_CLOSED", "Voting permanently closed by Chief Officer.");
    return true;
}

// ==========================================
// VOTING LOGIC`
// ==========================================
bool ElectionController::castVote(const QString &candidateCnic,
                                  const QString &tokenId,
                                  QByteArray &out_receiptHash)
{
    if (getCurrentState() != "OPEN")
        return false;

    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 1. Get previous hash (if genesis block, returns empty)
    QByteArray prevHash = m_voteRepo->getLatestVoteHash();

    // 2. Generate new cryptographic hash for the chain
    QString dataToHash = candidateCnic + tokenId + timestamp;
    auto newHashOpt = CryptoEngine::getInstance().generateBlockHash(dataToHash, prevHash);

    if (!newHashOpt.has_value())
        return false;
    out_receiptHash = newHashOpt.value().toHex(); // Give to voter as receipt

    // 3. Prepare the records
    VoteRecord vote;
    vote.setCandidateCnic(candidateCnic);
    vote.setTimestamp(timestamp);
    vote.setCurrentHash(newHashOpt.value());

    Token usedToken;
    usedToken.setTokenId(tokenId);
    usedToken.setUsedAt(QDateTime::currentDateTime());

    // 4. Save to Database (DB Engineer MUST wrap this in a single SQLite Transaction)
    // We assume m_voteRepo->insertVoteTransaction takes both to ensure atomicity
    bool success = m_voteRepo->insertVoteTransaction(vote, usedToken);

    if (success) {
        logEvent("VOTE_CAST", "A ballot was successfully secured in the ledger.");
    }
    return success;
}