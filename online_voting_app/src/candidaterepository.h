#ifndef CANDIDATEREPOSITORY_H
#define CANDIDATEREPOSITORY_H

#include "states.h"
#include "candidate.h"
#include <mongocxx/collection.hpp>
#include <optional>

class candidaterepository : public ICandidateRepository {
private:
    mongocxx::collection m_collection;

public:
    candidaterepository();

    bool insertCandidate(const Candidate &candidate) override;

    bool addStatusChangeRequest(const QString &targetCandidateCnic,
                                const QString &requestingAdminId,
                                const ApprovalStatus &status) override;

    Candidate* getCandidatesByElection(const QString &electionId, int &candidatesSize) override;
};
#endif // CANDIDATEREPOSITORY_H
