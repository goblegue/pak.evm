#ifndef CANDIDATEREPOSITORY_H
#define CANDIDATEREPOSITORY_H
#include "../entities/candidates.h"

class CandidateRepository : public ICandidateRepository {
public:
    bool insertCandidate(const Candidate &candidate) override;
    Candidate *getAllCandidates(int &candidatesSize) override;
    bool clearAllCandidates() override;
};
#endif
