#ifndef VOTEREPOSITORY_H
#define VOTEREPOSITORY_H

#include "../entities/tokens.h"
#include "../entities/voting_record.h"

class VoteRepository : public IVoteRepository
{
public:
    bool insertVoteTransaction(const VoteRecord &vote, const Token &token) override;

    QByteArray getLatestVoteHash() override;
    VoteRecord *getAllVotesForAudit(int &votesSize) override;
    candidateVotes *getElectionTally(int &tallySize) override;

    int getTotalVotesCount() override;
};

#endif // VOTEREPOSITORY_H
