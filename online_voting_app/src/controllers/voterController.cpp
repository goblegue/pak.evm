#include "controllers/voterController.h"
#include "services/QR/qr.h"
#include "services/email/emailservice.h"
#include <QFile>
#include <QDir>
#include <QUuid>

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
    if (m_tokenRepo == nullptr || m_electionRepo == nullptr)
    {
        return nullopt;
    }
    if (!canRequestToken(userCnic, electionId))
    {
        return nullopt;
    }

    Token newToken;
    newToken.setId("TKN-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper());
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
    if (m_tokenRepo == nullptr)
    {
        tokensSize = 0;
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
    if (m_tokenRepo == nullptr)
    {
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

bool TokenController::saveToken(const Token &token)
{
    if (m_tokenRepo == nullptr)
    {
        return false;
    }
    return m_tokenRepo->insertToken(token);
}

bool TokenController::sendTokenToEmail(const Token &token, const QString &email)
{
    QString subject = "Your Official Voting Token - Election " + token.getElectionId();

    // 1. Generate the QR Code Image
    QString payload = QR::preparePayload(token);
    QImage qrCode = QR::generateQRCode(payload);

    // 2. Save the QR Code to a temporary file so EmailService can attach it
    QString tempFileName = QString("voting_token_%1_%2.png")
                               .arg(token.getElectionId())
                               .arg(token.getUserCnic());

    QString attachmentPath = QDir::tempPath() + "/" + tempFileName;

    if (!qrCode.save(attachmentPath, "PNG"))
    {
        return false;
    }

    QString htmlBody = QString(
                           "<div style=\"font-family: 'Segoe UI', Arial, sans-serif; max-width: 600px; margin: 0 auto; border: 1px solid #e0e0e0; border-radius: 10px; padding: 25px; box-shadow: 0 4px 8px rgba(0,0,0,0.05);\">"
                           "<div style=\"text-align: center; border-bottom: 2px solid #2E7D32; padding-bottom: 15px; margin-bottom: 20px;\">"
                           "<h1 style=\"color: #2E7D32; margin: 0;\">Official Voting Token</h1>"
                           "<p style=\"color: #555; margin-top: 5px;\">Pakistan Election Commission - Secure EVM System</p>"
                           "</div>"

                           "<p style=\"font-size: 16px; color: #333;\">Dear Citizen,</p>"
                           "<p style=\"font-size: 15px; color: #555; line-height: 1.5;\">You have successfully registered to vote. Please find your secure cryptographic voting token details below. <b>The QR code has been attached to this email.</b></p>"

                           "<div style=\"background-color: #f8f9fa; border-left: 4px solid #1565C0; padding: 15px; border-radius: 4px; margin: 25px 0;\">"
                           "<p style=\"margin: 5px 0;\"><strong style=\"color: #333;\">Voter CNIC:</strong> <span style=\"color: #1565C0;\">%1</span></p>"
                           "<p style=\"margin: 5px 0;\"><strong style=\"color: #333;\">Election ID:</strong> %2</p>"
                           "<p style=\"margin: 5px 0;\"><strong style=\"color: #333;\">Issued At:</strong> %3</p>"
                           "<p style=\"margin: 15px 0 5px 0; word-wrap: break-word;\"><strong style=\"color: #333;\">Cryptographic Signature:</strong><br><span style=\"font-family: monospace; font-size: 12px; color: #666;\">%4</span></p>"
                           "</div>"

                           "<h3 style=\"color: #1565C0; border-bottom: 1px solid #eee; padding-bottom: 10px;\">Instructions for Voting Day:</h3>"
                           "<ol style=\"color: #555; font-size: 15px; line-height: 1.6;\">"
                           "<li>Go to your assigned offline voting station.</li>"
                           "<li>Open this email on your smartphone and download the attached <b>QR Code Image</b>.</li>"
                           "<li>Hold the QR code up to the scanner at the Electronic Voting Machine (EVM) terminal.</li>"
                           "<li>Cast your vote securely and anonymously!</li>"
                           "</ol>"

                           "<div style=\"background-color: #ffebee; border: 1px solid #ef9a9a; border-radius: 5px; padding: 15px; text-align: center; margin-top: 30px;\">"
                           "<p style=\"color: #c62828; font-size: 14px; margin: 0; font-weight: bold;\">⚠️ WARNING</p>"
                           "<p style=\"color: #d32f2f; font-size: 13px; margin-top: 5px;\">DO NOT SHARE THIS EMAIL OR QR CODE WITH ANYONE. IT IS YOUR UNIQUE, SINGLE-USE BALLOT PASS.</p>"
                           "</div>"
                           "</div>")
                           .arg(token.getUserCnic())
                           .arg(token.getElectionId())
                           .arg(token.getIssuedAt().toString("dd MMMM yyyy, hh:mm AP")) // Formats nicely like: 20 April 2026, 09:30 AM
                           .arg(token.getTokenSignature());

    bool emailSuccess = EmailService::getInstance().sendEmail(
        email,
        subject,
        htmlBody,
        true,
        attachmentPath);

    QFile::remove(attachmentPath);

    return emailSuccess;
}
