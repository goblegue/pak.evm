#include "controllers/voterController.h"
#include "services/QR/qr.h"

TokenController::TokenController() : m_tokenRepo(nullptr), m_electionRepo(nullptr) {}

TokenController::~TokenController() {}

TokenController &TokenController::getInstance()
{
    static TokenController instance;
    return instance;
}

void TokenController::injectRepositories(ITokenRepository *tokenRepo, IElectionRepository *electionRepo)
{
    m_tokenRepo = tokenRepo;
    m_electionRepo = electionRepo;
}

bool TokenController::canRequestToken(const QString &userCnic, const QString &electionId)
{
    if (m_tokenRepo->hasUserRequestedToken(userCnic, electionId))
    {
        return false;
    }

    auto electionOpt = m_electionRepo->getElectionById(electionId);
    if (!electionOpt.has_value())
    {
        return false;
    }

    if (electionOpt->getStatus() != ElectionState::Published && electionOpt->getStatus() != ElectionState::VotingOpen)
    {
        return false;
    }

    return true;
}

optional<QImage> TokenController::requestVotingToken(const QString &userCnic, const QString &electionId, const QByteArray &privateKey)
{
    if(m_tokenRepo==nullptr || m_electionRepo==nullptr){
        return nullopt;
    }
    if (!canRequestToken(userCnic, electionId))
    {
        return nullopt;
    }

    Token newToken;
    newToken.setUserCnic(userCnic);
    newToken.setElectionId(electionId);
    newToken.setIssuedAt(QDateTime::currentDateTime());

    auto signatureOpt = Token::generateSignature(userCnic, electionId, newToken.getIssuedAt(), privateKey);
    if (!signatureOpt.has_value())
    {
        return nullopt;
    }
    newToken.setTokenSignature(signatureOpt.value());

    if (!m_tokenRepo->insertToken(newToken))
    {
        return nullopt;
    }

    // Generate QR code for the token
    QString payload = QR::preparePayload(newToken);
    QImage qrCode = QR::generateQRCode(payload);
    return qrCode;
}

Token *TokenController::getVoterTokens(const QString &userCnic, int &tokensSize)
{
    if(m_tokenRepo==nullptr){
        tokensSize=0;
        return nullptr;
    }
    return m_tokenRepo->getTokensByUser(userCnic, tokensSize);
}

optional<QImage> TokenController::getQrCodeForToken(const Token &token)
{
    QString payload = QR::preparePayload(token);
    QImage qrCode = QR::generateQRCode(payload);
    return qrCode;
}

optional<int> TokenController::getTokenCountForElection(const QString &electionId)
{
    if(m_tokenRepo==nullptr){
        return nullopt;
    }
    int count{};
    int dummySize{};
    Token *tokens = m_tokenRepo->getTokensByElection(electionId, dummySize);
    if (tokens != nullptr)
    {
        count = dummySize;
        delete[] tokens;
    }
    return count;
}


        