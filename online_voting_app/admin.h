#ifndef ADMIN_H
#define ADMIN_H

#include "states.h"
#include "user.h"
#include <optional>

class Admin : public User
{
private:
    ApprovalStatus m_status;
    Qstring *m_approvedByAdminIds;
    int m_approvalCount;

public:
    Admin()
        : User()
        , m_status(ApprovalStatus::Pending)
    {}

    ApprovalStatus getStatus() const { return m_status; }
    void setStatus(ApprovalStatus status) { m_status = status; }

    QString *getApprovedBy() const { return m_approvedByAdminIds; }
    void addApprover(const QString &adminId)
    {
        for (int i = 0; i < m_approvalCount; ++i) {
            if (m_approvedByAdminIds[i] == adminId) {
                return; // Already approved by this admin
            }
        }
        // Add new approver
        QString *newApprovers = new QString[m_approvalCount + 1];
        for (int i = 0; i < approvalCount; ++i) {
            newApprovers[i] = m_approvedByAdminIds[i];
        }
        newApprovers[m_approvalCount] = adminId;
        delete[] m_approvedByAdminIds;
        m_approvedByAdminIds = newApprovers;
        m_approvalCount++;
    }

    ~Admin() { delete[] m_approvedByAdminIds; }
};

class IAdminRepository
{
public:
    virtual ~IAdminRepository() = default;
    virtual bool insertAdmin(const Admin &admin) = 0;
    virtual std::optional<Admin> getAdminByCnic(const QString &cnic) = 0;
    virtual bool addApprovalSignature(const QString &targetAdminCnic,
                                      const QString &approvingAdminId)
        = 0;
};

#endif // ADMIN_H
