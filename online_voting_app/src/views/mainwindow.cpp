#include "views/mainwindow.h"
#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUuid>
#include "controllers/adminController.h"
#include "controllers/auth_manager.h"
#include "controllers/candidateController.h"
#include "controllers/electionController.h"
#include "controllers/voterController.h"
#include "models/Models.h"
#include "models/entities/election.h"
#include "models/entities/user.h"
#include "services/email/emailservice.h"
#include "ui_mainwindow.h"
#include "views/AdminCandidatePage.h"
#include "votertokenmess.h"
#include <optional>

#define prod

MainWindow::MainWindow(const AppConfig &config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_config(config)
{
    ui->setupUi(this);

    this->setFocus();

    // 1. Create the custom page purely in C++
    m_loginPage = new LoginPage(this);
    m_signupPage = new SignupPage(this); // admin
    m_adminInnerPage_Candidates = new AdminCandidatePage(this);
    m_adminInnerPage_Admins = new AdminManagementPage(this);
    m_adminInnerPage_Elections = new AdminElectionsPage(this);
    m_adminInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    m_adminInnerPage_CreateElection = new AdminCreateElectionPage(this);
    
    // user
    userInnerPage_ActiveElections = new UserActiveElectionsPage(this);
    userInnerPage_MyTokens = new UserMyTokensPage(this);
    userInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    userInnerPage_CandidateDetails->setUserMode(true);
    userInnerPage_Candidacy = new UserCandidacyPage(this);

    // 2. Add it to the Stacked Widget manually
    ui->MainStack->insertWidget(0, m_loginPage);
    ui->MainStack->insertWidget(1, m_signupPage);

    // admin
    ui->adminContentStack->addWidget(m_adminInnerPage_Candidates);
    ui->adminContentStack->addWidget(m_adminInnerPage_Admins);
    ui->adminContentStack->addWidget(m_adminInnerPage_Elections);
    ui->adminContentStack->addWidget(m_adminInnerPage_CandidateDetails);
    ui->adminContentStack->addWidget(m_adminInnerPage_CreateElection);
    // user
    ui->userContentStack->addWidget(userInnerPage_ActiveElections);
    ui->userContentStack->addWidget(userInnerPage_MyTokens);
    ui->userContentStack->addWidget(userInnerPage_CandidateDetails);
    ui->userContentStack->addWidget(userInnerPage_Candidacy);

    ui->MainStack->setCurrentIndex(0);

    connect(m_loginPage,
            &LoginPage::goToSignupRequested,
            this,
            &MainWindow::handleGoToSignupRequested);
    connect(m_loginPage, &LoginPage::loginSuccessUser, this, &MainWindow::handleLoginSuccessUser);
    connect(m_loginPage, &LoginPage::loginSuccessAdmin, this, &MainWindow::handleLoginSuccessAdmin);
    connect(m_loginPage, &LoginPage::loginSuccessAdminPending, this, &MainWindow::handleLoginSuccessAdminPending);
    connect(m_signupPage, &SignupPage::goToLoginRequested, this, &MainWindow::handleGoToLoginRequested);
    connect(m_signupPage, &SignupPage::signupSuccessUser, this, &MainWindow::handleSignupSuccessUser);
    connect(m_signupPage, &SignupPage::signupSuccessAdminPending, this, &MainWindow::handleSignupSuccessAdminPending);
    // admin
    connect(m_adminInnerPage_Candidates, &AdminCandidatePage::electionSelected,
            this, &MainWindow::handleElectionSelectedForCandidates);
    connect(m_adminInnerPage_Elections, &AdminElectionsPage::navigateToCreateElection,
            this, &MainWindow::handleNavigateToCreateElection);
    connect(m_adminInnerPage_Candidates, &AdminCandidatePage::navigateToCandidateDetails,
            this, &MainWindow::handleNavigateToCandidateDetails);
    connect(m_adminInnerPage_CandidateDetails, &AdminCandidateDetailsPage::backBtnClicked,
            this, &MainWindow::handleBackToCandidateList);
    connect(m_adminInnerPage_CandidateDetails, &AdminCandidateDetailsPage::candidateStatusChangeRequested,
            this, &MainWindow::handleCandidateStatusChangeRequested);
    connect(m_adminInnerPage_CreateElection, &AdminCreateElectionPage::backBtnClicked,
            this, &MainWindow::handleBackToElectionList);
    connect(m_adminInnerPage_CreateElection, &AdminCreateElectionPage::createElectionRequested,
            this, &MainWindow::handleCreateElectionSubmit);
    connect(m_adminInnerPage_Elections, &AdminElectionsPage::electionStatusChangeRequested,
            this, &MainWindow::handleElectionStatusChangeRequested);
    connect(m_adminInnerPage_Elections, &AdminElectionsPage::getConfigRequested,
            this, &MainWindow::handleGetConfigurationRequested);
    // user
    connect(userInnerPage_ActiveElections, &UserActiveElectionsPage::electionSelected,
            this, &MainWindow::handleUserElectionSelected);
    connect(userInnerPage_ActiveElections, &UserActiveElectionsPage::generateTokenRequested,
            this, &MainWindow::handleGenerateTokenRequested);
    connect(userInnerPage_MyTokens, &UserMyTokensPage::emailTokenRequested,
            this, &MainWindow::handleEmailTokenRequested);
    connect(userInnerPage_ActiveElections, &UserActiveElectionsPage::navigateToCandidateDetails,
            this, &MainWindow::handleUserNavigateToCandidateDetails);
    connect(userInnerPage_CandidateDetails, &AdminCandidateDetailsPage::backBtnClicked,
            this, &MainWindow::handleUserBackToActiveElections);
    connect(userInnerPage_Candidacy, &UserCandidacyPage::submitApplicationRequested,
            this, &MainWindow::handleCandidacyApplicationSubmit);
    // ADD THIS CONNECTION:
    connect(m_adminInnerPage_Admins,
            &AdminManagementPage::adminStatusChangeRequested,
            this,
            &MainWindow::handleAdminStatusChangeRequested);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Navigation

void MainWindow::handleGoToSignupRequested()
{
    ui->MainStack->setCurrentIndex(1); // SignupPageEnum is at index 1
    this->setFocus();
}

void MainWindow::handleGoToLoginRequested()
{
    ui->MainStack->setCurrentIndex(0); // LoginPage is at index 0
    this->setFocus();
}

// Authentication

void MainWindow::handleLoginSuccessUser()
{
    QString username = AuthManager::getInstance().getCurrentUser()->getName();
    loadAdminProfile(username, "");
    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
}

void MainWindow::handleLoginSuccessAdmin()
{
    QString username = AuthManager::getInstance().getCurrentUser()->getName();
    loadAdminProfile(username, "");
    ui->MainStack->setCurrentIndex(StackedPages::AdminDashPage);
}

void MainWindow::handleLoginSuccessAdminPending()
{
    ui->MainStack->setCurrentIndex(2);
}

void MainWindow::handleSignupSuccessUser()
{
    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
}

void MainWindow::handleSignupSuccessAdminPending()
{
    ui->MainStack->setCurrentIndex(2);
}

void MainWindow::on_adminWaitBackBtn_clicked()
{
    ui->MainStack->setCurrentIndex(StackedPages::SignupPageEnum);
    this->setFocus();
}

void MainWindow::on_btnUserHome_clicked()
{
    ui->userContentStack->setCurrentIndex(0);
    this->setFocus();
}

// void MainWindow::on_btnUserResults_clicked()
// {
//     ui->userContentStack->setCurrentIndex(2);
//     this->setFocus();
// }

void MainWindow::on_btnUserLogout_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Logout Confirmation",
                                  "Are you sure you want to log out of the Pak Voting System?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        AuthManager::getInstance().logout();

        ui->MainStack->setCurrentIndex(StackedPages::Login_Page);
        this->setFocus();
        QMessageBox::information(this, "Logged Out", "You have been securely logged out.");
    }
}

