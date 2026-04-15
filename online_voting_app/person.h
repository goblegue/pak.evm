#ifndef PERSON_H
#define PERSON_H

#include <QDateTime>
#include <QString>

class Person
{
protected:
    QString m_id;
    QString m_cnic;
    QString m_name;
    QString m_email;
    QByteArray m_passwordHash;
    int m_salt;
    QDateTime m_createdAt;

public:
    virtual ~Person() = default;

    // Getters
    QString getId() const { return m_id; }
    QString getCnic() const { return m_cnic; }
    QString getName() const { return m_name; }
    QString getEmail() const { return m_email; }
    QByteArray getPasswordHash() const { return m_passwordHash; }
    int getSalt() const { return m_salt; }
    QDateTime getCreatedAt() const { return m_createdAt; }

    // Setters
    void setId(const QString &id) { m_id = id; }
    void setCnic(const QString &cnic) { m_cnic = cnic; }
    void setName(const QString &name) { m_name = name; }
    void setEmail(const QString &email) { m_email = email; }
    void setPassword(const QByteArray &hash, const int &salt)
    {
        m_passwordHash = hash;
        m_salt = salt;
    }
    void setCreatedAt(const QDateTime &time) { m_createdAt = time; }
};

#endif // PERSON_H
