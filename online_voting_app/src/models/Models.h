#ifndef ADMIN_MODELS_H
#define ADMIN_MODELS_H

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>
#include "models/entities/election.h"
#include "models/entities/candidate.h"
#include <QSet>
#include <QDateTime>
#include <QImage>
#include "models/states.h"
#include "models/entities/admin.h"
#include "models/entities/election.h"
#include "models/entities/voters.h"

// Define custom roles so the Delegate can fetch specific data
enum CustomRoles {
    CandidateStatusRole = Qt::UserRole + 1,
    CandidateSymbolNameRole = Qt::UserRole + 2,
    CandidateSymbolImageRole = Qt::UserRole + 3
};

// Add a specific role for Admin Status
enum AdminCustomRoles
{
    AdminStatusRole = Qt::UserRole + 10,
    LocalVoteRole = Qt::UserRole + 11, // NEW: Tracks the button's locked state
    AdminVotedRole = Qt::UserRole + 12
};

// Add Custom Roles for the Election Delegate to use
enum ElectionCustomRoles {
    ElectionStatusRole = Qt::UserRole + 20,
    ElectionExpandedRole = Qt::UserRole + 21,
    ElectionStartTimeRole = Qt::UserRole + 22,
    ElectionEndTimeRole = Qt::UserRole + 23,
    ElectionVotedRole = Qt::UserRole + 24
};

// Custom roles for the Token Delegate
enum TokenCustomRoles {
    TokenExpandedRole = Qt::UserRole + 30,
    TokenSignatureRole = Qt::UserRole + 31,
    TokenIssueDateRole = Qt::UserRole + 32,
    TokenStationRole = Qt::UserRole + 33,
    TokenQRCodeRole = Qt::UserRole + 34
};
// ==========================================
// ELECTION LIST MODEL
// ==========================================
class ElectionListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QList<Election> m_elections;
    QString m_currentAdminId;

public:
    explicit ElectionListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    void setElections(Election *electionsArray, int size)
    {
        beginResetModel();
        m_elections.clear();
        for (int i = 0; i < size; ++i)
            m_elections.append(electionsArray[i]);
        endResetModel();
    }
    Election getElectionAt(int index) const { return m_elections.at(index); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_elections.count();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || index.row() >= m_elections.count())
            return QVariant();

        const Election &election = m_elections.at(index.row());

        if (role == Qt::DisplayRole)
            return election.getTitle();

        if (role == ElectionVotedRole) {
            // FIX: We use the 'election' variable we just created above! No Admins here!
            return election.hasAdminVoted(m_currentAdminId);
        }
        return QVariant();
    }
    void setCurrentAdminId(const QString &id) {
        m_currentAdminId = id;
    }
};

// ==========================================
// CANDIDATE LIST MODEL
// ==========================================
class CandidateListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QList<Candidate> m_candidates;

    QString statusToString(ApprovalStatus status) const
    {
        switch (status)
        {
        case ApprovalStatus::Pending:
            return "Pending";
        case ApprovalStatus::Approved:
            return "Approved";
        case ApprovalStatus::Rejected:
            return "Rejected";
        default:
            return "Unknown";
        }
    }

public:
    explicit CandidateListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    void setCandidates(Candidate *candidatesArray, int size)
    {
        beginResetModel();
        m_candidates.clear();
        for (int i = 0; i < size; ++i)
            m_candidates.append(candidatesArray[i]);
        endResetModel();
    }
    Candidate getCandidateAt(int index) const { return m_candidates.at(index); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_candidates.count();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_candidates.count()) return QVariant();
        const Candidate &candidate = m_candidates.at(index.row());

        if (role == Qt::DisplayRole) {
            return QString("%1\nParty: %2\nSymbol: %3\nStatus: %4")
                .arg(candidate.getUserCnic())
                .arg(candidate.getPartyName())
                .arg(candidate.getSymbolName())
                .arg(statusToString(candidate.getStatus()));
        }
        if (role == CandidateStatusRole) return static_cast<int>(candidate.getStatus());

        // Pass the Base64 Image to the Delegate
        if (role == CandidateSymbolImageRole) return candidate.getSymbolBase64();

        return QVariant();
    }
};

// ==========================================
// CANDIDATE FILTER PROXY MODEL
// ==========================================
class CandidateFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
private:
    int m_filterStatus = -1; // -1 means "Show All"

