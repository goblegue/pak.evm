#ifndef USERRESULTSPAGE_H
#define USERRESULTSPAGE_H

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "models/Models.h"
#include "models/entities/result.h" // Added our new entity

class UserResultsPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserResultsPage(QWidget *parent = nullptr);
    void loadElections(Election *elections, int size);

    // UPDATED: Use the C++ Model instead of JSON
    void displayResults(const Result &result);

signals:
    void electionSelectedForResults(QString electionId);

private slots:
    void onElectionClicked(const QModelIndex &index);

private:
    // Left Side
    QListView *electionListView;
    ElectionListModel *electionModel;

    // Right Side
    QStackedWidget *rightStackedWidget;
    QWidget *placeholderWidget;
    QWidget *resultsContainer;

    QLabel *rightTitleLabel;
    QScrollArea *chartScrollArea;
    QWidget *chartWidget;
    QVBoxLayout *chartLayout;

    QLabel *winnerLabel;
    QLabel *totalTokensLabel;
    QLabel *pollClosedLabel;

    QString currentSelectedElectionId;

    void setupUi();
    void clearResults();

    // UPDATED: Now requires electionId to fetch real names from DB
    QString getCandidateName(const QString &cnic, const QString &electionId);
};

#endif // USERRESULTSPAGE_H