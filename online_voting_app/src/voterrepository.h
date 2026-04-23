#ifndef VOTERREPOSITORY_H
#define VOTERREPOSITORY_H

#include "./voters/voters.h"
#include <mongocxx/collection.hpp>
#include <optional>

#include "states.h"
class voterrepository : public ITokenRepository
{
private:
    mongocxx::collection m_collection;

public:
    voterrepository();

    bool insertToken(const Voters &token) override;

    bool hasUserRequestedToken(const QString &cnic, const QString &electionId) override;

    Voters *getTokensByElection(const QString &electionId, int &votersSize) override;
};

#endif // VOTERREPOSITORY_H
