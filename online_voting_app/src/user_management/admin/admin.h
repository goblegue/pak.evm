#ifndef ADMIN_H
#define ADMIN_H

#include "../../states.h"
#include "../user/user.h"
#include <optional>
#include <QString>

class Admin : public User
{
private:
    ApprovalStatus m_status;
    StatusChangeRequest *m_statusChangeRequests;
    int m_statusChangeCount;

public:
    Admin()
        : User(), m_status(ApprovalStatus::Pending), m_statusChangeCount(0), m_statusChangeRequests(nullptr)
    {
    }

    Admin(const Admin &other)
        : User(other), // Copy the base class data
          m_status(other.m_status),
          m_statusChangeCount(other.m_statusChangeCount)
    {
        if (other.m_statusChangeCount > 0)
        {
            m_statusChangeRequests = new StatusChangeRequest[m_statusChangeCount];
            for (int i = 0; i < m_statusChangeCount; ++i)
            {
                m_statusChangeRequests[i] = other.m_statusChangeRequests[i];
            }
        }
        else
        {
            m_statusChangeRequests = nullptr;
        }
    }

    Admin &operator=(const Admin &other)
    {
        if (this != &other)
        {                           // Prevent self-assignment crash
            User::operator=(other); // Copy base class
            m_status = other.m_status;

            // Delete old memory before making new memory!
            delete[] m_statusChangeRequests;

            m_statusChangeCount = other.m_statusChangeCount;
            if (other.m_statusChangeCount > 0)
            {
                m_statusChangeRequests = new StatusChangeRequest[m_statusChangeCount];
                for (int i = 0; i < m_statusChangeCount; ++i)
                {
                    m_statusChangeRequests[i] = other.m_statusChangeRequests[i];
                }
            }
            else
            {
                m_statusChangeRequests = nullptr;
            }
        }
        return *this;
    }

    ApprovalStatus getStatus() const { return m_status; }
    void setStatus(ApprovalStatus status) { m_status = status; }

    int getStatusCount(ApprovalStatus status)
    {
        int count{};
        for (int i{}; i < m_statusChangeCount; i++)
        {
            if (m_statusChangeRequests[i].status == status)
            {
                count++;
            }
        }
        return count;
    }

    void addStatusChangeRequest(const QString &adminId, const ApprovalStatus &status)
    {
        for (int i = 0; i < m_statusChangeCount; ++i)
        {
            if (m_statusChangeRequests[i].requestById == adminId)
            {
                return; // Already request by this admin
            }
        }
        // Add new approver
        StatusChangeRequest newRequest{status, adminId};
        StatusChangeRequest *newStatusChangeRequests = new StatusChangeRequest[m_statusChangeCount + 1];
        for (int i = 0; i < m_statusChangeCount; ++i)
        {
            newStatusChangeRequests[i] = m_statusChangeRequests[i];
        }

        newStatusChangeRequests[m_statusChangeCount] = newRequest;
        delete[] m_statusChangeRequests;
        m_statusChangeRequests = newStatusChangeRequests;
        m_statusChangeCount++;
    }

    ~Admin() { delete[] m_statusChangeRequests; }
};

class IAdminRepository
{
public:
    virtual ~IAdminRepository() = default;
    virtual bool insertAdmin(const Admin &admin) = 0;
    virtual std::optional<Admin> getAdminByCnic(const QString &cnic) = 0;
    virtual std::optional<Admin> getAdminByEmail(const QString &email) = 0;
    virtual bool addStatusChangeRequest(const QString &targetAdminCnic,
                                        const QString &requestingAdminId,
                                        const ApprovalStatus &status) = 0;
    virtual bool updateAdminStatus(const QString &cnic, const ApprovalStatus &status) = 0;
    virtual std::optional<StatusChangeRequest *> getStatusChangeRequests(const QString &cnic, int &count) = 0;
    virtual std::optional<Admin *> getAllAdmins(int &count) = 0;                                    
    virtual int getAdminCount() = 0;
};

#endif // ADMIN_H
