#ifndef VOTERS_H
#define VOTERS_H

#include <QDateTime>
#include <QString>
#include <optional>
#include "./crypto/cryptoengine.h"

class Voters
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
    QString getUserCnic() const { return m_userCnic; }
    QString getElectionId() const { return m_electionId; }
    QString getAssignedStationId() const { return m_assignedStationId; }
    QString getTokenSignature() const { return m_tokenSignature; }
    QDateTime getIssuedAt() const { return m_issuedAt; }

    // Setters
    void setId(const QString &id) { m_id = id; }
    void setUserCnic(const QString &cnic) { m_userCnic = cnic; }
    void setElectionId(const QString &electionId) { m_electionId = electionId; }
    void setAssignedStationId(const QString &stationId) { m_assignedStationId = stationId; }
    void setTokenSignature(const QString &sig) { m_tokenSignature = sig; }
    void setIssuedAt(const QDateTime &time) { m_issuedAt = time; }
    static optional<QString> generateSignature(const QString &userCnic, const QString &electionId, const QDateTime &issuedAt, const QByteArray &privateKey)
    {
        QString data = userCnic + electionId + issuedAt.toString(Qt::ISODate);
        auto signatureOpt = CryptoEngine::getInstance().signMessage(data.toUtf8(), privateKey);
        if (signatureOpt.has_value())
        {
            return QString::fromUtf8(signatureOpt.value());
        }
        return std::nullopt;
    }
};

class ITokenRepository
{
public:
    virtual ~ITokenRepository() = default;
    virtual bool insertToken(const Voters &token) = 0;
    // Critical function for security constraint:
    virtual bool hasUserRequestedToken(const QString &userCnic, const QString &electionId) = 0;
};

#endif // VOTERS_H
