
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <QString>
#include <QDateTime>
#include <cstddef>
class SystemConfig
{
private:
    QString m_deviceId;      // Assigned at first-time setup
    QString m_stationId;     // Loaded from USB
    QString m_electionId;    // Loaded from USB
    QString m_currentState;  // e.g., "SETUP", "OPEN", "PAUSED", "CLOSED"
    QString m_pollOpenedAt;  // Set when Admin clicks "Start Election"
    QString m_pollClosedAt;  // Set when Admin enters Master Password to close

public:
    SystemConfig() {}

    // Getters
    QString getDeviceId() const { return m_deviceId; }
    QString getStationId() const { return m_stationId; }
    QString getElectionId() const { return m_electionId; }
    QString getCurrentState() const { return m_currentState; }
    QString getPollOpenedAt() const { return m_pollOpenedAt; }
    QString getPollClosedAt() const { return m_pollClosedAt; }

    // Setters
    void setDeviceId(const QString &id) { m_deviceId = id; }
    void setStationId(const QString &id) { m_stationId = id; }
    void setElectionId(const QString &id) { m_electionId = id; }
    void setCurrentState(const QString &state) { m_currentState = state; }
    void setPollOpenedAt(const QString &time) { m_pollOpenedAt = time; }
    void setPollClosedAt(const QString &time) { m_pollClosedAt = time; }
};

class IConfigRepository
{
public:
    virtual ~IConfigRepository() = default;
    
    // Updates the single row in the database
    virtual bool saveConfig(const SystemConfig &config) = 0;
    
    // Retrieves the single row. If it doesn't exist, returns nullopt (meaning first boot)
    virtual std::optional<SystemConfig> getConfig() = 0; 
};

#endif 
