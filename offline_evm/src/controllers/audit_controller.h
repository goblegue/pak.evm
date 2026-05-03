#ifndef AUDIT_CONTROLLER_H
#define AUDIT_CONTROLLER_H

#include <QString>
#include "models/entities/system_config.h"

// Forward declarations
class IVoteRepository;
class IConfigRepository;
class ITokenRepository;

class AuditController
{
private:
    IVoteRepository *m_voteRepo;
    IConfigRepository *m_configRepo;
    ITokenRepository *m_tokenRepo;

    AuditController();
    ~AuditController() = default;

public:
    AuditController(const AuditController &) = delete;
    void operator=(const AuditController &) = delete;

    static AuditController &getInstance();

    void injectDependencies(IVoteRepository *vote, IConfigRepository *config, ITokenRepository *token);

    // 1. The Hash Chain Verification
    bool runForensicAudit(QString &out_failureDetails);

    // 2. The Results Generation (Only runs if Audit passes)
    bool exportFinalResults(const QString &outputFilePath);
};

#endif // AUDIT_CONTROLLER_H