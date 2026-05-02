#ifndef VOTEREPOSITORY_H
#define VOTEREPOSITORY_H

#include "../models/entities/voting_record.h"
#include "../models/entities/tokens.h"

class VoteRepository : public IVoteRepository {
public:

    bool insertVoteTransaction(const VoteRecord &vote, const Token &token);


    bool insertVote(const VoteRecord &vote) override; // You can leave this if the interface still has it, or remove it if Lead deleted it
    QByteArray getLatestVoteHash() override;
    VoteRecord *getAllVotesForAudit(int &votesSize) override;
    candidateVotes *getElectionTally(int &tallySize) override;
};

#endif // VOTEREPOSITORY_H
