#include "otprepository.h"

#include "otprepository.h"
#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

otprepository::otprepository() {
    m_collection = DatabaseManager::getInstance().getDatabase()["otp"];
}

bool otprepository::insertOtp(const OTP &otp) {
    try {
        auto builder = document{}
                       << "id" << otp.getId().toStdString()
                       << "email" << otp.getEmail().toStdString()
                       << "otpCode" << otp.getOtpCode().toStdString()
                       << "expiresAt" << static_cast<int64_t>(otp.getExpiresAt().toMSecsSinceEpoch());

        m_collection.insert_one(builder << finalize);
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<OTP> otprepository::getLatestOtpForEmail(const QString &email) {
    auto filter = document{} << "email" << email.toStdString() << finalize;

    // descending order to get latest
    auto sort_doc = document{} << "expiresAt" << -1 << finalize;
    mongocxx::options::find opts;
    opts.sort(sort_doc.view());

    auto result = m_collection.find_one(filter.view(), opts);

    if (result) {
        auto view = result->view();
        OTP otp;
        otp.setId(QString::fromStdString(view["id"].get_string().value.to_string()));
        otp.setEmail(QString::fromStdString(view["email"].get_string().value.to_string()));
        otp.setOtpCode(QString::fromStdString(view["otpCode"].get_string().value.to_string()));
        otp.setExpiresAt(QDateTime::fromMSecsSinceEpoch(view["expiresAt"].get_int64().value));
        return otp;
    }
    return std::nullopt;
}
