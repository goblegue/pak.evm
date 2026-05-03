#ifndef MODELS_H
#define MODELS_H

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>

// IMPORTANT: Make sure this include path points to where your
// Candidate class is actually located in your project folder!
#include "models/entities/candidates.h"

// Define custom roles so our Delegate can fetch specific data
enum CustomRoles {
    CandidateSymbolNameRole = Qt::UserRole + 1,
    CandidateSymbolImageRole = Qt::UserRole + 2,
    CandidateProfileImageRole = Qt::UserRole + 3,
    CandidateCnicRole = Qt::UserRole + 4,
    CandidatePartyRole = Qt::UserRole + 5,
    CandidateNameRole = Qt::UserRole + 6
};

// ==========================================
// CANDIDATE LIST MODEL
// ==========================================
class CandidateListModel : public QAbstractListModel {
    Q_OBJECT
private:
    QList<Candidate> m_candidates;

public:
    explicit CandidateListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    // Function to load the array of Candidate objects from your database/controller
    void setCandidates(Candidate* candidatesArray, int size) {
        beginResetModel();
        m_candidates.clear();
        for(int i = 0; i < size; ++i) {
            m_candidates.append(candidatesArray[i]);
        }
        endResetModel();
    }

    // Helper to get a full Candidate object at a specific row
    Candidate getCandidateAt(int index) const {
        return m_candidates.at(index);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_candidates.count();
    }

    // The core function that provides data to the UI (the Delegate)
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_candidates.count()) return QVariant();

        const Candidate &candidate = m_candidates.at(index.row());

        // 1. Used for the Search Bar (Search by Name, CNIC, or Party)
        if (role == Qt::DisplayRole) {
            return QString("%1\n%2\n%3").arg(candidate.getName(), candidate.getCnic(), candidate.getPartyName());
        }

        // 2. Used by the Delegates to draw UI
        if (role == CandidateSymbolNameRole) return candidate.getSymbolName();
        if (role == CandidateCnicRole) return candidate.getCnic();
        if (role == CandidatePartyRole) return candidate.getPartyName();
        if (role == CandidateProfileImageRole) return candidate.getProfileImageBase64();
        if (role == CandidateSymbolImageRole) return candidate.getSymbolBase64();
        if (role == CandidateNameRole) return candidate.getName(); // <-- FETCH USERNAME

        return QVariant();
    }
};


// ==========================================
// CANDIDATE SEARCH PROXY MODEL
// ==========================================
class CandidateSearchProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit CandidateSearchProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
    // The instant search logic
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

        QString mainText = sourceModel()->data(index, Qt::DisplayRole).toString();

        // Gets the text from your search bar and checks if it's inside the candidate's info
        QString searchStr = filterRegularExpression().pattern().toLower();
        if (searchStr.isEmpty()) return true;

        return mainText.toLower().contains(searchStr);
    }
};

#endif // MODELS_H
