#include "auth_manager.h"

const int OTP_EXPIRATION_SECONDS = 5 * 60; // 5 minutes

AuthManager::AuthManager()
    : m_userRepo(nullptr), m_adminRepo(nullptr), m_otpRepo(nullptr), m_currentUser(nullptr)
{
}

AuthManager::~AuthManager()
{
    logout();
}

AuthManager &AuthManager::getInstance()
{
    static AuthManager instance;
    return instance;
}

void AuthManager::injectRepositories(IUserRepository *userRepo, IAdminRepository *adminRepo, IOtpRepository *otpRepo)
{
    m_userRepo = userRepo;
    m_adminRepo = adminRepo;
    m_otpRepo = otpRepo;
}

SignUpResult AuthManager::signUp(User &user, const QString &password, bool applyForAdmin)
{
    if (!m_userRepo || !m_adminRepo)
    {
        return SignUpResult::SystemError; // Repositories not injected
    }

    if (m_userRepo->getUserByCnic(user.getCnic()).has_value() || m_userRepo->getUserByEmail(user.getEmail()).has_value())
    {
        return SignUpResult::UserAlreadyExists; // User with same CNIC or Email already exists
    }

    auto hashResultOpt = CryptoEngine::getInstance().hashData(password.toUtf8());
    if (!hashResultOpt.has_value())
    {
        return SignUpResult::SystemError; // Hashing failed
    }
    else
    {
        user.setPassword(hashResultOpt->hash, hashResultOpt->salt);
        if (!m_userRepo->insertUser(user))
        {
            return SignUpResult::SystemError; // Failed to insert user into database
        };
        if (applyForAdmin)
        {
            Admin admin;
            admin.setId(user.getId());
            admin.setCnic(user.getCnic());
            admin.setName(user.getName());
            admin.setEmail(user.getEmail());
            admin.setPassword(hashResultOpt->hash, hashResultOpt->salt);
            if (!m_adminRepo->insertAdmin(admin))
            {
                return SignUpResult::SystemError; // Failed to insert admin into database
            };
            m_currentUser = make_unique<Admin>(admin);
            return SignUpResult::SuccessAdminCreated;

        }
        m_currentUser = make_unique<User>(user);
        return SignUpResult::SuccessUserCreated;
    }
}

bool AuthManager::requestOtp(const QString &email, EmailService &emailService)
{
    if (!m_otpRepo)
    {
        return false; // Repository not injected
    }

    QString otpCode = QString::number(CryptoEngine::getInstance().generateRandomInt(100000, 999999));
    OTP otp{};
    otp.setEmail(email);
    otp.setOtpCode(otpCode);
    otp.setExpiresAt(QDateTime::currentDateTime().addSecs(OTP_EXPIRATION_SECONDS));
    if (!m_otpRepo->insertOtp(otp))
    {
        return false; // Failed to insert OTP into database
    }

    return emailService.sendEmail(email, "Email Verification", "Your OTP is: " + otpCode + "\nIt will expire in 5 minutes.\n\n Do not share this with anyone");
}

bool AuthManager::verifyOtp(const QString &email, const QString &otpCode)
{
    if (!m_otpRepo || !m_userRepo)
    {
        return false; // Repositories not injected
    }

    auto otpOpt = m_otpRepo->getLatestOtpForEmail(email);
    if (!otpOpt.has_value() || otpOpt->isExpired())
    {
        return false; // OTP not found, expired, or does not match
    }
    if(otpOpt->getOtpCode() == otpCode)
    {
        m_userRepo->updateUserEmailVerification(email, true);
        return true;
    }
    return false;
}

LoginResult AuthManager::login(const QString &password, const QString &cnic, const QString &email)
{
    if (!m_userRepo || !m_adminRepo)
    {
        return LoginResult::SystemError; // Repositories not injected
    }
    auto userOpt = cnic.isEmpty() ? m_userRepo->getUserByEmail(email) : m_userRepo->getUserByCnic(cnic);
    if(!userOpt.has_value())
    {
        return LoginResult::InvalidCnicOrEmail; // User not found
    }

    auto hashResultOpt = CryptoEngine::getInstance().hashData(password.toUtf8(), userOpt->getSalt());

    if(!hashResultOpt.has_value())
    {
        return LoginResult::SystemError; // Hashing failed
    }

    if(hashResultOpt->hash != userOpt->getPasswordHash())
    {
        return LoginResult::InvalidPassword; // Password is incorrect
    }
    
    

    if (!userOpt->isEmailVerified())
    {
        return LoginResult::EmailNotVerified; // Email not verified
    }

    auto adminOpt = cnic.isEmpty() ? m_adminRepo->getAdminByEmail(email) : m_adminRepo->getAdminByCnic(cnic);

    if(adminOpt.has_value() && adminOpt->getStatus() == ApprovalStatus::Pending)
    {
        m_currentUser = make_unique<User>(adminOpt.value());
        return LoginResult::SuccessAdminPending; // Admin status pending
    }
    else if(adminOpt.has_value() && adminOpt->getStatus() == ApprovalStatus::Approved)
    {
        m_currentUser = make_unique<Admin>(adminOpt.value());
        return LoginResult::SuccessAdminLoggedIn; // Admin logged in
    }
    m_currentUser = make_unique<User>(userOpt.value());
    return LoginResult::SuccessUserLoggedIn; // Regular user logged in
}
