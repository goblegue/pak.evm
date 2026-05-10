#ifndef CRYPTOENGINE_H
#define CRYPTOENGINE_H

#include <QByteArray>
#include "sodium.h"
#include <optional>

class CryptoEngine
{
private:
    bool initSodium();
    CryptoEngine();
    ~CryptoEngine() = default;

public:
    /**
     * @struct KeyPair
     * @brief Uses QByteArray for binary Ed25519 keys.
     */
    struct KeyPair
    {
        QByteArray publicKey;
        QByteArray privateKey;
    };
    /**
     * @struct HashResult
     * @brief Uses QByteArray to store raw binary hash and salt safely.
     */
    struct HashResult
    {
        QByteArray hash;
        long long salt;
    };
    static CryptoEngine &getInstance();
    CryptoEngine(const CryptoEngine &) = delete;
    void operator=(const CryptoEngine &) = delete;

    long long generateRandomInt(long long min, long long max);

    std::optional<HashResult> hashData(const QByteArray &data, long long salt = -1);

    std::optional<KeyPair> generateKeyPair();

    std::optional<QByteArray> signMessage(const QByteArray &message, const QByteArray &privateKey);

    int verifySignature(const QByteArray &message,
                        const QByteArray &signature,
                        const QByteArray &publicKey);
};

#endif 
