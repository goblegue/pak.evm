#include "DatabaseManager.h"
#include <QDebug>

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::init(const QString& dbPath, const QString& password) {

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "Cannot open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec(QString("PRAGMA key = '%1';").arg(password));

    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlQuery query;
    bool success = true;

    // Create tables using Qt's query executor
    if (!query.exec("CREATE TABLE IF NOT EXISTS Candidates (id TEXT PRIMARY KEY, name TEXT)")) success = false;
    if (!query.exec("CREATE TABLE IF NOT EXISTS UsedTokens (token_id TEXT PRIMARY KEY)")) success = false;
    if (!query.exec("CREATE TABLE IF NOT EXISTS Votes ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "candidate_id TEXT, "
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                    "current_hash TEXT)")) success = false;

    if (!success) {
        qDebug() << "Failed to create tables!";
    }
    return success;
}

void DatabaseManager::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}


bool DatabaseManager::insertCandidate(const Candidate& cand) { return true; }
bool DatabaseManager::isTokenUsed(const QString& tokenId) { return false; }
bool DatabaseManager::insertVoteRecord(const VoteRecord& vote) { return true; }
QString DatabaseManager::getPreviousHash() { return "00000000000000000000000000000000"; }
