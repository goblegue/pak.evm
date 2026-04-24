#ifndef ADMIN_MODELS_H
#define ADMIN_MODELS_H

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>
#include "election/election.h"
#include "candidate/candidate.h"
#include "states.h"


// Define custom roles so the Delegate can fetch specific data
enum CustomRoles {
    CandidateStatusRole = Qt::UserRole + 1,
    CandidateNameRole = Qt::UserRole + 2
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


#endif // ADMIN_MODELS_H
