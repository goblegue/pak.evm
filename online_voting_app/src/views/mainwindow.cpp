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

#define deve

MainWindow::MainWindow(const AppConfig &config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_config(config)
{
    ui->setupUi(this);

    this->setFocus();

    
    m_loginPage = new LoginPage(this);
    m_signupPage = new SignupPage(this); // admin
    m_adminInnerPage_Candidates = new AdminCandidatePage(this);
    m_adminInnerPage_Admins = new AdminManagementPage(this);
    m_adminInnerPage_Elections = new AdminElectionsPage(this);
    m_adminInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    m_adminInnerPage_CreateElection = new AdminCreateElectionPage(this);
    
    userInnerPage_ActiveElections = new UserActiveElectionsPage(this);
    userInnerPage_MyTokens = new UserMyTokensPage(this);
    userInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    userInnerPage_CandidateDetails->setUserMode(true); 
    userInnerPage_Candidacy = new UserCandidacyPage(this);

    
    ui->MainStack->insertWidget(0, m_loginPage);
    ui->MainStack->insertWidget(1, m_signupPage); 
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

    ui->MainStack->setCurrentIndex(4);

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
    
    connect(m_adminInnerPage_Admins,
            &AdminManagementPage::adminStatusChangeRequested,
            this,
            &MainWindow::handleAdminStatusChangeRequested);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleGoToSignupRequested()
{
    ui->MainStack->setCurrentIndex(1); 
    this->setFocus();
}

void MainWindow::handleGoToLoginRequested()
{
    ui->MainStack->setCurrentIndex(0); 
    this->setFocus();
}



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
    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
}

void MainWindow::handleSignupSuccessUser()
{
    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
}

void MainWindow::handleSignupSuccessAdminPending()
{
    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
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

void MainWindow::on_btnUserResults_clicked()
{
    ui->userContentStack->setCurrentIndex(2);
    this->setFocus();
}

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

    
    m_adminInnerPage_Candidates->loadElections(approvedElections, approvedCount);

    
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
    if (!AuthManager::getInstance().isLoggedIn() || !AuthManager::getInstance().getCurrentUser()) {
        QMessageBox::critical(this, "Authentication Error", "No authenticated admin found. Please log in again.");
        return;
    }
    QString currentAdminId = AuthManager::getInstance().getCurrentUser()->getId();
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

    int adminCount = 0;
    auto adminOpt = AdminController::getInstance().getAllAdminsExcept(currentUserCnic, adminCount);

    if (!adminOpt) {
        QMessageBox::critical(this, "Error", "Failed to load admins.");
        return;
    }

    Admin *adminList = adminOpt.value();
    m_adminInnerPage_Admins->loadAdmins(adminList, adminCount, currentAdminId);

    delete[] adminList;
}

void MainWindow::on_adminSidebarElectionsBtn_clicked()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Elections);
#ifdef prod
    if (!AuthManager::getInstance().isLoggedIn() || !AuthManager::getInstance().getCurrentUser()) {
        QMessageBox::critical(this,
                              "Authentication Error",
                              "No authenticated admin found. Please log in again.");
        return;
    }
    QString currentAdminId = AuthManager::getInstance().getCurrentUser()->getId();
#endif

#ifdef deve
    QString currentAdminId = "ROOT_001";
#endif

    int electionCount = 0;
    Election *electionList = ElectionController::getInstance().getAllElections(electionCount);

    if (!electionList) {
        QMessageBox::critical(this, "Error", "Failed to load elections.");
        return;
    }

    m_adminInnerPage_Elections->loadElections(electionList, electionCount, currentAdminId);

    delete[] electionList;
}


void MainWindow::handleNavigateToCandidateDetails(Candidate selectedCandidate)
{
    
    m_adminInnerPage_CandidateDetails->setCandidate(selectedCandidate);

    
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_CandidateDetails);
}

void MainWindow::handleBackToCandidateList()
{
    
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Candidates);
}


