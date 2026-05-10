#ifndef USERRESULTSPAGE_H
#define USERRESULTSPAGE_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStackedWidget> // <-- ADD THIS
#include "models/Models.h"

class UserResultsPage : public QWidget {
    Q_OBJECT
    
public:
    explicit UserResultsPage(QWidget *parent = nullptr);
    void loadElections(Election* elections, int size);
    void displayResults(const QJsonObject &resultJson);
    
signals:
    void electionSelectedForResults(QString electionId);
    
private slots:
    void onElectionClicked(const QModelIndex &index);
    
private:
    // Left Side
    QListView *electionListView;
    ElectionListModel *electionModel;
    
    // Right Side (UPDATED FOR PLACEHOLDER)
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
    QString getCandidateName(const QString &cnic); 
};

#endif // USERRESULTSPAGE_H