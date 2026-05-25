#include "models/repositories/electionrepository.h"

#include "models/databasemanager.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <cstdint>
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

electionrepository::electionrepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Elections"];
}

bool electionrepository::insertElection(const Election &election)
{
    try
    {
        auto builder = document{};
        builder << "election_id" << election.getId().toStdString()
                << "title" << election.getTitle().toStdString()
                << "publishTime" << static_cast<int64_t>(election.getPublishTime().toMSecsSinceEpoch())
                << "startTime" << static_cast<int64_t>(election.getStartTime().toMSecsSinceEpoch())
                << "endTime" << static_cast<int64_t>(election.getEndTime().toMSecsSinceEpoch())
                // FIXED: Force MongoDB to save this strictly as a 32-bit int
                << "status" << bsoncxx::types::b_int32{static_cast<int32_t>(election.getStatus())};

        m_collection.insert_one(builder << finalize);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool electionrepository::updateElectionState(const QString &electionId, ElectionState newState)
{
    auto filter = document{} << "election_id" << electionId.toStdString() << finalize;
    // FIXED: Force MongoDB to save this strictly as a 32-bit int
    auto update = document{} << "$set" << open_document
                             << "status" << bsoncxx::types::b_int32{static_cast<int32_t>(newState)}
                             << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

bool electionrepository::addStatusChangeRequest(const QString &targetElectionId, const QString &requestingAdminId, const ApprovalStatus status)
{
    auto filter = document{} << "election_id" << targetElectionId.toStdString() << finalize;
    auto update = document{} << "$push" << open_document
                             << "statusChangeRequests" << open_document
                             << "requestById" << requestingAdminId.toStdString()
                             // FIXED: Force MongoDB to save this strictly as a 32-bit int
                             << "status" << bsoncxx::types::b_int32{static_cast<int32_t>(status)}
                             << close_document << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

std::optional<Election> electionrepository::getElectionById(const QString &id)
{
    auto filter = document{} << "election_id" << id.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());
    if (!result)
        return std::nullopt;

    auto view = result->view();
    Election election;
    election.setId(QString::fromUtf8(view["election_id"].get_string().value.data()));
    election.setTitle(QString::fromUtf8(view["title"].get_string().value.data()));
    election.setStartTime(QDateTime::fromMSecsSinceEpoch(view["startTime"].get_int64().value));
    election.setEndTime(QDateTime::fromMSecsSinceEpoch(view["endTime"].get_int64().value));
    election.setPublishTime(QDateTime::fromMSecsSinceEpoch(view["publishTime"].get_int64().value));

    // --- SAFE READER FIX FOR ELECTION STATUS ---
    int32_t statusVal = 0;
    if (view["status"].type() == bsoncxx::type::k_int32) {
        statusVal = view["status"].get_int32().value;
    } else if (view["status"].type() == bsoncxx::type::k_int64) {
        statusVal = static_cast<int32_t>(view["status"].get_int64().value);
    } else if (view["status"].type() == bsoncxx::type::k_double) {
        statusVal = static_cast<int32_t>(view["status"].get_double().value);
    }
    election.setStatus(static_cast<ElectionState>(statusVal));
    // -------------------------------------------

    if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
    {
        for (auto &&reqDoc : view["statusChangeRequests"].get_array().value)
        {
            auto req = reqDoc.get_document().view();

            // --- SAFE READER FIX FOR APPROVAL STATUS ---
            int32_t reqStatusVal = 0;
            if (req["status"].type() == bsoncxx::type::k_int32) {
                reqStatusVal = req["status"].get_int32().value;
            } else if (req["status"].type() == bsoncxx::type::k_int64) {
                reqStatusVal = static_cast<int32_t>(req["status"].get_int64().value);
            } else if (req["status"].type() == bsoncxx::type::k_double) {
                reqStatusVal = static_cast<int32_t>(req["status"].get_double().value);
            }

            election.addStatusChangeRequest(QString::fromUtf8(
                                                req["requestById"].get_string().value.data()),
                                            static_cast<ApprovalStatus>(reqStatusVal));
        }
    }
    return election;
}

Election *electionrepository::getAllElections(int &electionsSize)
{
    int count = static_cast<int>(m_collection.count_documents({}));
    electionsSize = count;
    if (count == 0)
        return nullptr;

    Election *electionArray = new Election[count];

    auto cursor = m_collection.find({});
    int i = 0;
    for (auto &&doc : cursor)
    {
        auto view = doc;
        electionArray[i].setId(QString::fromUtf8(doc["election_id"].get_string().value.data()));
        electionArray[i].setTitle(QString::fromUtf8(doc["title"].get_string().value.data()));
        electionArray[i].setPublishTime(QDateTime::fromMSecsSinceEpoch(doc["publishTime"].get_int64().value));
        electionArray[i].setStartTime(QDateTime::fromMSecsSinceEpoch(doc["startTime"].get_int64().value));
        electionArray[i].setEndTime(QDateTime::fromMSecsSinceEpoch(doc["endTime"].get_int64().value));

        if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
        {
            auto requestsArray = view["statusChangeRequests"].get_array().value;
            for (auto &&reqDoc : requestsArray)
            {
                auto reqView = reqDoc.get_document().view();
                QString requesterId = QString::fromUtf8(
                    reqView["requestById"].get_string().value.data());

                // --- SAFE READER FIX FOR APPROVAL STATUS ---
                int32_t reqStatusVal = 0;
                if (reqView["status"].type() == bsoncxx::type::k_int32) {
                    reqStatusVal = reqView["status"].get_int32().value;
                } else if (reqView["status"].type() == bsoncxx::type::k_int64) {
                    reqStatusVal = static_cast<int32_t>(reqView["status"].get_int64().value);
                } else if (reqView["status"].type() == bsoncxx::type::k_double) {
                    reqStatusVal = static_cast<int32_t>(reqView["status"].get_double().value);
                }

                ApprovalStatus reqStatus = static_cast<ApprovalStatus>(reqStatusVal);
                electionArray[i].addStatusChangeRequest(requesterId, reqStatus);
            }
        }

        // --- SAFE READER FIX FOR ELECTION STATUS ---
        int32_t statusVal = 0;
        if (doc["status"].type() == bsoncxx::type::k_int32) {
            statusVal = doc["status"].get_int32().value;
        } else if (doc["status"].type() == bsoncxx::type::k_int64) {
            statusVal = static_cast<int32_t>(doc["status"].get_int64().value);
        } else if (doc["status"].type() == bsoncxx::type::k_double) {
            statusVal = static_cast<int32_t>(doc["status"].get_double().value);
        }
        electionArray[i].setStatus(static_cast<ElectionState>(statusVal));
        // -------------------------------------------

        i++;
    }

    return electionArray;
}