void MainWindow::handleCandidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus)
{
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    bool success = CandidateController::getInstance().requestCandidateStatusChange(targetCnic,
                                                                                   currentAdminCnic,
                                                                                   newStatus);
    if (success)
    {
        QMessageBox::information(this, "Success", "Status change request logged.");
        handleBackToCandidateList(); 
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
    
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

    
    bool success = ElectionController::getInstance().requestElectionStatusChange(electionId,
                                                                                 currentAdminCnic,
                                                                                 newStatus);

    if (success) {
        QMessageBox::information(this,
                                 "Success",
                                 "Election status change request logged successfully.\n\nIt will "
                                 "change state once the required threshold of Admins approve it.");
    } else {
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
    
    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();

    
    bool success = AdminController::getInstance().addStatusChangeRequest(targetCnic,
                                                                         currentAdminCnic,
                                                                         newStatus);

    if (success) {
        QMessageBox::information(this,
                                 "Success",
                                 "Administrator status change request logged successfully.");
    } else {
        QMessageBox::critical(this,
                              "Error",
                              "Failed to log Admin status change. Ensure you are an approved Admin "
                              "and haven't already voted on this user.");
    }
}

void MainWindow::handleNavigateToCreateElection()
{
    
    m_adminInnerPage_CreateElection->resetForm();

    
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_CreateElection);
}

void MainWindow::handleBackToElectionList()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Elections);
}


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
    handleBackToElectionList(); 
}


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
    if (!tokenOpt.has_value()) {
        QMessageBox::critical(this, "Error", "Failed to generate token. Please try again.");
        return;
    }
    Token newToken = tokenOpt.value();

    auto tokenQrCodeOpt = TokenController::getInstance().getQrCodeForToken(newToken);

    if (!tokenQrCodeOpt.has_value()) {
        QMessageBox::critical(this, "Error", "Token created but Failed to generate QR code.");
        return;
    }

    QImage tokenQrCode = tokenQrCodeOpt.value();
    
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    
    buffer.open(QIODevice::WriteOnly);

    
    tokenQrCode.save(&buffer, "PNG");

    
    QByteArray base64Bytes = byteArray.toBase64();

    bool sendTokenToEmailSuccess = TokenController::getInstance().sendTokenToEmail(newToken,
                                                                                   userEmail);

    if (!sendTokenToEmailSuccess) {
        QMessageBox::critical(this, "Error", "Token created by failed to be send to mail");
        return;
    }

    
    VoterTokenMess tokenPopup(base64Bytes, electionId, this);

    
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


void MainWindow::handleUserNavigateToCandidateDetails(Candidate selectedCandidate)
{
    
    userInnerPage_CandidateDetails->setCandidate(selectedCandidate);

    
    ui->userContentStack->setCurrentWidget(userInnerPage_CandidateDetails);
}

void MainWindow::handleUserBackToActiveElections()
{
    
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);
}


void MainWindow::on_userSidebarLocateStationBtn_clicked()
{
    QMessageBox::information(this, "Coming Soon",
                             "📍 The Locate Station feature is currently under development and will be available soon!");
}


void MainWindow::on_userSidebarCandidacyBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_Candidacy);

    int electionCount = 0;
    Election *draftedElections = ElectionController::getInstance().getElectionsByStatus(ElectionState::Drafted, electionCount);

    userInnerPage_Candidacy->loadPublishedElections(draftedElections, electionCount);
    delete[] draftedElections;
}


void MainWindow::handleCandidacyApplicationSubmit(Candidate newCandidate)
{
#ifdef deve
    QString currentUserCnic = "1234567891011";
#endif
#ifdef prod
    QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
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


void MainWindow::handleGetConfigurationRequested(QString electionId) {

    QMessageBox::information(this, "Get Configuration",
                             "Ready to fetch configuration for Election ID:\n" + electionId +
                                 "\n\n(Backend logic to generate/download the JSON file will go here!)");

   
}



void MainWindow::loadUserProfile(const QString &fullName, const QString &imagePath)
{
    
    ui->userNameLabel->setText("Hello, " + fullName);

    
    QPixmap originalImage;

    
    if (!imagePath.isEmpty() && QFile::exists(imagePath))
    {
        originalImage.load(imagePath);
    }
    else
    {
        
        originalImage.load(":/images/white_default_profpic.png");
    }

    
    QPixmap circularImage(50, 50);
    circularImage.fill(Qt::transparent);

    QPainter painter(&circularImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, 50, 50);
    painter.setClipPath(path);

    
    QPixmap scaledOriginal = originalImage.scaled(50,
                                                  50,
                                                  Qt::KeepAspectRatioByExpanding,
                                                  Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledOriginal);

    
    ui->userProfilePicBtn->setIcon(QIcon(circularImage));
    ui->userProfilePicBtn->setIconSize(QSize(50, 50));
}
void MainWindow::loadAdminProfile(const QString &fullName, const QString &imagePath)
{
    
    ui->adminNameLabel->setText("Hello, " + fullName);

    
    QPixmap originalImage;

    
    if (!imagePath.isEmpty() && QFile::exists(imagePath))
    {
        originalImage.load(imagePath);
    }
    else
    {
        
        originalImage.load(":/images/white_default_profpic.png");
    }

    
    QPixmap circularImage(50, 50);
    circularImage.fill(Qt::transparent);

    QPainter painter(&circularImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, 50, 50);
    painter.setClipPath(path);


    QPixmap scaledOriginal = originalImage.scaled(50,
                                                  50,
                                                  Qt::KeepAspectRatioByExpanding,
                                                  Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledOriginal);

    
    ui->adminProfilePicBtn->setIcon(QIcon(circularImage));
    ui->adminProfilePicBtn->setIconSize(QSize(50, 50));
}
