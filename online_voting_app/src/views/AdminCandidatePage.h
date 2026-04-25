#ifndef ADMINCANDIDATEPAGE_H
#define ADMINCANDIDATEPAGE_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include "models/Models.h" // Include the models we just made
#include "models/entities/election.h"
#include "models/entities/candidate.h"

class AdminCandidatePage : public QWidget {
    Q_OBJECT

public:
    explicit AdminCandidatePage(QWidget *parent = nullptr);

    // Methods for the Lead/Backend to inject the data arrays
    void loadElections(Election* elections, int size);
    void loadCandidates(Candidate* candidates, int size);

signals:
    // Signal to tell the Main Stacked Widget to change to the "Details" page
    void navigateToCandidateDetails(Candidate selectedCandidate);

    // Signal for the filter logic
    void filterRequested();

    // Signal when election is clicked so DB can fetch new candidates
    void electionSelected(QString electionId);

private slots:
    void onElectionClicked(const QModelIndex &index);
    void onCandidateClicked(const QModelIndex &index);
    void showFilterMenu();
    void applyFilter(int status, QString filterName);

private:
    QListView *electionListView;
    QListView *candidateListView;
    QPushButton *filterBtn;

    ElectionListModel *electionModel;
    CandidateListModel *candidateModel;

    QMenu *filterMenu; // The dropdown menu
    CandidateFilterProxyModel *proxyModel; // Our new magic filter!

    void setupUi();
};

#endif // ADMINCANDIDATEPAGE_H
