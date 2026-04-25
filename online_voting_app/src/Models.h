#ifndef ADMIN_MODELS_H
#define ADMIN_MODELS_H

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>
#include "election/election.h"
#include "candidate/candidate.h"
#include "states.h"
#include "user_management/admin/admin.h"


// Define custom roles so the Delegate can fetch specific data
enum CustomRoles {
    CandidateStatusRole = Qt::UserRole + 1,
    CandidateNameRole = Qt::UserRole + 2
};

// Add a specific role for Admin Status
enum AdminCustomRoles {
    AdminStatusRole = Qt::UserRole + 10,
    LocalVoteRole = Qt::UserRole + 11 // NEW: Tracks the button's locked state
};

// ==========================================
// ELECTION LIST MODEL
// ==========================================
class ElectionListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Election> m_elections;

public:
    explicit ElectionListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    void setElections(Election* electionsArray, int size) {
        beginResetModel();
        m_elections.clear();
        for(int i = 0; i < size; ++i) m_elections.append(electionsArray[i]);
        endResetModel();
    }
    Election getElectionAt(int index) const { return m_elections.at(index); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_elections.count();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_elections.count()) return QVariant();
        const Election &election = m_elections.at(index.row());
        if (role == Qt::DisplayRole) return election.getTitle();
        return QVariant();
    }
};

// ==========================================
// CANDIDATE LIST MODEL
// ==========================================
class CandidateListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Candidate> m_candidates;

    QString statusToString(ApprovalStatus status) const {
        switch(status) {
        case ApprovalStatus::Pending: return "Pending";
        case ApprovalStatus::Approved: return "Approved";
        case ApprovalStatus::Rejected: return "Rejected";
        default: return "Unknown";
        }
    }

public:
    explicit CandidateListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    void setCandidates(Candidate* candidatesArray, int size) {
        beginResetModel();
        m_candidates.clear();
        for(int i = 0; i < size; ++i) m_candidates.append(candidatesArray[i]);
        endResetModel();
    }
    Candidate getCandidateAt(int index) const { return m_candidates.at(index); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_candidates.count();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_candidates.count()) return QVariant();
        const Candidate &candidate = m_candidates.at(index.row());

        if (role == Qt::DisplayRole) {
            return QString("%1 - %2\nStatus: %3")
                .arg(candidate.getUserCnic())
                .arg(candidate.getPartyName())
                .arg(statusToString(candidate.getStatus()));
        }
        // Return the raw status integer for the Delegate to read
        if (role == CandidateStatusRole) {
            return static_cast<int>(candidate.getStatus());
        }
        return QVariant();
    }
};

// ==========================================
// CANDIDATE FILTER PROXY MODEL
// ==========================================
class CandidateFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
private:
    int m_filterStatus = -1; // -1 means "Show All"

public:
    explicit CandidateFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    // Function to change the filter and force the UI to update
    void setFilterStatus(int status) {
        m_filterStatus = status;
        invalidateFilter(); // Tells Qt to re-run the filter logic immediately
    }

protected:
    // This is the magic Qt function. If it returns true, the row is shown.
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (m_filterStatus == -1) return true; // Show everything

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
class AdminListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Admin> m_admins;
    QMap<QString, int> m_localVotes; // Tracks locks: 0=Unlocked, 1=Approved, 2=Rejected

    QString statusToString(ApprovalStatus status) const {
        switch(status) {
            case ApprovalStatus::Pending: return "Pending";
            case ApprovalStatus::Approved: return "Approved";
            case ApprovalStatus::Rejected: return "Rejected";
            default: return "Unknown";
        }
    }

public:
    explicit AdminListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    void setAdmins(Admin* adminsArray, int size) {
        beginResetModel();
        m_admins.clear();
        m_localVotes.clear(); // Reset locks when reloading
        for(int i = 0; i < size; ++i) m_admins.append(adminsArray[i]);
        endResetModel();
    }

    Admin getAdminAt(int index) const { return m_admins.at(index); }

    // THIS FUNCTION LOCKS THE BUTTON
    void setLocalVote(QString cnic, int vote) {
        m_localVotes[cnic] = vote;
        // Tell the UI to repaint this specific row
        for(int i=0; i<m_admins.count(); i++) {
            if(m_admins[i].getCnic() == cnic) {
                QModelIndex idx = index(i);
                emit dataChanged(idx, idx, {LocalVoteRole});
                break;
            }
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_admins.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_admins.count()) return QVariant();
        
        const Admin &admin = m_admins.at(index.row());

        if (role == Qt::DisplayRole) {
            // Displays: "Name (Email) - CNIC \n Status: Pending"
            return QString("%1 (%2) - CNIC: %3\nStatus: %4")
                .arg(admin.getName())
                .arg(admin.getEmail())
                .arg(admin.getCnic())
                .arg(statusToString(admin.getStatus()));
        }
        
        // Pass the raw enum integer for the Delegate and Proxy filter to read
        if (role == AdminStatusRole) {
            return static_cast<int>(admin.getStatus());
        }
        if (role == LocalVoteRole) {
            return m_localVotes.value(admin.getCnic(), 0);
        }
        return QVariant();
    }

    
};

// ==========================================
// ADMIN FILTER PROXY MODEL
// ==========================================
class AdminFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
private:
    int m_filterStatus = -1; // -1 means "Show All"

public:
    explicit AdminFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    void setFilterStatus(int status) {
        m_filterStatus = status;
        invalidateFilter(); 
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (m_filterStatus == -1) return true; 

        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        int status = sourceModel()->data(index, AdminStatusRole).toInt();

        return status == m_filterStatus;
    }
};

#endif // ADMIN_MODELS_H
