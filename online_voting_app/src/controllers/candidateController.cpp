#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>


#include "controllers/candidateController.h"

const int CANDIDATE_APPROVAL_THRESHOLD = 3;
const int CANDIDATE_REJECTION_THRESHOLD = 2;

CandidateController::CandidateController() : m_candidateRepo(nullptr), m_adminRepo(nullptr), m_electionRepo(nullptr) {}

CandidateController::~CandidateController() {}

CandidateController &CandidateController::getInstance()
{
    static CandidateController instance;
    return instance;
}

void CandidateController::injectRepositories(ICandidateRepository *candidateRepo, IAdminRepository *adminRepo, IElectionRepository *electionRepo)
{
    m_candidateRepo = candidateRepo;
    m_adminRepo = adminRepo;
    m_electionRepo = electionRepo;
}

bool CandidateController::createCandidate(Candidate &candidate)
{
    if (!m_candidateRepo || !m_electionRepo)
    {
        return false; // Repositories not injected
    }
    int candidatesSize{};
    Candidate *existingCandidates = m_candidateRepo->getCandidates(candidatesSize, candidate.getElectionId()); // Get existing candidates for the election to check for duplicate CNIC
    for (int i = 0; i < candidatesSize; ++i)
    {
        if (existingCandidates[i].getUserCnic() == candidate.getUserCnic())
        {
            delete[] existingCandidates;
            return false; // Candidate with same CNIC already exists for the election
        }
    }
    delete[] existingCandidates;
    auto electionOpt = m_electionRepo->getElectionById(candidate.getElectionId());
    if (!electionOpt.has_value())
    {
        return false; // Election not found
    }
    Election election = electionOpt.value();
    if (election.getStatus() == ElectionState::Drafted ||
        election.getStatus() == ElectionState::Rejected ||
        election.getStatus() == ElectionState::ResultsAnnounced ||
        election.getStatus() == ElectionState::VotingOpen ||
        election.getStatus() == ElectionState::VotingClosed)
    {
        return false; // Cannot add candidates to elections that are not active
    }
    candidate.setStatus(ApprovalStatus::Pending); // New candidates start with Pending status
    return m_candidateRepo->insertCandidate(candidate);
}

bool CandidateController::requestCandidateStatusChange(const QString &candidateCnic, const QString &adminCnic, ApprovalStatus status)
{
    if (!m_candidateRepo || !m_adminRepo)
    {
        return false; // Repositories not injected
    }
    auto adminOpt = m_adminRepo->getAdminByCnic(adminCnic);
    if (!adminOpt.has_value())
    {
        return false; // Admin not found
    }
    Admin admin = adminOpt.value();
    if (admin.getStatus() != ApprovalStatus::Approved)
    {
        return false; // Only approved admins can request status changes
    }
    int candidatesSize{};
    Candidate *candidates = m_candidateRepo->getCandidates(candidatesSize);
    Candidate *targetCandidate = nullptr;
    for (int i = 0; i < candidatesSize; ++i)
    {
        if (candidates[i].getUserCnic() == candidateCnic)
        {
            targetCandidate = &candidates[i];
            break;
        }
    }
    if (!targetCandidate)
    {
        delete[] candidates;
        return false; // Candidate not found
    }
    auto electionOpt = m_electionRepo->getElectionById(targetCandidate->getElectionId());
    if (!electionOpt.has_value())
    {
        delete[] candidates;
        return false;
    }

    Election election = electionOpt.value();
    if (election.getStatus() == ElectionState::Drafted ||
        election.getStatus() == ElectionState::Rejected ||
        election.getStatus() == ElectionState::ResultsAnnounced ||
        election.getStatus() == ElectionState::VotingOpen ||
        election.getStatus() == ElectionState::VotingClosed)
    {
        delete[] candidates;
        return false; // Cannot change candidate status for elections that are not active
    }

    targetCandidate->addStatusChangeRequest(admin.getId(), status);

    int approvedCount = targetCandidate->getStatusCount(ApprovalStatus::Approved);
    int rejectedCount = targetCandidate->getStatusCount(ApprovalStatus::Rejected);

    int totalAdmins = m_adminRepo->getAdminCount();
    if (approvedCount > (totalAdmins / CANDIDATE_APPROVAL_THRESHOLD))
    {
        delete[] candidates;
        return m_candidateRepo->updateCandidateStatus(candidateCnic, ApprovalStatus::Approved);
    }
    if (rejectedCount > (totalAdmins / CANDIDATE_REJECTION_THRESHOLD))
    {
        delete[] candidates;
        return m_candidateRepo->updateCandidateStatus(candidateCnic, ApprovalStatus::Rejected);
    }
    delete[] candidates;
    return true;
}

Candidate *CandidateController::getCandidatesByElection(const QString &electionId, int &candidatesSize, bool isAdminRequesting)
{
    if (!m_candidateRepo || !m_electionRepo)
    {
        candidatesSize = 0;
        return nullptr; // Repositories not injected
    }
    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        candidatesSize = 0;
        return nullptr; // Election not found
    }
    Election election = electionOpt.value();
    if (!isAdminRequesting && (election.getStatus() == ElectionState::Drafted || election.getStatus() == ElectionState::Rejected))
    {
        candidatesSize = 0;
        return nullptr; // Voters cannot see candidates for elections that are not active
    }
    return m_candidateRepo->getCandidates(candidatesSize, electionId);
}

Candidate *CandidateController::getCandidatesByElectionAndStatus(const QString &electionId, ApprovalStatus status, int &candidatesSize)
{
    if (!m_candidateRepo || !m_electionRepo)
    {
        candidatesSize = 0;
        return nullptr; // Repositories not injected
    }
    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        candidatesSize = 0;
        return nullptr; // Election not found
    }
    Election election = electionOpt.value();
    if (election.getStatus() == ElectionState::Drafted || election.getStatus() == ElectionState::Rejected)
    {
        candidatesSize = 0;
        return nullptr; // Cannot see candidates for elections that are not active
    }
    return m_candidateRepo->getCandidatesByStatus(candidatesSize, electionId, status);
}

QString CandidateController::getCandidatesJsonByElection(const QString &electionId)
{
    int candidatesSize{};
    Candidate *candidates = getCandidatesByElectionAndStatus(electionId, ApprovalStatus::Approved, candidatesSize);
    if (!candidates)
    {
        return "[]"; // No candidates found, return empty JSON array
    }

    QJsonArray jsonArray;
    for (int i = 0; i < candidatesSize; ++i)
    {
        QJsonObject candidateObj;
        // Set the keys and values (very similar to a Map)
        candidateObj["cnic"] = candidates[i].getUserCnic();
        candidateObj["party"] = candidates[i].getPartyName();
        candidateObj["symbolName"] = candidates[i].getSymbolName();
        candidateObj["symbolData"] = candidates[i].getSymbolBase64();
        candidateObj["imageData"] = candidates[i].getProfileImageBase64();

        // Add this candidate object into our array
        jsonArray.append(candidateObj);
    }
    delete[] candidates;

    QJsonDocument jsonDoc(jsonArray);

    return jsonDoc.toJson(QJsonDocument::Compact).constData();
}
