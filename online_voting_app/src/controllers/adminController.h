#ifndef ADMINCONTROLLER_H
#define ADMINCONTROLLER_H

#include "models/entities/admin.h"
#include "models/states.h"
#include <optional>
#include <QString>

class IAdminRepository; 

class AdminController
{
private:
    IAdminRepository *m_adminRepo;
    AdminController();
    ~AdminController();

public:
    AdminController(const AdminController &) = delete;
    void operator=(const AdminController &) = delete;

    static AdminController &getInstance();
    void injectRepositories(IAdminRepository *adminRepo);

    bool addStatusChangeRequest(const QString &targetAdminCnic,
                                const QString &requestingAdminId,
                                const ApprovalStatus &status);

    int getAdminCount();

    std::optional<Admin *> getAllAdminsExcept(const QString &cnic, int &count);
};

#endif 