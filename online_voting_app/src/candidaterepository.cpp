#include "candidaterepository.h"
#include "databasemanager.h"

#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

candidaterepository::candidaterepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Candidates"];
}

bool candidaterepository::insertCandidate(const Candidate &candidate)
{
    auto db = DatabaseManager::getInstance().getDatabase();

    auto electionFilter = document{} << "id" << candidate.getElectionId().toStdString() << "status"
                                     << static_cast<int>(ElectionState::Draft) << finalize;

    if (!db["Elections"].find_one(electionFilter.view()))
    {
        return false;
    }

    try
    {
        auto builder = document{};
        builder << "id" << candidate.getId().toStdString()
                << "candidateCnic" << candidate.getUserCnic().toStdString()
                << "candidateName" << candidate.getName().toStdString()
                << "electionId" << candidate.getElectionId().toStdString()
                << "partyName" << candidate.getPartyName().toStdString()
                << "symbolName" << candidate.getSymbolName().toStdString()
                << "status" << static_cast<int>(candidate.getStatus());

        m_collection.insert_one(builder << finalize);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Candidate *candidaterepository::getCandidates(int &candidatesSize, const QString &electionId)
{
    auto filter = document{} << "electionId" << electionId.toStdString() << finalize;

    candidatesSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (candidatesSize == 0)
        return nullptr;

    Candidate *candidates = new Candidate[candidatesSize];

    auto cursor = m_collection.find(filter.view());
    int i = 0;
    for (auto &&doc : cursor)
    {
        candidates[i].setId(QString::fromUtf8(doc["id"].get_string().value.data()));
        candidates[i].setUserCnic(QString::fromUtf8(doc["candidateCnic"].get_string().value.data()));
        candidates[i].setElectionId(QString::fromUtf8(doc["electionId"].get_string().value.data()));
        candidates[i].setPartyName(QString::fromUtf8(doc["partyName"].get_string().value.data()));
        candidates[i].setSymbolName(QString::fromUtf8(doc["symbolName"].get_string().value.data()));
        candidates[i].setName(QString::fromUtf8(doc["candidateName"].get_string().value.data()));

        // For the Status line, ensure you use get_int32() correctly
        candidates[i].setStatus(static_cast<ApprovalStatus>(doc["status"].get_int32().value));
        i++;
    }

    return candidates;
}

bool candidaterepository::addStatusChangeRequest(const QString &targetCandidateCnic,
                                                 const QString &requestingAdminId,
                                                 const ApprovalStatus &status)
{
    auto filter = document{} << "candidateCnic" << targetCandidateCnic.toStdString() << finalize;
    auto update = document{} << "$push" << open_document
                             << "statusChangeRequests" << open_document
                             << "requestById" << requestingAdminId.toStdString()
                             << "status" << static_cast<int>(status)
                             << close_document << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

Candidate *candidaterepository::getCandidatesByStatus(int &candidatesSize, const QString &electionId, ApprovalStatus status)
{
    // For now, return nullptr so you can at least compile and test
    candidatesSize = 0;
    return nullptr;
}

bool candidaterepository::updateCandidateStatus(const QString &candidateCnic, ApprovalStatus newStatus)
{
    // Write the MongoDB update logic here later
    return true;
}
