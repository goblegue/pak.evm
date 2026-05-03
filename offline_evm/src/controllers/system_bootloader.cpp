#include "system_bootloader.h"
#include "models/repos/DatabaseManager.h"

// Repositories
#include "models/repos/configrepo.h"
#include "models/repos/candidaterepo.h"
#include "models/repos/voterepository.h"
#include "models/repos/auditlogrepository.h"
#include "models/repos/tokenrepo.h"
#include "models/repos/workerrepository.h"

// Controllers
#include "controllers/auth_manager.h"
#include "controllers/election_controller.h"
#include "controllers/audit_controller.h"
#include "controllers/system_controller.h"

SystemBootLoader::SystemBootLoader() {}
SystemBootLoader::~SystemBootLoader() {}

void SystemBootLoader::initializeSystem()
{
    // 1. Initialize the SQLite Database
    DatabaseManager::instance().init("offline_evm_vault.db");

    // 2. Instantiate Repositories
    m_configRepo = std::make_unique<ConfigRepository>();
    m_candidateRepo = std::make_unique<CandidateRepository>();
    m_voteRepo = std::make_unique<VoteRepository>();
    m_auditRepo = std::make_unique<AuditLogRepository>();
    m_tokenRepo = std::make_unique<TokenRepository>();
    m_workerRepo = std::make_unique<WorkerRepository>();

    // 3. Inject Dependencies into Controllers
    AuthManager::getInstance().injectDependencies(
        m_workerRepo.get(),
        m_tokenRepo.get());

    ElectionController::getInstance().injectDependencies(
        m_configRepo.get(),
        m_candidateRepo.get(),
        m_voteRepo.get(),
        m_auditRepo.get(),
        m_tokenRepo.get());

    SystemController::getInstance().injectDependencies(
        m_configRepo.get(), m_auditRepo.get());
    AuditController::getInstance().injectDependencies(
        m_voteRepo.get(), m_configRepo.get(), m_tokenRepo.get());
}