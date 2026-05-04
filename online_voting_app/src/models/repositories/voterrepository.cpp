#include "models/repositories/voterrepository.h"

#include "models/databasemanager.h"

#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

TokenRepository::TokenRepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Token"];
}

bool TokenRepository::insertToken(const Token &token)
{
    auto db = DatabaseManager::getInstance().getDatabase();

    auto electionFilter = document{} << "election_id" << token.getElectionId().toStdString() << "status"
                                     << open_document << "$in" << open_array
                                     << static_cast<int>(ElectionState::Published)
                                     << static_cast<int>(ElectionState::VotingOpen) << close_array
                                     << close_document << finalize;

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
                << "user_cnic" << token.getUserCnic().toStdString()
                << "election_id" << token.getElectionId().toStdString()
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

bool TokenRepository::hasUserRequestedToken(const QString &cnic, const QString &electionId)
{
    auto filter = document{} << "user_cnic" << cnic.toStdString()
                             << "election_id" << electionId.toStdString()
                             << finalize;
    auto result = m_collection.find_one(filter.view());
    return static_cast<bool>(result);
}

Token *TokenRepository::getTokensByElection(const QString &electionId, int &votersSize)
{
    auto filter = document{} << "election_id" << electionId.toStdString() << finalize;

    votersSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (votersSize == 0)
        return nullptr;

    Token *tokens = new Token[votersSize];
    auto cursor = m_collection.find(filter.view());

    int i = 0;
    for (auto &&doc : cursor)
    {
        tokens[i].setId(QString::fromStdString(doc["id"].get_string().value.data()));
        tokens[i].setUserCnic(QString::fromStdString(doc["user_cnic"].get_string().value.data()));
        tokens[i].setElectionId(QString::fromStdString(doc["election_id"].get_string().value.data()));
        tokens[i].setAssignedStationId(
            QString::fromStdString(doc["assignedStationId"].get_string().value.data()));
        tokens[i].setTokenSignature(
            QString::fromStdString(doc["tokenSignature"].get_string().value.data()));
        tokens[i].setIssuedAt(QDateTime::fromMSecsSinceEpoch(doc["issuedAt"].get_int64().value));
        i++;
    }
    return tokens;
}

Token *TokenRepository::getTokensByUser(const QString &userCnic, int &tokensSize)
{
    auto filter = document{} << "user_cnic" << userCnic.toStdString() << finalize;

    tokensSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (tokensSize == 0)
        return nullptr;

    Token *tokens = new Token[tokensSize];
    auto cursor = m_collection.find(filter.view());

    int i = 0;
    for (auto &&doc : cursor)
    {
        tokens[i].setId(QString::fromStdString(doc["id"].get_string().value.data()));
        tokens[i].setUserCnic(QString::fromStdString(doc["user_cnic"].get_string().value.data()));
        tokens[i].setElectionId(QString::fromStdString(doc["election_id"].get_string().value.data()));
        tokens[i].setAssignedStationId(
            QString::fromStdString(doc["assignedStationId"].get_string().value.data()));
        tokens[i].setTokenSignature(
            QString::fromStdString(doc["tokenSignature"].get_string().value.data()));
        tokens[i].setIssuedAt(QDateTime::fromMSecsSinceEpoch(doc["issuedAt"].get_int64().value));
        i++;
    }
    return tokens;
}
