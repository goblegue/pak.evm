#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <QString>

// [NEW] Strict Enum for Election State
enum class ElectionState {
    Setup,          // 0: Fresh boot, waiting for Master Key
    ReadyWaiting,   // 1: JSON loaded, waiting for Start Time countdown
    Open,           // 2: Countdown hit 0, QR scanner is active
    Paused,         // 3: Emergency pause by Poll Worker
    Closed          // 4: Election over, waiting for Master Key to tally
};

class SystemConfig
{
private:
    QString m_deviceId;              
    QString m_stationId;             
    QString m_electionId;            
    ElectionState m_currentState;    // [CHANGED] Now uses the Enum instead of QString
    QString m_systemPublicKeyBase64; 
    
    qint64 m_scheduledStartTime;     // [CHANGED] qint64 for MSecSinceEpoch timers
    qint64 m_scheduledEndTime;       // [CHANGED] qint64 for MSecSinceEpoch timers
    
    QString m_pollOpenedAt;          
    QString m_pollClosedAt;          

public:
    SystemConfig() : m_currentState(ElectionState::Setup), 
                     m_scheduledStartTime(0), 
                     m_scheduledEndTime(0) {}

    // Getters
    QString getDeviceId() const { return m_deviceId; }
    QString getStationId() const { return m_stationId; }
    QString getElectionId() const { return m_electionId; }
    ElectionState getCurrentState() const { return m_currentState; } // [CHANGED]
    QString getPollOpenedAt() const { return m_pollOpenedAt; }
    QString getPollClosedAt() const { return m_pollClosedAt; }
    QString getPublicKeyBase64() const { return m_systemPublicKeyBase64; }
    qint64 getScheduledStartTime() const { return m_scheduledStartTime; } // [CHANGED]
    qint64 getScheduledEndTime() const { return m_scheduledEndTime; }     // [CHANGED]

    // Setters
    void setDeviceId(const QString &id) { m_deviceId = id; }
    void setStationId(const QString &id) { m_stationId = id; }
    void setElectionId(const QString &id) { m_electionId = id; }
    void setCurrentState(ElectionState state) { m_currentState = state; } // [CHANGED]
    void setPollOpenedAt(const QString &time) { m_pollOpenedAt = time; }
    void setPollClosedAt(const QString &time) { m_pollClosedAt = time; }
    void setPublicKeyBase64(const QString &key) { m_systemPublicKeyBase64 = key; }
    void setScheduledStartTime(qint64 time) { m_scheduledStartTime = time; } // [CHANGED]
    void setScheduledEndTime(qint64 time) { m_scheduledEndTime = time; }     // [CHANGED]
};

class IConfigRepository
{
public:
    virtual ~IConfigRepository() = default;
    virtual bool saveConfig(const SystemConfig &config) = 0;
    virtual std::optional<SystemConfig> getConfig() = 0;
};

#endif // SYSTEM_CONFIG_H