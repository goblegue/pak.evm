#include "configrepo.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>

bool ConfigRepository::saveConfig(const SystemConfig &c)
{
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    
    // [FIXED] Added pub_key_b64, scheduled_start, and scheduled_end to the SQL query
    q.prepare("INSERT OR REPLACE INTO SystemConfig (id, device_id, station_id, election_id, current_state, pub_key_b64, scheduled_start, scheduled_end, poll_opened_at, poll_closed_at) VALUES (1, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    q.addBindValue(c.getDeviceId());
    q.addBindValue(c.getStationId());
    q.addBindValue(c.getElectionId());
    q.addBindValue(static_cast<int>(c.getCurrentState()));
    q.addBindValue(c.getPublicKeyBase64());            // [NEW]
    q.addBindValue(c.getScheduledStartTime());         // [NEW]
    q.addBindValue(c.getScheduledEndTime());           // [NEW]
    q.addBindValue(c.getPollOpenedAt());
    q.addBindValue(c.getPollClosedAt());
    
    return q.exec();
}

std::optional<SystemConfig> ConfigRepository::getConfig()
{
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    
    // [FIXED] Must SELECT the new columns too!
    QSqlQuery q("SELECT device_id, station_id, election_id, current_state, pub_key_b64, scheduled_start, scheduled_end, poll_opened_at, poll_closed_at FROM SystemConfig WHERE id = 1");

    if (q.exec() && q.next())
    {
        SystemConfig c;
        c.setDeviceId(q.value(0).toString());
        c.setStationId(q.value(1).toString());
        c.setElectionId(q.value(2).toString());
        c.setCurrentState(static_cast<ElectionState>(q.value(3).toInt()));
        
        c.setPublicKeyBase64(q.value(4).toString());           // [NEW]
        c.setScheduledStartTime(q.value(5).toLongLong());      // [NEW] Parse safely as int64
        c.setScheduledEndTime(q.value(6).toLongLong());        // [NEW] Parse safely as int64
        
        c.setPollOpenedAt(q.value(7).toString());
        c.setPollClosedAt(q.value(8).toString());
        
        return c;
    }
    return std::nullopt;
}