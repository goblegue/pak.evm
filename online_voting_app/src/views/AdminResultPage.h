#ifndef ADMINRESULTPAGE_H
#define ADMINRESULTPAGE_H

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "models/Models.h"
#include "models/entities/result.h"
#include "models/states.h"

class AdminResultPage : public QWidget
{
    Q_OBJECT

public:
    explicit AdminResultPage(QWidget *parent = nullptr);
    void loadElections(Election *elections, int size);

    // NEW: Inject keys so the Controller can decrypt files
    void setCryptoKeys(const QByteArray &publicKey, const QByteArray &privateKey);

signals:
    void electionSelected(QString electionId);
    void resultsSaved(); // Emit when an election is successfully finalized

private slots:
    void onElectionClicked(const QModelIndex &index);
    void onLoadResultsClicked();
    void onSaveClicked();
    void onCancelClicked();

private:
    // Left Side
    QListView *electionListView;
    ElectionListModel *electionModel;

    // Right Side
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

    // --- State Management ---
    QByteArray m_publicKey;
    QByteArray m_privateKey;
    QString currentSelectedElectionId;
    ElectionState currentElectionState;
    Result currentPreviewedResult;

    void setupUi();
    void clearResults();
    void displayResults(const Result &result);

    // Fetch real names from CandidateController
    QString getCandidateName(const QString &cnic, const QString &electionId);
};

#endif // ADMINRESULTPAGE_H