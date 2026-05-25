#include "mainwindow.h"
#include <QMessageBox>
#include "controllers/auth_manager.h"
#include "controllers/candidate_controller.h"
#include "controllers/election_controller.h"
#include "controllers/system_controller.h"
#include "ui_mainwindow.h"

#define deve

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Auto-generated UI from designer

    // Call our newly organized function
    setupKioskUi();
}

MainWindow::~MainWindow()
{
    if (scanPage)
    {
        scanPage->stopCamera();
    }
    delete ui;
}

// ==========================================
// KIOSK UI & LOGIC INITIALIZATION
// ==========================================
void MainWindow::setupKioskUi()
{
    // 1. Main Window Properties
    this->setWindowTitle("Offline EVM System");
    this->resize(1000, 650);

    m_systemIsPaused = false;

    // 2. Initialize Pages
    preElectionPage = new PreElectionPage(this);
    postElectionPage = new PostElectionPage(this);
    scanPage = new EvmScanPage(this);
    votingPage = new EvmVotingPage(this);
    adminDashboard = new OfflineAdminDashboard(this);
    adminAuthPage = new OfflineAdminAuthPage(this);

    // 3. Setup the Master Stacked Widget
    mainKioskStack = new QStackedWidget(this);
    this->setCentralWidget(mainKioskStack);

    mainKioskStack->addWidget(preElectionPage);
    mainKioskStack->addWidget(scanPage);
    mainKioskStack->addWidget(votingPage);
    mainKioskStack->addWidget(postElectionPage);
    mainKioskStack->addWidget(adminDashboard);
    mainKioskStack->addWidget(adminAuthPage);
#ifdef prod
    // 4. Set Election Times (Mock data for testing)
    currentElectionStartTime = QDateTime::fromMSecsSinceEpoch(
        SystemController::getInstance()
            .getSystemConfig()
            .getScheduledStartTime()); // Starts in 5 seconds
    currentElectionEndTime = QDateTime::fromMSecsSinceEpoch(
        SystemController::getInstance()
            .getSystemConfig()
            .getScheduledEndTime()); // Ends 15 seconds later
#endif

#ifdef deve
    currentElectionStartTime = QDateTime::currentDateTime().addSecs(5); // Starts in 5 seconds
    currentElectionEndTime = currentElectionStartTime.addSecs(1000);    // Ends 15 seconds later
#endif
    // 5. Setup Timers & Connections
    kioskHeartbeat = new QTimer(this);
    connect(kioskHeartbeat, &QTimer::timeout, this, &MainWindow::onHeartbeatTick);
    kioskHeartbeat->start(1000); // 1-second heartbeat

    connect(scanPage, &EvmScanPage::frameReadyForBackend, this, &MainWindow::processCameraString);
    connect(scanPage,
            &EvmScanPage::proceedToVotingClicked,
            this,
            &MainWindow::handleProceedToVoting);
    connect(votingPage, &EvmVotingPage::candidateVoted, this, &MainWindow::handleCandidateVoted);
    connect(adminDashboard,
            &OfflineAdminDashboard::pauseVotingRequested,
            this,
            &MainWindow::handleEmergencyPause);
    connect(adminDashboard,
            &OfflineAdminDashboard::extendTimeRequested,
            this,
            &MainWindow::handleTimeExtension);
    connect(adminDashboard,
            &OfflineAdminDashboard::forceCloseRequested,
            this,
            &MainWindow::handleEmergencyForceClose);
    connect(adminDashboard,
            &OfflineAdminDashboard::closeDashboardRequested,
            this,
            &MainWindow::handleCloseAdminDashboard);
    connect(scanPage,
            &EvmScanPage::secretAdminDashboardRequested,
            this,
            &MainWindow::handleSecretKnockDetected);
    connect(adminAuthPage,
            &OfflineAdminAuthPage::authSuccessful,
            this,
            &MainWindow::handleAdminAuthSuccess);
    connect(adminAuthPage,
            &OfflineAdminAuthPage::backToScanRequested,
            this,
            &MainWindow::handleAdminAuthBack);
}

// ==========================================
// CAMERA PROCESSOR
// ==========================================

void MainWindow::processCameraString(QString qrPayload, QString cnic)
{
    verifyScannedToken(cnic, qrPayload);
}

