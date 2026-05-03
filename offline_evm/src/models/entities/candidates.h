#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <QString>
#include <cstddef>
class Candidate
{
private:
    QString m_cnic;
    QString m_name;
    QString m_partyName;
    QString m_symbolName;
    QString m_symbolBase64;
    QString m_profileImageBase64;

public:
    Candidate() {}

    Candidate(const Candidate &other)
        : m_cnic(other.m_cnic),
          m_name(other.m_name),
          m_partyName(other.m_partyName),
          m_symbolName(other.m_symbolName),
          m_symbolBase64(other.m_symbolBase64),
          m_profileImageBase64(other.m_profileImageBase64)

    {
    }

    Candidate &operator=(const Candidate &other)
    {
        if (this != &other)
        {
            m_cnic = other.m_cnic;
            m_partyName = other.m_partyName;
            m_symbolName = other.m_symbolName;
            m_symbolBase64 = other.m_symbolBase64;
            m_profileImageBase64 = other.m_profileImageBase64;
            m_name = other.m_name;
        }
        return *this;
    }

    // Getters
    QString getCnic() const { return m_cnic; }
    QString getPartyName() const { return m_partyName; }
    QString getSymbolName() const { return m_symbolName; }
    QString getSymbolBase64() const { return m_symbolBase64; }
    QString getProfileImageBase64() const { return m_profileImageBase64; }
    QString getName() const { return m_name; }
    // Setters
    void setCnic(const QString &cnic) { m_cnic = cnic; }
    void setPartyName(const QString &party) { m_partyName = party; }
    void setSymbolName(const QString &name) { m_symbolName = name; }
    void setSymbolBase64(const QString &b64) { m_symbolBase64 = b64; }
    void setProfileImageBase64(const QString &b64) { m_profileImageBase64 = b64; }
    void setName(const QString &name) { m_name = name; }
};

class ICandidateRepository
{
public:
    virtual ~ICandidateRepository() = default;
    virtual bool insertCandidate(const Candidate &candidate) = 0;
    virtual Candidate *getAllCandidates(int &candidatesSize) = 0;
    virtual bool clearAllCandidates() = 0; // Run when prepping a new election
};

#endif // CANDIDATE_H
