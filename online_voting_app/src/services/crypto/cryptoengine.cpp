#include <optional>
#include <limits>
#include <QBuffer>     // Required for QBuffer
#include <QDataStream> // Required for QDataStream
#include <QDebug>
#include <QIODevice> // Required for QIODevice::WriteOnly flags
#include "services/crypto/cryptoengine.h"
#include "sodium.h"

using namespace std;

bool CryptoEngine::initSodium()
{
    return (sodium_init() >= 0);
}

CryptoEngine::CryptoEngine()
{
    if (!initSodium())
    {
        qFatal("CRITICAL SECURITY ERROR: Failed to initialize libsodium. "
               "The application cannot run without a valid cryptographic engine.");
    }
}

CryptoEngine &CryptoEngine::getInstance()
{
    static CryptoEngine instance;
    return instance;
}

long long CryptoEngine::generateRandomInt(long long min, long long max)
{
    if (min > max)
    {
        qWarning()
            << "CryptoEngine Warning: generateRandomInt called with min > max. Swapping values.";
        swap(min, max);
    }
    // 2. Generate raw 64-bit randomness
    unsigned long long rawRandom;
    randombytes_buf(&rawRandom, sizeof(rawRandom));

    // 3. Make it positive (Mask out the sign bit)
    // This ensures the number is between 0 and LLONG_MAX
    rawRandom &= 0x7FFFFFFFFFFFFFFF;

    // 4. Calculate the range (Using unsigned to prevent overflow)
    unsigned long long range = static_cast<unsigned long long>(max - min) + 1;

    // 5. Apply the range and shift
    // We use the modulo operator here.
    // Architect's Note: While modulo has a tiny bias, at a 64-bit scale,
    // the bias is mathematically invisible and safe for a voting salt.
    return min + static_cast<long long>(rawRandom % range);
}

std::optional<CryptoEngine::HashResult> CryptoEngine::hashData(const QByteArray &data, long long salt)
{
    if (salt < 0)
    {
        salt = generateRandomInt(0, numeric_limits<long long>::max());
    }
    // converting the salt integer to a QByteArray of the correct size for crypto_pwhash
    QByteArray saltArray{};
    saltArray.fill(0, crypto_pwhash_SALTBYTES);

    QBuffer buffer(&saltArray);

    buffer.open(QIODevice::WriteOnly);

    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (qint64)salt;

    buffer.close();

    // Hash the data using Argon2id
    QByteArray outHash(32, 0);

    int result = crypto_pwhash(reinterpret_cast<unsigned char *>(outHash.data()),
                               static_cast<unsigned long long>(outHash.size()),
                               data.constData(),
                               static_cast<unsigned long long>(data.size()),
                               reinterpret_cast<const unsigned char *>(saltArray.constData()),
                               crypto_pwhash_OPSLIMIT_INTERACTIVE,
                               crypto_pwhash_MEMLIMIT_INTERACTIVE,
                               crypto_pwhash_ALG_ARGON2ID13);
    if (result == 0)
        return HashResult{outHash, salt};
    else
        return nullopt;
}

std::optional<CryptoEngine::KeyPair> CryptoEngine::generateKeyPair()
{
    QByteArray publicKey, privateKey;
    publicKey.resize(crypto_sign_PUBLICKEYBYTES);
    privateKey.resize(crypto_sign_SECRETKEYBYTES);

    int result = crypto_sign_keypair(reinterpret_cast<unsigned char *>(publicKey.data()),
                                     reinterpret_cast<unsigned char *>(privateKey.data()));

    if (result == 0)
        return KeyPair{publicKey, privateKey};
    else
        return nullopt;
}

std::optional<QByteArray> CryptoEngine::signMessage(const QByteArray &message,
                                                    const QByteArray &privateKey)
{
    if (privateKey.size() != crypto_sign_SECRETKEYBYTES)
    {
        qWarning() << "CryptoEngine Warning: Invalid private key size for signing!";
        return nullopt;
    }

    QByteArray signature;
    signature.resize(crypto_sign_BYTES);

    unsigned long long sigLen{};
    int result = crypto_sign_detached(reinterpret_cast<unsigned char *>(signature.data()),
                                      &sigLen,
                                      reinterpret_cast<const unsigned char *>(message.constData()),
                                      static_cast<unsigned long long>(message.size()),
                                      reinterpret_cast<const unsigned char *>(
                                          privateKey.constData()));
    if (result == 0)
    {
        signature.resize(sigLen);
        return signature;
    }
    else
    {
        return nullopt;
    }
}

int CryptoEngine::verifySignature(const QByteArray &message,
                                  const QByteArray &signature,
                                  const QByteArray &publicKey)
{
    if (publicKey.size() != crypto_sign_PUBLICKEYBYTES)
    {
        qWarning() << "CryptoEngine Error: Invalid public key size for verification!";
        return -1;
    }

    if (signature.size() != crypto_sign_BYTES)
    {
        qWarning() << "CryptoEngine Error: Signature must be exactly" << crypto_sign_BYTES
                   << "bytes!";
        return -1;
    }

    int result = crypto_sign_verify_detached(reinterpret_cast<const unsigned char *>(
                                                 signature.constData()),
                                             reinterpret_cast<const unsigned char *>(
                                                 message.constData()),
                                             static_cast<unsigned long long>(message.size()),
                                             reinterpret_cast<const unsigned char *>(
                                                 publicKey.constData()));

    if (result == 0)
    {
        return 1;
    }
    else
        return 0;
}

std::optional<QByteArray> CryptoEngine::decryptMessage(const QByteArray &encryptedMessage,
                                                       const QByteArray &publicKey,
                                                       const QByteArray &privateKey)
{
    // 1. Validate inputs
    if (publicKey.size() != crypto_sign_PUBLICKEYBYTES || privateKey.size() != crypto_sign_SECRETKEYBYTES) {
        qWarning() << "CryptoEngine Error: Invalid key sizes for decryption!";
        return std::nullopt;
    }

    if (encryptedMessage.size() < crypto_box_SEALBYTES) {
        qWarning() << "CryptoEngine Error: Encrypted message is too short to be a valid sealed box!";
        return std::nullopt;
    }

    // 2. Convert Ed25519 Keys to X25519 Keys
    unsigned char curve25519_pk[crypto_box_PUBLICKEYBYTES];
    unsigned char curve25519_sk[crypto_box_SECRETKEYBYTES];

    if (crypto_sign_ed25519_pk_to_curve25519(curve25519_pk, reinterpret_cast<const unsigned char*>(publicKey.constData())) != 0) {
        return std::nullopt;
    }
    if (crypto_sign_ed25519_sk_to_curve25519(curve25519_sk, reinterpret_cast<const unsigned char*>(privateKey.constData())) != 0) {
        return std::nullopt;
    }

    // 3. Prepare plaintext buffer
    QByteArray decrypted;
    decrypted.resize(encryptedMessage.size() - crypto_box_SEALBYTES);

    // 4. Decrypt the Sealed Box
    int result = crypto_box_seal_open(
        reinterpret_cast<unsigned char *>(decrypted.data()),
        reinterpret_cast<const unsigned char *>(encryptedMessage.constData()),
        encryptedMessage.size(),
        curve25519_pk,
        curve25519_sk
    );

    if (result == 0) {
        return decrypted;
    } else {
        qWarning() << "CryptoEngine Error: Decryption failed! The data was tampered with or keys are incorrect.";
        return std::nullopt;
    }
}
