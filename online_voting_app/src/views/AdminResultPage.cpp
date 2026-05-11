#include "views/AdminResultPage.h"
#include <QFrame>
#include "Delegates.h"
#include "controllers/candidateController.h"
#include "controllers/resultController.h"

AdminResultPage::AdminResultPage(QWidget *parent)
    : QWidget(parent)
{
    electionModel = new ElectionListModel(this);
    setupUi();
}

void AdminResultPage::setCryptoKeys(const QByteArray &publicKey, const QByteArray &privateKey)
{
    m_publicKey = publicKey;
    m_privateKey = privateKey;
}

void AdminResultPage::setupUi()
{
    // ... KEEP YOUR EXACT SAME setupUi() CODE FROM BEFORE ...
    // (I am omitting it here to save your tokens, DO NOT change your UI drawing code!)
}

void AdminResultPage::loadElections(Election *elections, int size)
{
    electionModel->setElections(elections, size);
     clearResults();
}

void AdminResultPage::onElectionClicked(const QModelIndex &index)
{
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();
    currentElectionState = selected.getStatus();

    rightTitleLabel->setText("Results: " + selected.getTitle());
    clearResults();

    // LOGIC ROUTING BASED ON STATE
    if (currentElectionState == ElectionState::VotingClosed) {
        // Mode 1: Awaiting Upload
        loadBtn->show();
        loadBtn->setEnabled(true);
    } else if (currentElectionState == ElectionState::ResultsAnnounced) {
        // Mode 2: Already Finalized, View Only
        loadBtn->hide();

        auto resultOpt = ResultController::getInstance().getElectionResults(
            currentSelectedElectionId);
        if (resultOpt.has_value()) {
            displayResults(resultOpt.value());
            saveBtn->hide(); // Hide save/cancel because it's already in DB
            cancelBtn->hide();
        } else {
            QMessageBox::warning(
                this,
                "Error",
                "Election marked as announced but results are missing from Database.");
        }
    } else {
        loadBtn->hide();
    }
}

void AdminResultPage::clearResults()
{
    // 1. Safety check just in case setupUi() hasn't finished yet
    if (!chartLayout)
        return;

    // 2. Safely clear the layout
    QLayoutItem *child;
    while ((child = chartLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();        // Remove it from view immediately
            child->widget()->deleteLater(); // Queue it for safe memory deletion
        }
        delete child; // The layout item itself is safe to delete instantly
    }

    // 3. Reset Labels (with safety checks)
    if (winnerLabel)
        winnerLabel->setText("🏆 Winner: -");
    if (totalTokensLabel)
        totalTokensLabel->setText("🎫 Total Votes Cast: -");
    if (pollClosedLabel)
        pollClosedLabel->setText("⏱ Polling Closed At: -");

    // 4. Hide Buttons
    if (saveBtn)
        saveBtn->hide();
    if (cancelBtn)
        cancelBtn->hide();
}

void AdminResultPage::onLoadResultsClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select Encrypted Results File",
                                                    "",
                                                    "JSON Files (*.json);;All Files (*.*)");
    if (filePath.isEmpty())
        return;

    // 1. Delegate all decryption and parsing to the Controller!
    auto resultOpt = ResultController::getInstance().loadAndPreviewResultFile(filePath,
                                                                              m_publicKey,
                                                                              m_privateKey);

    if (!resultOpt.has_value()) {
        QMessageBox::critical(this,
                              "Decryption Failed",
                              "Invalid cryptographic file signature. Data is corrupted, forged, or "
                              "from a different keypair.");
        return;
    }

    Result res = resultOpt.value();

    // 2. Security Verifications
    if (res.getElectionId() != currentSelectedElectionId) {
        QMessageBox::warning(this, "Mismatch", "This results file belongs to a different election!");
        return;
    }

    if (res.getAuditStatus() != "PASSED") {
        QMessageBox::critical(
            this,
            "Audit Failed",
            "Warning: The EVM forensic audit for this file FAILED. Data cannot be trusted.");
        return;
    }

    // 3. Cache it for saving later, and draw the UI
    currentPreviewedResult = res;
    displayResults(res);

    // Show action buttons for the Admin to confirm
    saveBtn->show();
    cancelBtn->show();
}

