#include "models/repositories/otprepository.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

otprepository::otprepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["otp"];
}

bool otprepository::insertOtp(const OTP &otp)
{
    try
    {
        auto final_doc = document{}
                         << "id" << otp.getId().toStdString()
                         << "email" << otp.getEmail().toStdString()
                         << "otpCode" << otp.getOtpCode().toStdString()
                         << "expiresAt" << bsoncxx::types::b_int64{static_cast<int64_t>(otp.getExpiresAt().toMSecsSinceEpoch())}
                         << finalize;

        m_collection.insert_one(final_doc.view());
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

std::optional<OTP> otprepository::getLatestOtpForEmail(const QString &email)
{
    auto filter = document{} << "email" << email.toStdString() << finalize;

    // descending order to get latest
    auto sort_doc = document{} << "expiresAt" << -1 << finalize;
    mongocxx::options::find opts;
    opts.sort(sort_doc.view());

    auto result = m_collection.find_one(filter.view(), opts);

    if (result)
    {
        auto view = result->view();
        OTP otp;
        otp.setId(QString::fromUtf8(view["id"].get_string().value.data()));
        otp.setEmail(QString::fromUtf8(view["email"].get_string().value.data()));
        otp.setOtpCode(QString::fromUtf8(view["otpCode"].get_string().value.data()));
        otp.setExpiresAt(QDateTime::fromMSecsSinceEpoch(view["expiresAt"].get_int64().value));
        return otp;
    }
    return std::nullopt;
}
