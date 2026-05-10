

#include "auth_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "services/crypto/crypto_engine.h"
#include <sodium.h>

AuthManager::AuthManager()
    : m_workerRepo(nullptr)
    , m_usedTokenRepo(nullptr)
    , m_currentWorker(nullptr)
    , m_isMasterUnlocked(false)
    , m_configRepo(nullptr)
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

void AuthManager::injectDependencies(IWorkerRepository *workerRepo,
                                     ITokenRepository *tokenRepo,
                                     IConfigRepository *configRepo)
{
    m_workerRepo = workerRepo;
    m_usedTokenRepo = tokenRepo;
    m_configRepo = configRepo;
}

// ==========================================
// WORKER CREATION & LOGIN
// ==========================================
bool AuthManager::createOperationalWorker(const QString &username, const QString &password)
{
    if (!m_workerRepo) return false;

    // Generate a secure 16-byte salt for this worker
    QByteArray salt(crypto_pwhash_SALTBYTES, Qt::Uninitialized);
    randombytes_buf(salt.data(), salt.size());

    QByteArray pwdBytes = password.toUtf8();
    
    // Assumes CryptoEngine::hashWorkerPassword is implemented in your codebase
    auto hashOpt = CryptoEngine::getInstance().hashWorkerPassword(pwdBytes, salt);
    
    sodium_memzero(pwdBytes.data(), pwdBytes.size()); // Wipe immediately

    if (!hashOpt.has_value()) return false;

    PollWorker worker;
    worker.setUsername(username);
    worker.setPassword(hashOpt.value(), salt);

    return m_workerRepo->insertWorker(worker);
}

bool AuthManager::loginWorker(const QString &username, const QString &password)
{
    if (!m_workerRepo) return false;

    auto workerOpt = m_workerRepo->getWorkerByUsername(username);
    if (!workerOpt.has_value()) return false;

    PollWorker worker = workerOpt.value();

    QByteArray pwdBytes = password.toUtf8();
    QByteArray salt = worker.getSalt();

    auto hashOpt = CryptoEngine::getInstance().hashWorkerPassword(pwdBytes, salt);
    sodium_memzero(pwdBytes.data(), pwdBytes.size()); 

    if (!hashOpt.has_value())
        return false;
    QByteArray hashedInput = hashOpt.value();

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
    auto storedAuditKeyOpt = CryptoEngine::getInstance().loadAuditKeyFromVault();
    if (!storedAuditKeyOpt.has_value()) return false;

    auto derivedKeysOpt = CryptoEngine::getInstance().keyDerivationFunc(masterPassword);
    if (!derivedKeysOpt.has_value()) return false;

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
    QByteArray m_systemPublicKey = QByteArray::fromBase64(
        m_configRepo->getConfig()->getPublicKeyBase64().toUtf8());
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

    if (m_usedTokenRepo->isTokenUsed(out_tokenId)) {
        sodium_memzero(cnic.data(), cnic.capacity() * sizeof(QChar));
        return TokenResult::AlreadyUsed;
    }

    QString dataToVerify = cnic + "|" + electionId + "|" + issuedAt;

    bool isValid = CryptoEngine::getInstance().verifyTokenSignature(dataToVerify, signatureBase64, m_systemPublicKey);

    // m_usedTokenRepo->markTokenAsUsed(out_tokenId); // Mark as used regardless of validity to prevent replay

    //[CRITICAL FIX] Used .size() instead of .capacity() to prevent heap corruption!
    sodium_memzero(cnic.data(), cnic.size() * sizeof(QChar));
    sodium_memzero(dataToVerify.data(), dataToVerify.size() * sizeof(QChar));

    if (!isValid) return TokenResult::InvalidSignature;

    return TokenResult::Valid;
}