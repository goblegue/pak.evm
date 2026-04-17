#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>

class DatabaseManager
{
public:
    static DatabaseManager &getInstance();
    mongocxx::database &getDatabase() { return db; }
    // Collections Setup
    void setupSchema();

private:
    DatabaseManager();
    mongocxx::instance inst{};
    mongocxx::client client;
    mongocxx::database db;
};

#endif // DATABASEMANAGER_H