// ==========================================
// BUSINESS LOGIC: Verifies the token payload
// ==========================================

void MainWindow::verifyScannedToken(QString cnic, QString qrPayload)
{
    qInfo() << "Received QR Payload for verification:\n " << qrPayload;
    QString out_tokenId;

    // Ask AuthManager to run the heavy cryptography checks
    AuthManager::TokenResult result = AuthManager::getInstance().verifyVoterToken(cnic,
                                                                                  qrPayload,
                                                                                  out_tokenId);

    // Handle the UI feedback based on the exact result
    switch (result)
    {
    case AuthManager::TokenResult::Valid:
        m_activeTokenId = out_tokenId; // Store the valid token ID for the voting phase
        scanPage->markScanSuccessful(out_tokenId);
        break;

    case AuthManager::TokenResult::AlreadyUsed:
        QMessageBox::critical(
            this,
            "Fraud Alert",
            "SECURITY LOCKOUT: This token has already been used to cast a ballot!");
        break;

    case AuthManager::TokenResult::InvalidSignature:
        QMessageBox::critical(this,
                              "Verification Failed",
                              "Cryptographic signature is invalid. Ensure your CNIC is typed "
                              "correctly and you are using an official token.");
        break;

    case AuthManager::TokenResult::ParseError:
        QMessageBox::warning(this,
                             "Scan Error",
                             "Unrecognized QR Code format. Please scan a valid PAK.EVM token.");
        break;
    }
}

