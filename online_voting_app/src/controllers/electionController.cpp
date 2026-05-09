#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir> 

#include "controllers/electionController.h"
#include "controllers/candidateController.h"
#include "services/crypto/cryptoengine.h"
#include "services/email/emailservice.h"
#include <optional>

using namespace std;

const int ELECTION_APPROVAL_THRESHOLD = 2;  // More than 50% of admins must approve
const int ELECTION_REJECTION_THRESHOLD = 3; // More than 33% of admins

ElectionController::ElectionController() : m_electionRepo(nullptr), m_adminRepo(nullptr) {}

ElectionController::~ElectionController() {}

ElectionController &ElectionController::getInstance()
{
    static ElectionController instance;
    return instance;
}

void ElectionController::injectRepositories(IElectionRepository *electionRepo, IAdminRepository *adminRepo)
{
    m_electionRepo = electionRepo;
    m_adminRepo = adminRepo;
}

bool ElectionController::createElection(const Election &election)
{
    if (!m_electionRepo)
    {
        return false; // Repository not injected
    }
    int electionsSize{};
    Election *existingElections = m_electionRepo->getAllElections(electionsSize); // Get all elections to check for duplicate ID
    for (int i = 0; i < electionsSize; ++i)
    {
        if (existingElections[i].getId() == election.getId() ||
            (existingElections[i].getTitle() == election.getTitle() &&
             existingElections[i].getStatus() != ElectionState::ResultsAnnounced))
        {
            delete[] existingElections;
            return false; // Election with same ID or title which results are not announced already exists
        }
    }
    delete[] existingElections;
    return m_electionRepo->insertElection(election);
}

bool ElectionController::requestElectionStatusChange(const QString &electionId, const QString &adminId, ApprovalStatus status)
{
    if (!m_electionRepo || !m_adminRepo)
    {
        return false; // Repositories not injected
    }
    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        return false; // Election not found
    }
    Election election = electionOpt.value();
    auto adminOpt = m_adminRepo->getAdminByCnic(adminId);
    if (!adminOpt.has_value())
    {
        return false; // Admin not found
    }
    Admin admin = adminOpt.value();
    if (admin.getStatus() != ApprovalStatus::Approved)
    {
        return false; // Admin not approved
    }
    m_electionRepo->addStatusChangeRequest(electionId, adminId, status);
    election.addStatusChangeRequest(adminId, status);
    int approvedCount = election.getStatusCount(ApprovalStatus::Approved);
    int rejectedCount = election.getStatusCount(ApprovalStatus::Rejected);

    int totalAdmins = m_adminRepo->getApprovedAdminCount();
    if (approvedCount > (totalAdmins / ELECTION_APPROVAL_THRESHOLD))
    {
        return m_electionRepo->updateElectionState(electionId, ElectionState::Drafted);
    }
    if (rejectedCount > (totalAdmins / ELECTION_REJECTION_THRESHOLD))
    {
        return m_electionRepo->updateElectionState(electionId, ElectionState::Rejected);
    }
    return true;
}

Election *ElectionController::getAllElections(int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    return m_electionRepo->getAllElections(electionsSize);
}

Election *ElectionController::getElectionsByStatus(ElectionState status, int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    Election *allElections = m_electionRepo->getAllElections(electionsSize);
    Election *filteredElections = new Election[electionsSize];
    int count = 0;
    for (int i = 0; i < electionsSize; ++i)
    {
        if (allElections[i].getStatus() == status)
        {
            filteredElections[count++] = allElections[i];
        }
    }
    delete[] allElections;
    electionsSize = count;
    return filteredElections;
}

Election *ElectionController::getElectionsForUser(int &electionsSize)
{
    if (!m_electionRepo)
    {
        electionsSize = 0;
        return nullptr; // Repository not injected
    }
    Election *allElections = m_electionRepo->getAllElections(electionsSize);
    Election *filteredElections = new Election[electionsSize];
    int count = 0;
    for (int i = 0; i < electionsSize; ++i)
    {
        if (allElections[i].getStatus() >= ElectionState::Published)
        {
            filteredElections[count++] = allElections[i];
        }
    }
    delete[] allElections;
    electionsSize = count;
    return filteredElections;
}

