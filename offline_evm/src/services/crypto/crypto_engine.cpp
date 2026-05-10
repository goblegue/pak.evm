#include "crypto_engine.h"
#include <QDebug>
#include <QFile>
#include <QSettings>
#include "sodium.h"

bool CryptoEngine::initSodium()
{
    return (sodium_init() >= 0);
}

CryptoEngine::CryptoEngine()
{
    if (!initSodium())
    {
        qFatal("CRITICAL SECURITY ERROR: Failed to initialize libsodium.");
    }
}

CryptoEngine &CryptoEngine::getInstance()
{
    static CryptoEngine instance;
    return instance;
}

QByteArray CryptoEngine::generateAndStoreRandomSalt()
{
    QByteArray salt(crypto_pwhash_SALTBYTES, Qt::Uninitialized);
    randombytes_buf(salt.data(), salt.size());

    QString saltHex = QString(salt.toHex());

    QSettings settings;
    settings.setValue("encryptionSalt", saltHex);
    settings.sync();
    return salt;
}
std::optional<QByteArray> CryptoEngine::hashWorkerPassword(const QString &password, QByteArray &salt)
{
    QByteArray hash(64, Qt::Uninitialized);
    QByteArray pwdBytes = password.toUtf8();

    if (salt.isEmpty())
    {
        salt = generateAndStoreRandomSalt();
    }

    int result = crypto_pwhash(
        reinterpret_cast<unsigned char *>(hash.data()),
        static_cast<unsigned long long>(hash.size()),
        pwdBytes.constData(),
        static_cast<unsigned long long>(pwdBytes.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_ARGON2ID13);
    if (result != 0)
    {
        qWarning() << "CryptoEngine Warning: Password hashing failed!";
        return std::nullopt;
    }
    return hash;
}

std::optional<CryptoEngine::KeyPair> CryptoEngine::keyDerivationFunc(const QString &masterKey)
{
    QByteArray hash(64, Qt::Uninitialized);
    QByteArray salt = generateAndStoreRandomSalt();
    QByteArray pwdBytes = masterKey.toUtf8();

    int result = crypto_pwhash(
        reinterpret_cast<unsigned char *>(hash.data()),
        static_cast<unsigned long long>(hash.size()),
        pwdBytes.constData(),
        static_cast<unsigned long long>(pwdBytes.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_ARGON2ID13);

    sodium_memzero(pwdBytes.data(), pwdBytes.size());

    if (result != 0)
    {
        qWarning() << "CryptoEngine Warning: Key derivation failed!";
        return std::nullopt;
    }

    QByteArray dek = hash.left(32);
    QByteArray auditKey = hash.right(32);

    sodium_memzero(hash.data(), hash.size());

    return KeyPair{dek, auditKey};
}

bool CryptoEngine::saveKeysToVault(const KeyPair &keys)
{
    // saving DEK to vault
    QFile vaultFile("dek_vault.bin");

    if (!vaultFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "CryptoEngine Error: Failed to open vault file for writing!";
        return false;
    }

    vaultFile.write(keys.DEK);

    bool permissionSet = vaultFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (!permissionSet)
    {
        qWarning() << "CryptoEngine Warning: Failed to set vault file permissions!";
    }

    vaultFile.close();

    // saving AuditKey to a separate vault
    QFile auditFile("audit_vault.bin");
    if (!auditFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "CryptoEngine Error: Failed to open audit vault file for writing!";
        return false;
    }
    auditFile.write(keys.AuditKey);
    bool auditPermissionSet = auditFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (!auditPermissionSet)
    {
        qWarning() << "CryptoEngine Warning: Failed to set audit vault file permissions!";
    }
    auditFile.close();

    return true;
}

bool CryptoEngine::generateAndStoreKeyPair(const QString &masterKey)
{
    auto keyPairOpt = keyDerivationFunc(masterKey);
    if (!keyPairOpt)
    {
        return false;
    }

    if (!saveKeysToVault(keyPairOpt.value()))
    {
        return false;
    }

    m_auditKey = keyPairOpt->AuditKey; // Store the audit key in memory for block hashing

    return true;
}

std::optional<QByteArray> CryptoEngine::loadDekFromVault()
{
    QFile vaultFile("dek_vault.bin");

    if (!vaultFile.exists())
    {
        qCritical() << "CRITICAL SECURITY ERROR: DEK vault file is missing!";
        return std::nullopt;
    }

    if (!vaultFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "CRITICAL ERROR: OS denied access to DEK vault!";
        return std::nullopt;
    }

    QByteArray rawDek = vaultFile.readAll();
    vaultFile.close();

    if (rawDek.size() != 32)
    {
        return std::nullopt;
    }

    return rawDek;
}

std::optional<QByteArray> CryptoEngine::loadAuditKeyFromVault()
{
    QFile auditFile("audit_vault.bin");

    if (!auditFile.exists())
    {
        qCritical() << "CRITICAL SECURITY ERROR: Audit vault file is missing!";
        return std::nullopt;
    }

    if (!auditFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "CRITICAL ERROR: OS denied access to Audit vault!";
        return std::nullopt;
    }

    QByteArray rawAuditKey = auditFile.readAll();
    auditFile.close();

    if (rawAuditKey.size() != 32)
    {
        return std::nullopt;
    }

    return rawAuditKey;
}

bool CryptoEngine::verifyTokenSignature(const QString &payload, const QString &signatureBase64, const QByteArray &publicKey)
{
    if (publicKey.size() != crypto_sign_PUBLICKEYBYTES)
    {
        qCritical() << "CryptoEngine Warning: Invalid public key size for signature verification!";
        return false;
    }
    QByteArray signature = QByteArray::fromBase64(signatureBase64.toUtf8());

    if (signature.size() != crypto_sign_BYTES)
    {
        qWarning() << "CryptoEngine Warning: Invalid signature size for verification!";
        return false;
    }

    QByteArray message = payload.toUtf8();

    int result = crypto_sign_verify_detached(
        reinterpret_cast<const unsigned char *>(signature.constData()),
        reinterpret_cast<const unsigned char *>(message.constData()),
        static_cast<unsigned long long>(message.size()),
        reinterpret_cast<const unsigned char *>(publicKey.constData()));

    return (result == 0);
}

std::optional<QByteArray> CryptoEngine::generateBlockHash(const QString &blockData, const QByteArray &previousHash)
{
    if (m_auditKey.isEmpty() || m_auditKey.size() > crypto_generichash_KEYBYTES_MAX)
    {
        qCritical() << "CryptoEngine Error: Invalid audit key for block hash generation!";
        return std::nullopt;
    }

    QByteArray inputData = previousHash + blockData.toUtf8();

    QByteArray blockHash(crypto_generichash_BYTES, Qt::Uninitialized);

    int result = crypto_generichash(
        reinterpret_cast<unsigned char *>(blockHash.data()),
        blockHash.size(),
        reinterpret_cast<const unsigned char *>(inputData.constData()),
        inputData.size(),
        reinterpret_cast<const unsigned char *>(m_auditKey.constData()),
        m_auditKey.size());

    if (result != 0)
    {
        qCritical() << "CryptoEngine Error: Failed to generate block hash!";
        return std::nullopt;
    }

    return blockHash;
}

std::optional<QByteArray> CryptoEngine::encryptMessage(const QByteArray &message, const QByteArray &recipientPublicKey)
{
    if (recipientPublicKey.size() != crypto_sign_PUBLICKEYBYTES)
    {
        qWarning() << "CryptoEngine Error: Invalid public key size for encryption!";
        return std::nullopt;
    }

    // 1. Convert Ed25519 Public Key (Signing) to X25519 Public Key (Encryption)
    unsigned char curve25519_pk[crypto_box_PUBLICKEYBYTES];
    if (crypto_sign_ed25519_pk_to_curve25519(curve25519_pk, reinterpret_cast<const unsigned char *>(recipientPublicKey.constData())) != 0)
    {
        qWarning() << "CryptoEngine Error: Failed to convert public key for encryption!";
        return std::nullopt;
    }

    // 2. Prepare ciphertext buffer (Message Size + Sealed Box Overhead)
    QByteArray ciphertext;
    ciphertext.resize(message.size() + crypto_box_SEALBYTES);

    // 3. Encrypt the message (Anonymous Sealed Box)
    int result = crypto_box_seal(
        reinterpret_cast<unsigned char *>(ciphertext.data()),
        reinterpret_cast<const unsigned char *>(message.constData()),
        message.size(),
        curve25519_pk);

    if (result == 0)
    {
        return ciphertext;
    }
    return std::nullopt;
}
