#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <memory>

#include <QString>

#include "models/entities/admin.h"
#include "models/entities/user.h"
#include "models/entities/otp.h"
#include "services/email/emailservice.h"
#include "services/crypto/cryptoengine.h"

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
        SuccessUserLoggedIn,  
        SuccessAdminLoggedIn, 
        SuccessAdminPending,  
        InvalidCnicOrEmail,   
        InvalidPassword,      
        EmailNotVerified,     
        SystemError           
    };

    enum class SignUpResult
    {
        SuccessUserCreated,  
        SuccessAdminCreated, 
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
#endif 
