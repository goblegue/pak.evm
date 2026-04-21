#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <memory>

#include <QString>

#include "../admin.h"
#include "../user.h"
#include "../otp.h"
#include "../email/emailservice.h"
#include "../crypto/cryptoengine.h"

using namespace std;

class AuthManager
{
private:
    IUserRepository *m_userRepo;
    IAdminRepository *m_adminRepo;
    IOtpRepository *m_otpRepo;
    unique_ptr<User> m_currentUser;

    AuthManager();
    ~AuthManager();

public:
    enum class LoginResult
    {
        SuccessUserLoggedIn,     // Logged in as regular user
        SuccessAdminLoggedIn,    // Logged in as admin
        SuccessAdminPending, // Logged in, but admin status isn't approved yet
        InvalidCnicOrEmail,  // No user found with given CNIC or Email
        InvalidPassword,     // User found, but password is incorrect
        EmailNotVerified,    // Password correct, but needs OTP verification
        SystemError          // Database or Crypto failure
    };

    enum class SignUpResult
    {
        SuccessUserCreated,       // User created successfully
        SuccessAdminCreated,      // User created and applied for admin successfully
        UserAlreadyExists,
        SystemError
    };

    AuthManager(const AuthManager &) = delete;
    void operator=(const AuthManager &) = delete;

    static AuthManager &getInstance();
    void injectRepositories(IUserRepository *userRepo, IAdminRepository *adminRepo, IOtpRepository *otpRepo);

    bool isLoggedIn() const { return m_currentUser != nullptr; };
    User *getCurrentUser() const { return m_currentUser.get(); }
    void logout() { m_currentUser.reset(); }

    SignUpResult signUp(User &user, const QString &password, bool applyForAdmin = false);
    bool requestOtp(const QString &email);
    bool verifyOtp(const QString &email, const QString &otpCode);

    LoginResult login(const QString &password, const QString &cnic = "", const QString &email = "");
};
#endif // AUTH_MANAGER_H
