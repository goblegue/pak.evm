#include "auth_manager.h"
#include <QUuid>

const int OTP_EXPIRATION_SECONDS = 5 * 60; 

AuthManager::AuthManager()
    : m_userRepo(nullptr)
    , m_adminRepo(nullptr)
    , m_otpRepo(nullptr)
    , m_currentUser(nullptr)
{}

AuthManager::~AuthManager()
{
    logout();
}

AuthManager &AuthManager::getInstance()
{
    static AuthManager instance;
    return instance;
}

void AuthManager::injectRepositories(IUserRepository *userRepo,
                                     IAdminRepository *adminRepo,
                                     IOtpRepository *otpRepo)
{
    m_userRepo = userRepo;
    m_adminRepo = adminRepo;
    m_otpRepo = otpRepo;
}

AuthManager::SignUpResult AuthManager::signUp(User &user,
                                              const QString &password,
                                              bool applyForAdmin)
{
    if (!m_userRepo || !m_adminRepo) {
        return SignUpResult::SystemError; 
    }

    if (m_userRepo->getUserByCnic(user.getCnic()).has_value()
        || m_userRepo->getUserByEmail(user.getEmail()).has_value()) {
        return SignUpResult::UserAlreadyExists; 
    }

    auto hashResultOpt = CryptoEngine::getInstance().hashData(password.toUtf8());
    if (!hashResultOpt.has_value()) {
        return SignUpResult::SystemError; 
    } else {
        user.setPassword(hashResultOpt->hash, hashResultOpt->salt);
        if (!m_userRepo->insertUser(user)) {
            return SignUpResult::SystemError; 
        };
        if (applyForAdmin) {
            Admin admin;
            admin.setId(user.getId());
            admin.setCnic(user.getCnic());
            admin.setName(user.getName());
            admin.setEmail(user.getEmail());
            admin.setPassword(hashResultOpt->hash, hashResultOpt->salt);
            if (!m_adminRepo->insertAdmin(admin)) {
                return SignUpResult::SystemError; 
            };
            m_currentUser = make_unique<Admin>(admin);
            return SignUpResult::SuccessAdminCreated;
        }
        m_currentUser = make_unique<User>(user);
        return SignUpResult::SuccessUserCreated;
    }
}

bool AuthManager::requestOtp(const QString &email)
{
    if (!m_otpRepo) {
        return false; 
    }

    QString otpCode = QString::number(CryptoEngine::getInstance().generateRandomInt(100000, 999999));
    OTP otp{};
    otp.setId("OTP-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper());
    otp.setEmail(email);
    otp.setOtpCode(otpCode);
    otp.setExpiresAt(QDateTime::currentDateTime().addSecs(OTP_EXPIRATION_SECONDS));
    if (!m_otpRepo->insertOtp(otp)) {
        return false; 
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    
    QString uniqueSubject = "Email Verification For [" + timestamp + "]";

    return EmailService::getInstance()
        .sendEmail(email,
                   uniqueSubject,
                   "Your OTP is: " + otpCode
                       + "\nIt will expire in 5 minutes.\n\n Do not share this with anyone");
}

bool AuthManager::verifyOtp(const QString &email, const QString &otpCode)
{
    if (!m_otpRepo || !m_userRepo) {
        return false; 
    }

    auto otpOpt = m_otpRepo->getLatestOtpForEmail(email);
    if (!otpOpt.has_value() || otpOpt->isExpired()) {
        return false; 
    }
    if (otpOpt->getOtpCode() == otpCode) {
        m_userRepo->updateUserEmailVerification(email, true);
        return true;
    }
    return false;
}

AuthManager::LoginResult AuthManager::login(const QString &password,
                                            const QString &cnic,
                                            const QString &email)
{
    if (!m_userRepo || !m_adminRepo) {
        return LoginResult::SystemError; 
    }
    auto userOpt = cnic.isEmpty() ? m_userRepo->getUserByEmail(email)
                                  : m_userRepo->getUserByCnic(cnic);
    if (!userOpt.has_value()) {
        return LoginResult::InvalidCnicOrEmail;
    }

    auto hashResultOpt = CryptoEngine::getInstance().hashData(password.toUtf8(), userOpt->getSalt());

    if (!hashResultOpt.has_value()) {
        return LoginResult::SystemError; 
    }

    if (hashResultOpt->hash != userOpt->getPasswordHash()) {
        return LoginResult::InvalidPassword; 
    }

    if (!userOpt->isEmailVerified()) {
        return LoginResult::EmailNotVerified; 
    }

    auto adminOpt = cnic.isEmpty() ? m_adminRepo->getAdminByEmail(email)
                                   : m_adminRepo->getAdminByCnic(cnic);

    if (adminOpt.has_value() && adminOpt->getStatus() == ApprovalStatus::Pending) {
        m_currentUser = make_unique<Admin>(adminOpt.value());
        return LoginResult::SuccessAdminPending; 
    } else if (adminOpt.has_value() && adminOpt->getStatus() == ApprovalStatus::Approved) {
        m_currentUser = make_unique<Admin>(adminOpt.value());
        return LoginResult::SuccessAdminLoggedIn; 
    }
    m_currentUser = make_unique<User>(userOpt.value());
    return LoginResult::SuccessUserLoggedIn; 
}