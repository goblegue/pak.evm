#ifndef USER_H
#define USER_H

#include <optional>

class User
{
protected:
    QString m_id;
    QString m_cnic;
    QString m_name;
    QString m_email;
    QByteArray m_passwordHash;
    long long m_salt;
    bool m_isEmailVerified;

public:
    User() = default;
    
    User(const User &other)
        : m_id(other.m_id),
          m_cnic(other.m_cnic),
          m_name(other.m_name),
          m_email(other.m_email),
          m_passwordHash(other.m_passwordHash),
          m_salt(other.m_salt),
          m_isEmailVerified(other.m_isEmailVerified)
    {
    }

    virtual ~User() = default;

    // Getters
    QString getId() const { return m_id; }
    QString getCnic() const { return m_cnic; }
    QString getName() const { return m_name; }
    QString getEmail() const { return m_email; }
    QByteArray getPasswordHash() const { return m_passwordHash; }
    int getSalt() const { return m_salt; }

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
    bool isEmailVerified() const { return m_isEmailVerified; }
    void setEmailVerified(bool verified) { m_isEmailVerified = verified; }

    User &operator=(const User &other)
    {
        if (this != &other) // Prevent self-assignment crash
        {
            m_id = other.m_id;
            m_cnic = other.m_cnic;
            m_name = other.m_name;
            m_email = other.m_email;
            m_passwordHash = other.m_passwordHash;
            m_salt = other.m_salt;
            m_isEmailVerified = other.m_isEmailVerified;
        }
        return *this;
    }
};

class IUserRepository
{
public:
    virtual ~IUserRepository() = default;
    virtual bool insertUser(const User &user) = 0;
    virtual std::optional<User> getUserByCnic(const QString &cnic) = 0;
    virtual std::optional<User> getUserByEmail(const QString &email) = 0;
    virtual bool updateUserEmailVerification(const QString &email, bool status) = 0;
};

#endif // USER_H