void MainWindow::on_btnAdminLogout_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Logout Confirmation",
                                  "Are you sure you want to log out of the Pak Voting System?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        AuthManager::getInstance().logout();

        ui->MainStack->setCurrentIndex(StackedPages::Login_Page);
        this->setFocus();
        QMessageBox::information(this, "Logged Out", "You have been securely logged out.");
    }
}

void MainWindow::on_adminSidebarCandidatesBtn_clicked()
{
    m_adminInnerPage_Candidates->clearData();
    // 1. Change the nested stacked widget to show the candidate page
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Candidates);

    int electionCount = 0;
    Election *electionList = ElectionController::getInstance().getAllElections(electionCount);

    Election *approvedElections = new Election[0];
    int approvedCount = 0;

    for (int i = 0; i < electionCount; i++)
    {
        if (electionList[i].getStatus() != ElectionState::Rejected &&
            electionList[i].getStatus() != ElectionState::Pending)
        {
            Election *temp = new Election[approvedCount + 1];
            for (int j = 0; j < approvedCount; j++)
            {
                temp[j] = approvedElections[j];
            }
            delete[] approvedElections;
            approvedElections = temp;
            approvedElections[approvedCount] = electionList[i];
            approvedCount++;
        }
    }

    // 3. Load them into the UI
    m_adminInnerPage_Candidates->loadElections(approvedElections, approvedCount);

    // 4. Cleanup memory to prevent leaks!
    delete[] electionList;
    delete[] approvedElections;
}

