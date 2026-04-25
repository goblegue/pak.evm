#include "adminrepository.h"
#include "./user_management/admin/admin.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <iterator>
#include <bsoncxx/types.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

using namespace std;

adminrepository::adminrepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Admins"];
}

bool adminrepository::insertAdmin(const Admin &admin)
{
    try
    {
        auto builder = document{};
        builder << "cnic" << admin.getCnic().toStdString() << "name"
                << admin.getName().toStdString() << "email" << admin.getEmail().toStdString()
                << "id" << admin.getId().toStdString() << "isEmailVerified"
                << admin.isEmailVerified() << "status"
                << static_cast<int>(admin.getStatus()) // Store Enum as Int
                << "salt" << bsoncxx::types::b_int64{static_cast<int64_t>(admin.getSalt())};

        bsoncxx::types::b_binary binary_hash;
        binary_hash.bytes = reinterpret_cast<const uint8_t *>(admin.getPasswordHash().data());
        binary_hash.size = static_cast<uint32_t>(admin.getPasswordHash().size());
        binary_hash.sub_type = bsoncxx::binary_sub_type::k_binary;

        builder << "passwordHash" << binary_hash;

        auto final_doc = builder << finalize;
        m_collection.insert_one(final_doc.view());
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

optional<Admin> adminrepository::getAdminByEmail(const QString &email)
{
    auto filter = document{} << "email" << email.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());

    if (result)
    {
        auto view = result->view();
        Admin admin;

        admin.setCnic(QString::fromUtf8(view["cnic"].get_string().value.data()));
        admin.setName(QString::fromUtf8(view["name"].get_string().value.data()));
        admin.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        admin.setStatus(static_cast<ApprovalStatus>(view["status"].get_int32().value));
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(reqView["status"].get_int32().value);
                admin.addStatusChangeRequest(requesterId, reqStatus);
            }
        }

        auto binary = view["passwordHash"].get_binary();
        QByteArray hash(reinterpret_cast<const char *>(binary.bytes), binary.size);
        admin.setPassword(hash, view["salt"].get_int32().value);
        return admin;
    }
    return nullopt;
}

bool adminrepository::addStatusChangeRequest(const QString &targetAdminCnic,
                                             const QString &requestingAdminId,
                                             const ApprovalStatus &status)
{
    auto filter = document{} << "cnic" << targetAdminCnic.toStdString() << finalize;

    auto update = document{} << "$push" << open_document
                             << "statusChangeRequests" << open_document
                             << "requestById" << requestingAdminId.toStdString()
                             << "status" << static_cast<int>(status)
                             << close_document
                             << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

optional<Admin> adminrepository::getAdminByCnic(const QString &cnic)
{
    auto filter = document{} << "cnic" << cnic.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());

    if (result)
    {
        auto view = result->view();
        Admin admin;

        admin.setCnic(QString::fromUtf8(view["cnic"].get_string().value.data()));
        admin.setName(QString::fromUtf8(view["name"].get_string().value.data()));
        admin.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        admin.setId(QString::fromUtf8(view["id"].get_string().value.data()));
        admin.setEmailVerified(view["isEmailVerified"].get_bool().value);

        admin.setStatus(static_cast<ApprovalStatus>(view["status"].get_int32().value));
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;

            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(reqView["status"].get_int32().value);

                admin.addStatusChangeRequest(requesterId, reqStatus);
            }
        }
        auto binary = view["passwordHash"].get_binary();
        QByteArray hash(reinterpret_cast<const char *>(binary.bytes), binary.size);
        admin.setPassword(hash, view["salt"].get_int32().value);

        return admin;
    }
    return nullopt;
}

int adminrepository::getAdminCount()
{
    return static_cast<int>(m_collection.count_documents({}));
}

bool adminrepository::updateAdminStatus(const QString &cnic, const ApprovalStatus &status)
{
    auto filter = document{} << "cnic" << cnic.toStdString() << finalize;
    auto update = document{} << "$set" << open_document
                             << "status" << static_cast<int>(status)
                             << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

optional<StatusChangeRequest *> adminrepository::getStatusChangeRequests(const QString &cnic, int &count)
{
    auto filter = document{} << "cnic" << cnic.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());

    if (result)
    {
        auto view = result->view();
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            count = static_cast<int>(distance(requestsArray.begin(), requestsArray.end()));
            StatusChangeRequest *requests = new StatusChangeRequest[count];
            int index = 0;
            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(reqView["status"].get_int32().value);
                requests[index++] = {reqStatus, requesterId};
            }
            return requests;
        }
    }
    count = 0;
    return std::nullopt;
}

std::optional<Admin *> adminrepository::getAllAdmins(int &count)
{
    auto cursor = m_collection.find({});
    count = getAdminCount();
    if (count == 0)
    {
        return std::nullopt;
    }

    Admin *admins = new Admin[count];
    int index = 0;
    for (auto &&doc : cursor)
    {
        auto view = doc;
        Admin admin;
        admin.setCnic(QString::fromUtf8(view["cnic"].get_string().value.data()));
        admin.setName(QString::fromUtf8(view["name"].get_string().value.data()));
        admin.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        admin.setId(QString::fromUtf8(view["id"].get_string().value.data()));
        admin.setEmailVerified(view["isEmailVerified"].get_bool().value);
        admin.setStatus(static_cast<ApprovalStatus>(view["status"].get_int32().value));
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(reqView["status"].get_int32().value);
                admin.addStatusChangeRequest(requesterId, reqStatus);
            }
        }
        auto binary = view["passwordHash"].get_binary();
        QByteArray hash(reinterpret_cast<const char *>(binary.bytes), binary.size);
        admin.setPassword(hash, view["salt"].get_int32().value);
        admins[index++] = admin;
    }
    return admins;
}

