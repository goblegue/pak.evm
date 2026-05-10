#include "system_controller.h"

SystemController::SystemController()
    : m_configRepo(nullptr),
m_auditRepo(nullptr)
{
}

SystemController &SystemController::getInstance()
{
    static SystemController instance;
    return instance;
}

void SystemController::injectDependencies(IConfigRepository *configRepo, IAuditLogRepository *auditRepo)
{
    m_configRepo = configRepo;
    m_auditRepo = auditRepo;
}

bool SystemController::isSystemConfigured()
{
    if (!m_configRepo)
        return false;
    return m_configRepo->getConfig().has_value();
}

SystemConfig SystemController::getSystemConfig()
{
    if (!m_configRepo)
        return SystemConfig(); // Returns an empty default config

    auto configOpt = m_configRepo->getConfig();
    if (configOpt.has_value())
    {
        return configOpt.value();
    }

    return SystemConfig(); // Fallback if no config exists yet
}

AuditLog *SystemController::getForensicLogs(int &out_size)
{
    if (!m_auditRepo)
    {
        out_size = 0;
        return nullptr;
    }

    // Returns logs ordered by timestamp for the UI table
    return m_auditRepo->getAllLogs(out_size);
}

bool SystemController::saveSystemPublicKey(const QString &publicKeyBase64)
{
    if (!m_configRepo)
        return false;

    // Fetch existing or get a fresh one
    SystemConfig config = getSystemConfig();
    
    // Update just the public key
    config.setPublicKeyBase64(publicKeyBase64);
    
    // Pass it to the repository
    return m_configRepo->saveConfig(config);
}