bool ElectionController::sendElectionConfigToAdmin(const QString &electionId, const QString &adminCnic, const QByteArray &privateKey, const QByteArray &publicKey)
{
    if (!m_electionRepo || !m_adminRepo)
    {
        return false; // Repositories not injected
    }
    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        return false; // Election not found
    }
    Election election = electionOpt.value();
    if (!(election.getStatus() == ElectionState::Published) &&
        !(election.getStatus() == ElectionState::VotingOpen))
    {
        return false; // Election not active
    }
    auto adminOpt = m_adminRepo->getAdminByCnic(adminCnic);
    if (!adminOpt.has_value())
    {
        return false; // Admin not found
    }
    Admin admin = adminOpt.value();
    if (admin.getStatus() != ApprovalStatus::Approved)
    {
        return false; // Admin not approved
    }

    // --- Build the Payload ---
    QJsonObject electionJson;
    electionJson["id"] = election.getId();
    electionJson["title"] = election.getTitle();
    electionJson["publishTime"] = election.getPublishTime().toString(Qt::ISODate); // (If you have this getter)
    electionJson["startTime"] = election.getStartTime().toString(Qt::ISODate);
    electionJson["endTime"] = election.getEndTime().toString(Qt::ISODate);

    QString candidatesJsonString = CandidateController::getInstance().getCandidatesJsonByElection(electionId);
    QByteArray candidateBytes = candidatesJsonString.toUtf8();
    QJsonDocument candidateJsonDoc = QJsonDocument::fromJson(candidateBytes);

    if (candidateJsonDoc.isNull() || !candidateJsonDoc.isArray())
    {
        return false;
    }
    QJsonArray candidateJsonArray = candidateJsonDoc.array();

    QJsonObject payload;
    payload["election"] = electionJson;
    payload["candidates"] = candidateJsonArray;
    payload["exportTimeStamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(payload);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact); // Must be compact for strict signature!

    auto signatureOpt = CryptoEngine::getInstance().signMessage(jsonData, privateKey);
    if (!signatureOpt.has_value())
    {
        return false;
    }

    QByteArray signature = signatureOpt.value();
    payload["signature"] = QString::fromLatin1(signature.toBase64());

    // --- Save File to Temporary Path ---
    doc.setObject(payload);
    QString fileName = "election.config.json";                  // Required File Name
    QString attachmentPath = QDir::tempPath() + "/" + fileName; // Safe OS path
    QFile file(attachmentPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    // Save as Indented so the Admin can look at it if they want (but they shouldn't edit it!)
    QByteArray finalJsonData = doc.toJson(QJsonDocument::Indented);
    file.write(finalJsonData);
    file.close();

    // --- Prepare the Public Key for Copy/Pasting ---
    QString pubKeyBase64 = QString::fromUtf8(publicKey.toBase64());

    // --- Craft Stylized HTML Email ---
    QString adminEmail = admin.getEmail();
    QString subject = QString("EVM Setup Configuration - %1").arg(election.getTitle());

    QString htmlBody = QString(
                           "<div style=\"font-family: 'Segoe UI', Arial, sans-serif; max-width: 650px; margin: 0 auto; border: 1px solid #e0e0e0; border-radius: 10px; padding: 25px; box-shadow: 0 4px 8px rgba(0,0,0,0.05);\">"
                           "<div style=\"text-align: center; border-bottom: 2px solid #1565C0; padding-bottom: 15px; margin-bottom: 20px;\">"
                           "<h1 style=\"color: #1565C0; margin: 0;\">Election Configuration Export</h1>"
                           "<p style=\"color: #555; margin-top: 5px;\">Pakistan Election Commission - Secure EVM System</p>"
                           "</div>"

                           "<p style=\"font-size: 16px; color: #333;\">Dear Administrator %1,</p>"
                           "<p style=\"font-size: 15px; color: #555; line-height: 1.5;\">The configuration for <b>%2</b> has been successfully signed and exported. You must use the attached file and the cryptographic key below to initialize the Offline EVM Terminal.</p>"

                           "<div style=\"background-color: #f8f9fa; border-left: 4px solid #F57F17; padding: 15px; border-radius: 4px; margin: 25px 0;\">"
                           "<h3 style=\"margin-top: 0; color: #E65100;\">System Public Key:</h3>"
                           "<p style=\"margin: 10px 0; word-wrap: break-word; font-family: monospace; font-size: 14px; color: #333; background: #e0e0e0; padding: 10px; border-radius: 4px;\">%3</p>"
                           "</div>"

                           "<h3 style=\"color: #1565C0; border-bottom: 1px solid #eee; padding-bottom: 10px;\">EVM Initialization Instructions:</h3>"
                           "<ol style=\"color: #555; font-size: 15px; line-height: 1.6;\">"
                           "<li>Copy the <b>Public Key</b> provided in the orange box above.</li>"
                           "<li>Launch the Offline EVM (System 2) and paste the key into the initial setup screen.</li>"
                           "<li>Download the attached <b>election.config.json</b> file to a secure USB Flash Drive.</li>"
                           "<li>Plug the USB into the EVM terminal to securely load the candidates and election rules.</li>"
                           "</ol>"

                           "<div style=\"background-color: #ffebee; border: 1px solid #ef9a9a; border-radius: 5px; padding: 15px; text-align: center; margin-top: 30px;\">"
                           "<p style=\"color: #c62828; font-size: 14px; margin: 0; font-weight: bold;\">⚠️ FORENSIC WARNING</p>"
                           "<p style=\"color: #d32f2f; font-size: 13px; margin-top: 5px;\">Do not open or modify the <code>election.config.json</code> file in any text editor. Altering even a single space will invalidate the cryptographic signature, and the EVM will reject the election initialization.</p>"
                           "</div>"
                           "</div>")
                           .arg(admin.getName())
                           .arg(election.getTitle())
                           .arg(pubKeyBase64);


    bool emailSuccess = EmailService::getInstance().sendEmail(adminEmail, subject, htmlBody, true, attachmentPath);

    QFile::remove(attachmentPath); // Memory management: Delete temp file from OS

    return emailSuccess;
}

// std::optional<Election> ElectionController::getElectionById(const QString &id)
// {
//     if (!m_electionRepo)
//     {
//         return std::nullopt; // Repository not injected
//     }
//     return m_electionRepo->getElectionById(id);
// }
