#include "tokenrepo.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>

bool TokenRepository::markTokenAsUsed(const Token &t) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    q.prepare("INSERT INTO UsedTokens (token_id, used_at) VALUES (?, ?)");
    q.addBindValue(t.getTokenId());
    q.addBindValue(t.getUsedAt().toString(Qt::ISODate));
    return q.exec();
}

bool TokenRepository::isTokenUsed(const QString &tokenId) {
    std::lock_guard<std::mutex> lock(DatabaseManager::instance().getMutex());
    QSqlQuery q;
    q.prepare("SELECT 1 FROM UsedTokens WHERE token_id = ?");
    q.addBindValue(tokenId);
    return (q.exec() && q.next());
}
