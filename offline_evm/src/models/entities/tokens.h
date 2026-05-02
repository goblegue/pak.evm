
#ifndef TOKEN_H
#define TOKEN_H

#include <QString>
#include <QDateTime>
#include <cstddef>
class Token
{
private:
    QString m_tokenId;
    QDateTime m_usedAt;


public:
    Token() {}

    QString getTokenId() const { return m_tokenId; }
    QDateTime getUsedAt() const { return m_usedAt; }

    void setTokenId(const QString &id) { m_tokenId = id; }
    void setUsedAt(const QDateTime &time) { m_usedAt = time; }
};

class ITokenRepository
{
public:
    virtual ~ITokenRepository() = default;
    virtual bool markTokenAsUsed(const Token &token) = 0;
    virtual bool isTokenUsed(const QString &tokenId) = 0; 
    
};

#endif // TOKEN_H
