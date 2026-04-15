#ifndef VOTING_STATION_H
#define VOTING_STATION_H

#include <QString>

class VotingStation
{
private:
    QString m_id;
    QString m_stationName;
    QString m_city;
    QString m_address;
    bool m_isActive;

public:
    VotingStation()
        : m_isActive(true)
    {}

    QString getId() const { return m_id; }
    QString getStationName() const { return m_stationName; }
    QString getCity() const { return m_city; }
    QString getAddress() const { return m_address; }
    bool isActive() const { return m_isActive; }

    void setId(const QString &id) { m_id = id; }
    void setStationName(const QString &name) { m_stationName = name; }
    void setCity(const QString &city) { m_city = city; }
    void setAddress(const QString &address) { m_address = address; }
    void setIsActive(bool active) { m_isActive = active; }
};

class IVotingStationRepository
{
public:
    virtual ~IVotingStationRepository() = default;
    virtual bool insertStation(const VotingStation &station) = 0;
    virtual VotingStation *getAllActiveStations(int &vSSize) = 0;
    virtual VotingStation *getStationsByCity(const QString &city, int &vSSize) = 0;
};

#endif // VOTING_STATION_H
