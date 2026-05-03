#include "OfflineAdminDashboard.h"
#include <QMessageBox>
#include <QInputDialog>

OfflineAdminDashboard::OfflineAdminDashboard(QWidget *parent) : QWidget(parent), m_isPaused(false) {
    setupUi();
}

void OfflineAdminDashboard::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // 1. TOP BAR (Red/Black Gradient)
    // ==========================================
    QFrame *topBar = new QFrame(this);
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #101010); border: none; }");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);

    QLabel *titleLabel = new QLabel("POLL WORKER OVERRIDE", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: 900; color: white; letter-spacing: 2px; background: transparent; padding-left: 20px;");

    closeDashBtn = new QPushButton("✖ Close Dashboard", this);
    closeDashBtn->setCursor(Qt::PointingHandCursor);
    closeDashBtn->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0.1); color: white; border: 1px solid white; border-radius: 6px; padding: 8px 15px; font-weight: bold; }"
                                "QPushButton:hover { background-color: rgba(255,255,255,0.2); }");

    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(closeDashBtn);
    mainLayout->addWidget(topBar);

    // ==========================================
    // 2. MAIN DASHBOARD CONTENT
    // ==========================================
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(50, 40, 50, 40);
    contentLayout->setSpacing(30);

    // --- CARD 1: SYSTEM HEALTH & STATS ---
    QFrame *statsCard = new QFrame(this);
    statsCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #BDC3C7; border-radius: 8px; }");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsCard);
    statsLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *statsTitle = new QLabel("📊 System Health & Activity", this);
    statsTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; border: none;");
    tokensScannedLabel = new QLabel("Total Tokens Successfully Scanned: 0", this);
    tokensScannedLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #27AE60; border: none; margin-top: 10px;");

    statsLayout->addWidget(statsTitle);
    statsLayout->addWidget(tokensScannedLabel);
    contentLayout->addWidget(statsCard);

    // --- CARD 2: TIME EXTENSION ---
    QFrame *timeCard = new QFrame(this);
    timeCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #BDC3C7; border-radius: 8px; }");
    QVBoxLayout *timeLayout = new QVBoxLayout(timeCard);
    timeLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *timeTitle = new QLabel("⏱ Election Time Extension", this);
    timeTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; border: none;");

    currentEndTimeLabel = new QLabel("Current End Time: --", this);
    currentEndTimeLabel->setStyleSheet("font-size: 16px; color: #7F8C8D; border: none; margin-bottom: 10px;");
    // --- CARD 2: TIME EXTENSION ---
    // ... (Keep the card setup and title) ...

    QHBoxLayout *timeInputLayout = new QHBoxLayout();

    // REPLACE THE QDateTimeEdit WITH THIS:
    extensionCombo = new QComboBox(this);
    extensionCombo->addItems({
        "+ 15 Minutes",
        "+ 30 Minutes",
        "+ 1 Hour",
        "+ 2 Hours"
    });
    extensionCombo->setStyleSheet("QComboBox { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px 15px; font-size: 16px; font-weight: bold; background-color: #F8F9F9; color: #2C3E50; }"
                                  "QComboBox:focus { border: 2px solid #3498DB; }");
    extensionCombo->setFixedSize(250, 45);
    extensionCombo->setCursor(Qt::PointingHandCursor);

    extendTimeBtn = new QPushButton("Request Extension", this);
    extendTimeBtn->setCursor(Qt::PointingHandCursor);
    extendTimeBtn->setFixedSize(200, 45);
    extendTimeBtn->setStyleSheet("QPushButton { background-color: #3498DB; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #2980B9; }");

    timeInputLayout->addWidget(new QLabel("Extend Voting By:", this)); // Updated label
    timeInputLayout->addWidget(extensionCombo);
    timeInputLayout->addSpacing(20);
    timeInputLayout->addWidget(extendTimeBtn);
    timeInputLayout->addStretch();

    timeLayout->addWidget(timeTitle);
    timeLayout->addWidget(currentEndTimeLabel);
    timeLayout->addLayout(timeInputLayout);
    contentLayout->addWidget(timeCard);

    // --- CARD 3: EMERGENCY CONTROLS ---
    QFrame *emergencyCard = new QFrame(this);
    emergencyCard->setStyleSheet("QFrame { background-color: #FDEDEC; border: 2px solid #E74C3C; border-radius: 8px; }");
    QVBoxLayout *emergencyLayout = new QVBoxLayout(emergencyCard);
    emergencyLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *emergencyTitle = new QLabel("🚨 EMERGENCY CONTROLS", this);
    emergencyTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #C0392B; border: none;");

    QHBoxLayout *emergencyBtnsLayout = new QHBoxLayout();

    pauseBtn = new QPushButton("⏸ Pause Voting", this);
    pauseBtn->setCursor(Qt::PointingHandCursor);
    pauseBtn->setFixedSize(250, 50);
    pauseBtn->setStyleSheet("QPushButton { background-color: #F39C12; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #D68910; }");

    forceCloseBtn = new QPushButton("🛑 EMERGENCY FORCE CLOSE", this);
    forceCloseBtn->setCursor(Qt::PointingHandCursor);
    forceCloseBtn->setFixedSize(300, 50);
    forceCloseBtn->setStyleSheet("QPushButton { background-color: #C0392B; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #922B21; }");

    emergencyBtnsLayout->addWidget(pauseBtn);
    emergencyBtnsLayout->addStretch();
    emergencyBtnsLayout->addWidget(forceCloseBtn);

    emergencyLayout->addWidget(emergencyTitle);
    emergencyLayout->addSpacing(15);
    emergencyLayout->addLayout(emergencyBtnsLayout);
    contentLayout->addWidget(emergencyCard);

    contentLayout->addStretch();
    mainLayout->addWidget(contentWidget);

    // ==========================================
    // CONNECTIONS
    // ==========================================
    connect(closeDashBtn, &QPushButton::clicked, this, [this](){ emit closeDashboardRequested(); });
    connect(pauseBtn, &QPushButton::clicked, this, &OfflineAdminDashboard::onPauseToggled);
    connect(extendTimeBtn, &QPushButton::clicked, this, &OfflineAdminDashboard::onExtendTimeClicked);
    connect(forceCloseBtn, &QPushButton::clicked, this, &OfflineAdminDashboard::onForceCloseClicked);
}

