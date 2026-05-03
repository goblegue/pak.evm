#ifndef VOTE_RECORD_H
#define VOTE_RECORD_H

#include <QString>
#include <QByteArray>
#include <cstddef>
#include "tokens.h"

class Token; // Forward declaration
class VoteRecord
{
private:
    int m_id;
    QString m_candidateCnic;
    QString m_timestamp;
    QByteArray m_currentHash;
    QByteArray m_previousHash;

public:
    VoteRecord() : m_id(-1) {}

    VoteRecord(const VoteRecord &other)
        : m_id(other.m_id),
          m_candidateCnic(other.m_candidateCnic),
          m_timestamp(other.m_timestamp),
          m_currentHash(other.m_currentHash),
          m_previousHash(other.m_previousHash)
    {
    }

    VoteRecord &operator=(const VoteRecord &other)
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_candidateCnic = other.m_candidateCnic;
            m_timestamp = other.m_timestamp;
            m_currentHash = other.m_currentHash;
            m_previousHash = other.m_previousHash;
        }
        return *this;
    }

    int getId() const { return m_id; }
    QString getCandidateCnic() const { return m_candidateCnic; }
    QString getTimestamp() const { return m_timestamp; }
    QByteArray getCurrentHash() const { return m_currentHash; }

    void setId(int id) { m_id = id; }
    void setCandidateCnic(const QString &cnic) { m_candidateCnic = cnic; }
    void setTimestamp(const QString &timestamp) { m_timestamp = timestamp; }
    void setCurrentHash(const QByteArray &hash) { m_currentHash = hash; }

    QByteArray getPreviousHash() const { return m_previousHash; }
    void setPreviousHash(const QByteArray &hash) { m_previousHash = hash; }
};

struct candidateVotes
{
    QString candidateCnic;
    int totalVotes;
};

class IVoteRepository
{
public:
    virtual ~IVoteRepository() = default;

    // VERY IMPORTANT: MUST BE WRAPPED IN SQLite `BEGIN TRANSACTION`
    virtual bool insertVoteTransaction(const VoteRecord &vote, const Token &token) = 0;

    // Returns the hash of the last inserted vote. Returns empty if genesis vote.
    virtual QByteArray getLatestVoteHash() = 0;

    // Returns all votes ordered strictly by m_id ASC for the forensic audit loop
    virtual VoteRecord *getAllVotesForAudit(int &votesSize) = 0;

    // Runs the GROUP BY SQL query and returns the results
    virtual candidateVotes *getElectionTally(int &tallySize) = 0;

    // [NEW] Extremely fast O(1) query: SELECT COUNT(*) FROM Votes;
    virtual int getTotalVotesCount() = 0;
};

#endif // VOTE_RECORD_H
