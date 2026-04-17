#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <memory>

#include "admin.h"
#include "user.h"
#include "otp.h"
#include "../email/emailservice.h"
#include "../crypto/cryptoengine.h"

using namespace std;

class AuthManager
{
private:
    IUserRepository *userRepo;
    IAdminRepository *adminRepo;
    IOtpRepository *otpRepo;
    unique_ptr<User> currentUser;

    AuthManager();
    ~AuthManager();

public:
    enum class LoginResult
    {
        Success,
        SuccessAdminPending, // Logged in, but admin status isn't approved yet
        InvalidCredentials,  // Wrong CNIC/Email or Password
        EmailNotVerified,    // Password correct, but needs OTP verification
        SystemError          // Database or Crypto failure
    };
    
    AuthManager(const AuthManager &) = delete;
    void operator=(const AuthManager &) = delete;

    static AuthManager &getInstance();
    void injectRepositories(IUserRepository *userRepo, IAdminRepository *adminRepo, IOtpRepository *otpRepo);


    User *getCurrentUser() const { return currentUser.get(); }
    void logout();

    bool signUp(const User &user, const QString &password, bool applyForAdmin = false);
    bool requestOtp(const QString &email, EmailService &emailService);
    bool verifyOtp(const QString &email, const QString &otpCode);

    LoginResult login(const QString &password, const QString &cnic = "", const QString &email = "");
};
#endif // AUTH_MANAGER_H