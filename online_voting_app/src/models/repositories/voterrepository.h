#ifndef VOTERREPOSITORY_H
#define VOTERREPOSITORY_H

#include "models/entities/voters.h"
#include <mongocxx/collection.hpp>
#include <optional>

#include "models/states.h"
class TokenRepository : public ITokenRepository
{
private:
    mongocxx::collection m_collection;

public:
    TokenRepository();

    bool insertToken(const Token &token) override;

    bool hasUserRequestedToken(const QString &cnic, const QString &electionId) override;

    Token *getTokensByElection(const QString &electionId, int &votersSize) override;
    Token *getTokensByUser(const QString &userCnic, int &tokensSize) override;
};

#endif // VOTERREPOSITORY_H
