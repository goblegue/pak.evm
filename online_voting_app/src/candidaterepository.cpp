#include "candidaterepository.h"
#include "databasemanager.h"

#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;

candidaterepository::candidaterepository() {
    m_collection = DatabaseManager::getInstance().getDatabase()["Candidates"];
}

bool candidaterepository::insertCandidate(const Candidate &candidate) {
    auto db = DatabaseManager::getInstance().getDatabase();

    auto electionFilter = document{}
                          << "id" << candidate.getElectionId().toStdString()
                          << "status" << static_cast<int>(ElectionState::Published)
                          << finalize;

    if (!db["Elections"].find_one(electionFilter.view())) {
        return false;
    }

    try {
        auto builder = document{};
        builder << "id" << candidate.getId().toStdString()
                << "userCnic" << candidate.getUserCnic().toStdString()
                << "electionId" << candidate.getElectionId().toStdString()
                << "partyName" << candidate.getPartyName().toStdString()
                << "symbolName" << candidate.getSymbolName().toStdString()
                << "status" << static_cast<int>(candidate.getStatus());

        m_collection.insert_one(builder << finalize);
        return true;
    } catch (...) {
        return false;
    }
}

Candidate* candidaterepository::getCandidatesByElection(const QString &electionId, int &candidatesSize) {
    auto filter = document{} << "electionId" << electionId.toStdString() << finalize;

    candidatesSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (candidatesSize == 0) return nullptr;

    Candidate* candidates = new Candidate[candidatesSize];

    auto cursor = m_collection.find(filter.view());
    int i = 0;
    for (auto&& doc : cursor) {
        candidates[i].setId(QString::fromStdString(doc["id"].get_string().value.to_string()));
        candidates[i].setUserCnic(QString::fromStdString(doc["userCnic"].get_string().value.to_string()));
        candidates[i].setElectionId(QString::fromStdString(doc["electionId"].get_string().value.to_string()));
        candidates[i].setPartyName(QString::fromStdString(doc["partyName"].get_string().value.to_string()));
        candidates[i].setSymbolName(QString::fromStdString(doc["symbolName"].get_string().value.to_string()));
        candidates[i].setStatus(static_cast<ApprovalStatus>(doc["status"].get_int32().value));
        i++;
    }

    return candidates;
}

bool candidaterepository::addStatusChangeRequest(const QString &targetCandidateCnic,
                                                 const QString &requestingAdminId,
                                                 const ApprovalStatus &status) {
    auto filter = document{} << "userCnic" << targetCandidateCnic.toStdString() << finalize;
    auto update = document{} << "$push" << open_document
                             << "statusChangeRequests" << open_document
                             << "requestById" << requestingAdminId.toStdString()
                             << "status" << static_cast<int>(status)
                             << close_document << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

