#include "adminController.h"
#include "models/repositories/adminrepository.h"

const int ADMIN_APPROVAL_THRESHOLD = 2;  // More than 50% of admins must approve
const int ADMIN_REJECTION_THRESHOLD = 3; // More than 33% of admins must reject

AdminController::AdminController() : m_adminRepo(nullptr) {}

AdminController::~AdminController() {}

AdminController &AdminController::getInstance()
{
    static AdminController instance;
    return instance;
}

void AdminController::injectRepositories(IAdminRepository *adminRepo)
{
    m_adminRepo = adminRepo;
}


bool AdminController::addStatusChangeRequest(const QString &targetAdminCnic, const QString &requestingAdminId, const ApprovalStatus &status)
{
    if (!m_adminRepo)
        return false;

    auto targetOpt = m_adminRepo->getAdminByCnic(targetAdminCnic);
    if (!targetOpt.has_value())
        return false;
    Admin targetAdmin = targetOpt.value();

    auto reqOpt = m_adminRepo->getAdminByCnic(requestingAdminId);
    if (!reqOpt.has_value())
        return false;
    Admin reqAdmin = reqOpt.value();

    if (reqAdmin.getStatus() != ApprovalStatus::Approved)
    {
        return false; // Only approved admins can request a status change
    }

    if (targetAdmin.hasAdminVoted(reqAdmin.getCnic())) {
        return false;
    }

    if (!m_adminRepo->addStatusChangeRequest(targetAdminCnic, requestingAdminId, status)) {
        return false;
    }

    targetAdmin.addStatusChangeRequest(requestingAdminId, status);

    int approvedCount = targetAdmin.getStatusCount(ApprovalStatus::Approved);
    int rejectedCount = targetAdmin.getStatusCount(ApprovalStatus::Rejected);

    int totalAdmins = m_adminRepo->getApprovedAdminCount();

    if (approvedCount > (totalAdmins / ADMIN_APPROVAL_THRESHOLD))
    {
        return m_adminRepo->updateAdminStatus(targetAdminCnic, ApprovalStatus::Approved);
    }
    if (rejectedCount > (totalAdmins / ADMIN_REJECTION_THRESHOLD))
    {
        return m_adminRepo->updateAdminStatus(targetAdminCnic, ApprovalStatus::Rejected);
    }

    return true;
}

int AdminController::getAdminCount()
{
    if (!m_adminRepo)
        return 0;
    return m_adminRepo->getAdminCount();
}


std::optional<Admin *> AdminController::getAllAdminsExcept(const QString &cnic, int &count)
{
    if (!m_adminRepo)
    {
        count = 0;
        return std::nullopt;
    }
    return m_adminRepo->getAllAdminsExcept(cnic, count);
}