void MainWindow::handleElectionSelectedForCandidates(QString electionId)
{
    int candidateCount = 0;
    Candidate *candidatesForElection = CandidateController::getInstance()
                                           .getCandidatesByElection(electionId,
                                                                    candidateCount,
                                                                    true);
    m_adminInnerPage_Candidates->loadCandidates(candidatesForElection, candidateCount);

    delete[] candidatesForElection;
}

void MainWindow::on_adminSidebarAdminsBtn_clicked()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Admins);
#ifdef prod
    if (!AuthManager::getInstance().isLoggedIn() || !AuthManager::getInstance().getCurrentUser())
    {
        QMessageBox::critical(this, "Authentication Error", "No authenticated admin found. Please log in again.");
        return;
    }
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

#endif

#ifdef deve

    QString currentAdminId = "ROOT_001";
    QString currentUserCnic = "00000-0000000-1";

#endif

    int adminCount = 0;
    auto adminOpt = AdminController::getInstance().getAllAdminsExcept(currentUserCnic, adminCount);

    if (!adminOpt)
    {
        QMessageBox::critical(this, "Error", "Failed to load admins.");
        return;
    }

    Admin *adminList = adminOpt.value();
    m_adminInnerPage_Admins->loadAdmins(adminList, adminCount, currentUserCnic);

    delete[] adminList;
}
// ---------------------------------------------------------
// Triggered when the Admin clicks "Elections" on Left Sidebar
// ---------------------------------------------------------
void MainWindow::on_adminSidebarElectionsBtn_clicked()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Elections);
#ifdef prod
    if (!AuthManager::getInstance().isLoggedIn() || !AuthManager::getInstance().getCurrentUser())
    {
        QMessageBox::critical(this,
                              "Authentication Error",
                              "No authenticated admin found. Please log in again.");
        return;
    }
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
#endif

#ifdef deve
    QString currentAdminCnic = "ROOT_001";
#endif

    int electionCount = 0;
    Election *electionList = ElectionController::getInstance().getAllElections(electionCount);

    if (!electionList)
    {
        QMessageBox::critical(this, "Error", "Failed to load elections.");
        return;
    }

    m_adminInnerPage_Elections->loadElections(electionList, electionCount, currentAdminCnic);

    delete[] electionList;
}

// ---------------------------------------------------------
// Navigation: Go TO Details Page
// ---------------------------------------------------------
void MainWindow::handleNavigateToCandidateDetails(Candidate selectedCandidate)
{
    // 1. Pass the data to the page UI
    m_adminInnerPage_CandidateDetails->setCandidate(selectedCandidate);

    // 2. Tell the StackedWidget to change screens!
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_CandidateDetails);
}

// ---------------------------------------------------------
// Navigation: Go BACK to List
// ---------------------------------------------------------
void MainWindow::handleBackToCandidateList()
{
    // Change the screen back to the candidates list

    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Candidates);
}

// ---------------------------------------------------------
// Action: Test the Approve/Reject Buttons
// ---------------------------------------------------------
void MainWindow::handleCandidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus)
{
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    bool success = CandidateController::getInstance().requestCandidateStatusChange(targetCnic,
                                                                                   currentAdminCnic,
                                                                                   newStatus);
    if (success)
    {
        QMessageBox::information(this, "Success", "Status change request logged.");
        handleBackToCandidateList();            // Kick them back to the list
        on_adminSidebarCandidatesBtn_clicked(); // Refresh the list to show updated status
    }
    else
    {
        QMessageBox::critical(this,
                              "Error",
                              "Failed to log status change request. Please try again.");
    }
}

