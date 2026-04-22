#include "voterrepository.h"

#include "databasemanager.h"

#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

voterrepository::voterrepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Voters"];
}

bool voterrepository::insertToken(const Voters &token)
{
    auto db = DatabaseManager::getInstance().getDatabase();

    auto electionFilter = document{}
                          << "id" << token.getElectionId().toStdString()
                          << "status" << open_document
                            << $in << open_array
                                << static_cast<int>(ElectionState::Published)
                                << static_cast<int>(ElectionState::Voting)
                            << close_array
                          << close_document
                          << finalize;

    if (!db["Elections"].find_one(electionFilter.view()))
    {
        return false;
    }

    if (hasUserRequestedToken(token.getUserCnic(), token.getElectionId()))
    {
        return false;
    }

    try
    {
        auto builder = document{};
        builder << "id" << token.getId().toStdString()
                << "userCnic" << token.getUserCnic().toStdString()
                << "electionId" << token.getElectionId().toStdString()
                << "assignedStationId" << token.getAssignedStationId().toStdString()
                << "tokenSignature" << token.getTokenSignature().toStdString()
                << "issuedAt" << static_cast<int64_t>(token.getIssuedAt().toMSecsSinceEpoch());

        m_collection.insert_one(builder << finalize);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool voterrepository::hasUserRequestedToken(const QString &cnic, const QString &electionId)
{
    auto filter = document{} << "userCnic" << cnic.toStdString()
                             << "electionId" << electionId.toStdString()
                             << finalize;
    auto result = m_collection.find_one(filter.view());
    return static_cast<bool>(result);
}

Voters *voterrepository::getTokensByElection(const QString &electionId, int &votersSize)
{
    auto filter = document{} << "electionId" << electionId.toStdString() << finalize;

    votersSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (votersSize == 0)
        return nullptr;

    Voters *tokens = new Voters[votersSize];
    auto cursor = m_collection.find(filter.view());

    int i = 0;
    for (auto &&doc : cursor)
    {

        tokens[i].setId(QString::fromStdString(doc["id"].get_string().value.to_string()));
        tokens[i].setUserCnic(QString::fromStdString(doc["userCnic"].get_string().value.to_string()));
        tokens[i].setElectionId(QString::fromStdString(doc["electionId"].get_string().value.to_string()));
        tokens[i].setAssignedStationId(QString::fromStdString(doc["assignedStationId"].get_string().value.to_string()));
        tokens[i].setTokenSignature(QString::fromStdString(doc["tokenSignature"].get_string().value.to_string()));
        tokens[i].setIssuedAt(QDateTime::fromMSecsSinceEpoch(doc["issuedAt"].get_int64().value));
        i++;
    }
    return tokens;
}
