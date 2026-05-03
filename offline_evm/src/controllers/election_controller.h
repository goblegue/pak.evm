#ifndef ELECTION_CONTROLLER_H
#define ELECTION_CONTROLLER_H

#include <QString>
#include "models/entities/audit_log.h"
#include "models/entities/candidates.h" // Fixed spelling
#include "models/entities/system_config.h"
#include "models/entities/tokens.h"        // Fixed spelling
#include "models/entities/voting_record.h" // Fixed spelling
#include <memory>

// Forward Declarations of Interfaces
class IConfigRepository;
class ICandidateRepository;
class IVoteRepository;
class IAuditLogRepository;
class ITokenRepository;

class ElectionController
{
private:
    IConfigRepository *m_configRepo;
    ICandidateRepository *m_candidateRepo;
    IVoteRepository *m_voteRepo;
    IAuditLogRepository *m_auditRepo;
    ITokenRepository *m_tokenRepo;

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
                            ITokenRepository *token);

    // --- SETUP FLOW ---
    bool loadElectionDataFromUSB(const QString &jsonFilePath);

    // --- AUTOMATED FLOW HELPERS (Called by UI QTimer) ---
    qint64 getSecondsUntilStart();
    qint64 getSecondsUntilEnd();
    bool autoOpenElection();
    bool autoCloseElection();

    // --- POLL WORKER EMERGENCY CONTROLS ---
    bool pauseElection();
    bool resumeElection(); // [NEW] Needed to un-pause
    bool extendElectionTime(int addedMinutes); // [NEW]
    bool closeElection(const QString &masterPassword); // Emergency force close

    // --- VOTING LOGIC ---
    bool castVote(const QString &candidateCnic, const QString &tokenId, QByteArray &out_receiptHash);
    
    // --- UTILITY ---
    ElectionState getCurrentState(); // [CHANGED] Returns Enum
};

#endif // ELECTION_CONTROLLER_H