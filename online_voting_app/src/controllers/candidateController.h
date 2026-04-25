#ifndef CANDIDATE_CONTROLLER_H
#define CANDIDATE_CONTROLLER_H

#include "models/entities/candidate.h"
#include "models/entities/admin.h"
#include "models/entities/election.h"

class CandidateController
{
    ICandidateRepository *m_candidateRepo;
    IAdminRepository *m_adminRepo;
    IElectionRepository *m_electionRepo;
    CandidateController();
    ~CandidateController();

public:
    CandidateController(const CandidateController &) = delete;
    void operator=(const CandidateController &) = delete;

    static CandidateController &getInstance();
    void injectRepositories(ICandidateRepository *candidateRepo, IAdminRepository *adminRepo, IElectionRepository *electionRepo);

    bool createCandidate(Candidate &candidate);
    bool requestCandidateStatusChange(const QString &candidateCnic, const QString &adminCnic, ApprovalStatus status);

    Candidate *getCandidatesByElection(const QString &electionId, int &candidatesSize, bool isAdminRequesting = false);

    Candidate *getCandidatesByElectionAndStatus(const QString &electionId, ApprovalStatus status, int &candidatesSize);

    QString getCandidatesJsonByElection(const QString &electionId);
};

#endif // CANDIDATE_CONTROLLER_H