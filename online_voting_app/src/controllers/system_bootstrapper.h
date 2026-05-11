#ifndef SYSTEM_BOOTSTRAPPER_H
#define SYSTEM_BOOTSTRAPPER_H

#include <QByteArray>
#include <QString>
#include <memory>

// Forward declare the abstract interfaces to keep the header clean
class IUserRepository;
class IAdminRepository;
class IElectionRepository;
class ICandidateRepository;
class IOtpRepository;
class ITokenRepository;
class IVotingStationRepository;
class IPollResultRepository;

// A struct to hold our loaded environment variables
struct AppConfig
{
    QByteArray publicKey;
    QByteArray privateKey;
    QString smtpHost;
    int smtpPort;
    QString smtpEmail;
    QString smtpPassword;
};

class SystemBootstrapper
{
private:
    // Repositories managed by Smart Pointers (Zero Memory Leaks!)
    std::unique_ptr<IUserRepository> m_userRepo;
    std::unique_ptr<IAdminRepository> m_adminRepo;
    std::unique_ptr<IElectionRepository> m_electionRepo;
    std::unique_ptr<ICandidateRepository> m_candidateRepo;
    std::unique_ptr<IOtpRepository> m_otpRepo;
    std::unique_ptr<ITokenRepository> m_voterRepo;
    std::unique_ptr<IVotingStationRepository> m_stationRepo;
    std::unique_ptr<IPollResultRepository> m_resultRepo;

    AppConfig m_config;

    // Helper functions
    void loadOrGenerateEnv();
    void instantiateRepositories();
    void injectControllers();

public:
    SystemBootstrapper();
    ~SystemBootstrapper();

    // The single function main.cpp needs to call
    void initializeSystem();
    void bootstrapFirstAdmins();

    // Allows main.cpp or controllers to get the private key if needed
    AppConfig getConfig() const { return m_config; }
};

#endif // SYSTEM_BOOTSTRAPPER_H