void MainWindow::handleElectionStatusChangeRequested(QString electionId, ApprovalStatus newStatus)
{
    // 1. Get the current logged-in Admin's CNIC
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

    // 2. Send it to the backend ElectionController
    bool success = ElectionController::getInstance().requestElectionStatusChange(electionId,
                                                                                 currentAdminCnic,
                                                                                 newStatus);

    if (success)
    {
        QMessageBox::information(this,
                                 "Success",
                                 "Election status change request logged successfully.\n\nIt will "
                                 "change state once the required threshold of Admins approve it.");
    }
    else
    {
        QMessageBox::critical(this,
                              "Error",
                              "Failed to log election status change. Ensure you are an approved "
                              "Admin and haven't already voted on this election.");
    }
}

void MainWindow::handleAdminStatusChangeRequested(QString currentUserId,
                                                  QString targetCnic,
                                                  ApprovalStatus newStatus)
{
    // 1. Get the current logged-in Admin's CNIC
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

    // 2. Send it to the backend AdminController
    bool success = AdminController::getInstance().addStatusChangeRequest(targetCnic,
                                                                         currentAdminCnic,
                                                                         newStatus);

    if (success)
    {
        QMessageBox::information(this,
                                 "Success",
                                 "Administrator status change request logged successfully.");
    }
    else
    {
        QMessageBox::critical(this,
                              "Error",
                              "Failed to log Admin status change. Ensure you are an approved Admin "
                              "and haven't already voted on this user.");
    }
}

void MainWindow::handleNavigateToCreateElection()
{
    // Important: Reset the dates to ensure they calculate "2 days from TODAY" accurately
    m_adminInnerPage_CreateElection->resetForm();

    // Swap the screen
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_CreateElection);
}

void MainWindow::handleBackToElectionList()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Elections);
}

// ---------------------------------------------------------
// Triggered when Admin clicks "Create Election"
// ---------------------------------------------------------
void MainWindow::handleCreateElectionSubmit(QString title, QDateTime publishTime, QDateTime startTime, QDateTime endTime)
{
    QString id = "ELEC-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper();
    Election newElection;
    newElection.setId(id);
    newElection.setTitle(title);
    newElection.setPublishTime(publishTime);
    newElection.setStartTime(startTime);
    newElection.setEndTime(endTime);
    bool success = ElectionController::getInstance().createElection(newElection);
    if (success)
    {
        QMessageBox::information(this, "Success", "Election created as Pending  \nWaiting for other admins to approve.");
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to create election.");
    }
    handleBackToElectionList();            // Go back to list after success
    on_adminSidebarElectionsBtn_clicked(); // Refresh the list to show the new election
}

//---------------user-pages---------------
void MainWindow::on_userSidebarActiveElectionsBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);
    int electionCount = 0;
    Election *activeElections = ElectionController::getInstance().getElectionsForUser(electionCount);
    userInnerPage_ActiveElections->loadElections(activeElections, electionCount);
    delete[] activeElections;
}

void MainWindow::handleUserElectionSelected(QString electionId)
{
    // When the user clicks an election, load the mock candidates for it
    int candidateCount = 0;
    Candidate *candidatsForElection = CandidateController::getInstance()
                                          .getCandidatesByElection(electionId,
                                                                   candidateCount,
                                                                   false);

    int approvedCandidateCount = 0;
    Candidate *approvedCandidates = new Candidate[0];

    for (int i = 0; i < candidateCount; i++)
    {
        if (candidatsForElection[i].getStatus() == ApprovalStatus::Approved)
        {
            Candidate *temp = new Candidate[approvedCandidateCount + 1];
            for (int j = 0; j < approvedCandidateCount; j++)
            {
                temp[j] = approvedCandidates[j];
            }
            delete[] approvedCandidates;
            approvedCandidates = temp;
            approvedCandidates[approvedCandidateCount] = candidatsForElection[i];
            approvedCandidateCount++;
        }
    }

    userInnerPage_ActiveElections->loadCandidates(approvedCandidates, approvedCandidateCount);
    delete[] approvedCandidates;
    delete[] candidatsForElection;
}

