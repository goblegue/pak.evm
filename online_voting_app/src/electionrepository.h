#ifndef ELECTIONREPOSITORY_H
#define ELECTIONREPOSITORY_H

#include "./election/election.h"
#include <mongocxx/collection.hpp>
#include <optional>
#include "states.h"

class electionrepository : public IElectionRepository
{
private:
    mongocxx::collection m_collection;

public:
    electionrepository();

    bool insertElection(const Election &election) override;
    bool updateElectionState(const QString &electionId, ElectionState newState) override;

    bool addStatusChangeRequest(const QString &targetElectionId,
                                const QString &requestingAdminId,
                                const ApprovalStatus status) override;

    Election *getAllElections(int &electionsSize) override;
    std::optional<Election> getElectionById(const QString &id) override;
};

#endif // ELECTIONREPOSITORY_H
