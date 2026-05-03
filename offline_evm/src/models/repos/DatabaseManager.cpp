#include "DatabaseManager.h"

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

std::mutex& DatabaseManager::getMutex() { return m_mutex; }

bool DatabaseManager::init(const QString& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "DB Open Error:" << m_db.lastError().text();
        return false;
    }
    return createTables();
}

void DatabaseManager::close() {
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::createTables()
{
    QSqlQuery q;
    bool ok = true;

    if (!q.exec("CREATE TABLE IF NOT EXISTS Candidates (cnic TEXT PRIMARY KEY, name TEXT, "
                "party_name TEXT, symbol_name TEXT, symbol_b64 TEXT, profile_b64 TEXT)"))
        ok = false;

    if (!q.exec("CREATE TABLE IF NOT EXISTS AuditLogs (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "timestamp TEXT, event_type TEXT, description TEXT)"))
        ok = false;

    // [CHANGED] salt is now BLOB for Libsodium
    if (!q.exec("CREATE TABLE IF NOT EXISTS PollWorkers (username TEXT PRIMARY KEY, password_hash "
                "BLOB, salt BLOB)"))
        ok = false;

    // [CHANGED] current_state is INTEGER (for the Enum), added epoch timers and pub_key
    if (!q.exec("CREATE TABLE IF NOT EXISTS SystemConfig (id INTEGER PRIMARY KEY CHECK (id = 1), "
                "device_id TEXT, station_id TEXT, election_id TEXT, current_state INTEGER, "
                "pub_key_b64 TEXT, scheduled_start INTEGER, scheduled_end INTEGER, poll_opened_at "
                "TEXT, poll_closed_at TEXT)"))
        ok = false;

    if (!q.exec("CREATE TABLE IF NOT EXISTS UsedTokens (token_id TEXT PRIMARY KEY, used_at TEXT)"))
        ok = false;

    if (!q.exec("CREATE TABLE IF NOT EXISTS Votes (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "candidate_cnic TEXT, timestamp TEXT, current_hash BLOB, previous_hash BLOB)"))
        ok = false;

    if (!ok) {
        qCritical() << "Failed to create one or more tables:" << q.lastError().text();
    }
    return ok;
}
bool DatabaseManager::cleanupForNewElection() {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction()) return false;

    QSqlQuery q(db);
    bool ok = true;

    if(!q.exec("DELETE FROM Candidates")) ok = false;
    if(!q.exec("DELETE FROM UsedTokens")) ok = false;
    if(!q.exec("DELETE FROM Votes")) ok = false;

    if (ok && db.commit()) {
        return true;
    } else {
        db.rollback();
        return false;
    }
}

bool DatabaseManager::validateMasterPassword() {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery q;
    if (q.exec("SELECT count(*) FROM sqlite_master;")) {
        if (q.next()) return true;
    }
    return false;
}
