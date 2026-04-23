#ifndef ELECTIONMODEL_H
#define ELECTIONMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "./election/election.h"

class ElectionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // Define custom "Roles" so the UI knows exactly what piece of data to ask for
    enum ElectionRoles
    {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DetailsRole,
        StatusRole
    };

    explicit ElectionModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    // Load your dynamic array into this Qt-friendly QList
    void setElections(Election *electionsArray, int count)
    {
        beginResetModel();
        m_elections.clear();
        for (int i = 0; i < count; ++i)
        {
            m_elections.append(electionsArray[i]);
        }
        endResetModel();
    }

    // 1. Tells the ListView how many items to draw
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_elections.count();
    }

    // 2. The ListView calls this function to get the data for every single card
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || index.row() >= m_elections.count())
            return QVariant();

        const Election &election = m_elections.at(index.row());

        // Map the roles to your Election class getters!
        if (role == IdRole)
        {
            return election.getId();
        }
        else if (role == TitleRole)
        {
            return election.getTitle();
        }
        else if (role == DetailsRole)
        {
            QString dateString = QString("Start: %1 | End: %2")
                                     .arg(election.getStartTime().toString("dd MMM yyyy"))
                                     .arg(election.getEndTime().toString("dd MMM yyyy"));
            return dateString;
        }
        else if (role == StatusRole)
        {
            return static_cast<int>(election.getStatus());
        }

        return QVariant();
    }

private:
    QList<Election> m_elections;
};

#endif // ELECTIONMODEL_H
