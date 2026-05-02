#include "controllers/auth_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "../services/crypto/crypto_engine.h"
#include <sodium.h>

AuthManager::AuthManager() 
    : m_workerRepo(nullptr), m_usedTokenRepo(nullptr),
      m_currentWorker(nullptr), m_isMasterUnlocked(false)
{
}

AuthManager::~AuthManager() {
    logoutWorker();
    lockMasterAuthority();
}

AuthManager &AuthManager::getInstance() {
    static AuthManager instance;
    return instance;
}

void AuthManager::injectDependencies(IWorkerRepository *workerRepo, ITokenRepository *tokenRepo)
{
    m_workerRepo = workerRepo;
    m_usedTokenRepo = tokenRepo;
}

// ==========================================
// worker OPERATIONAL LOGIN
// ==========================================
bool AuthManager::loginWorker(const QString &username, const QString &password)
{
    if (!m_workerRepo) return false;

    auto workerOpt = m_workerRepo->getWorkerByUsername(username);
    if (!workerOpt.has_value()) return false;

    PollWorker worker = workerOpt.value();

    QByteArray pwdBytes = password.toUtf8();
    QByteArray salt = worker.getSalt();

    // Use CryptoEngine to hash the input with the worker's stored salt
    auto hashOpt = CryptoEngine::getInstance().hashWorkerPassword(pwdBytes, salt);
    if (!hashOpt.has_value())
        return false;
    QByteArray hashedInput = hashOpt.value();

    sodium_memzero(pwdBytes.data(), pwdBytes.size()); 

    if (hashedInput.isEmpty()) return false;

    // Constant-time comparison
    if (sodium_memcmp(hashedInput.constData(), worker.getPasswordHash().constData(), hashedInput.size()) == 0) {
        m_currentWorker = std::make_unique<PollWorker>(worker);
        return true;
    }
    
    return false;
}

void AuthManager::logoutWorker() {
    m_currentWorker.reset();
}

// ==========================================
// MASTER CRYPTOGRAPHIC UNLOCK
// ==========================================
bool AuthManager::unlockMasterAuthority(const QString &masterPassword)
{
    // 1. Load the true AuditKey from the OS Vault
    auto storedAuditKeyOpt = CryptoEngine::getInstance().loadAuditKeyFromVault();
    if (!storedAuditKeyOpt.has_value()) return false;

    // 2. Re-derive the keys using the provided Master Password
    auto derivedKeysOpt = CryptoEngine::getInstance().keyDerivationFunc(masterPassword);
    if (!derivedKeysOpt.has_value()) return false;

    // 3. Compare the derived AuditKey with the stored one. If they match, the password is correct!
    if (sodium_memcmp(derivedKeysOpt->AuditKey.constData(), 
                      storedAuditKeyOpt->constData(), 
                      storedAuditKeyOpt->size()) == 0) 
    {
        m_isMasterUnlocked = true;
        return true;
    }
    
    return false;
}

void AuthManager::lockMasterAuthority() {
    m_isMasterUnlocked = false;
}

// ==========================================
// VOTER TOKEN VERIFICATION
// ==========================================
AuthManager::TokenResult AuthManager::verifyVoterToken(QString &cnic, const QString &qrPayload, QString &out_tokenId)
{
    if (!m_usedTokenRepo || m_systemPublicKey.isEmpty()) return TokenResult::ParseError;

    QJsonDocument doc = QJsonDocument::fromJson(qrPayload.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        sodium_memzero(cnic.data(), cnic.capacity() * sizeof(QChar));
        return TokenResult::ParseError;
    }

    QJsonObject obj = doc.object();
    out_tokenId = obj["tokenId"].toString();
    QString electionId = obj["electionId"].toString();
    QString issuedAt = obj["issuedAt"].toString();
    QString signatureBase64 = obj["signature"].toString();

    // Double-vote prevention
    if (m_usedTokenRepo->isTokenUsed(out_tokenId)) {
        sodium_memzero(cnic.data(), cnic.capacity() * sizeof(QChar));
        return TokenResult::AlreadyUsed;
    }

    // Reconstruct data string exactly as System 1 built it
    QString dataToVerify = cnic + electionId + issuedAt;

    // Verify signature using the new CryptoEngine
    bool isValid = CryptoEngine::getInstance().verifyTokenSignature(dataToVerify, signatureBase64, m_systemPublicKey);

    // CRITICAL: SECURE WIPE OF TRANSIENT VARIABLES
    sodium_memzero(cnic.data(), cnic.capacity() * sizeof(QChar));
    sodium_memzero(dataToVerify.data(), dataToVerify.capacity() * sizeof(QChar));

    if (!isValid) return TokenResult::InvalidSignature;

    return TokenResult::Valid;
}