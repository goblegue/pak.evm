#ifndef CRYPTOENGINE_H
#define CRYPTOENGINE_H

#include <QByteArray>
#include <QString>
#include <optional>


class CryptoEngine
{
private:
    QByteArray m_auditKey;

    bool initSodium();
    CryptoEngine();
    ~CryptoEngine() = default;

public:
    struct KeyPair
    {
        QByteArray DEK;
        QByteArray AuditKey;
    };

    static CryptoEngine &getInstance();
    CryptoEngine(const CryptoEngine &) = delete;
    void operator=(const CryptoEngine &) = delete;

    QByteArray generateAndStoreRandomSalt();
    std::optional<KeyPair> keyDerivationFunc(const QString &masterKey);
    bool saveKeysToVault(const KeyPair &keys);
    std::optional<QByteArray> loadDekFromVault();
    std::optional<QByteArray> loadAuditKeyFromVault();
    bool generateAndStoreKeyPair(const QString &masterKey);

    bool verifyTokenSignature(const QString &payload,
                              const QString &signatureBase64,
                              const QByteArray &publicKey);

    std::optional<QByteArray> generateBlockHash(const QString &blockData,
                                                const QByteArray &previousHash);
};

#endif