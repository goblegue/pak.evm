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
#include "models/Models.h" 
#include "models/entities/election.h"
#include "models/entities/candidate.h"

class AdminCandidatePage : public QWidget {
    Q_OBJECT

public:
    explicit AdminCandidatePage(QWidget *parent = nullptr);

    
    void loadElections(Election* elections, int size);
    void loadCandidates(Candidate* candidates, int size);

signals:
    
    void navigateToCandidateDetails(Candidate selectedCandidate);

    
    void filterRequested();

    
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

    QMenu *filterMenu; 
    CandidateFilterProxyModel *proxyModel; 

    void setupUi();
};

#endif 
