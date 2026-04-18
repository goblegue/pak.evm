#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <QString>
#include "states.h"
#include <optional>

class Candidate
{
private:
    QString m_id;
    QString m_userCnic;
    QString m_electionId;
    QString m_partyName;
    QString m_symbol;
    QString m_educationLevel;
    QString m_previousHistory;
    QString m_manifesto;
    QString m_symbolName;
    QString m_symbolBase64;
    QString m_profileImageBase64;
    StatusChangeRequest *m_statusChangeRequests;
    int m_statusChangeCount;
    ApprovalStatus m_status;

public:
    Candidate()
        : m_status(ApprovalStatus::Pending), m_statusChangeCount(0), m_statusChangeRequests(nullptr)
    {
    }

    Candidate(const Candidate &other)
        : m_id(other.m_id),
          m_userCnic(other.m_userCnic),
          m_electionId(other.m_electionId),
          m_partyName(other.m_partyName),
          m_symbol(other.m_symbol),
          m_educationLevel(other.m_educationLevel),
          m_previousHistory(other.m_previousHistory),
          m_manifesto(other.m_manifesto),
          m_symbolName(other.m_symbolName),
          m_symbolBase64(other.m_symbolBase64),
          m_profileImageBase64(other.m_profileImageBase64),
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

    // Getters
    QString getId() const { return m_id; }
    QString getUserCnic() const { return m_userCnic; }
    QString getElectionId() const { return m_electionId; }
    QString getPartyName() const { return m_partyName; }
    QString getEducationLevel() const { return m_educationLevel; }
    QString getPreviousHistory() const { return m_previousHistory; }
    QString getManifesto() const { return m_manifesto; }
    QString getSymbolName() const { return m_symbolName; }
    QString getSymbolBase64() const { return m_symbolBase64; }
    QString getProfileImageBase64() const { return m_profileImageBase64; }
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
    ApprovalStatus getStatus() const { return m_status; }

    // Setters
    void setId(const QString &id) { m_id = id; }
    void setUserCnic(const QString &cnic) { m_userCnic = cnic; }
    void setElectionId(const QString &electionId) { m_electionId = electionId; }
    void setPartyName(const QString &party) { m_partyName = party; }
    void setEducationLevel(const QString &edu) { m_educationLevel = edu; }
    void setPreviousHistory(const QString &history) { m_previousHistory = history; }
    void setManifesto(const QString &manifesto) { m_manifesto = manifesto; }
    void setSymbolName(const QString &name) { m_symbolName = name; }
    void setSymbolBase64(const QString &b64) { m_symbolBase64 = b64; }
    void setProfileImageBase64(const QString &b64) { m_profileImageBase64 = b64; }
    void setStatus(ApprovalStatus status) { m_status = status; }

    void addStatusChangeRequest(const QString &adminId, const ApprovalStatus &status)
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
    Candidate &operator=(const Candidate &other)
    {
        if (this != &other)             // Prevent self-assignment crash
        {
            m_id = other.m_id;
            m_userCnic = other.m_userCnic;
            m_electionId = other.m_electionId;
            m_partyName = other.m_partyName;
            m_symbol = other.m_symbol;
            m_educationLevel = other.m_educationLevel;
            m_previousHistory = other.m_previousHistory;
            m_manifesto = other.m_manifesto;
            m_symbolName = other.m_symbolName;
            m_symbolBase64 = other.m_symbolBase64;
            m_profileImageBase64 = other.m_profileImageBase64;
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
    ~Candidate() { delete[] m_statusChangeRequests; }
};

class ICandidateRepository
{
public:
    virtual ~ICandidateRepository() = default;
    virtual bool insertCandidate(const Candidate &candidate) = 0;
    virtual bool addStatusChangeRequest(const QString &targetCandidateCnic,
                                        const QString &requestingAdminId,
                                        const ApprovalStatus &status) = 0;
    virtual Candidate *getCandidatesByElection(const QString &electionId, int &candidatesSize) = 0;
};

#endif // CANDIDATE_H