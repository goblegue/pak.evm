#ifndef ELECTION_MANAGER_H
#define ELECTION_MANAGER_H

#include "election.h"
#include "../user_management/admin/admin.h"

class ElectionManager
{
private:
    IElectionRepository *m_electionRepo;
    IAdminRepository *m_adminRepo;

}

#endif // ELECTION_MANAGER_H