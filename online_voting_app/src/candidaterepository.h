#ifndef CANDIDATEREPOSITORY_H
#define CANDIDATEREPOSITORY_H

#include "states.h"
#include "./candidate/candidate.h"
#include <mongocxx/collection.hpp>
#include <optional>

class candidaterepository : public ICandidateRepository
{
private:
    mongocxx::collection m_collection;

public:
    candidaterepository();
    ~candidaterepository() = default;

    bool insertCandidate(const Candidate &candidate) override;

    bool addStatusChangeRequest(const QString &targetCandidateCnic,
                                const QString &requestingAdminId,
                                const ApprovalStatus &status) override;

    Candidate *getCandidates(int &candidatesSize, const QString &electionId = "") override;

    // Ensure these exactly match the interface signatures
    Candidate *getCandidatesByStatus(int &candidatesSize, const QString &electionId, ApprovalStatus status) override;
    bool updateCandidateStatus(const QString &candidateCnic, ApprovalStatus newStatus) override;
};
#endif // CANDIDATEREPOSITORY_H
