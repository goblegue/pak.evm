#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "models/databasemanager.h"
#include "models/entities/user.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <optional>
#include <QString>
#include <QByteArray>

class userrepository : public IUserRepository
{
private:
    mongocxx::collection m_collection;

public:
    userrepository();
    bool insertUser(const User &user) override;
    std::optional<User> getUserByCnic(const QString &cnic) override;
    std::optional<User> getUserByEmail(const QString &email) override;
    bool updateUserEmailVerification(const QString &email, bool status) override;
};

#endif
