#ifndef EVMVOTINGPAGE_H
#define EVMVOTINGPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Models.h"

class EvmVotingPage : public QWidget {
    Q_OBJECT

public:
    explicit EvmVotingPage(QWidget *parent = nullptr);
    void loadCandidates(Candidate* candidates, int size);
    void updateTimeRemaining(const QString &timeString);

signals:
    void candidateVoted(Candidate selectedCandidate);

private slots:
    void onSearchTextChanged(const QString &text);
    void onVoteButtonClicked(const QModelIndex &proxyIndex);

private:
    QLabel *timeRemainingLabel;
    QLineEdit *searchBar;
    QListView *candidateListView;

    CandidateListModel *candidateModel;
    CandidateSearchProxyModel *proxyModel;

    void setupUi();
};

#endif // EVMVOTINGPAGE_H
