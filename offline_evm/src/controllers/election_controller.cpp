#include "election_controller.h"
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "auth_manager.h"
#include "services/crypto/crypto_engine.h"

ElectionController::ElectionController()
    : m_configRepo(nullptr), m_candidateRepo(nullptr), m_voteRepo(nullptr), m_auditRepo(nullptr), m_tokenRepo(nullptr)
{
}

ElectionController &ElectionController::getInstance()
{
    static ElectionController instance;
    return instance;
}

void ElectionController::injectDependencies(IConfigRepository *config,
                                            ICandidateRepository *candidate,
                                            IVoteRepository *vote,
                                            IAuditLogRepository *audit,
                                            ITokenRepository *token)
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

ElectionState ElectionController::getCurrentState()
{
    auto config = m_configRepo->getConfig();
    return config.has_value() ? config->getCurrentState() : ElectionState::Setup;
}

// ==========================================
// SETUP FLOW
// ==========================================
bool ElectionController::loadElectionDataFromUSB(const QString &jsonFilePath)
{
    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull() || !doc.isObject())
    {
        return false;
    }

    QJsonObject root = doc.object();

    // ==============================================================
    // 1. CRYPTOGRAPHIC VERIFICATION
    // ==============================================================
    QString signatureBase64 = root["signature"].toString();
    if (signatureBase64.isEmpty())
    {
        logEvent("USB_LOAD_FAILED", "Missing digital signature in JSON.");
        return false;
    }

    // Create a copy of the JSON and remove the signature to recreate the original signed payload
    QJsonObject verifyObj = root;
    verifyObj.remove("signature");

    // Convert back to Compact JSON string exactly as System 1 generated it
    QJsonDocument verifyDoc(verifyObj);
    QString payloadToVerify = QString::fromUtf8(verifyDoc.toJson(QJsonDocument::Compact));

    // Get the Public Key that the Admin entered during the Wizard (Step 2)
    SystemConfig config = m_configRepo->getConfig().value_or(SystemConfig());
    QByteArray pubKey = QByteArray::fromBase64(config.getPublicKeyBase64().toUtf8());

    // Verify!
    if (!CryptoEngine::getInstance().verifyTokenSignature(payloadToVerify, signatureBase64, pubKey))
    {
        logEvent("USB_LOAD_FAILED", "SECURITY ALERT: JSON signature verification failed. File may be tampered with!");
        return false;
    }

    // ==============================================================
    // 2. PARSE AND SAVE SYSTEM CONFIGURATION
    // ==============================================================
    QJsonObject electionData = root["election"].toObject();

    config.setElectionId(electionData["id"].toString());
    config.setStationId("LOCAL-STATION-1"); // Assigned during setup

    // [CHANGED] Parse the ISO 8601 Strings into MSecsSinceEpoch for our Timers
    QDateTime startTime = QDateTime::fromString(electionData["startTime"].toString(), Qt::ISODate);
    QDateTime endTime = QDateTime::fromString(electionData["endTime"].toString(), Qt::ISODate);

    if (!startTime.isValid() || !endTime.isValid())
    {
        logEvent("USB_LOAD_FAILED", "Invalid time formats in JSON.");
        return false;
    }

    config.setScheduledStartTime(startTime.toMSecsSinceEpoch());
    config.setScheduledEndTime(endTime.toMSecsSinceEpoch());
    config.setCurrentState(ElectionState::ReadyWaiting);

    m_configRepo->saveConfig(config);

    // ==============================================================
    // 3. WIPE OLD CANDIDATES & LOAD NEW ONES
    // ==============================================================
    m_candidateRepo->clearAllCandidates();

    QJsonArray candidatesArray = root["candidates"].toArray();
    for (const QJsonValue &val : candidatesArray)
    {
        QJsonObject candObj = val.toObject();
        Candidate c;
        c.setCnic(candObj["cnic"].toString());
        c.setPartyName(candObj["party"].toString());
        c.setSymbolName(candObj["symbolName"].toString());
        c.setSymbolBase64(candObj["symbolData"].toString());

        // Note: Using .toString() gracefully returns "" if a field is missing in JSON
        c.setName(candObj["name"].toString());
        c.setProfileImageBase64(candObj["imageData"].toString());

        m_candidateRepo->insertCandidate(c);
    }

    logEvent("USB_LOAD", "Election configuration verified and loaded from USB by Master Admin.");
    return true;
}

