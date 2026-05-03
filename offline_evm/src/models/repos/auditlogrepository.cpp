#include "auditlogrepository.h"
#include "DatabaseManager.h"

bool AuditLogRepository::insertLog(const AuditLog &log) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q; q.prepare("INSERT INTO AuditLogs (timestamp, event_type, description) VALUES (?, ?, ?)");
    q.addBindValue(log.getTimestamp()); q.addBindValue(log.getEventType()); q.addBindValue(log.getDescription());
    return q.exec();
}

AuditLog* AuditLogRepository::getAllLogs(int &size) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery countQ("SELECT COUNT(*) FROM AuditLogs");
    if(countQ.next()) size = countQ.value(0).toInt();
    if(size == 0) return nullptr;

    AuditLog* arr = new AuditLog[size];

    QSqlQuery q("SELECT timestamp, event_type, description FROM AuditLogs ORDER BY timestamp ASC");
    int i = 0;
    while(q.next()) {
        arr[i].setTimestamp(q.value(0).toString()); arr[i].setEventType(q.value(1).toString()); arr[i].setDescription(q.value(2).toString());
        i++;
    }
    return arr;
}
