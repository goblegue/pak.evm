#include "AdminResultPage.h"
#include "Delegates.h" // For EmptyStateListView and ElectionDelegate
#include <QFile>
#include <QTextStream>
#include <QFrame>

AdminResultPage::AdminResultPage(QWidget *parent) : QWidget(parent) {
    electionModel = new ElectionListModel(this);
    setupUi();
}

void AdminResultPage::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ==========================================
    // LEFT SIDE: ELECTION LIST (30%)
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *electionLabel = new QLabel("<b>Select Election</b>", this);
    electionLabel->setStyleSheet("font-size: 20px; color: #3b5998;");

    electionListView = new EmptyStateListView("No elections available.", this);
    electionListView->setModel(electionModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this));
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);

    // ==========================================
    // RIGHT SIDE: RESULTS DASHBOARD (70%)
    // ==========================================
    rightContainer = new QWidget(this);
    rightContainer->setStyleSheet("QWidget#rightContainer { background-color: white; border: 2px solid #BDC3C7; border-radius: 10px; }");
    rightContainer->setObjectName("rightContainer");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    rightLayout->setSpacing(20);

    // --- 1. Top Header & Load Button ---
    QHBoxLayout *rightHeader = new QHBoxLayout();
    rightTitleLabel = new QLabel("Election Results", this);
    rightTitleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #3b5998; border: none;");

    loadBtn = new QPushButton("📂 Load Result File", this);
    loadBtn->setCursor(Qt::PointingHandCursor);
    loadBtn->setFixedSize(180, 40);
    loadBtn->setStyleSheet("QPushButton { background-color: #3b5998; color: white; border-radius: 6px; font-size: 14px; font-weight: bold; border: none; }"
                           "QPushButton:hover { background-color: #4b6ea8; }");
    loadBtn->setEnabled(false); // Locked until election is selected

    rightHeader->addWidget(rightTitleLabel);
    rightHeader->addStretch();
    rightHeader->addWidget(loadBtn);

    // --- 2. Chart Area (Scrollable Horizontal Bar Graph) ---
    QLabel *graphTitle = new QLabel("<b>Voting Tally</b>", this);
    graphTitle->setStyleSheet("font-size: 18px; color: #314a75; border: none;");

    chartScrollArea = new QScrollArea(this);
    chartScrollArea->setWidgetResizable(true);
    chartScrollArea->setFixedHeight(250); // Fixed size as requested
    chartScrollArea->setStyleSheet("QScrollArea { border: 2px solid #ECF0F1; border-radius: 6px; background-color: #FDFEFE; }");

    chartWidget = new QWidget();
    chartWidget->setStyleSheet("background-color: transparent; border: none;");
    chartLayout = new QVBoxLayout(chartWidget);
    chartLayout->setAlignment(Qt::AlignTop);
    chartScrollArea->setWidget(chartWidget);

    // --- 3. Stats Area ---
    QFrame *statsFrame = new QFrame(this);
    statsFrame->setStyleSheet("QFrame { background-color: #F8F9F9; border: 2px solid #3b5998; border-radius: 8px; }");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsFrame);
    statsLayout->setSpacing(10);

    QString statStyle = "font-size: 16px; color: #2C3E50; font-weight: bold; border: none;";
    winnerLabel = new QLabel("🏆 Winner: -", this);
    totalTokensLabel = new QLabel("🎫 Total Tokens Scanned: -", this);
    pollClosedLabel = new QLabel("⏱ Polling Closed At: -", this);

    winnerLabel->setStyleSheet("font-size: 20px; color: #27AE60; font-weight: 900; border: none;"); // Highlight winner
    totalTokensLabel->setStyleSheet(statStyle);
    pollClosedLabel->setStyleSheet(statStyle);

    statsLayout->addWidget(winnerLabel);
    statsLayout->addWidget(totalTokensLabel);
    statsLayout->addWidget(pollClosedLabel);

    // --- 4. Bottom Action Buttons ---
    QHBoxLayout *actionLayout = new QHBoxLayout();
    cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedSize(140, 45);
    cancelBtn->setStyleSheet("QPushButton { background-color: transparent; color: #3b5998; border: 2px solid #3b5998; border-radius: 6px; font-size: 16px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #e0eaf5; }");

    saveBtn = new QPushButton("✔ Save Results", this);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setFixedSize(200, 45);
    saveBtn->setStyleSheet("QPushButton { background-color: #3b5998; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; }"
                           "QPushButton:hover { background-color: #4b6ea8; }");

    // Hide initially until results are loaded
    cancelBtn->hide();
    saveBtn->hide();

    actionLayout->addStretch();
    actionLayout->addWidget(cancelBtn);
    actionLayout->addWidget(saveBtn);

    // Assemble Right Side
    rightLayout->addLayout(rightHeader);
    rightLayout->addWidget(graphTitle);
    rightLayout->addWidget(chartScrollArea);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(statsFrame);
    rightLayout->addStretch();
    rightLayout->addLayout(actionLayout);

    // Assemble Main Layout (30% / 70% Split!)
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(rightContainer, 7);

    // Connections
    connect(electionListView, &QListView::clicked, this, &AdminResultPage::onElectionClicked);
    connect(loadBtn, &QPushButton::clicked, this, &AdminResultPage::onLoadResultsClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &AdminResultPage::onCancelClicked);
    connect(saveBtn, &QPushButton::clicked, this, &AdminResultPage::onSaveClicked);
}

// ==========================================
// LOGIC
// ==========================================
void AdminResultPage::loadElections(Election* elections, int size) {
    electionModel->setElections(elections, size);
    clearResults();
}

void AdminResultPage::onElectionClicked(const QModelIndex &index) {
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();

    rightTitleLabel->setText("Results: " + selected.getTitle());
    loadBtn->setEnabled(true);
    clearResults();
}

