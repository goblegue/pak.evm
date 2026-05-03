#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include <QString>
#include <optional>
#include "models/entities/system_config.h"
#include "models/entities/audit_log.h"

class IConfigRepository;
class IAuditLogRepository;

class SystemController
{
private:
    IConfigRepository *m_configRepo;
    IAuditLogRepository *m_auditRepo;

    SystemController();
    ~SystemController() = default;

public:
    SystemController(const SystemController &) = delete;
    void operator=(const SystemController &) = delete;

    static SystemController &getInstance();

    void injectDependencies(IConfigRepository *configRepo, IAuditLogRepository *auditRepo);

    // --- SYSTEM CONFIGURATION ---
    // Returns true if a config exists (meaning the USB has been loaded at least once)
    bool isSystemConfigured(); 
    
    // Returns the current configuration so the UI can draw the timers and headers
    SystemConfig getSystemConfig(); 

    // --- FORENSIC LOGGING (For the Admin Dashboard) ---
    // Returns all system events for the Poll Worker to view on screen
    AuditLog* getForensicLogs(int &out_size);
};

#endif // SYSTEM_CONTROLLER_H