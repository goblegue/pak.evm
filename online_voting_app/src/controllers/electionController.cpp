#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>


#include "controllers/electionController.h"
#include "controllers/candidateController.h"
#include "services/crypto/cryptoengine.h"
#include "services/email/emailservice.h"
#include <optional>

using namespace std;

const int ELECTION_APPROVAL_THRESHOLD = 2; // More than 50% of admins must approve
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

    int totalAdmins = m_adminRepo->getAdminCount();
    if (approvedCount > (totalAdmins / ELECTION_APPROVAL_THRESHOLD))
    {
        return m_electionRepo->updateElectionState(electionId, ElectionState::Published);
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

 bool ElectionController::sendElectionDataToAdmin(const QString &electionId, const QString &adminId, const QByteArray &privateKey)
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
    if(/*!(election.getStatus() == ElectionState::Published)||*/
       !(election.getStatus() == ElectionState::VotingOpen))
    {
        return false; // Election not active
    }
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

    QJsonObject electionJson;
    electionJson["id"] = election.getId();
    electionJson["title"] = election.getTitle();
    electionJson["startTime"] = election.getStartTime().toString(Qt::ISODate);
    electionJson["endTime"] = election.getEndTime().toString(Qt::ISODate);

    

    QString candidatesJsonString = CandidateController::getInstance().getCandidatesJsonByElection(electionId);

    QByteArray candidateBytes = candidatesJsonString.toUtf8();

    QJsonDocument candidateJsonDoc = QJsonDocument::fromJson(candidateBytes);

    if (candidateJsonDoc.isNull() && !candidateJsonDoc.isArray()){
        return false;
    }
    QJsonArray candidateJsonArray = candidateJsonDoc.array();

    QJsonObject payload;
    payload["election"] = electionJson;
    payload["candidates"] = candidateJsonArray;
    payload["exportTimeStamp"]= QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(payload);

    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    auto signatureOpt = CryptoEngine::getInstance().signMessage(jsonData, privateKey);

    if (!signatureOpt.has_value())
    {
        return false;
    }

    QByteArray signature = signatureOpt.value();

    payload["signature"] = QString(signature.toBase64());

    doc.setObject(payload);
    QString fileName = QString("election_%1_data.json").arg(electionId);
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    jsonData = doc.toJson(QJsonDocument::Indented);

    file.write(jsonData);
    file.close();

    QString adminEmail = admin.getEmail();
    QString subject = QString("Election Data for %1").arg(election.getTitle());
    QString body= QString("Hello %1,\n\nPlease find attached the requested election data in json formate below\n\nElection Commission\n\nDo not replay to this mail" )
    .arg(admin.getName());

    bool emailSuccess = EmailService::getInstance().sendEmail(adminEmail,subject,body,false,fileName);

    file.remove();

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