public:
    explicit CandidateFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    // Function to change the filter and force the UI to update
    void setFilterStatus(int status)
    {
        m_filterStatus = status;
        invalidateFilter(); // Tells Qt to re-run the filter logic immediately
    }

protected:
    // This is the magic Qt function. If it returns true, the row is shown.
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        if (m_filterStatus == -1)
            return true; // Show everything

        // Ask the original model for the status of this specific row
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        int status = sourceModel()->data(index, CandidateStatusRole).toInt();

        // Only show it if the status matches our filter
        return status == m_filterStatus;
    }
};

// ==========================================
// ADMIN LIST MODEL
// ==========================================
class AdminListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QList<Admin> m_admins;
    QMap<QString, int> m_localVotes; // Tracks locks: 0=Unlocked, 1=Approved, 2=Rejected
    QString m_currentAdminId;

    QString statusToString(ApprovalStatus status) const
    {
        switch (status)
        {
        case ApprovalStatus::Pending:
            return "Pending";
        case ApprovalStatus::Approved:
            return "Approved";
        case ApprovalStatus::Rejected:
            return "Rejected";
        default:
            return "Unknown";
        }
    }

public:
    explicit AdminListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    void setAdmins(Admin *adminsArray, int size)
    {
        beginResetModel();
        m_admins.clear();
        m_localVotes.clear(); // Reset locks when reloading
        for (int i = 0; i < size; ++i)
            m_admins.append(adminsArray[i]);
        endResetModel();
    }

    Admin getAdminAt(int index) const { return m_admins.at(index); }

    // THIS FUNCTION LOCKS THE BUTTON
    void setLocalVote(QString cnic, int vote)
    {
        m_localVotes[cnic] = vote;
        // Tell the UI to repaint this specific row
        for (int i = 0; i < m_admins.count(); i++)
        {
            if (m_admins[i].getCnic() == cnic)
            {
                QModelIndex idx = index(i);
                emit dataChanged(idx, idx, {LocalVoteRole});
                break;
            }
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_admins.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_admins.count()) return QVariant();

        const Admin &admin = m_admins.at(index.row());

        if (role == Qt::DisplayRole) {
            return QString("%1 (%2) - CNIC: %3\nStatus: %4")
                .arg(admin.getName())
                .arg(admin.getEmail())
                .arg(admin.getCnic())
                .arg(statusToString(admin.getStatus()));
        }
        if (role == AdminStatusRole) return static_cast<int>(admin.getStatus());

        if (role == AdminVotedRole) {
            return admin.hasAdminVoted(m_currentAdminId);
        }
        if (role == LocalVoteRole) {
            return m_localVotes.value(admin.getCnic(), 0);
        }

        return QVariant();
    }
    void setCurrentAdminId(const QString &id) {
        m_currentAdminId = id;
    }
};

// ==========================================
// ADMIN FILTER PROXY MODEL
// ==========================================
class AdminFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
private:
    int m_filterStatus = -1; // -1 means "Show All"

public:
    explicit AdminFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setFilterStatus(int status)
    {
        m_filterStatus = status;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        if (m_filterStatus == -1)
            return true;

        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        int status = sourceModel()->data(index, AdminStatusRole).toInt();

        return status == m_filterStatus;
    }
};

// ==========================================
// ELECTION LIST MODEL (UPDATED FOR ACCORDION)
// ==========================================
class ManageElectionListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Election> m_elections;
    QSet<QString> m_expandedItems;
    QString m_currentAdminId;

    // MISSING VARIABLE ADDED:
    QMap<QString, int> m_localVotes;

    QString statusToString(ElectionState status) const {
        switch(status) {
        case ElectionState::Drafted: return "Draft";
        case ElectionState::Rejected: return "Rejected";
        case ElectionState::Published: return "Published";
        case ElectionState::VotingOpen: return "Voting Open";
        case ElectionState::VotingClosed: return "Voting Closed";
        case ElectionState::ResultsAnnounced: return "Results Announced";
        default: return "Unknown";
        }
    }

