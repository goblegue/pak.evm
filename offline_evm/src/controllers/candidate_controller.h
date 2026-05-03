

#ifndef CANDIDATE_CONTROLLER_H
#define CANDIDATE_CONTROLLER_H

#include "models/entities/candidates.h"

class ICandidateRepository;

class CandidateController
{
private:
    ICandidateRepository *m_candidateRepo;

    CandidateController();
    ~CandidateController() = default;

public:
    CandidateController(const CandidateController &) = delete;
    void operator=(const CandidateController &) = delete;

    static CandidateController &getInstance();

    void injectDependencies(ICandidateRepository *candidateRepo);

    // Frontend uses this to render the voting screen
    Candidate *getAllCandidates(int &out_size);
};

#endif // CANDIDATE_CONTROLLER_H