// ---------------------------------------------------------
// Triggered when User clicks "Register & Generate Token"
// ---------------------------------------------------------
void MainWindow::handleGenerateTokenRequested(QString electionId)
{
#ifdef prod
    QString userEmail = AuthManager::getInstance().getCurrentUser()->getEmail();
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
#endif
#ifdef deve
    QString userEmail = "pak.evm.project@gmail.com";
    QString currentUserCnic = "00000-0000000-1";
#endif
    auto tokenOpt = TokenController::getInstance().createAndSaveToken(currentUserCnic,
                                                                      electionId,
                                                                      m_config.privateKey);
    if (!tokenOpt.has_value())
    {
        QMessageBox::critical(this, "Error", "Failed to generate token. Please try again.");
        return;
    }
    Token newToken = tokenOpt.value();

    auto tokenQrCodeOpt = TokenController::getInstance().getQrCodeForToken(newToken);

    if (!tokenQrCodeOpt.has_value())
    {
        QMessageBox::critical(this, "Error", "Token created but Failed to generate QR code.");
        return;
    }

    QImage tokenQrCode = tokenQrCodeOpt.value();
    // converting image to base64 string
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    // 3. Open the buffer so we can write to it
    buffer.open(QIODevice::WriteOnly);

    // 4. "Save" the image into the buffer in PNG format
    // (PNG is best because it preserves transparency/backgrounds)
    tokenQrCode.save(&buffer, "PNG");

    // 5. Convert the raw bytes into a safe Base64 string
    QByteArray base64Bytes = byteArray.toBase64();

    bool sendTokenToEmailSuccess = TokenController::getInstance().sendTokenToEmail(newToken,
                                                                                   userEmail);

    if (!sendTokenToEmailSuccess)
    {
        QMessageBox::critical(this, "Error", "Token created by failed to be send to mail");
        return;
    }

    // 2. Create the Dialog using the Designer class
    VoterTokenMess tokenPopup(base64Bytes, electionId, this);

    // 3. Show it modally (blocks the rest of the app until they click OK)
    tokenPopup.exec();
}

void MainWindow::on_userSidebarMyTokensBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_MyTokens);

    int tokenCount = 0;
#ifdef prod
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
#endif
#ifdef deve
    QString currentUserCnic = "00000-0000000-1";
#endif
    Token *issuedTokens = TokenController::getInstance().getVoterTokens(currentUserCnic, tokenCount);

    userInnerPage_MyTokens->loadTokens(issuedTokens, tokenCount);
    delete[] issuedTokens;
}

// ---------------------------------------------------------
// Triggered when User clicks "✉ Send to Email" on a Token
// ---------------------------------------------------------
void MainWindow::handleEmailTokenRequested(Token selectedToken)
{
#ifdef deve
    QString userEmail = "pak.evm.project@gmail.com";
#endif
#ifdef prod
    QString userEmail = AuthManager::getInstance().getCurrentUser()->getEmail();
#endif
    bool emailSuccess = TokenController::getInstance().sendTokenToEmail(selectedToken, userEmail);

    if (emailSuccess)
    {
        QMessageBox::information(this, "Success", "Token sent to your email!");
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to send token email. Please try again.");
    }
}
// Note: When you load your mock candidates in handleUserElectionSelected,
// you can now add mockBase64 strings if you want to test the symbol images!

void MainWindow::handleUserNavigateToCandidateDetails(Candidate selectedCandidate)
{
    // Pass data to the UI
    userInnerPage_CandidateDetails->setCandidate(selectedCandidate);

    // Change the USER stacked widget screen
    ui->userContentStack->setCurrentWidget(userInnerPage_CandidateDetails);
}

void MainWindow::handleUserBackToActiveElections()
{
    // Go back to the active elections list
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);
}

// ---------------------------------------------------------
// Triggered when User clicks "Locate Station"
// ---------------------------------------------------------
void MainWindow::on_userSidebarLocateStationBtn_clicked()
{
    QMessageBox::information(this, "Coming Soon",
                             "📍 The Locate Station feature is currently under development and will be available soon!");
}

// ---------------------------------------------------------
// Triggered when User clicks "Run for Office" on Sidebar
// ---------------------------------------------------------
void MainWindow::on_userSidebarCandidacyBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_Candidacy);

    int electionCount = 0;
    Election *draftedElections = ElectionController::getInstance().getElectionsByStatus(ElectionState::Drafted, electionCount);

    userInnerPage_Candidacy->loadPublishedElections(draftedElections, electionCount);
    delete[] draftedElections;
}