void AdminResultPage::clearResults() {
    // Clear the chart
    QLayoutItem *child;
    while ((child = chartLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    winnerLabel->setText("🏆 Winner: -");
    totalTokensLabel->setText("🎫 Total Tokens Scanned: -");
    pollClosedLabel->setText("⏱ Polling Closed At: -");

    saveBtn->hide();
    cancelBtn->hide();
}

void AdminResultPage::onLoadResultsClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select Encrypted Results File", "", "JSON Files (*.json);;All Files (*.*)");
    if(filePath.isEmpty()) return;

    processEncryptedResultFile(filePath);
}

void AdminResultPage::processEncryptedResultFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot open file.");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // =========================================================
    // SECURITY & DECRYPTION LAYER (For Backend Engineer)
    // =========================================================
    /*
     * QByteArray decryptedData = CryptoEngine::getInstance().decryptFileWithPrivateKey(fileData, m_config.privateKey);
     * if (decryptedData.isEmpty()) {
     *     QMessageBox::critical(this, "Decryption Failed", "Invalid cryptographic file signature. Data is corrupted or forged.");
     *     return;
     * }
     * QJsonDocument doc = QJsonDocument::fromJson(decryptedData);
     */

    // MOCK: Assuming the file is already decrypted plain JSON for UI purposes
    QJsonDocument doc = QJsonDocument::fromJson(fileData);
    if(doc.isNull()) {
        QMessageBox::critical(this, "Error", "Invalid JSON format.");
        return;
    }

    QJsonObject root = doc.object();

    // Verification Check
    QJsonObject crypto = root["cryptographic_proof"].toObject();
    if(crypto["audit_status"].toString() != "PASSED") {
        QMessageBox::critical(this, "Audit Failed", "Warning: The forensic audit for this file FAILED. Data cannot be trusted.");
        return;
    }

    // Verify it matches the clicked election
    QJsonObject meta = root["metadata"].toObject();
    if(meta["election_id"].toString() != currentSelectedElectionId) {
        QMessageBox::warning(this, "Mismatch", "This results file belongs to a different election!");
        return;
    }

    displayResults(root);
}

void AdminResultPage::displayResults(const QJsonObject &resultJson) {
    clearResults(); // Reset UI
    currentResultData = resultJson;

    QJsonObject meta = resultJson["metadata"].toObject();
    QJsonObject stats = resultJson["statistics"].toObject();
    QJsonArray tally = resultJson["tally"].toArray();

    int totalVotes = stats["total_votes_cast"].toInt();

    // 1. Update Stats Labels
    totalTokensLabel->setText(QString("🎫 Total Tokens Scanned: %1").arg(stats["total_tokens_consumed"].toInt()));

    QString rawTime = meta["poll_closed_at"].toString();
    QDateTime closedTime = QDateTime::fromString(rawTime, Qt::ISODate);
    pollClosedLabel->setText("⏱ Polling Closed At: " + closedTime.toString("MMM dd, yyyy - hh:mm AP"));

    // 2. Build the Bar Graph & Find Winner
    int maxVotes = -1;
    QString winnerName = "Tie / Undecided";

    for (int i = 0; i < tally.size(); ++i) {
        QJsonObject candObj = tally[i].toObject();
        int votes = candObj["total_votes"].toInt();
        QString cnic = candObj["candidate_cnic"].toString();
        QString candName = getCandidateName(cnic); // Resolve CNIC to Name

        // Find Winner
        if (votes > maxVotes) {
            maxVotes = votes;
            winnerName = candName;
        }

        // Calculate Percentage
        double percentage = totalVotes > 0 ? ((double)votes / totalVotes) * 100.0 : 0;

        // --- Create Graph Row ---
        QWidget *rowWidget = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 5, 0, 5);

        QLabel *nameLbl = new QLabel(candName, rowWidget);
        nameLbl->setFixedWidth(150);
        nameLbl->setStyleSheet("font-size: 14px; font-weight: bold; color: #34495E; border: none;");

        // The Bar!
        QProgressBar *bar = new QProgressBar(rowWidget);
        bar->setRange(0, totalVotes); // Standardizes the graph width
        bar->setValue(votes);
        bar->setTextVisible(false); // Hide default text
        bar->setFixedHeight(25);
        bar->setStyleSheet("QProgressBar { border: 1px solid #BDC3C7; border-radius: 4px; background: #ECF0F1; }"
                           "QProgressBar::chunk { background-color: #3b5998; border-radius: 3px; }"); // Dark Red bar

        QLabel *pctLbl = new QLabel(QString::number(percentage, 'f', 1) + "% (" + QString::number(votes) + " Votes)", rowWidget);
        pctLbl->setFixedWidth(120);
        pctLbl->setStyleSheet("font-size: 14px; color: #7F8C8D; border: none;");

        rowLayout->addWidget(nameLbl);
        rowLayout->addWidget(bar);
        rowLayout->addWidget(pctLbl);

        chartLayout->addWidget(rowWidget);
    }

    winnerLabel->setText("🏆 Winner: " + winnerName);

    // Show Action Buttons
    saveBtn->show();
    cancelBtn->show();
}

// Mock Database Fetch
QString AdminResultPage::getCandidateName(const QString &cnic) {
    if (cnic == "1234567891011") return "John Doe";
    if (cnic == "42101-222-2") return "Jane Smith";
    if (cnic == "42101-333-3") return "Ahmed Khan";
    return cnic; // Fallback to CNIC if name not found
}

void AdminResultPage::onSaveClicked() {
    QMessageBox::information(this, "Saved", "Results have been permanently published to the online system!");
    clearResults();
}

void AdminResultPage::onCancelClicked() {
    clearResults();
}
