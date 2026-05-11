#include "controllers/system_bootstrapper.h"
#include <QDebug>
#include <QFile>
#include <QMap>
#include <QTextStream>

// Repositories & Managers
#include "models/repositories/adminrepository.h"
#include "models/repositories/candidaterepository.h"
#include "models/databasemanager.h"
#include "models/repositories/electionrepository.h"
#include "models/repositories/otprepository.h"
#include "models/repositories/userrepository.h"
#include "models/repositories/voterrepository.h"
#include "models/repositories/votingstationrepository.h"
#include "models/repositories/resultrepository.h"

// Controllers
#include "controllers/auth_manager.h"
#include "controllers/candidateController.h"
#include "controllers/electionController.h"
#include "controllers/adminController.h"
#include "controllers/voterController.h"

// Services
#include "services/crypto/cryptoengine.h"
#include "services/email/emailservice.h"

SystemBootstrapper::SystemBootstrapper() {}
SystemBootstrapper::~SystemBootstrapper() {}

void SystemBootstrapper::initializeSystem()
{
    qDebug() << "[Bootstrapper] Initializing System 1...";

    // 1. Setup Environment & Keys
    loadOrGenerateEnv();

    // 2. Setup Database
    DatabaseManager::getInstance().setupSchema();

    // 3. Configure Email Service with your specific credentials
    EmailService::getInstance().configure(m_config.smtpHost,
                                          m_config.smtpPort,
                                          m_config.smtpEmail,
                                          m_config.smtpPassword);

    // 4. Instantiate Repositories
    instantiateRepositories();

    // 5. Inject dependencies into Singletons
    injectControllers();

    // 6. BOOTSTRAP: Add the first 2 Admins if they don't exist
    bootstrapFirstAdmins();

    qDebug() << "[Bootstrapper] System Ready.";
}

void SystemBootstrapper::loadOrGenerateEnv()
{
    QFile envFile(".env");
    QMap<QString, QString> envMap;

    if (envFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&envFile);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("#"))
                continue;
            int eq = line.indexOf('=');
            if (eq > 0)
                envMap[line.left(eq).trimmed()] = line.mid(eq + 1).trimmed();
        }
        envFile.close();
    }

    bool needsRewrite = false;

    // Generate Crypto Keys if missing
    if (!envMap.contains("PUBLIC_KEY") || !envMap.contains("PRIVATE_KEY"))
    {
        auto keyPairOpt = CryptoEngine::getInstance().generateKeyPair();
        if (keyPairOpt.has_value())
        {
            envMap["PUBLIC_KEY"] = QString(keyPairOpt->publicKey.toBase64());
            envMap["PRIVATE_KEY"] = QString(keyPairOpt->privateKey.toBase64());
            needsRewrite = true;
        }
    }

    // MANDATORY EMAIL SETUP (Your requested info)
    if (envMap["SMTP_HOST"] != "smtp.gmail.com")
    {
        envMap["SMTP_HOST"] = "smtp.gmail.com";
        needsRewrite = true;
    }
    if (envMap["SMTP_PORT"] != "465")
    {
        envMap["SMTP_PORT"] = "465";
        needsRewrite = true;
    }
    if (envMap["SMTP_EMAIL"] != "pak.evm.project@gmail.com")
    {
        envMap["SMTP_EMAIL"] = "pak.evm.project@gmail.com";
        needsRewrite = true;
    }
    if (envMap["SMTP_PASSWORD"] != "dtgn pptc jspd vjnk")
    {
        envMap["SMTP_PASSWORD"] = "dtgn pptc jspd vjnk";
        needsRewrite = true;
    }

    if (needsRewrite)
    {
        if (envFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&envFile);
            out << "# Pak EVM Project - Auto Generated Config\n";
            for (auto it = envMap.begin(); it != envMap.end(); ++it)
            {
                out << it.key() << "=" << it.value() << "\n";
            }
            envFile.close();
        }
    }

    m_config.publicKey = QByteArray::fromBase64(envMap["PUBLIC_KEY"].toUtf8());
    m_config.privateKey = QByteArray::fromBase64(envMap["PRIVATE_KEY"].toUtf8());
    m_config.smtpHost = envMap["SMTP_HOST"];
    m_config.smtpPort = envMap["SMTP_PORT"].toInt();
    m_config.smtpEmail = envMap["SMTP_EMAIL"];
    m_config.smtpPassword = envMap["SMTP_PASSWORD"];
}

void SystemBootstrapper::bootstrapFirstAdmins()
{
    // Check if we already have admins to avoid duplicate insertion errors
    if (m_adminRepo->getAdminCount() >= 2)
    {
        qDebug() << "[Bootstrapper] System already has bootstrapped admins.";
        return;
    }

    qDebug() << "[Bootstrapper] Inserting first 2 Super-Admins...";

    QString names[2] = {"Root Admin One", "Root Admin Two"};
    QString cnics[2] = {"00000-0000000-1", "00000-0000000-2"};
    QString emails[2] = {"pak.evm.project@gmail.com", "loco.am.dev@gmail.com"};
    QString rawPass = "1234";

    for (int i = 0; i < 2; ++i)
    {
        Admin admin;
        admin.setId(QString("ROOT_00%1").arg(i + 1));
        admin.setName(names[i]);
        admin.setCnic(cnics[i]);
        admin.setEmail(emails[i]);
        admin.setEmailVerified(true);
        admin.setStatus(ApprovalStatus::Approved); // Direct approval

        // Securely hash the bootstrap password
        auto hashRes = CryptoEngine::getInstance().hashData(rawPass.toUtf8());
        if (hashRes)
        {
            admin.setPassword(hashRes->hash, hashRes->salt);
            m_adminRepo->insertAdmin(admin);

            // Also insert into Users collection so they can log in as voters if needed
            m_userRepo->insertUser(admin);
        }
    }
    qDebug() << "[Bootstrapper] Bootstrap complete. Use '1234' to log in.";
}

void SystemBootstrapper::instantiateRepositories()
{

    // unique_ptr<T>(new T()) is used to safely manage heap memory
    m_userRepo = std::unique_ptr<IUserRepository>(new userrepository());
    m_adminRepo = std::unique_ptr<IAdminRepository>(new adminrepository());
    m_electionRepo = std::unique_ptr<IElectionRepository>(new electionrepository());
    m_candidateRepo = std::unique_ptr<ICandidateRepository>(new candidaterepository());
    m_otpRepo = std::unique_ptr<IOtpRepository>(new otprepository());
    m_voterRepo = std::unique_ptr<ITokenRepository>(new TokenRepository());
    m_stationRepo = std::unique_ptr<IVotingStationRepository>(new votingstationrepository());
    m_resultRepo = std::unique_ptr<IPollResultRepository>(new ResultRepository());
}

void SystemBootstrapper::injectControllers()
{
    // get() returns the raw pointer without deleting the unique_ptr
    AuthManager::getInstance().injectRepositories(m_userRepo.get(),
                                                  m_adminRepo.get(),
                                                  m_otpRepo.get());

    ElectionController::getInstance().injectRepositories(m_electionRepo.get(), m_adminRepo.get());

    CandidateController::getInstance().injectRepositories(m_candidateRepo.get(),
                                                          m_adminRepo.get(),
                                                          m_electionRepo.get());

    TokenController::getInstance().injectRepositories(m_voterRepo.get(), m_electionRepo.get());

    AdminController::getInstance().injectRepositories(m_adminRepo.get());
}
