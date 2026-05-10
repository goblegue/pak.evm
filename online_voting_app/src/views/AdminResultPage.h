#ifndef ADMINRESULTPAGE_H
#define ADMINRESULTPAGE_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileDialog>
#include <QMessageBox>
#include "models/Models.h"

class AdminResultPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminResultPage(QWidget *parent = nullptr);
    void loadElections(Election* elections, int size);

signals:
    void electionSelected(QString electionId);

private slots:
    void onElectionClicked(const QModelIndex &index);
    void onLoadResultsClicked();
    void onSaveClicked();
    void onCancelClicked();

private:
    // Left Side (30%)
    QListView *electionListView;
    ElectionListModel *electionModel;

    // Right Side (70%)
    QWidget *rightContainer;
    QLabel *rightTitleLabel;
    QPushButton *loadBtn;

    // Chart Area
    QScrollArea *chartScrollArea;
    QWidget *chartWidget;
    QVBoxLayout *chartLayout;

    // Stats Area
    QLabel *winnerLabel;
    QLabel *totalTokensLabel;
    QLabel *pollClosedLabel;

    // Bottom Action Buttons
    QPushButton *saveBtn;
    QPushButton *cancelBtn;

    QString currentSelectedElectionId;
    QJsonObject currentResultData;

    void setupUi();
    void clearResults();
    void processEncryptedResultFile(const QString &filePath);
    void displayResults(const QJsonObject &resultJson);

    // Helper to simulate fetching Candidate Name from CNIC
    QString getCandidateName(const QString &cnic);
};

#endif // ADMINRESULTPAGE_H
