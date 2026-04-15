#ifndef OTP_H
#define OTP_H

#include <QDateTime>
#include <QString>
#include <optional>

class OTP
{
private:
    QString m_id;
    QString m_email;
    QString m_otpCode;
    QDateTime m_expiresAt;

public:
    QString getId() const { return m_id; }
    QString getEmail() const { return m_email; }
    QString getOtpCode() const { return m_otpCode; }
    QDateTime getExpiresAt() const { return m_expiresAt; }

    void setId(const QString &id) { m_id = id; }
    void setEmail(const QString &email) { m_email = email; }
    void setOtpCode(const QString &code) { m_otpCode = code; }
    void setExpiresAt(const QDateTime &time) { m_expiresAt = time; }

    // Helper logic function
    bool isExpired() const { return QDateTime::currentDateTime() > m_expiresAt; }
};

class IOtpRepository
{
public:
    virtual ~IOtpRepository() = default;
    virtual bool insertOtp(const OTP &otp) = 0;
    virtual std::optional<OTP> getLatestOtpForEmail(const QString &email) = 0;
    virtual void deleteOtpRecord(const QString &email) = 0;
};

#endif // OTP_H