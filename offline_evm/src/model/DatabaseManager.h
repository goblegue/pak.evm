#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <mutex>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool init(const QString& dbPath);
    void close();

    // The Lead's special queries
    bool cleanupForNewElection();
    bool validateMasterPassword();

    // Global Mutex for thread safety across all repositories
    std::mutex& getMutex();

private:
    DatabaseManager() = default;
    ~DatabaseManager() { close(); }

    QSqlDatabase m_db;
    std::mutex m_mutex;
    bool createTables();
};

#endif