// ==========================================
// LOGIC / DATA INJECTION
// ==========================================
void OfflineAdminDashboard::updateStats(int totalTokensScanned) {
    tokensScannedLabel->setText(QString("Total Tokens Successfully Scanned: %1").arg(totalTokensScanned));
}

void OfflineAdminDashboard::setCurrentEndTime(const QDateTime &currentEnd) {
    currentEndTimeLabel->setText("Current End Time: " + currentEnd.toString("MMM dd, yyyy - hh:mm AP"));

    if(extensionCombo) extensionCombo->setCurrentIndex(0);
}

void OfflineAdminDashboard::setPausedState(bool isPaused) {
    m_isPaused = isPaused;
    if (m_isPaused) {
        pauseBtn->setText("▶ Resume Voting");
        pauseBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #219653; }");
    } else {
        pauseBtn->setText("⏸ Pause Voting");
        pauseBtn->setStyleSheet("QPushButton { background-color: #F39C12; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #D68910; }");
    }
}

// ==========================================
// ACTIONS
// ==========================================
void OfflineAdminDashboard::onPauseToggled() {
    emit pauseVotingRequested(!m_isPaused); // Request the opposite of current state
}

void OfflineAdminDashboard::onExtendTimeClicked() {
    // Figure out how many minutes they selected
    int minutesToAdd = 0;
    switch(extensionCombo->currentIndex()) {
    case 0: minutesToAdd = 15; break;
    case 1: minutesToAdd = 30; break;
    case 2: minutesToAdd = 60; break;
    case 3: minutesToAdd = 120; break;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Extension",
                                                              QString("Are you sure you want to legally extend the voting period by %1 minutes?\n\nThis action will be permanently logged in the Forensic Audit Logs.").arg(minutesToAdd),
                                                              QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        // Emit the minutes to the MainWindow!
        emit extendTimeRequested(minutesToAdd);
    }
}

void OfflineAdminDashboard::onForceCloseClicked() {
    // 1. First Warning
    QMessageBox::StandardButton reply = QMessageBox::critical(this, "CRITICAL WARNING",
                                                              "Are you absolutely sure you want to FORCE CLOSE the election?\n\nThis will immediately lock the terminal. Voters in line will NOT be able to cast their ballots. This action cannot be undone.",
                                                              QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {

        // 2. Open the QDialog to ask for the Master Key
        bool ok;
        QString masterKey = QInputDialog::getText(this,
                                                  "Authorization Required",
                                                  "Please enter the Master Password to confirm shutdown:",
                                                  QLineEdit::Password, // Masks the text with dots!
                                                  "", &ok);

        // 3. Check if they clicked "OK"
        if (ok) {

            if (masterKey.isEmpty()) {
                QMessageBox::warning(this, "Error", "Master Password cannot be empty. Shutdown aborted.");
                return;
            }

            // ==========================================
            // MOCK VERIFICATION (For UI Testing)
            // ==========================================
            // Right now, we will hardcode "admin123" to test the UI.
            // Your backend developer will later replace this with the real hash check!
            if (masterKey == "admin123") {

                // If correct, emit the signal to shut down the machine!
                QMessageBox::information(this, "Verified", "Master Key accepted. Forcing system shutdown...");
                emit forceCloseRequested();

            } else {
                // If wrong, kick them out!
                QMessageBox::critical(this, "Access Denied", "Incorrect Master Password! Emergency shutdown aborted.");
            }
        }
    }
}
