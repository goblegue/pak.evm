#ifndef VOTER_CONTROLLER_H
#define VOTER_CONTROLLER_H

#include <optional>

#include <QImage>

#include "models/entities/voters.h"
#include "models/entities/user.h"
#include "models/entities/election.h"

using namespace std;

class TokenController
{
private:
    ITokenRepository *m_tokenRepo;
    IElectionRepository *m_electionRepo;
    TokenController();
    ~TokenController();

public:
    TokenController(const TokenController &) = delete;
    void operator=(const TokenController &) = delete;

    static TokenController &getInstance();
    void injectRepositories(ITokenRepository *tokenRepo, IElectionRepository *electionRepo);

    bool canRequestToken(const QString &userCnic, const QString &electionId);

    optional<QImage> requestVotingToken(const QString &userCnic, const QString &electionId, const QByteArray &privateKey);

    bool saveToken(const Token &token);

    Token *getVoterTokens(const QString &userCnic, int &tokensSize);

    bool sendTokenToEmail(const Token &token, const QString &email);

    optional<QImage> getQrCodeForToken(const Token &token);

    std::optional<int> getTokenCountForElection(const QString &electionId);
};
#endif // VOTER_CONTROLLER_H