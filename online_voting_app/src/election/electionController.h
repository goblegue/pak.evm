#ifndef ELECTION_CONTROLLER_H
#define ELECTION_CONTROLLER_H

#include <QString>

#include "election.h"
#include "../user_management/admin/admin.h"

class ElectionController
{
private:
    IElectionRepository *m_electionRepo;
    IAdminRepository *m_adminRepo;
    ElectionController();
    ~ElectionController();

public:
    ElectionController(const ElectionController &) = delete;
    void operator=(const ElectionController &) = delete;

    static ElectionController &getInstance();
    void injectRepositories(IElectionRepository *electionRepo, IAdminRepository *adminRepo);

    bool createElection(const Election &election);
    bool requestElectionStatusChange(const QString &electionId, const QString &adminId, ApprovalStatus status);

    Election *getAllElections(int &electionsSize);
    Election *getElectionsByStatus(ElectionState status, int &electionsSize);
    Election *getElectionsForUser(int &electionsSize); // For voters, returns only elections with status greater than or equal to Published
    // std::optional<Election> getElectionById(const QString &id);

    bool sendElectionDataToAdmin(const QString &electionId, const QString &adminId, const QByteArray &privateKey);
};

#endif // ELECTION_CONTROLLER_H