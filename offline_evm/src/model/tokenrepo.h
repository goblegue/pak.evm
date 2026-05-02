#ifndef TOKENREPO_H
#define TOKENREPO_H

#include "../models/entities/tokens.h"

class TokenRepository : public ITokenRepository {
public:
    bool markTokenAsUsed(const Token &token) override;
    bool isTokenUsed(const QString &tokenId) override;
};

#endif