// ---------------------------------------------------------
// Triggered when User clicks "Submit Application" on Candidacy Page
// ---------------------------------------------------------
void MainWindow::handleCandidacyApplicationSubmit(Candidate newCandidate)
{
#ifdef deve
    QString currentUserCnic = "1234567891011";
    QString currentUserName = "John Doe";
#endif
#ifdef prod
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    QString currentUserName = AuthManager::getInstance().getCurrentUser()->getName();
#endif
    newCandidate.setUserCnic(currentUserCnic);

    newCandidate.setStatus(ApprovalStatus::Pending);
    newCandidate.setId("CAND-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper());

    bool success = CandidateController::getInstance().createCandidate(newCandidate);
    if (!success)
    {
        QMessageBox::critical(this, "Error", "Failed to submit application. Please try again.");
        return;
    }

    QMessageBox::information(this, "Application Submitted",
                             "Your candidacy application for '" + newCandidate.getPartyName() + "' has been successfully submitted!\n\n"
                                                                                                "Please wait for admin approval. You will see your status update in the Active Elections tab once approved.");
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);
}

// ---------------------------------------------------------
// Triggered when Admin clicks "Get Configuration" on Election
// ---------------------------------------------------------
void MainWindow::handleGetConfigurationRequested(QString electionId)
{
    QMessageBox::information(this,
                             "Get Configuration",
                             "Ready to fetch configuration for Election ID:" + electionId);
    QString AdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    ElectionController::getInstance().sendElectionConfigToAdmin(electionId,
                                                                AdminCnic,
                                                                m_config.privateKey,
                                                                m_config.publicKey);
}

// Helping Functions

void MainWindow::loadUserProfile(const QString &fullName, const QString &imagePath)
{
    // 1. Set the Dynamic Name
    ui->userNameLabel->setText("Hello, " + fullName);

    // 2. Load the Dynamic Image
    QPixmap originalImage;

    // Check if the user has an uploaded image path, and if the file actually exists
    if (!imagePath.isEmpty() && QFile::exists(imagePath))
    {
        originalImage.load(imagePath);
    }
    else
    {
        // Fallback: If they haven't uploaded one, load a default silhouette from your resources
        originalImage.load(":/images/white_default_profpic.png");
    }

    // 3. Make the Image a Perfect Circle
    QPixmap circularImage(50, 50);
    circularImage.fill(Qt::transparent);

    QPainter painter(&circularImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, 50, 50);
    painter.setClipPath(path);

    // Scale the image down so it fits nicely inside the 50x50 circle
    QPixmap scaledOriginal = originalImage.scaled(50,
                                                  50,
                                                  Qt::KeepAspectRatioByExpanding,
                                                  Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledOriginal);

    // 4. Apply to the Button
    ui->userProfilePicBtn->setIcon(QIcon(circularImage));
    ui->userProfilePicBtn->setIconSize(QSize(50, 50));
}
void MainWindow::loadAdminProfile(const QString &fullName, const QString &imagePath)
{
    // 1. Set the Dynamic Name
    ui->adminNameLabel->setText("Hello, " + fullName);

    // 2. Load the Dynamic Image
    QPixmap originalImage;

    // Check if the user has an uploaded image path, and if the file actually exists
    if (!imagePath.isEmpty() && QFile::exists(imagePath))
    {
        originalImage.load(imagePath);
    }
    else
    {
        // Fallback: If they haven't uploaded one, load a default silhouette from your resources
        originalImage.load(":/images/white_default_profpic.png");
    }

    // 3. Make the Image a Perfect Circle
    QPixmap circularImage(50, 50);
    circularImage.fill(Qt::transparent);

    QPainter painter(&circularImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, 50, 50);
    painter.setClipPath(path);

    // Scale the image down so it fits nicely inside the 50x50 circle
    QPixmap scaledOriginal = originalImage.scaled(50,
                                                  50,
                                                  Qt::KeepAspectRatioByExpanding,
                                                  Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledOriginal);

    // 4. Apply to the Button
    ui->adminProfilePicBtn->setIcon(QIcon(circularImage));
    ui->adminProfilePicBtn->setIconSize(QSize(50, 50));
}
