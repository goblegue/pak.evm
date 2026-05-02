#include "voterepository.h"
#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool VoteRepository::insertVoteTransaction(const VoteRecord &vote, const Token &token) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction()) {
        qCritical() << "Failed to start database transaction!";
        return false;
    }

    QSqlQuery voteQuery(db);
    voteQuery.prepare("INSERT INTO Votes (candidate_cnic, timestamp, current_hash, previous_hash) "
                      "VALUES (:cnic, :ts, :hash, :phash)");
    voteQuery.bindValue(":cnic", vote.getCandidateCnic());
    voteQuery.bindValue(":ts", vote.getTimestamp());
    voteQuery.bindValue(":hash", vote.getCurrentHash());
    voteQuery.bindValue(":phash", vote.getPreviousHash());

    if (!voteQuery.exec()) {
        qCritical() << "Vote insertion failed. Rolling back.";
        db.rollback();
        return false;
    }

    QSqlQuery tokenQuery(db);
    tokenQuery.prepare("INSERT INTO UsedTokens (token_id, used_at) VALUES (:tid, :ts)");
    tokenQuery.bindValue(":tid", token.getTokenId());
    tokenQuery.bindValue(":ts", token.getUsedAt().toString(Qt::ISODate));

    if (!tokenQuery.exec()) {
        qCritical() << "Token burning failed. Rolling back.";
        db.rollback();
        return false;
    }

    if (db.commit()) {
        return true;
    } else {
        db.rollback();
        return false;
    }
}

bool VoteRepository::insertVote(const VoteRecord &v) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("INSERT INTO Votes (candidate_cnic, timestamp, current_hash, previous_hash) VALUES (?, ?, ?, ?)");
    q.addBindValue(v.getCandidateCnic()); q.addBindValue(v.getTimestamp());
    q.addBindValue(v.getCurrentHash()); q.addBindValue(v.getCurrentHash());

    if (!q.exec()) {
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    return true;
}

QByteArray VoteRepository::getLatestVoteHash() {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q("SELECT current_hash FROM Votes ORDER BY id DESC LIMIT 1");
    if (q.next()) return q.value(0).toByteArray();
    return QByteArray();
}

VoteRecord* VoteRepository::getAllVotesForAudit(int &size) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery countQ("SELECT COUNT(*) FROM Votes");
    if(countQ.next()) size = countQ.value(0).toInt();
    if(size == 0) return nullptr;

    VoteRecord* arr = new VoteRecord[size];
    QSqlQuery q("SELECT id, candidate_cnic, timestamp, current_hash FROM Votes ORDER BY id ASC");
    int i = 0;
    while(q.next()) {
        arr[i].setId(q.value(0).toInt()); arr[i].setCandidateCnic(q.value(1).toString());
        arr[i].setTimestamp(q.value(2).toString()); arr[i].setCurrentHash(q.value(3).toByteArray());
        i++;
    }
    return arr;
}

candidateVotes* VoteRepository::getElectionTally(int &size) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery countQ("SELECT COUNT(DISTINCT candidate_cnic) FROM Votes");
    if(countQ.next()) size = countQ.value(0).toInt();
    if(size == 0) return nullptr;

    candidateVotes* arr = new candidateVotes[size];
    QSqlQuery q("SELECT candidate_cnic, COUNT(*) FROM Votes GROUP BY candidate_cnic");
    int i = 0;
    while(q.next()) {
        arr[i].candidateCnic = q.value(0).toString();
        arr[i].totalVotes = q.value(1).toInt();
        i++;
    }
    return arr;
}
