#include "candidate_controller.h"

CandidateController::CandidateController() : m_candidateRepo(nullptr) {}

CandidateController &CandidateController::getInstance()
{
    static CandidateController instance;
    return instance;
}

void CandidateController::injectDependencies(ICandidateRepository *candidateRepo)
{
    m_candidateRepo = candidateRepo;
}

Candidate *CandidateController::getAllCandidates(int &out_size)
{
    if (!m_candidateRepo)
    {
        out_size = 0;
        return nullptr;
    }

    return m_candidateRepo->getAllCandidates(out_size);
}