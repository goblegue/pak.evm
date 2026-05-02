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
        builder << "id" << election.getId().toStdString()
                << "title" << election.getTitle().toStdString()
                << "publishTime" << static_cast<int64_t>(election.getPublishTime().toMSecsSinceEpoch())
                << "startTime" << static_cast<int64_t>(election.getStartTime().toMSecsSinceEpoch())
                << "endTime" << static_cast<int64_t>(election.getEndTime().toMSecsSinceEpoch())
                << "status" << static_cast<int>(election.getStatus());

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
    auto filter = document{} << "id" << electionId.toStdString() << finalize;
    auto update = document{} << "$set" << open_document << "status" << static_cast<int>(newState) << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

bool electionrepository::addStatusChangeRequest(const QString &targetElectionId, const QString &requestingAdminId, const ApprovalStatus status)
{
    auto filter = document{} << "id" << targetElectionId.toStdString() << finalize;
    auto update = document{} << "$push" << open_document
                             << "statusChangeRequests" << open_document
                             << "requestById" << requestingAdminId.toStdString()
                             << "status" << static_cast<int>(status)
                             << close_document << close_document << finalize;

    auto result = m_collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
}

std::optional<Election> electionrepository::getElectionById(const QString &id)
{
    auto filter = document{} << "id" << id.toStdString() << finalize;
    auto result = m_collection.find_one(filter.view());
    if (!result)
        return std::nullopt;

    auto view = result->view();
    Election election;
    election.setId(QString::fromUtf8(view["id"].get_string().value.data()));
    election.setTitle(QString::fromUtf8(view["title"].get_string().value.data()));
    election.setStartTime(QDateTime::fromMSecsSinceEpoch(view["startTime"].get_int64().value));
    election.setEndTime(QDateTime::fromMSecsSinceEpoch(view["endTime"].get_int64().value));
    election.setPublishTime(QDateTime::fromMSecsSinceEpoch(view["publishTime"].get_int64().value));
    election.setStatus(static_cast<ElectionState>(view["status"].get_int32().value));

    if (view["statusChangeRequests"] && view["statusChangeRequests"].type() == bsoncxx::type::k_array)
    {
        for (auto &&doc : view["statusChangeRequests"].get_array().value)
        {
            auto req = doc.get_document().view();
            election.addStatusChangeRequest(QString::fromUtf8(
                                                req["requestById"].get_string().value.data()),
                                            static_cast<ApprovalStatus>(
                                                req["status"].get_int32().value));
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
        electionArray[i].setId(QString::fromUtf8(doc["id"].get_string().value.data()));
        electionArray[i].setTitle(QString::fromUtf8(doc["title"].get_string().value.data()));
        electionArray[i].setPublishTime(QDateTime::fromMSecsSinceEpoch(doc["publishTime"].get_int64().value));
        electionArray[i].setStartTime(QDateTime::fromMSecsSinceEpoch(doc["startTime"].get_int64().value));
        electionArray[i].setEndTime(QDateTime::fromMSecsSinceEpoch(doc["endTime"].get_int64().value));
        electionArray[i].setStatus(static_cast<ElectionState>(doc["status"].get_int32().value));
        i++;
    }

    return electionArray;
}
