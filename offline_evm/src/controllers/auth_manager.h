
#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <QString>
#include <QByteArray>
#include <memory>
#include <optional>

#include "models/entities/poll_worker.h" // Contains PollWorker
#include "models/entities/tokens.h"

class AuthManager
{
private:
    IWorkerRepository *m_workerRepo;
    ITokenRepository *m_usedTokenRepo;

    std::unique_ptr<PollWorker> m_currentWorker;
    bool m_isMasterUnlocked;
    QByteArray m_systemPublicKey; // Loaded from SystemConfig 

    AuthManager();
    ~AuthManager();

public:
    enum class TokenResult {
        Valid,
        AlreadyUsed,
        InvalidSignature,
        InvalidElection,
        ParseError
    };

    AuthManager(const AuthManager &) = delete;
    void operator=(const AuthManager &) = delete;

    static AuthManager &getInstance();
    
    void injectDependencies(IWorkerRepository *workerRepo, ITokenRepository *tokenRepo);
    void setSystemPublicKey(const QByteArray &publicKey) { m_systemPublicKey = publicKey; }

    // --- WORKER MANAGEMENT ---
    bool createOperationalWorker(const QString &username, const QString &password); // [NEW]
    bool loginWorker(const QString &username, const QString &password);
    void logoutWorker();
    bool isWorkerLoggedIn() const { return m_currentWorker != nullptr; }
    PollWorker *getCurrentWorker() const { return m_currentWorker.get(); }

    // --- MASTER AUTHORITY ---
    bool unlockMasterAuthority(const QString &masterPassword);
    void lockMasterAuthority();
    bool isMasterUnlocked() const { return m_isMasterUnlocked; }

    // --- VOTER TOKEN VERIFICATION ---
    TokenResult verifyVoterToken(QString &cnic, const QString &qrPayload, QString &out_tokenId);
};

#endif // AUTH_MANAGER_H