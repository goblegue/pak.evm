#ifndef AUDITLOGREPOSITORY_H
#define AUDITLOGREPOSITORY_H
#include "../entities/audit_log.h"

class AuditLogRepository : public IAuditLogRepository {
public:
    bool insertLog(const AuditLog &log) override;
    AuditLog *getAllLogs(int &logsSize) override;
};
#endif
