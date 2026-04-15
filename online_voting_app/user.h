#ifndef USER_H
#define USER_H

#include "person.h"
#include <optional>

class User : public Person
{
private:
    bool m_isEmailVerified;

public:
    User()
        : m_isEmailVerified(false)
    {}

    bool isEmailVerified() const { return m_isEmailVerified; }
    void setEmailVerified(bool verified) { m_isEmailVerified = verified; }
};

class IUserRepository
{
public:
    virtual ~IUserRepository() = default;
    virtual bool insertUser(const User &user) = 0;
    virtual std::optional<User> getUserByCnic(const QString &cnic) = 0;
    virtual std::optional<User> getUserByEmail(const QString &email) = 0;
    virtual bool updateUserEmailVerification(const QString &cnic, bool status) = 0;
};

#endif // USER_H