// ==========================================
// KIOSK STATE MACHINE (HEARTBEAT)
// ==========================================
void MainWindow::onHeartbeatTick()
{
    QDateTime now = QDateTime::currentDateTime();

    // Fetch the actual current state from the database
    ElectionState dbState = ElectionController::getInstance().getCurrentState();

    // STATE 1: PRE-ELECTION (Waiting to start)
    if (now < currentElectionStartTime)
    {
        if (mainKioskStack->currentWidget() != preElectionPage)
        {
            mainKioskStack->setCurrentWidget(preElectionPage);
        }

        // Only calculate based on the current overridden times
        qint64 secondsLeft = now.secsTo(currentElectionStartTime);
        preElectionPage->updateCountdown(formatTime(secondsLeft));
    }

    // STATE 2: ACTIVE ELECTION (Scanning OR Voting)
    else if (now >= currentElectionStartTime && now < currentElectionEndTime)
    {

        // [CRITICAL FIX] If DB still says ReadyWaiting (1), officially open it!
        if (dbState == ElectionState::ReadyWaiting || dbState == ElectionState::Setup)
        {
            ElectionController::getInstance().autoOpenElection(); // Updates DB to State 2
        }

        QWidget *currentScreen = mainKioskStack->currentWidget();

        if (currentScreen != scanPage && currentScreen != votingPage && currentScreen != adminDashboard && currentScreen != adminAuthPage)
        {
            mainKioskStack->setCurrentWidget(scanPage);
        }

        qint64 secondsLeft = now.secsTo(currentElectionEndTime);
        QString timeString = formatTime(secondsLeft);

        scanPage->updateTimeRemaining(timeString);
        votingPage->updateTimeRemaining(timeString);
    }

    // STATE 3: POST-ELECTION (Finished)
    else
    {

        // [CRITICAL FIX] If DB still says Open (2), officially close it!
        if (dbState == ElectionState::Open)
        {
            ElectionController::getInstance().autoCloseElection(); // Updates DB to State 4
        }

        if (mainKioskStack->currentWidget() != postElectionPage)
        {
            mainKioskStack->setCurrentWidget(postElectionPage);
            kioskHeartbeat->stop();
        }
    }
}
// ==========================================
// HELPERS
// ==========================================
QString MainWindow::formatTime(qint64 totalSeconds)
{
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

void MainWindow::handleProceedToVoting()
{
    scanPage->stopCamera();
    // 1. Load mock candidates into the voting page
    int candidateCount = 0;
    Candidate *candidateList = CandidateController::getInstance().getAllCandidates(candidateCount);

    votingPage->loadCandidates(candidateList, candidateCount);
    delete[] candidateList;
    // 2. Switch the screen to the voting page!
    mainKioskStack->setCurrentWidget(votingPage);
}

void MainWindow::handleCloseAdminDashboard()
{
    // Send them back to the scanner
    scanPage->resetScanner();
    mainKioskStack->setCurrentWidget(scanPage);
}

void MainWindow::handleEmergencyPause(bool pause)
{
    m_systemIsPaused = pause;
    adminDashboard->setPausedState(pause);

    if (pause)
    {
        // Mute the heartbeat UI updates or switch to a "PAUSED" screen
        // AuditLogRepo->insertLog({"EMERGENCY_PAUSE", "Poll worker paused the terminal."});
    }
    else
    {
        // AuditLogRepo->insertLog({"EMERGENCY_RESUME", "Poll worker resumed the terminal."});
    }
}

void MainWindow::handleTimeExtension(int minutesToAdd)
{
    // 1. Mathematically add the minutes to the current end time
    currentElectionEndTime = currentElectionEndTime.addSecs(minutesToAdd * 60);

    // 2. Instantly update the dashboard UI so the Admin sees the new time
    adminDashboard->setCurrentEndTime(currentElectionEndTime);

    // 3. Show Success Message
    QMessageBox::information(
        this,
        "Extension Applied",
        QString("Success! The election has been extended by %1 minutes.\nNew End Time: %2")
            .arg(minutesToAdd)
            .arg(currentElectionEndTime.toString("hh:mm AP")));

    // WHEN READY: AuditLogRepo->insertLog({"TIME_EXTENSION", QString("Extended by %1 mins").arg(minutesToAdd)});
}

void MainWindow::handleEmergencyForceClose()
{
    // Instantly force the End Time to right now!
    // The Heartbeat timer will catch this 1 second later and automatically lock down the machine!
    currentElectionEndTime = QDateTime::currentDateTime();

    // AuditLogRepo->insertLog({"FORCE_CLOSE", "Poll worker triggered emergency shutdown."});
}

// ---------------------------------------------------------
// EMERGENCY KIOSK OVERRIDE WORKFLOW
// ---------------------------------------------------------
void MainWindow::handleSecretKnockDetected()
{
    // 1. 5 Taps Detected! Go to Auth Page first.
    adminAuthPage->resetForm();
    // scanPage->stopCamera(); // Pause camera to save CPU
    mainKioskStack->setCurrentWidget(adminAuthPage);
}

void MainWindow::handleAdminAuthBack()
{
    // Admin clicked Back, return to scanner.
    // scanPage->resetScanner();
    mainKioskStack->setCurrentWidget(scanPage);
}

void MainWindow::handleAdminAuthSuccess(QString adminCnic)
{
    // 2. Auth Success! Now we actually open the Secret Dashboard.

    adminDashboard->setCurrentEndTime(currentElectionEndTime);
    adminDashboard->setPausedState(m_systemIsPaused);
    // pass the adminCnic to the dashboard if needed for logging

    mainKioskStack->setCurrentWidget(adminDashboard);
}

void MainWindow::handleCandidateVoted(Candidate selectedCandidate)
{
    if (m_activeTokenId.isEmpty())
    {
        QMessageBox::critical(this, "Session Error", "No active token found! Session aborted.");
        scanPage->resetScanner();
        mainKioskStack->setCurrentWidget(scanPage);
        return;
    }

    // 2. Call the Backend to Cast the Vote!
    QByteArray receiptHash;
    bool success = ElectionController::getInstance().castVote(
        selectedCandidate.getCnic(),
        m_activeTokenId,
        receiptHash);

    // 3. Provide Feedback to the Voter
    if (success)
    {
        QMessageBox::information(this,
                                 "Vote Cast Successfully",
                                 "You successfully voted for: " + selectedCandidate.getPartyName() +
                                     "\n\nYour Anonymous Receipt Hash:\n" + QString(receiptHash));
    }
    else
    {
        QMessageBox::critical(this,
                              "Vote Failed",
                              "An error occurred while securing your ballot. Please contact a poll worker.");
    }

    // 4. [SECURITY] Wipe the token from the UI layer immediately
    m_activeTokenId.clear();

    // 5. Reset the scanner and return to the start screen for the NEXT voter
    scanPage->resetScanner();
    mainKioskStack->setCurrentWidget(scanPage);
}