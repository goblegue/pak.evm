#ifndef CRYPTOENGINE_H
#define CRYPTOENGINE_H

#include <QByteArray>
#include "sodium.h"
#include <optional>

/**
 * @struct HashResult
 * @brief Uses QByteArray to store raw binary hash and salt safely.
 */
struct HashResult
{
    QByteArray hash;
    int salt;
};

/**
 * @struct KeyPair
 * @brief Uses QByteArray for binary Ed25519 keys.
 */
struct KeyPair
{
    QByteArray publicKey;
    QByteArray privateKey;
};

class CryptoEngine
{
private:
    bool initSodium();
    CryptoEngine();
    ~CryptoEngine() = default;

public:
    static CryptoEngine &getInstance();
    CryptoEngine(const CryptoEngine &) = delete;
    void operator=(const CryptoEngine &) = delete;

    int generateRandomInt(int min, int max);

    std::optional<HashResult> hashData(const QByteArray &data, int salt = -1);

    std::optional<KeyPair> generateKeyPair();

    std::optional<QByteArray> signMessage(const QByteArray &message, const QByteArray &privateKey);

    int verifySignature(const QByteArray &message,
                        const QByteArray &signature,
                        const QByteArray &publicKey);
};

#endif // CRYPTOENGINE_H
