#include "workerrepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>

bool WorkerRepository::insertWorker(const PollWorker &w)
{
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    q.prepare("INSERT INTO PollWorkers (username, password_hash, salt) VALUES (?, ?, ?)");
    q.addBindValue(w.getUsername());
    q.addBindValue(w.getPasswordHash());
    q.addBindValue(w.getSalt());
    return q.exec();
}

std::optional<PollWorker> WorkerRepository::getWorkerByUsername(const QString &username)
{
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    q.prepare("SELECT password_hash, salt FROM PollWorkers WHERE username = ?");
    q.addBindValue(username);

    if (q.exec() && q.next())
    {
        PollWorker w;
        w.setUsername(username);
        w.setPassword(q.value(0).toByteArray(), q.value(1).toByteArray());
        return w;
    }
    return std::nullopt;
}

int WorkerRepository::getAdminCount()
{
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q("SELECT COUNT(*) FROM PollWorkers");
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}
