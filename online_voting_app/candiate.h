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
    ApprovalStatus m_status;

public:
    Candidate()
        : m_status(ApprovalStatus::Pending)
    {}

    // Getters
    QString getId() const { return m_id; }
    QString getUserCnic() const { return m_userCnic; }
    QString getElectionId() const { return m_electionId; }
    QString getPartyName() const { return m_partyName; }
    QString getEducationLevel() const { return m_educationLevel; }
    QString getPreviousHistory() const { return m_previousHistory; }
    QString getManifesto() const { return m_manifesto; }
    QString getWikiLink() const { return m_wikiLink; }
    QString getSymbolName() const { return m_symbolName; }
    QString getSymbolBase64() const { return m_symbolBase64; }
    QString getProfileImageBase64() const { return m_profileImageBase64; }
    ApprovalStatus getStatus() const { return m_status; }

    // Setters
    void setId(const QString &id) { m_id = id; }
    void setUserCnic(const QString &cnic) { m_userCnic = cnic; }
    void setElectionId(const QString &electionId) { m_electionId = electionId; }
    void setPartyName(const QString &party) { m_partyName = party; }
    void setEducationLevel(const QString &edu) { m_educationLevel = edu; }
    void setPreviousHistory(const QString &history) { m_previousHistory = history; }
    void setManifesto(const QString &manifesto) { m_manifesto = manifesto; }
    void setWikiLink(const QString &link) { m_wikiLink = link; }
    void setSymbolName(const QString &name) { m_symbolName = name; }
    void setSymbolBase64(const QString &b64) { m_symbolBase64 = b64; }
    void setProfileImageBase64(const QString &b64) { m_profileImageBase64 = b64; }
    void setStatus(ApprovalStatus status) { m_status = status; }
};

class ICandidateRepository
{
public:
    virtual ~ICandidateRepository() = default;
    virtual bool insertCandidate(const Candidate &candidate) = 0;
    virtual bool updateCandidateStatus(const QString &candidateId, ApprovalStatus status) = 0;
    virtual Candidate *getCandidatesByElection(const QString &electionId, int &candidatesSize) = 0;
};

#endif // CANDIDATE_H