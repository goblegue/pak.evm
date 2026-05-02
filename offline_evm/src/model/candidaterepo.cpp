#include "candidaterepo.h"
#include "DatabaseManager.h"

bool CandidateRepository::insertCandidate(const Candidate &c) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    q.prepare("INSERT INTO Candidates (cnic, name, party_name, symbol_name, symbol_b64, profile_b64) VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(c.getCnic()); q.addBindValue(c.getName()); q.addBindValue(c.getPartyName());
    q.addBindValue(c.getSymbolName()); q.addBindValue(c.getSymbolBase64()); q.addBindValue(c.getProfileImageBase64());
    return q.exec();
}

bool CandidateRepository::clearAllCandidates() {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q; return q.exec("DELETE FROM Candidates");
}

Candidate* CandidateRepository::getAllCandidates(int &size) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery countQ("SELECT COUNT(*) FROM Candidates");
    if(countQ.next()) size = countQ.value(0).toInt();
    if(size == 0) return nullptr;

    Candidate* arr = new Candidate[size]; // Note: Backend must delete[] this later!
    QSqlQuery q("SELECT cnic, name, party_name, symbol_name, symbol_b64, profile_b64 FROM Candidates");
    int i = 0;
    while(q.next()) {
        arr[i].setCnic(q.value(0).toString()); arr[i].setName(q.value(1).toString());
        arr[i].setPartyName(q.value(2).toString()); arr[i].setSymbolName(q.value(3).toString());
        arr[i].setSymbolBase64(q.value(4).toString()); arr[i].setProfileImageBase64(q.value(5).toString());
        i++;
    }
    return arr;
}
