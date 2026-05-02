#ifndef ELECTION_CONTROLLER_H
#define ELECTION_CONTROLLER_H

#include <QString>
#include "models/entities/audit_log.h"
#include "models/entities/candidates.h"
#include "models/entities/system_config.h"
#include "models/entities/tokens.h"
#include "models/entities/voting_record.h"
#include <memory>

// Forward Declarations of Interfaces
class IConfigRepository;
class ICandidateRepository;
class IVoteRepository;
class IAuditLogRepository;
class IUsedTokenRepository;

class ElectionController
{
private:
    IConfigRepository *m_configRepo;
    ICandidateRepository *m_candidateRepo;
    IVoteRepository *m_voteRepo;
    IAuditLogRepository *m_auditRepo;
    IUsedTokenRepository *m_tokenRepo;

    ElectionController();
    ~ElectionController() = default;

    void logEvent(const QString &eventType, const QString &description);

public:
    ElectionController(const ElectionController &) = delete;
    void operator=(const ElectionController &) = delete;

    static ElectionController &getInstance();

    void injectDependencies(IConfigRepository *config, 
                            ICandidateRepository *candidate,
                            IVoteRepository *vote, 
                            IAuditLogRepository *audit,
                            IUsedTokenRepository *token);

    // --- ELECTION LIFECYCLE ---
    bool loadElectionDataFromUSB(const QString &jsonFilePath);
    bool startElection();
    bool pauseElection();
    bool closeElection(const QString &masterPassword);

    // --- VOTING LOGIC ---
    bool castVote(const QString &candidateCnic, const QString &tokenId, QByteArray &out_receiptHash);
    
    // --- UTILITY ---
    QString getCurrentState();
};

#endif // ELECTION_CONTROLLER_H