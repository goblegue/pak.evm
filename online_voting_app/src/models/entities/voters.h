#ifndef VOTERS_H
#define VOTERS_H
#include <optional>

#include <QDateTime>
#include <QString>
#include <QUuid>
#include "services/crypto/cryptoengine.h"

using namespace std;

class Token
{
private:
    QString m_id;
    QString m_userCnic;
    QString m_electionId;
    QString m_assignedStationId;
    QString m_tokenSignature;
    QDateTime m_issuedAt;

public:
    // Getters
    QString getId() const { return m_id; }
    QString getElectionId() const { return m_electionId; }
    QString getAssignedStationId() const { return m_assignedStationId; }
    QString getTokenSignature() const { return m_tokenSignature; }
    QDateTime getIssuedAt() const { return m_issuedAt; }
    QString getUserCnic() const { return m_userCnic; }

    // Setters
    void generateId() { m_id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    void setElectionId(const QString &electionId) { m_electionId = electionId; }
    void setAssignedStationId(const QString &stationId) { m_assignedStationId = stationId; }
    void setTokenSignature(const QString &sig) { m_tokenSignature = sig; }
    void setIssuedAt(const QDateTime &time) { m_issuedAt = time; }
    void setUserCnic(const QString &cnic) { m_userCnic = cnic; }
    void setId(const QString &id) { m_id = id; }

    static optional<QString> generateSignature(const QString &Id, const QString &electionId, const QDateTime &issuedAt, const QByteArray &privateKey)
    {
        QString data = Id + "|" + electionId + "|" + issuedAt.toString(Qt::ISODate);
        auto signatureOpt = CryptoEngine::getInstance().signMessage(data.toUtf8(), privateKey);
        if (signatureOpt.has_value())
        {
            return QString::fromUtf8(signatureOpt.value().toBase64());
        }
        return std::nullopt;
    }
};

class ITokenRepository
{
public:
    virtual ~ITokenRepository() = default;
    virtual bool insertToken(const Token &token) = 0;
    virtual Token *getTokensByElection(const QString &electionId, int &votersSize) = 0;
    virtual Token *getTokensByUser(const QString &userCnic, int &tokensSize) = 0;
    // Critical function for security constraint:
    virtual bool hasUserRequestedToken(const QString &userCnic, const QString &electionId) = 0;
};

#endif // VOTERS_H
