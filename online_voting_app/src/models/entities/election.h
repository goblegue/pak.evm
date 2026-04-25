#ifndef ELECTION_H
#define ELECTION_H

#include <QDateTime>
#include <QString>
#include "models/states.h"
#include <optional>

class Election
{
private:
    QString m_id;
    QString m_title;
    QDateTime m_startTime;
    QDateTime m_endTime;
    ElectionState m_status;
    StatusChangeRequest *m_statusChangeRequests;
    int m_statusChangeCount;

public:
    Election()
        : m_status(ElectionState::Draft), m_statusChangeCount(0), m_statusChangeRequests(nullptr)
    {
    }
    Election(const Election &other)
        : m_id(other.m_id),
          m_title(other.m_title),
          m_startTime(other.m_startTime),
          m_endTime(other.m_endTime),
          m_status(other.m_status),
          m_statusChangeCount(other.m_statusChangeCount)
    {
        if (other.m_statusChangeCount > 0)
        {
            m_statusChangeRequests = new StatusChangeRequest[m_statusChangeCount];
            for (int i = 0; i < m_statusChangeCount; ++i)
            {
                m_statusChangeRequests[i] = other.m_statusChangeRequests[i];
            }
        }
        else
        {
            m_statusChangeRequests = nullptr;
        }
    }

    QString getId() const { return m_id; }
    QString getTitle() const { return m_title; }
    QDateTime getStartTime() const { return m_startTime; }
    QDateTime getEndTime() const { return m_endTime; }
    ElectionState getStatus() const { return m_status; }
    int getStatusCount(ApprovalStatus status)
    {
        int count{};
        for (int i{}; i < m_statusChangeCount; i++)
        {
            if (m_statusChangeRequests[i].status == status)
            {
                count++;
            }
        }
        return count;
    }
    void addStatusChangeRequest(const QString &adminId, const ApprovalStatus status)
    {
        for (int i = 0; i < m_statusChangeCount; ++i)
        {
            if (m_statusChangeRequests[i].requestById == adminId)
            {
                return; // Already request by this admin
            }
        }
        // Add new approver
        StatusChangeRequest newRequest{status, adminId};
        StatusChangeRequest *newStatusChangeRequests = new StatusChangeRequest[m_statusChangeCount + 1];
        for (int i = 0; i < m_statusChangeCount; ++i)
        {
            newStatusChangeRequests[i] = m_statusChangeRequests[i];
        }

        newStatusChangeRequests[m_statusChangeCount] = newRequest;
        delete[] m_statusChangeRequests;
        m_statusChangeRequests = newStatusChangeRequests;
        m_statusChangeCount++;
    }

    void setId(const QString &id) { m_id = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setStartTime(const QDateTime &time) { m_startTime = time; }
    void setEndTime(const QDateTime &time) { m_endTime = time; }
    void setStatus(ElectionState status) { m_status = status; }

    Election &operator=(const Election &other)
    {
        if (this != &other)
        { // Prevent self-assignment crash
            m_id = other.m_id;
            m_title = other.m_title;
            m_startTime = other.m_startTime;
            m_endTime = other.m_endTime;
            m_status = other.m_status;

            // Delete old memory before making new memory!
            delete[] m_statusChangeRequests;

            m_statusChangeCount = other.m_statusChangeCount;
            if (other.m_statusChangeCount > 0)
            {
                m_statusChangeRequests = new StatusChangeRequest[m_statusChangeCount];
                for (int i = 0; i < m_statusChangeCount; ++i)
                {
                    m_statusChangeRequests[i] = other.m_statusChangeRequests[i];
                }
            }
            else
            {
                m_statusChangeRequests = nullptr;
            }
        }
        return *this;
    }

    ~Election() { delete[] m_statusChangeRequests; }
};

class IElectionRepository
{
public:
    virtual ~IElectionRepository() = default;
    virtual bool insertElection(const Election &election) = 0;
    virtual bool updateElectionState(const QString &electionId, ElectionState newState) = 0;
    virtual bool addStatusChangeRequest(const QString &targetElectionId,
                                        const QString &requestingAdminId,
                                        const ApprovalStatus status) = 0;
    virtual Election *getAllElections(int & electionsSize) = 0;
    virtual std::optional<Election> getElectionById(const QString &id) = 0;
};

#endif // ELECTION_H
