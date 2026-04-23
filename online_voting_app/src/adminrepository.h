#ifndef ADMINREPOSITORY_H
#define ADMINREPOSITORY_H
#include "./user_management/admin/admin.h"
#include "databasemanager.h"
#include "states.h"
#include <mongocxx/collection.hpp>

class adminrepository : public IAdminRepository
{
private:
    mongocxx::collection m_collection;

public:
    adminrepository();
    ~adminrepository() = default;
    bool insertAdmin(const Admin &admin) override;
    std::optional<Admin> getAdminByCnic(const QString &cnic) override;
    std::optional<Admin> getAdminByEmail(const QString &email) override;

    bool addStatusChangeRequest(const QString &targetAdminCnic,
                                const QString &requestingAdminId,
                                const ApprovalStatus &status) override;

    int getAdminCount() override;
};

#endif // ADMINREPOSITORY_H