void AdminResultPage::displayResults(const Result &res)
{
    clearResults();

    int totalVotes = res.getTotalVotesCast();
    totalTokensLabel->setText(QString("🎫 Total Votes Cast: %1").arg(totalVotes));

    pollClosedLabel->setText("⏱ Polling Closed At: "
                             + res.getPollClosedAt().toString("MMM dd, yyyy - hh:mm AP"));

    int maxVotes = -1;
    QString winnerName = "Tie / Undecided";

    CandidateTally *tally = res.getTally();
    int tallyCount = res.getTallyCount();

    for (int i = 0; i < tallyCount; ++i) {
        int votes = tally[i].totalVotes;
        QString cnic = tally[i].candidateCnic;

        // Fetch Real Name from DB!
        QString candName = getCandidateName(cnic, res.getElectionId());

        if (votes > maxVotes) {
            maxVotes = votes;
            winnerName = candName;
        }

        double percentage = totalVotes > 0 ? ((double) votes / totalVotes) * 100.0 : 0;

        // --- Create Graph Row (SAME AS YOUR ORIGINAL CODE) ---
        QWidget *rowWidget = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 5, 0, 5);

        QLabel *nameLbl = new QLabel(candName, rowWidget);
        nameLbl->setFixedWidth(150);
        nameLbl->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495E; border: none;");

        QProgressBar *bar = new QProgressBar(rowWidget);
        bar->setRange(0, totalVotes);
        bar->setValue(votes);
        bar->setTextVisible(false);
        bar->setFixedHeight(25);
        bar->setStyleSheet(
            "QProgressBar { border: 1px solid #BDC3C7; border-radius: 4px; background: #ECF0F1; }"
            "QProgressBar::chunk { background-color: #3b5998; border-radius: 3px; }");

        QLabel *pctLbl = new QLabel(QString::number(percentage, 'f', 1) + "% ("
                                        + QString::number(votes) + " Votes)",
                                    rowWidget);
        pctLbl->setFixedWidth(120);
        pctLbl->setStyleSheet("font-size: 14px; color: #7F8C8D; border: none;");

        rowLayout->addWidget(nameLbl);
        rowLayout->addWidget(bar);
        rowLayout->addWidget(pctLbl);

        chartLayout->addWidget(rowWidget);
    }

    winnerLabel->setText("🏆 Winner: " + winnerName);
}

// Replaces the Mock Data!
QString AdminResultPage::getCandidateName(const QString &cnic, const QString &electionId)
{
    int size = 0;
    Candidate *candidates = CandidateController::getInstance()
                                .getCandidatesByElectionAndStatus(electionId,
                                                                  ApprovalStatus::Approved,
                                                                  size);

    QString name = cnic; // Fallback to CNIC if missing

    if (candidates) {
        for (int i = 0; i < size; ++i) {
            if (candidates[i].getUserCnic() == cnic) {
                name = candidates[i].getName();
                if (name.isEmpty())
                    name = candidates[i].getPartyName(); // Secondary fallback
                break;
            }
        }
        delete[] candidates; // Memory management!
    }
    return name;
}

void AdminResultPage::onSaveClicked()
{
    // Automatically saves to DB and updates Election State to ResultsAnnounced!
    bool success = ResultController::getInstance().commitFinalResults(currentPreviewedResult);

    if (success) {
        QMessageBox::information(
            this,
            "Saved",
            "Results have been permanently published! Election is officially concluded.");
        clearResults();
        loadBtn->hide();
        emit resultsSaved(); // Tells MainWindow to refresh the sidebar
    } else {
        QMessageBox::critical(this, "Error", "Failed to save results. They might already exist.");
    }
}

void AdminResultPage::onCancelClicked()
{
    clearResults();
    loadBtn->show();
}