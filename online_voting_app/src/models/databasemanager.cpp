#include "models/databasemanager.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/options/index.hpp>
#include <chrono>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

DatabaseManager::DatabaseManager() : client{mongocxx::uri{}}, db{client["online_voting_system"]} {}

DatabaseManager &DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::setupSchema()
{
    auto index_options = mongocxx::options::index{};
    index_options.expire_after(std::chrono::seconds(600));

    db["Users"].create_index(
        document{} << "cnic" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["Users"].create_index(
        document{} << "email" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["Tokens"].create_index(
        document{} << "user_cnic" << 1 << "election_id" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["otp"].create_index(
        document{} << "expires_at" << 1 << finalize,
        index_options);

    db["Elections"].create_index(
        document{} << "election_id" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["Voting_Stations"].create_index(
        document{} << "stationName" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["Admins"].create_index(
        document{} << "email" << 1 << finalize,
        mongocxx::options::index{}.unique(true));
    db["Admins"].create_index(
        document{} << "cnic" << 1 << finalize,
        mongocxx::options::index{}.unique(true));

    db["Candidates"].create_index(
        document{} << "candidate_id" << 1 << finalize,
        mongocxx::options::index{}.unique(true));
}
