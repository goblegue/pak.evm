#ifndef RESULT_H
#define RESULT_H

#include <QString>
#include <QDateTime>
#include <optional>

// The ONLY struct we need (for the dynamic array of votes)
struct CandidateTally {
    QString candidateCnic;
    int totalVotes;
};

class Result {
private:
    QString m_electionId;
    QDateTime m_pollOpenedAt;
    QDateTime m_pollClosedAt;
    
    int m_totalVotesCast;
    
    QString m_auditStatus;
    QString m_finalLedgerHash;

    // Dynamic Array for Instructor Requirements
    CandidateTally* m_tally;
    int m_tallyCount;

public:
    Result() : m_tally(nullptr), m_tallyCount(0) {}

    // --- RULE OF THREE FOR DYNAMIC MEMORY ---
    ~Result() { delete[] m_tally; }

    Result(const Result &other)
        : m_electionId(other.m_electionId)
        , m_pollOpenedAt(other.m_pollOpenedAt)
        , m_pollClosedAt(other.m_pollClosedAt)
        , m_totalVotesCast(other.m_totalVotesCast)
        , m_auditStatus(other.m_auditStatus)
        , m_finalLedgerHash(other.m_finalLedgerHash)
        , m_tallyCount(other.m_tallyCount)
    {
        if (m_tallyCount > 0) {
            m_tally = new CandidateTally[m_tallyCount];
            for (int i = 0; i < m_tallyCount; ++i) m_tally[i] = other.m_tally[i];
        } else {
            m_tally = nullptr;
        }
    }

    Result& operator=(const Result& other) {
        if (this != &other) {
            m_electionId = other.m_electionId;

            m_pollOpenedAt = other.m_pollOpenedAt;
            m_pollClosedAt = other.m_pollClosedAt;

            m_totalVotesCast = other.m_totalVotesCast;
            m_auditStatus = other.m_auditStatus;
            m_finalLedgerHash = other.m_finalLedgerHash;

            delete[] m_tally; // Free old memory
            
            m_tallyCount = other.m_tallyCount;
            if (m_tallyCount > 0) {
                m_tally = new CandidateTally[m_tallyCount];
                for (int i = 0; i < m_tallyCount; ++i) m_tally[i] = other.m_tally[i];
            } else {
                m_tally = nullptr;
            }
        }
        return *this;
    }

    // --- GETTERS ---
    QString getElectionId() const { return m_electionId; }

    QDateTime getPollOpenedAt() const { return m_pollOpenedAt; }
    QDateTime getPollClosedAt() const { return m_pollClosedAt; }

    int getTotalVotesCast() const { return m_totalVotesCast; }
    QString getAuditStatus() const { return m_auditStatus; }
    QString getFinalLedgerHash() const { return m_finalLedgerHash; }
    CandidateTally* getTally() const { return m_tally; }
    int getTallyCount() const { return m_tallyCount; }

    // --- SETTERS ---
    void setElectionId(const QString& id) { m_electionId = id; }

    void setPollOpenedAt(const QDateTime& time) { m_pollOpenedAt = time; }
    void setPollClosedAt(const QDateTime& time) { m_pollClosedAt = time; }

    void setTotalVotesCast(int count) { m_totalVotesCast = count; }
    void setAuditStatus(const QString& status) { m_auditStatus = status; }
    void setFinalLedgerHash(const QString& hash) { m_finalLedgerHash = hash; }
    
    void setTallyData(CandidateTally* tallyArray, int count) {
        delete[] m_tally; // Free existing
        m_tallyCount = count;
        if (count > 0) {
            m_tally = new CandidateTally[count];
            for (int i = 0; i < count; ++i) m_tally[i] = tallyArray[i];
        } else {
            m_tally = nullptr;
        }
    }
};

// ==========================================
// INTERFACE FOR DATABASE LEAD
// ==========================================
class IPollResultRepository {
public:
    virtual ~IPollResultRepository() = default;
    
    virtual bool insertPollResult(const Result& result) = 0;
    
    // Per your request: Retrieve results for a specific election
    virtual std::optional<Result> getResultByElection(const QString& electionId) = 0;
};

#endif // RESULT_H