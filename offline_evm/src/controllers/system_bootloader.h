#ifndef SYSTEM2_BOOTSTRAPPER_H
#define SYSTEM2_BOOTSTRAPPER_H

#include <memory>

// Forward declarations
class IConfigRepository;
class ICandidateRepository;
class IVoteRepository;
class IAuditLogRepository;
class ITokenRepository;
class IWorkerRepository;

class SystemBootLoader
{
private:
    std::unique_ptr<IConfigRepository> m_configRepo;
    std::unique_ptr<ICandidateRepository> m_candidateRepo;
    std::unique_ptr<IVoteRepository> m_voteRepo;
    std::unique_ptr<IAuditLogRepository> m_auditRepo;
    std::unique_ptr<ITokenRepository> m_tokenRepo;
    std::unique_ptr<IWorkerRepository> m_workerRepo;

public:
    SystemBootLoader();
    ~SystemBootLoader();

    void initializeSystem();
};

#endif // SYSTEM2_BOOTSTRAPPER_H