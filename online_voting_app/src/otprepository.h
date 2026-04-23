#ifndef OTPREPOSITORY_H
#define OTPREPOSITORY_H

#include "./otp/otp.h"
#include "databasemanager.h"

class otprepository : public IOtpRepository
{
private:
    mongocxx::collection m_collection;

public:
    otprepository();
    ~otprepository() override = default;

    bool insertOtp(const OTP &otp) override;
    std::optional<OTP> getLatestOtpForEmail(const QString &email) override;
};

#endif // OTPREPOSITORY_H
