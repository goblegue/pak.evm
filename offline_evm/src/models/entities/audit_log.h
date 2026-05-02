
#ifndef AUDIT_LOG_H
#define AUDIT_LOG_H

#include <QString>
#include <QDateTime>
#include <cstddef>
class AuditLog
{
private:
    QString m_timestamp;
    QString m_eventType; // e.g., "SYSTEM_BOOT", "ADMIN_LOGIN", "USB_LOADED", "POLLS_CLOSED"
    QString m_description;

public:
    AuditLog() {}

    QString getTimestamp() const { return m_timestamp; }
    QString getEventType() const { return m_eventType; }
    QString getDescription() const { return m_description; }

    void setTimestamp(const QString &time) { m_timestamp = time; }
    void setEventType(const QString &type) { m_eventType = type; }
    void setDescription(const QString &desc) { m_description = desc; }
};

class IAuditLogRepository
{
public:
    virtual ~IAuditLogRepository() = default;
    virtual bool insertLog(const AuditLog &log) = 0;
    virtual AuditLog *getAllLogs(int &logsSize) = 0;
};

#endif // AUDIT_LOG_H
