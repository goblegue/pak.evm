#ifndef ELECTION_H
#define ELECTION_H

#include <QDateTime>
#include <QList>
#include <QString>
#include "states.h"
#include <optional>

class Election
{
private:
    QString m_id;
    QString m_title;
    QDateTime m_startTime;
    QDateTime m_endTime;
    ElectionState m_status;

public:
    Election()
        : m_status(ElectionState::Draft)
    {}

    QString getId() const { return m_id; }
    QString getTitle() const { return m_title; }
    QDateTime getStartTime() const { return m_startTime; }
    QDateTime getEndTime() const { return m_endTime; }
    ElectionState getStatus() const { return m_status; }

    void setId(const QString &id) { m_id = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setStartTime(const QDateTime &time) { m_startTime = time; }
    void setEndTime(const QDateTime &time) { m_endTime = time; }
    void setStatus(ElectionState status) { m_status = status; }
};

class IElectionRepository
{
public:
    virtual ~IElectionRepository() = default;
    virtual bool insertElection(const Election &election) = 0;
    virtual bool updateElectionState(const QString &electionId, ElectionState newState) = 0;
    virtual Election *getAllElections(int electionsSize) = 0;
    virtual std::optional<Election> getElectionById(const QString &id) = 0;
};

#endif // ELECTION_H