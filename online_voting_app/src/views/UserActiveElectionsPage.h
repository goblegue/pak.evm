#ifndef USERACTIVEELECTIONSPAGE_H
#define USERACTIVEELECTIONSPAGE_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QStackedWidget>
#include "./models/entities/election.h"
#include "./models/entities/candidate.h"
#include "./models/Models.h"


class UserActiveElectionsPage : public QWidget {
    Q_OBJECT

public:
    explicit UserActiveElectionsPage(QWidget *parent = nullptr);
    void loadElections(Election* elections, int size);
    void loadCandidates(Candidate* candidates, int size);
    void clearData();

signals:
    void electionSelected(QString electionId);
    void generateTokenRequested(QString electionId);
    void navigateToCandidateDetails(Candidate selectedCandidate);

private slots:
    void onElectionClicked(const QModelIndex &index);
    void onGenerateTokenClicked();
    void onCandidateClicked(const QModelIndex &index);

private:
    QListView *electionListView;
    ElectionListModel *electionModel;

    // --- RIGHT SIDE MANAGERS ---
    QStackedWidget *rightStackedWidget; // Swaps between placeholder and details
    QWidget *placeholderWidget;         // The empty state
    QWidget *detailsContainer;          // The actual details

    QLabel *detailTitleLabel;
    QLabel *detailStartTimeLabel;
    QLabel *detailEndTimeLabel;
    QPushButton *generateTokenBtn;

    QListView *candidateListView;
    CandidateListModel *candidateModel;

    QString currentSelectedElectionId;

    void setupUi();
};

#endif // USERACTIVEELECTIONSPAGE_H