public:
    explicit ManageElectionListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    void setCurrentAdminId(const QString &id) {
        m_currentAdminId = id;
    }

    void setElections(Election* electionsArray, int size) {
        beginResetModel();
        m_elections.clear();
        m_expandedItems.clear();
        m_localVotes.clear(); // Clear old votes when reloading!
        for(int i = 0; i < size; ++i) m_elections.append(electionsArray[i]);
        endResetModel();
    }

    Election getElectionAt(int index) const { return m_elections.at(index); }

    void toggleExpanded(QString electionId) {
        if (m_expandedItems.contains(electionId)) {
            m_expandedItems.remove(electionId);
        } else {
            m_expandedItems.insert(electionId);
        }

        for(int i = 0; i < m_elections.count(); ++i) {
            if(m_elections[i].getId() == electionId) {
                QModelIndex idx = index(i);
                emit dataChanged(idx, idx, {ElectionExpandedRole});
                break;
            }
        }
    }

    // ==========================================
    // MISSING FUNCTION ADDED:
    // ==========================================
    void setLocalVote(QString electionId, int voteCode) {
        m_localVotes[electionId] = voteCode;
        for(int i = 0; i < m_elections.count(); ++i) {
            if(m_elections[i].getId() == electionId) {
                QModelIndex idx = index(i);
                // Tell the Delegate to redraw the button!
                emit dataChanged(idx, idx, {LocalVoteRole});
                break;
            }
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_elections.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_elections.count()) return QVariant();

        const Election &election = m_elections.at(index.row());

        if (role == Qt::DisplayRole) return election.getTitle();
        if (role == ElectionStatusRole) return static_cast<int>(election.getStatus());
        if (role == ElectionExpandedRole) return m_expandedItems.contains(election.getId());
        if (role == ElectionStartTimeRole) return election.getStartTime().toString("MMM dd, yyyy - hh:mm AP");
        if (role == ElectionEndTimeRole) return election.getEndTime().toString("MMM dd, yyyy - hh:mm AP");

        if (role == ElectionVotedRole) {
            return election.hasAdminVoted(m_currentAdminId);
        }

        // ==========================================
        // MISSING ROLE CHECK ADDED:
        // ==========================================
        if (role == LocalVoteRole) {
            return m_localVotes.value(election.getId(), 0);
        }

        return QVariant();
    }
};

// ==========================================
// ELECTION FILTER PROXY MODEL
// ==========================================
class ElectionFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
private:
    int m_filterStatus = -1; // -1 means "Show All"

public:
    explicit ElectionFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setFilterStatus(int status) {
        m_filterStatus = status;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (m_filterStatus == -1) return true;

        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        int status = sourceModel()->data(index, ElectionStatusRole).toInt();

        return status == m_filterStatus;
    }
};

// ==========================================
// TOKEN LIST MODEL (ACCORDION STYLE)
// ==========================================
class TokenListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Token> m_tokens;
    QSet<QString> m_expandedItems; // Remembers expanded tokens

public:
    explicit TokenListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    void setTokens(Token* tokensArray, int size) {
        beginResetModel();
        m_tokens.clear();
        m_expandedItems.clear(); // Collapse all on load
        for(int i = 0; i < size; ++i) m_tokens.append(tokensArray[i]);
        endResetModel();
    }

    Token getTokenAt(int index) const { return m_tokens.at(index); }

    void toggleExpanded(QString tokenId) {
        if (m_expandedItems.contains(tokenId)) m_expandedItems.remove(tokenId);
        else m_expandedItems.insert(tokenId);

        for(int i = 0; i < m_tokens.count(); ++i) {
            if(m_tokens[i].getId() == tokenId) {
                QModelIndex idx = index(i);
                emit dataChanged(idx, idx, {TokenExpandedRole});
                break;
            }
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_tokens.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_tokens.count()) return QVariant();

        const Token &token = m_tokens.at(index.row());

        if (role == Qt::DisplayRole) return "Election: " + token.getElectionId();
        if (role == TokenExpandedRole) return m_expandedItems.contains(token.getId());
        if (role == TokenSignatureRole) return token.getTokenSignature();
        if (role == TokenIssueDateRole) return token.getIssuedAt().toString("MMM dd, yyyy - hh:mm AP");
        if (role == TokenStationRole) return token.getAssignedStationId();

        // ==========================================
        // CALL YOUR BACKEND DEVELOPER's FUNCTION HERE
        // ==========================================
        if (role == TokenQRCodeRole) {
            // Example: Ask the backend to generate the QR code using the Token's Signature
            // QImage generatedQr = BackendDeveloperClass::generateQRCode(token.getTokenSignature());
            // return generatedQr;

            // (Replace the lines above with the actual function call your backend dev gave you)
            return QImage(); // Temporary fallback until you plug their function in
        }

        return QVariant();
    }
};
#endif // ADMIN_MODELS_H
