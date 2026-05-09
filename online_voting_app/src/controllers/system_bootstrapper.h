#ifndef SYSTEM_BOOTSTRAPPER_H
#define SYSTEM_BOOTSTRAPPER_H

#include <QByteArray>
#include <QString>
#include <memory>

class IUserRepository;
class IAdminRepository;
class IElectionRepository;
class ICandidateRepository;
class IOtpRepository;
class ITokenRepository;
class IVotingStationRepository;

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
    
    std::unique_ptr<IUserRepository> m_userRepo;
    std::unique_ptr<IAdminRepository> m_adminRepo;
    std::unique_ptr<IElectionRepository> m_electionRepo;
    std::unique_ptr<ICandidateRepository> m_candidateRepo;
    std::unique_ptr<IOtpRepository> m_otpRepo;
    std::unique_ptr<ITokenRepository> m_voterRepo;
    std::unique_ptr<IVotingStationRepository> m_stationRepo;

    AppConfig m_config;

    
    void loadOrGenerateEnv();
    void instantiateRepositories();
    void injectControllers();

public:
    SystemBootstrapper();
    ~SystemBootstrapper();

    void initializeSystem();
    void bootstrapFirstAdmins();

    AppConfig getConfig() const { return m_config; }
};

#endif 