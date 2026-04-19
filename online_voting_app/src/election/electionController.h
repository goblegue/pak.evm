#ifndef ELECTION_CONTROLLER_H
#define ELECTION_CONTROLLER_H

#include "election.h"
#include "../user_management/admin/admin.h"

class ElectionManager
{
private:
    IElectionRepository *m_electionRepo;
    IAdminRepository *m_adminRepo;
    ElectionManager();
    ~ElectionManager();

public:
    ElectionManager(const ElectionManager &) = delete;
    void operator=(const ElectionManager &) = delete;

    static ElectionManager &getInstance();
    void injectRepositories(IElectionRepository *electionRepo, IAdminRepository *adminRepo);

    bool createElection(const Election &election);
    void requestElectionStatusChange(const QString &electionId, const QString &adminId, ApprovalStatus status);

    Election *getAllElections(int &electionsSize);
    Election *getElectionsByStatus(ElectionState status, int &electionsSize);
    Election *getElectionsForUser(int &electionsSize); // For voters, returns only elections with status greater than or equal to Published
    // std::optional<Election> getElectionById(const QString &id);
}

#endif // ELECTION_CONTROLLER_H