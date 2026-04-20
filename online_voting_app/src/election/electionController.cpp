#include "electionController.h"
#include <optional>

const int ELECTION_APPROVAL_THRESHOLD = 2; // More than 50% of admins must approve
const int ELECTION_REJECTION_THRESHOLD = 3; // More than 33% of admins

ElectionController::ElectionController() : m_electionRepo(nullptr), m_adminRepo(nullptr) {}

ElectionController::~ElectionController() {}

ElectionController &ElectionController::getInstance()
{
    static ElectionController instance;
    return instance;
}

void ElectionController::injectRepositories(IElectionRepository *electionRepo, IAdminRepository *adminRepo)
{
    m_electionRepo = electionRepo;
    m_adminRepo = adminRepo;
}

bool ElectionController::createElection(const Election &election)
{
    if (!m_electionRepo)
    {
        return false; // Repository not injected
    }
    int electionsSize{};
    Election *existingElections = m_electionRepo->getAllElections(electionsSize); // Get all elections to check for duplicate ID
    for (int i = 0; i < electionsSize; ++i)
    {
        if (existingElections[i].getId() == election.getId() ||
            (existingElections[i].getTitle() == election.getTitle() &&
             existingElections[i].getStatus() != ElectionState::ResultsAnnounced))
        {
            delete[] existingElections;
            return false; // Election with same ID or title which results are not announced already exists
        }
    }
    delete[] existingElections;
    return m_electionRepo->insertElection(election);
}

bool ElectionController::requestElectionStatusChange(const QString &electionId, const QString &adminId, ApprovalStatus status)
{
    if (!m_electionRepo || !m_adminRepo)
    {
        return false; // Repositories not injected
    }
    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        return false; // Election not found
    }
    Election election = electionOpt.value();
    auto adminOpt = m_adminRepo->getAdminByCnic(adminId);
    if (!adminOpt.has_value())
    {
        return false; // Admin not found
    }
    Admin admin = adminOpt.value();
    if (admin.getStatus() != ApprovalStatus::Approved)
    {
        return false; // Admin not approved
    }
    m_electionRepo->addStatusChangeRequest(electionId, adminId, status);
    election.addStatusChangeRequest(adminId, status);
    int approvedCount = election.getStatusCount(ApprovalStatus::Approved);
    int rejectedCount = election.getStatusCount(ApprovalStatus::Rejected);

    int totalAdmins = m_adminRepo->getAdminCount();
    if (approvedCount > (totalAdmins / ELECTION_APPROVAL_THRESHOLD))
    {
        return m_electionRepo->updateElectionState(electionId, ElectionState::Published);
    }
    if (rejectedCount > (totalAdmins / ELECTION_REJECTION_THRESHOLD))
    {
        return m_electionRepo->updateElectionState(electionId, ElectionState::Rejected);
    }
    return true;
}

Election *ElectionController::getAllElections(int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    return m_electionRepo->getAllElections(electionsSize);
}

Election *ElectionController::getElectionsByStatus(ElectionState status, int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    Election *allElections = m_electionRepo->getAllElections(electionsSize);
    Election *filteredElections = new Election[electionsSize];
    int count = 0;
    for (int i = 0; i < electionsSize; ++i)
    {
        if (allElections[i].getStatus() == status)
        {
            filteredElections[count++] = allElections[i];
        }
    }
    delete[] allElections;
    electionsSize = count;
    return filteredElections;
}

Election *ElectionController::getElectionsForUser(int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    Election *allElections = m_electionRepo->getAllElections(electionsSize);
    Election *filteredElections = new Election[electionsSize];
    int count = 0;
    for (int i = 0; i < electionsSize; ++i)
    {
        if (allElections[i].getStatus() >= ElectionState::Published)
        {
            filteredElections[count++] = allElections[i];
        }
    }
    delete[] allElections;
    electionsSize = count;
    return filteredElections;
}

// std::optional<Election> ElectionController::getElectionById(const QString &id)
// {
//     if (!m_electionRepo)
//     {
//         return std::nullopt; // Repository not injected
//     }
//     return m_electionRepo->getElectionById(id);
// }
