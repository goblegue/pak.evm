#include "userrepository.h"
#include <QDebug>
#include "../databasemanager.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

userrepository::userrepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Users"];
}
bool userrepository::insertUser(const User &user)
{
    try
    {
        // 1. Prepare the binary password hash
        QByteArray tempHash = user.getPasswordHash();
        bsoncxx::types::b_binary binary_hash;
        binary_hash.bytes = reinterpret_cast<const uint8_t *>(tempHash.constData());
        binary_hash.size = static_cast<uint32_t>(tempHash.size());
        binary_hash.sub_type = bsoncxx::binary_sub_type::k_binary;

        // 2. EXPLICITLY create a BSON 64-bit integer object for the salt

        // 3. Build the document
        auto final_doc = document{} << "cnic" << user.getCnic().toStdString() << "name"
                                    << user.getName().toStdString() << "id"
                                    << user.getId().toStdString() << "email"
                                    << user.getEmail().toStdString() << "isEmailVerified"
                                    << user.isEmailVerified() << "salt"
                                    << bsoncxx::types::b_int64{static_cast<int64_t>(user.getSalt())}
                                    << "passwordHash" << binary_hash << finalize;

        m_collection.insert_one(final_doc.view());
        return true;
    }
    catch (const std::exception &e)
    {
        qDebug() << "MongoDB Insert Error:" << e.what();
        return false;
    }
}

std::optional<User> userrepository::getUserByCnic(const QString &cnic)
{
    auto filter = document{} << "cnic" << cnic.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());

    if (result)
    {
        auto view = result->view();
        User user;
        user.setCnic(QString::fromUtf8(view["cnic"].get_string().value.data()));
        user.setName(QString::fromUtf8(view["name"].get_string().value.data()));
        user.setId(QString::fromUtf8(view["id"].get_string().value.data()));
        user.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        user.setEmailVerified(view["isEmailVerified"].get_bool().value);

        // Extracting binary hash back to QByteArray
        auto binary = view["passwordHash"].get_binary();
        QByteArray hash(reinterpret_cast<const char *>(binary.bytes), binary.size);
        user.setPassword(hash, view["salt"].get_int64().value);

        return user;
    }
    return std::nullopt;
}

std::optional<User> userrepository::getUserByEmail(const QString &email)
{
    // creating a filter
    auto filter = document{} << "email" << email.toStdString() << finalize;
    // searching
    auto result = m_collection.find_one(filter.view());

    if (result)
    {
        auto view = result->view();
        User user;
        user.setCnic(QString::fromUtf8(view["cnic"].get_string().value.data()));
        user.setName(QString::fromUtf8(view["name"].get_string().value.data()));
        user.setId(QString::fromUtf8(view["id"].get_string().value.data()));
        user.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        user.setEmailVerified(view["isEmailVerified"].get_bool().value);

        auto binary = view["passwordHash"].get_binary();
        QByteArray hash(reinterpret_cast<const char *>(binary.bytes), binary.size);
        user.setPassword(hash, view["salt"].get_int64().value);

        return user;
    }
    return std::nullopt;
}
bool userrepository::updateUserEmailVerification(const QString &email, bool status)
{
    auto filter = document{} << "email" << email.toStdString() << finalize;
    auto update = document{} << "$set" << open_document << "isEmailVerified" << status
                             << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}