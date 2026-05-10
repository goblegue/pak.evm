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
#include "controllers/audit_controller.h"
#include "controllers/auth_manager.h"
#include "controllers/candidate_controller.h"
#include "controllers/election_controller.h"
#include "controllers/system_controller.h"

#include "services/crypto/crypto_engine.h"

SystemBootLoader::SystemBootLoader() {}
SystemBootLoader::~SystemBootLoader() {}

#include <QDebug>


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
    AuthManager::getInstance().injectDependencies(m_workerRepo.get(),
                                                  m_tokenRepo.get(),
                                                  m_configRepo.get());

    ElectionController::getInstance().injectDependencies(
        m_configRepo.get(),
        m_candidateRepo.get(),
        m_voteRepo.get(),
        m_auditRepo.get(),
        m_tokenRepo.get());

    CandidateController::getInstance().injectDependencies(m_candidateRepo.get());

    SystemController::getInstance().injectDependencies(m_configRepo.get(), m_auditRepo.get());
    AuditController::getInstance().injectDependencies(
        m_voteRepo.get(), m_configRepo.get(), m_tokenRepo.get());

    // ==============================================================
    // 4. CRYPTOGRAPHIC KEY LOADING & VALIDATION
    // ==============================================================
    SystemConfig config = SystemController::getInstance().getSystemConfig();
    
    // If the system has already been set up, it MUST have the vault files.
    if (config.getCurrentState() != ElectionState::Setup)
    {
        auto auditKeyOpt = CryptoEngine::getInstance().loadAuditKeyFromVault();

        // If files are missing, deleted by a hacker, or empty:
        if (!auditKeyOpt.has_value()) {
            // qFatal will instantly crash the app and print this to the console.
            // This is the correct security protocol for missing cryptographic keys.
            qFatal("CRITICAL SECURITY ERROR: Election is configured, but cryptographic keys (.bin files) are missing or corrupted! System Halting.");
        }

        // Successfully loaded. Store them securely in the CryptoEngine's RAM.

        CryptoEngine::getInstance().setAuditKey(auditKeyOpt.value());
        
        qDebug() << "[Bootstrapper] Cryptographic Vault loaded successfully.";
    }
}