// ==========================================
// AUTOMATED TIMER FLOW
// ==========================================
qint64 ElectionController::getSecondsUntilStart()
{
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return 0;
    qint64 diffMSec = configOpt->getScheduledStartTime() - QDateTime::currentMSecsSinceEpoch();
    return (diffMSec > 0) ? (diffMSec / 1000) : 0;
}

qint64 ElectionController::getSecondsUntilEnd()
{
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return 0;
    qint64 diffMSec = configOpt->getScheduledEndTime() - QDateTime::currentMSecsSinceEpoch();
    return (diffMSec > 0) ? (diffMSec / 1000) : 0;
}

bool ElectionController::autoOpenElection()
{
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;
    SystemConfig config = configOpt.value();
    config.setCurrentState(ElectionState::Open);
    config.setPollOpenedAt(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_configRepo->saveConfig(config);
    logEvent("POLLS_OPENED", "Voting automatically commenced via timer.");
    return true;
}

bool ElectionController::autoCloseElection()
{
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;
    SystemConfig config = configOpt.value();
    config.setCurrentState(ElectionState::Closed);
    config.setPollClosedAt(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_configRepo->saveConfig(config);
    logEvent("POLLS_CLOSED", "Voting automatically closed via timer.");
    return true;
}

// ==========================================
// POLL WORKER EMERGENCY CONTROLS
// ==========================================
bool ElectionController::pauseElection()
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;

    SystemConfig config = configOpt.value();
    config.setCurrentState(ElectionState::Paused);
    m_configRepo->saveConfig(config);

    logEvent("POLLS_PAUSED", "Voting suspended by Worker " + AuthManager::getInstance().getCurrentWorker()->getUsername() + ".");
    return true;
}

bool ElectionController::resumeElection()
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;

    SystemConfig config = configOpt.value();
    config.setCurrentState(ElectionState::Open);
    m_configRepo->saveConfig(config);

    logEvent("POLLS_RESUMED", "Voting resumed by Worker.");
    return true;
}

bool ElectionController::extendElectionTime(int addedMinutes)
{
    if (!AuthManager::getInstance().getCurrentWorker())
        return false;
    auto configOpt = m_configRepo->getConfig();
    if (!configOpt)
        return false;

    SystemConfig config = configOpt.value();
    qint64 extraMSec = addedMinutes * 60 * 1000;
    config.setScheduledEndTime(config.getScheduledEndTime() + extraMSec);

    m_configRepo->saveConfig(config);
    logEvent("ELECTION_EXTENDED", QString("Voting extended by %1 minutes.").arg(addedMinutes));
    return true;
}

bool ElectionController::closeElection(const QString &masterPassword)
{
    if (!AuthManager::getInstance().unlockMasterAuthority(masterPassword))
    {
        logEvent("UNAUTHORIZED_CLOSE_ATTEMPT", "Failed attempt to force-close polls with invalid master password.");
        return false;
    }

    auto configOpt = m_configRepo->getConfig();
    SystemConfig config = configOpt.value();
    config.setCurrentState(ElectionState::Closed);
    config.setPollClosedAt(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_configRepo->saveConfig(config);

    logEvent("POLLS_FORCE_CLOSED", "Voting permanently closed by Chief Officer.");
    return true;
}

// ==========================================
// VOTING LOGIC
// ==========================================
bool ElectionController::castVote(const QString &candidateCnic,
                                  const QString &tokenId,
                                  QByteArray &out_receiptHash)
{
    if (getCurrentState() != ElectionState::Open)
        return false;

    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QByteArray prevHash = m_voteRepo->getLatestVoteHash();

    QString dataToHash = candidateCnic + timestamp;
    auto newHashOpt = CryptoEngine::getInstance().generateBlockHash(dataToHash, prevHash);

    if (!newHashOpt.has_value())
        return false;
    out_receiptHash = newHashOpt.value().toHex();

    VoteRecord vote;
    vote.setCandidateCnic(candidateCnic);
    vote.setTimestamp(timestamp);
    vote.setCurrentHash(newHashOpt.value());

    Token usedToken;
    usedToken.setTokenId(tokenId);
    usedToken.setUsedAt(QDateTime::currentDateTime());

    bool success = m_voteRepo->insertVoteTransaction(vote, usedToken);

    if (success)
    {
        logEvent("VOTE_CAST", "A ballot was successfully secured in the ledger.");
    }
    return success;
}