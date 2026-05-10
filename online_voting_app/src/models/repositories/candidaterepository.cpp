#include "models/repositories/candidaterepository.h"
#include "models/databasemanager.h"

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

    auto electionFilter = document{} << "election_id" << candidate.getElectionId().toStdString() << "status"
                                     << static_cast<int>(ElectionState::Drafted) << finalize;

    if (!db["Elections"].find_one(electionFilter.view()))
    {
        return false;
    }

    try
    {
        auto builder = document{};
        builder << "candidate_id" << candidate.getId().toStdString() << "userCnic"
                << candidate.getUserCnic().toStdString() << "name"
                << candidate.getName().toStdString() << "election_id"
                << candidate.getElectionId().toStdString() << "partyName"
                << candidate.getPartyName().toStdString() << "educationLevel"
                << candidate.getEducationLevel().toStdString() << "symbolName"
                << candidate.getSymbolName().toStdString() << "symbolBase64"
                << candidate.getSymbolBase64().toStdString() << "profileImageBase64"
                << candidate.getProfileImageBase64().toStdString() << "manifesto"
                << candidate.getManifesto().toStdString() << "status"
                << static_cast<int>(candidate.getStatus());

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
    auto filter = document{} << "election_id" << electionId.toStdString() << finalize;

    candidatesSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (candidatesSize == 0)
        return nullptr;

    Candidate *candidates = new Candidate[candidatesSize];

    auto cursor = m_collection.find(filter.view());
    int i = 0;
    for (auto &&doc : cursor)
    {
        auto view = doc;
        candidates[i].setId(QString::fromUtf8(doc["candidate_id"].get_string().value.data()));
        candidates[i].setName(QString::fromUtf8(doc["name"].get_string().value.data()));
        candidates[i].setUserCnic(QString::fromUtf8(doc["userCnic"].get_string().value.data()));
        candidates[i].setElectionId(QString::fromUtf8(doc["election_id"].get_string().value.data()));
        candidates[i].setEducationLevel(QString::fromUtf8(doc["educationLevel"].get_string().value.data()));
        candidates[i].setManifesto(QString::fromUtf8(doc["manifesto"].get_string().value.data()));
        candidates[i].setPartyName(QString::fromUtf8(doc["partyName"].get_string().value.data()));
        candidates[i].setSymbolBase64(QString::fromUtf8(doc["symbolBase64"].get_string().value.data()));
        candidates[i].setProfileImageBase64(QString::fromUtf8(doc["profileImageBase64"].get_string().value.data()));
        candidates[i].setSymbolName(QString::fromUtf8(doc["symbolName"].get_string().value.data()));
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(
                    reqView["status"].get_int32().value);
                candidates[i].addStatusChangeRequest(requesterId, reqStatus);
            }
        }
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
    auto filter = document{} << "userCnic" << targetCandidateCnic.toStdString() << finalize;
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
    auto filter = document{} << "election_id" << electionId.toStdString() << "status"
                             << static_cast<int>(status) << finalize;

    candidatesSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (candidatesSize == 0)
        return nullptr;

    Candidate *candidates = new Candidate[candidatesSize];

    auto cursor = m_collection.find(filter.view());
    int i = 0;
    for (auto &&doc : cursor)
    {
        auto view = doc;
        candidates[i].setId(QString::fromUtf8(doc["candidate_id"].get_string().value.data()));
        candidates[i].setName(QString::fromUtf8(doc["name"].get_string().value.data()));
        candidates[i].setUserCnic(QString::fromUtf8(doc["userCnic"].get_string().value.data()));
        candidates[i].setElectionId(QString::fromUtf8(doc["election_id"].get_string().value.data()));
        candidates[i].setEducationLevel(QString::fromUtf8(doc["educationLevel"].get_string().value.data()));
        candidates[i].setManifesto(QString::fromUtf8(doc["manifesto"].get_string().value.data()));
        candidates[i].setPartyName(QString::fromUtf8(doc["partyName"].get_string().value.data()));
        candidates[i].setSymbolBase64(QString::fromUtf8(doc["symbolBase64"].get_string().value.data()));
        candidates[i].setProfileImageBase64(QString::fromUtf8(doc["profileImageBase64"].get_string().value.data()));
        candidates[i].setSymbolName(QString::fromUtf8(doc["symbolName"].get_string().value.data()));
        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            for (auto &&doc : requestsArray)
            {
                auto reqView = doc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());
                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(
                    reqView["status"].get_int32().value);
                candidates[i].addStatusChangeRequest(requesterId, reqStatus);
            }
        }
        // For the Status line, ensure you use get_int32() correctly
        candidates[i].setStatus(static_cast<ApprovalStatus>(doc["status"].get_int32().value));
        i++;
    }

    return candidates;
}

bool candidaterepository::updateCandidateStatus(const QString &candidateCnic, ApprovalStatus newStatus)
{
    auto filter = document{} << "userCnic" << candidateCnic.toStdString() << finalize;
    auto update = document{} << "$set" << open_document << "status" << static_cast<int>(newStatus) << close_document << finalize;
    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

std::optional<Candidate> candidaterepository::getCandidateByCnic(const QString &cnic)
{
    auto filter = document{} << "userCnic" << cnic.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());
    if (!result)
        return std::nullopt;

    auto doc = result->view();
    Candidate candidate;
    candidate.setId(QString::fromUtf8(doc["candidate_id"].get_string().value.data()));
    candidate.setName(QString::fromUtf8(doc["name"].get_string().value.data()));
    candidate.setUserCnic(QString::fromUtf8(doc["userCnic"].get_string().value.data()));
    candidate.setElectionId(QString::fromUtf8(doc["election_id"].get_string().value.data()));
    candidate.setEducationLevel(QString::fromUtf8(doc["educationLevel"].get_string().value.data()));
    candidate.setManifesto(QString::fromUtf8(doc["manifesto"].get_string().value.data()));
    candidate.setPartyName(QString::fromUtf8(doc["partyName"].get_string().value.data()));
    candidate.setSymbolBase64(QString::fromUtf8(doc["symbolBase64"].get_string().value.data()));
    candidate.setProfileImageBase64(QString::fromUtf8(doc["profileImageBase64"].get_string().value.data()));
    candidate.setSymbolName(QString::fromUtf8(doc["symbolName"].get_string().value.data()));
    if (doc["statusChangeRequests"] && doc["statusChangeRequests"].type() == bsoncxx::type::k_array)
    {
        auto requestsArray = doc["statusChangeRequests"].get_array().value;
        for (auto &&doc : requestsArray)
        {
            auto reqView = doc.get_document().view();
            QString requesterId = QString::fromUtf8(
                reqView["requestById"].get_string().value.data());
            ApprovalStatus reqStatus = static_cast<ApprovalStatus>(
                reqView["status"].get_int32().value);
            candidate.addStatusChangeRequest(requesterId, reqStatus);
        }
    }
    // For the Status line, ensure you use get_int32() correctly
    candidate.setStatus(static_cast<ApprovalStatus>(doc["status"].get_int32().value));

    return candidate;
}
