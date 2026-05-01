#ifndef ADMIN_H
#define ADMIN_H

#include <QString>
#include <QByteArray>
#include <optional>

class PollWorker
{
private:
    QString m_username;
    QByteArray m_passwordHash;
    long long m_salt;

public:
    PollWorker() : m_salt(0) {}

    PollWorker(const PollWorker &other)
        : m_username(other.m_username),
          m_passwordHash(other.m_passwordHash),
          m_salt(other.m_salt)
    {}

    PollWorker &operator=(const PollWorker &other)
    {
        if (this != &other)
        {
            m_username = other.m_username;
            m_passwordHash = other.m_passwordHash;
            m_salt = other.m_salt;
        }
        return *this;
    }

    // Getters
    QString getUsername() const { return m_username; }
    QByteArray getPasswordHash() const { return m_passwordHash; }
    long long getSalt() const { return m_salt; }

    // Setters
    void setUsername(const QString &username) { m_username = username; }
    void setPassword(const QByteArray &hash, const long long &salt)
    {
        m_passwordHash = hash;
        m_salt = salt;
    }
};

class IWorkerRepository
{
public:
    virtual ~IWorkerRepository() = default;
    virtual bool insertWorker(const PollWorker &worker) = 0;
    virtual std::optional<PollWorker> getWorkerByUsername(const QString &username) = 0;
    virtual int getAdminCount() = 0;
};

#endif // ADMIN_H