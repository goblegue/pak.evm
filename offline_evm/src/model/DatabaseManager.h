#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


struct Candidate { QString id; QString name; };
struct VoteRecord { QString candidate_id; QString token_id; QString current_hash; };

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool init(const QString& dbPath, const QString& password);
    void close();

    // MVP CRUD Operations
    bool insertCandidate(const Candidate& cand);
    bool isTokenUsed(const QString& tokenId);
    bool insertVoteRecord(const VoteRecord& vote);
    QString getPreviousHash();

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    QSqlDatabase m_db;

    bool createTables();
};

#endif
