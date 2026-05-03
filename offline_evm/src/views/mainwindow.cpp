#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "controllers/auth_manager.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Auto-generated UI from designer

    // Call our newly organized function
    setupKioskUi();
}

MainWindow::~MainWindow()
{
    if (scanPage) {
        scanPage->stopCamera();
    }
    delete ui;
}

// ==========================================
// KIOSK UI & LOGIC INITIALIZATION
// ==========================================
void MainWindow::setupKioskUi() {
    // 1. Main Window Properties
    this->setWindowTitle("Offline EVM System");
    this->resize(1000, 650);

    // 2. Initialize Pages
    preElectionPage = new PreElectionPage(this);
    postElectionPage = new PostElectionPage(this);
    scanPage = new EvmScanPage(this);
    votingPage = new EvmVotingPage(this);

    // 3. Setup the Master Stacked Widget
    mainKioskStack = new QStackedWidget(this);
    this->setCentralWidget(mainKioskStack);

    mainKioskStack->addWidget(preElectionPage);
    mainKioskStack->addWidget(scanPage);
    mainKioskStack->addWidget(votingPage);
    mainKioskStack->addWidget(postElectionPage);

    // 4. Set Election Times (Mock data for testing)
    currentElectionStartTime = QDateTime::currentDateTime().addSecs(5); // Starts in 5 seconds
    currentElectionEndTime = currentElectionStartTime.addSecs(460);      // Ends 15 seconds later

    // 5. Setup Timers & Connections
    kioskHeartbeat = new QTimer(this);
    connect(kioskHeartbeat, &QTimer::timeout, this, &MainWindow::onHeartbeatTick);
    kioskHeartbeat->start(1000); // 1-second heartbeat

    connect(scanPage, &EvmScanPage::frameReadyForBackend,
            this, &MainWindow::processCameraString);
    connect(scanPage, &EvmScanPage::proceedToVotingClicked,
            this, &MainWindow::handleProceedToVoting);
    connect(votingPage, &EvmVotingPage::candidateVoted, this,[this](Candidate selected) {

        QMessageBox::information(this, "Vote Cast", "You successfully voted for: " + selected.getPartyName());

        // 1. Reset the scanner for the NEXT voter
        scanPage->resetScanner();

        // 2. Send the screen back to the scanner
        mainKioskStack->setCurrentWidget(scanPage);
    });
}

// ==========================================
// CAMERA PROCESSOR
// ==========================================
void MainWindow::processCameraString(QString base64ImageString, QString cnic) {

    // TEMPORARY MOCK: Since BackendCrypto isn't built yet, we will just pretend
    // the camera successfully found a JSON string so you can test the UI!
    QString qrPayload = "{\"tokenId\":\"TKN-123\", \"electionId\":\"ELEC-001\", \"issuedAt\":\"2026-05-02\", \"signature\":\"mock_sig\"}";

    // If you want to actually test the camera frame dropping, you can leave this:
    // if (qrPayload.isEmpty()) return;

    verifyScannedToken(cnic, qrPayload);
}

// ==========================================
// BUSINESS LOGIC: Verifies the token payload
// ==========================================
void MainWindow::verifyScannedToken(QString cnic, QString qrPayload) {

    QString out_tokenId;

    // Ask AuthManager to run the heavy cryptography checks
    AuthManager::TokenResult result = AuthManager::getInstance().verifyVoterToken(cnic, qrPayload, out_tokenId);

    // Handle the UI feedback based on the exact result
    switch (result) {
    case AuthManager::TokenResult::Valid:
        scanPage->markScanSuccessful(out_tokenId);
        break;

    case AuthManager::TokenResult::AlreadyUsed:
        QMessageBox::critical(this, "Fraud Alert",
                              "SECURITY LOCKOUT: This token has already been used to cast a ballot!");
        break;

    case AuthManager::TokenResult::InvalidSignature:
        QMessageBox::critical(this, "Verification Failed",
                              "Cryptographic signature is invalid. Ensure your CNIC is typed correctly and you are using an official token.");
        break;

    case AuthManager::TokenResult::ParseError:
        QMessageBox::warning(this, "Scan Error",
                             "Unrecognized QR Code format. Please scan a valid PAK.EVM token.");
        break;
    }
}

// ==========================================
// KIOSK STATE MACHINE (HEARTBEAT)
// ==========================================
void MainWindow::onHeartbeatTick() {
    QDateTime now = QDateTime::currentDateTime();

    // STATE 1: PRE-ELECTION (Waiting to start)
    if (now < currentElectionStartTime) {
        if (mainKioskStack->currentWidget() != preElectionPage) {
            mainKioskStack->setCurrentWidget(preElectionPage);
        }

        qint64 secondsLeft = now.secsTo(currentElectionStartTime);
        preElectionPage->updateCountdown(formatTime(secondsLeft));
    }

    // STATE 2: ACTIVE ELECTION (Scanning OR Voting)
    else if (now >= currentElectionStartTime && now < currentElectionEndTime) {

        QWidget *currentScreen = mainKioskStack->currentWidget();

        // ONLY force the screen to the scan page if we are coming from the Pre-Election page.
        // If the user is currently on the Voting Page, leave them alone!
        if (currentScreen != scanPage && currentScreen != votingPage) {
            mainKioskStack->setCurrentWidget(scanPage);
        }

        // Calculate time left until END
        qint64 secondsLeft = now.secsTo(currentElectionEndTime);
        QString timeString = formatTime(secondsLeft);

        // Update the clock on BOTH pages so the user sees it while voting!
        scanPage->updateTimeRemaining(timeString);
        votingPage->updateTimeRemaining(timeString);
    }

    // STATE 3: POST-ELECTION (Finished)
    else {
        // Ensure we are on the Closed screen
        if (mainKioskStack->currentWidget() != postElectionPage) {
            mainKioskStack->setCurrentWidget(postElectionPage);

            // Lock down the app, stop the camera, stop the timer
            scanPage->stopCamera();
            kioskHeartbeat->stop();
        }
    }
}
// ==========================================
// HELPERS
// ==========================================
QString MainWindow::formatTime(qint64 totalSeconds) {
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

void MainWindow::handleProceedToVoting() {

    //scanPage->stopCamera();
    // 1. Load mock candidates into the voting page
    int candidateCount = 3;
    Candidate* mockCandidates = new Candidate[candidateCount];

    // Mock Candidate 1
    mockCandidates[0].setName("Choco");
    mockCandidates[0].setCnic("42101-111-1");
    mockCandidates[0].setPartyName("Democratic Front");
    mockCandidates[0].setSymbolName("Eagle");
    // (Add mock Base64 strings for images if you have them)

    // Mock Candidate 2
    mockCandidates[1].setCnic("42101-222-2");
    mockCandidates[1].setPartyName("Liberty Party");
    mockCandidates[1].setSymbolName("Tiger");

    // Mock Candidate 3
    mockCandidates[2].setCnic("42101-333-3");
    mockCandidates[2].setPartyName("Justice Alliance");
    mockCandidates[2].setSymbolName("Book");


    votingPage->loadCandidates(mockCandidates, candidateCount);
    delete[] mockCandidates;

    // 2. Switch the screen to the voting page!
    mainKioskStack->setCurrentWidget(votingPage);
}
