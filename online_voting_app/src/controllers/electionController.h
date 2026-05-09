#ifndef ELECTION_CONTROLLER_H
#define ELECTION_CONTROLLER_H

#include <QString>

#include "models/entities/election.h"
#include "models/entities/admin.h"

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
    Election *getElectionsForUser(int &electionsSize); 
    

    bool sendElectionDataToAdmin(const QString &electionId, const QString &adminId, const QByteArray &privateKey);
};

#endif 