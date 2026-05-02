#include "views/mainwindow.h"
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDateTime>
#include "views/AdminCandidatePage.h"
#include "models/Models.h"
#include "models/entities/election.h"
#include "controllers/electionController.h"
#include "services/email/emailservice.h"
#include "controllers/auth_manager.h"
#include "models/entities/user.h"
#include "ui_mainwindow.h"
#include "votertokenmess.h"

MainWindow::MainWindow(const AppConfig &config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_config(config)
{
    ui->setupUi(this);

    this->setFocus();

    EmailService::getInstance().configure("smtp.gmail.com",
                                          465,
                                          "pak.evm.project@gmail.com",
                                          "dtgn pptc jspd vjnk");

    // 1. Create the custom page purely in C++
    m_loginPage = new LoginPage(this);
    // admin
    m_adminInnerPage_Candidates = new AdminCandidatePage(this);
    m_adminInnerPage_Admins = new AdminManagementPage(this);
    m_adminInnerPage_Elections = new AdminElectionsPage(this);
    m_adminInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    m_adminInnerPage_CreateElection = new AdminCreateElectionPage(this);
    // user
    userInnerPage_ActiveElections = new UserActiveElectionsPage(this);
    userInnerPage_MyTokens = new UserMyTokensPage(this);
    userInnerPage_CandidateDetails = new AdminCandidateDetailsPage(this);
    userInnerPage_CandidateDetails->setUserMode(true); // <--- HIDES THE ADMIN BUTTONS
    userInnerPage_Candidacy = new UserCandidacyPage(this);

    // 2. Add it to the Stacked Widget manually
    ui->MainStack->insertWidget(0, m_loginPage);
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

    ui->MainStack->setCurrentIndex(4);

    connect(m_loginPage, &LoginPage::goToSignupRequested, this, &MainWindow::handleGoToSignupRequested);
    connect(m_loginPage, &LoginPage::loginSuccessUser, this, &MainWindow::handleLoginSuccessUser);
    connect(m_loginPage, &LoginPage::loginSuccessAdmin, this, &MainWindow::handleLoginSuccessAdmin);
    connect(m_loginPage, &LoginPage::loginSuccessAdminPending, this, &MainWindow::handleLoginSuccessAdminPending);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Navigation

void MainWindow::handleGoToSignupRequested()
{
    ui->MainStack->setCurrentIndex(StackedPages::SignupPage);
    this->setFocus();
}

void MainWindow::on_goToLoginBtn_clicked()
{
    // Switch to Login Page(Index 0)
    ui->MainStack->setCurrentIndex(0);
    this->setFocus();
}

// Authentication

void MainWindow::handleLoginSuccessUser()
{
    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
}

void MainWindow::handleLoginSuccessAdmin()
{
    ui->MainStack->setCurrentIndex(StackedPages::AdminDashPage);
}

void MainWindow::handleLoginSuccessAdminPending()
{
    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
}

void MainWindow::on_signupSubmitBtn_clicked()
{
    // 1. Grab text from UI and use .trimmed() to remove accidental spacebars
    QString newUsername = ui->newUsernameInput->text().trimmed();
    QString newEmail = ui->emailInput->text().trimmed();
    QString newCnic = ui->cnicInput->text().trimmed();

    // Passwords should NOT be trimmed
    QString newPassword = ui->newPasswordInput->text();
    QString confirmPassword = ui->confirmPasswordInput->text();

    // 2. Validation Level 1: Check for Empty Fields
    if (newUsername.isEmpty() || newEmail.isEmpty() || newCnic.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "All fields must be filled out.");
        return;
    }

    // 3. Validation Level 2: Password Match Check
    if (newPassword != confirmPassword)
    {
        QMessageBox::warning(this,
                             "Security Error",
                             "Passwords do not match. Please type them carefully.");
        ui->newPasswordInput->clear();
        ui->confirmPasswordInput->clear();
        ui->newPasswordInput->setFocus();
        return;
    }

    // 4. Validation Level 3: Email Format Check
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(newEmail).hasMatch())
    {
        QMessageBox::warning(this, "Format Error", "Please enter a valid email address.");
        ui->emailInput->setFocus();
        return;
    }

    // 5. Validation Level 4: CNIC Format Check (Pakistani Standard)
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");
    if (!cnicRegex.match(newCnic).hasMatch())
    {
        QMessageBox::warning(this, "Format Error", "Please enter a valid 13-digit CNIC.");
        ui->cnicInput->setFocus();
        return;
    }
    User newUser{};

    newUser.setCnic(newCnic);
    newUser.setEmail(newEmail);
    newUser.setName(newUsername);

    bool isAdminRegistration = ui->adminCheckBox->isChecked();
    AuthManager::SignUpResult signupResult = AuthManager::getInstance().signUp(newUser,
                                                                               newPassword,
                                                                               isAdminRegistration);
    if (signupResult == AuthManager::SignUpResult::SuccessUserCreated || signupResult == AuthManager::SignUpResult::SuccessAdminCreated)
    {
        bool otpSendSuccess = AuthManager::getInstance().requestOtp(newEmail);
        if (!otpSendSuccess)
        {
            QMessageBox::critical(this, "Network Error", "Failed to send OTP email.");
            return;
        }
        bool verified = false;
        while (!verified)
        {
            bool ok = false;
            QString otp = QInputDialog::getText(
                this,
                tr("2FA Verification"),
                tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                QLineEdit::Password,
                "",
                &ok);
            ;

            if (!ok)
            {
                QMessageBox::information(
                    this,
                    "Cancelled",
                    "Verification cancelled. You can verify your email later by logging in.");
                ui->newUsernameInput->clear();
                ui->emailInput->clear();
                ui->cnicInput->clear();
                ui->newPasswordInput->clear();
                ui->confirmPasswordInput->clear();
                ui->adminCheckBox->setChecked(false);
                ui->MainStack->setCurrentIndex(StackedPages::Login_Page);
                return;
            }

            if (otp.trimmed().isEmpty())
            {
                QMessageBox::warning(this, "Error", "OTP field cannot be empty!");
                continue; // Skips the rest and asks again
            }

            // Condition 3: Verify the OTP
            if (AuthManager::getInstance().verifyOtp(newEmail, otp))
            {
                verified = true; // Breaks the loop naturally
                if (signupResult == AuthManager::SignUpResult::SuccessUserCreated)
                {
                    QMessageBox::information(
                        this,
                        "Registration Successful",
                        "Account created successfully! Welcome to the dashboard.");
                    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
                }
                else if (signupResult == AuthManager::SignUpResult::SuccessAdminCreated)
                {
                    QMessageBox::information(
                        this,
                        "Pending Authorization",
                        "Admin request submitted. Please wait for Two-Person authorization.");
                    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
                }
            }
            else
            {
                QMessageBox::warning(this, "Error", "Incorrect OTP. Please try again.");
            }
        }
    }

    else if (AuthManager::SignUpResult::UserAlreadyExists == signupResult)
    {
        QMessageBox::warning(this, "Error", "User Already Exists");
        // --- NORMAL VOTER FLOW ---
    }
    else if (AuthManager::SignUpResult::SystemError == signupResult)
    {
        QMessageBox::warning(this, "Error", "System Error");
    }

    // 7. Security Cleanup: Clear all inputs so the next person can't see them
    ui->newUsernameInput->clear();
    ui->emailInput->clear();
    ui->cnicInput->clear();
    ui->newPasswordInput->clear();
    ui->confirmPasswordInput->clear();

    // CRITICAL: Uncheck the box so it doesn't stay checked for the next user!
    ui->adminCheckBox->setChecked(false);
}

void MainWindow::on_adminWaitBackBtn_clicked()
{
    ui->MainStack->setCurrentIndex(StackedPages::SignupPage);
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
    // 1. Change the nested stacked widget to show the candidate page
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Candidates);

    // 2. Fetch all Elections from Database (Using Mock Data for now)
    int electionCount = 3;
    Election *mockElections = new Election[electionCount];

    mockElections[0].setId("ELEC-001");
    mockElections[0].setTitle("Presidential Election 2026");

    mockElections[1].setId("ELEC-002");
    mockElections[1].setTitle("Karachi Mayoral Election");

    mockElections[2].setId("ELEC-003");
    mockElections[2].setTitle("Punjab Provincial Assembly");

    // 3. Load them into the UI
    m_adminInnerPage_Candidates->loadElections(mockElections, electionCount);

    // 4. Cleanup memory to prevent leaks!
    delete[] mockElections;
}
void MainWindow::handleElectionSelectedForCandidates(QString electionId)
{
    // Normally: candidateRepo->getCandidatesByElection(electionId, size);

    // Mock Data for now (so the Frontend Engineer can test the UI colors)
    int candidateCount = 3;
    Candidate *mockCandidates = new Candidate[candidateCount];

    mockCandidates[0].setUserCnic("42101-1234567-1");
    mockCandidates[0].setPartyName("Democratic Party");
    mockCandidates[0].setStatus(ApprovalStatus::Approved); // Should show GREEN border

    mockCandidates[1].setUserCnic("42101-9876543-2");
    mockCandidates[1].setPartyName("Independent");
    mockCandidates[1].setStatus(ApprovalStatus::Pending); // Should show YELLOW border

    mockCandidates[2].setUserCnic("42101-5555555-3");
    mockCandidates[2].setPartyName("Liberty Front");
    mockCandidates[2].setStatus(ApprovalStatus::Rejected); // Should show RED border

    // Load them into the UI
    m_adminInnerPage_Candidates->loadCandidates(mockCandidates, candidateCount);

    // Cleanup memory
    delete[] mockCandidates;
}

void MainWindow::on_adminSidebarAdminsBtn_clicked()
{
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Admins);

    // Mock Data for the Admins
    int adminCount = 3;
    Admin *mockAdmins = new Admin[adminCount];

    mockAdmins[0].setName("Ahmed Munir");
    mockAdmins[0].setEmail("ahmed@pakevm.com");
    mockAdmins[0].setCnic("42101-1111111-1");
    mockAdmins[0].setStatus(ApprovalStatus::Approved);

    mockAdmins[1].setName("Ali Khan");
    mockAdmins[1].setEmail("ali@pakevm.com");
    mockAdmins[1].setCnic("42101-2222222-2");
    mockAdmins[1].setStatus(ApprovalStatus::Pending);

    mockAdmins[2].setName("Usman Tariq");
    mockAdmins[2].setEmail("usman@pakevm.com");
    mockAdmins[2].setCnic("42101-3333333-3");
    mockAdmins[2].setStatus(ApprovalStatus::Rejected);

    m_adminInnerPage_Admins->loadAdmins(mockAdmins, adminCount);

    delete[] mockAdmins;
}

// ---------------------------------------------------------
// Triggered when the Admin clicks "Elections" on Left Sidebar
// ---------------------------------------------------------
void MainWindow::on_adminSidebarElectionsBtn_clicked()
{
    // 1. Show the Elections page
    ui->adminContentStack->setCurrentWidget(m_adminInnerPage_Elections);

    // 2. Generate Mock Elections with various states to test the UI
    int electionCount = 5;
    Election *mockElections = new Election[electionCount];

    // Status: Published (Blue)
    mockElections[0].setId("ELEC-101");
    mockElections[0].setTitle("Federal Senate Election 2026");
    mockElections[0].setStartTime(QDateTime::currentDateTime().addDays(5));
    mockElections[0].setEndTime(QDateTime::currentDateTime().addDays(6));
    mockElections[0].setStatus(ElectionState::Published);

    // Status: Voting Open (Green)
    mockElections[1].setId("ELEC-102");
    mockElections[1].setTitle("Lahore Local Council");
    mockElections[1].setStartTime(QDateTime::currentDateTime().addDays(-1));
    mockElections[1].setEndTime(QDateTime::currentDateTime().addDays(1));
    mockElections[1].setStatus(ElectionState::VotingOpen);

    // Status: Draft (Grey)
    mockElections[2].setId("ELEC-103");
    mockElections[2].setTitle("Karachi Medical Board Draft");
    mockElections[2].setStartTime(QDateTime::currentDateTime().addDays(20));
    mockElections[2].setEndTime(QDateTime::currentDateTime().addDays(21));
    mockElections[2].setStatus(ElectionState::Drafted);

    // Status: Rejected (Red)
    mockElections[3].setId("ELEC-104");
    mockElections[3].setTitle("Fake Test Election");
    mockElections[3].setStartTime(QDateTime::currentDateTime());
    mockElections[3].setEndTime(QDateTime::currentDateTime().addDays(1));
    mockElections[3].setStatus(ElectionState::Rejected);

    // Status: Results Announced (Purple)
    mockElections[4].setId("ELEC-105");
    mockElections[4].setTitle("Sindh Bar Council 2025");
    mockElections[4].setStartTime(QDateTime::currentDateTime().addDays(-30));
    mockElections[4].setEndTime(QDateTime::currentDateTime().addDays(-29));
    mockElections[4].setStatus(ElectionState::ResultsAnnounced);

    // 3. Load the data into your custom UI
    m_adminInnerPage_Elections->loadElections(mockElections, electionCount);

    // 4. Prevent memory leaks!
    delete[] mockElections;
}

// ---------------------------------------------------------
// Navigation: Go TO Details Page
// ---------------------------------------------------------
void MainWindow::handleNavigateToCandidateDetails(Candidate selectedCandidate)
{
    // For testing purposes, let's inject a HUGE manifesto into the selected
    // candidate right before we show it, just to ensure the QScrollArea works perfectly.
    if (selectedCandidate.getManifesto().isEmpty())
    {
        selectedCandidate.setManifesto(
            "1. Economic Reform: We will introduce a comprehensive tax relief plan...\n\n"
            "2. Healthcare: Free access to primary care facilities across the province.\n\n"
            "3. Education: Building 50 new IT universities by the year 2028.\n\n"
            "4. Infrastructure: Expanding the Metro bus network to all major cities.\n\n"
            "5. Environment: Planting 10 million trees to combat urban heat islands.\n\n"
            "6. Law & Order: Increasing police presence and digitizing all FIRs.\n\n"
            "7. Youth Empowerment: Paid internships for 100,000 graduates.\n\n"
            "(Keep scrolling...) \n\n"
            "8. Foreign Policy: Enhancing trade with neighboring regions.\n\n"
            "9. Agriculture: Subsidies for solar-powered tube wells.");
        selectedCandidate.setPreviousHistory("Served as MPA from 2018-2023. Member of Finance Committee.");
        selectedCandidate.setEducationLevel("Ph.D. in Economics from LUMS");
    }

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
    // 1. Get current Admin ID (Using a fake one for testing if AuthManager isn't hooked up yet)
    // QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    QString currentAdminCnic = "42101-ADMIN-1";

    QString statusText = (newStatus == ApprovalStatus::Approved) ? "APPROVE" : "REJECT";

    // 2. Show a MessageBox to prove the signal works perfectly!
    QMessageBox::information(this, "Controller Simulation",
                             QString("Simulating passing data to CandidateController...\n\n"
                                     "Target Candidate: %1\n"
                                     "Action: %2\n"
                                     "Requested By: Admin %3")
                                 .arg(targetCnic, statusText, currentAdminCnic));

    /*
     * WHEN YOUR BACKEND IS READY, THIS IS ALL YOU WRITE HERE:
     *
     * bool success = CandidateController::getInstance().requestCandidateStatusChange(targetCnic, currentAdminCnic, newStatus);
     * if(success) {
     *     QMessageBox::information(this, "Success", "Status request logged.");
     *     handleBackToCandidateList(); // Kick them back to the list
     * }
     */
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
    QMessageBox::information(this, "Simulation",
                             QString("Ready to send to ElectionController!\n\nTitle: %1\nPublish: %2\nStart: %3\nEnd: %4")
                                 .arg(title,
                                      publishTime.toString("dd MMM yyyy"),
                                      startTime.toString("dd MMM yyyy"),
                                      endTime.toString("dd MMM yyyy")));

    Election newElection;
    newElection.setTitle(title);
    newElection.setPublishTime(publishTime);
    newElection.setStartTime(startTime);
    newElection.setEndTime(endTime);
    bool success = ElectionController::getInstance().createElection(newElection);
    if (success) {
        QMessageBox::information(this, "Success", "Election created as Draft.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to create election.");
    }
    handleBackToElectionList(); // Go back to list after success
}

//---------------user-pages---------------
void MainWindow::on_userSidebarActiveElectionsBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);

    // Mock Data for Elections
    int electionCount = 2;
    Election *mockElections = new Election[electionCount];

    mockElections[0].setId("ELEC-201");
    mockElections[0].setTitle("National General Election");
    mockElections[0].setStartTime(QDateTime::currentDateTime().addDays(-1));
    mockElections[0].setEndTime(QDateTime::currentDateTime().addDays(1));
    mockElections[0].setStatus(ElectionState::VotingOpen);

    mockElections[1].setId("ELEC-202");
    mockElections[1].setTitle("Provincial Assembly");
    mockElections[1].setStartTime(QDateTime::currentDateTime().addDays(5));
    mockElections[1].setEndTime(QDateTime::currentDateTime().addDays(6));
    mockElections[1].setStatus(ElectionState::Published);

    userInnerPage_ActiveElections->loadElections(mockElections, electionCount);
    delete[] mockElections;
}

void MainWindow::handleUserElectionSelected(QString electionId)
{
    // When the user clicks an election, load the mock candidates for it
    int candidateCount = 2;
    Candidate *mockCandidates = new Candidate[candidateCount];

    mockCandidates[0].setUserCnic("42101-111-1");
    mockCandidates[0].setPartyName("Democratic Front");
    mockCandidates[0].setStatus(ApprovalStatus::Approved);

    mockCandidates[1].setUserCnic("42101-222-2");
    mockCandidates[1].setPartyName("Liberty Party");
    mockCandidates[1].setStatus(ApprovalStatus::Approved);

    userInnerPage_ActiveElections->loadCandidates(mockCandidates, candidateCount);
    delete[] mockCandidates;
}

// ---------------------------------------------------------
// Triggered when User clicks "Register & Generate Token"
// ---------------------------------------------------------
void MainWindow::handleGenerateTokenRequested(QString electionId)
{

    /*
     * WHEN BACKEND IS READY, YOU WILL DO SOMETHING LIKE THIS:
     * QString currentUserId = AuthManager::getInstance().getCurrentUser()->getId();
     * bool success = TokenController::getInstance().generateAndSaveToken(currentUserId, electionId);
     * if (success) {
     *     QMessageBox::information(this, "Success", "Token securely generated! Check your 'My Tokens' tab.");
     * }
     */

    // 1. Get the data from your Backend/Database

    // Example Base64 string (your backend will provide a real one)
    QString backendBase64String = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=";

    // 2. Create the Dialog using the Designer class
    VoterTokenMess tokenPopup(backendBase64String, electionId, this);

    // 3. Show it modally (blocks the rest of the app until they click OK)
    tokenPopup.exec();
}

void MainWindow::on_userSidebarMyTokensBtn_clicked()
{
    ui->userContentStack->setCurrentWidget(userInnerPage_MyTokens);

    int tokenCount = 2;
    Token *mockTokens = new Token[tokenCount];

    mockTokens[0].setId("TKN-9991"); // Must have an ID for accordion to work!
    mockTokens[0].setElectionId("ELEC-201");
    mockTokens[0].setAssignedStationId("ST-A (Gulberg Branch)");
    mockTokens[0].setIssuedAt(QDateTime::currentDateTime());
    mockTokens[0].setTokenSignature("eyJhbGciOiJIUzI1NiIs...");

    mockTokens[1].setId("TKN-9992");
    mockTokens[1].setElectionId("ELEC-202");
    mockTokens[1].setAssignedStationId("ST-B (DHA Branch)");
    mockTokens[1].setIssuedAt(QDateTime::currentDateTime().addDays(-2));
    mockTokens[1].setTokenSignature("q8g9q8h24q8hg0284ghq...");

    userInnerPage_MyTokens->loadTokens(mockTokens, tokenCount);
    delete[] mockTokens;
}

// ---------------------------------------------------------
// Triggered when User clicks "✉ Send to Email" on a Token
// ---------------------------------------------------------
void MainWindow::handleEmailTokenRequested(Token selectedToken)
{
    // For now, just show a popup to prove the button works!
    QMessageBox::information(this, "Email Token",
                             "Ready to email token for Election:\n" + selectedToken.getElectionId() +
                                 "\n\n(Your Lead's EmailService will be connected here!)");

    /*
     * WHEN READY, YOU WILL USE YOUR LEAD'S EMAIL CLASS LIKE THIS:
     *
     * QString userEmail = AuthManager::getInstance().getCurrentUser()->getEmail();
     * QString body = "Your secure token hash is: " + selectedToken.getTokenSignature();
     *
     * EmailService::getInstance().sendEmail(userEmail, "Your Voting Token", body);
     * QMessageBox::information(this, "Success", "Token sent to your email!");
     */
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

    // Only fetch PUBLISHED elections for the candidacy form
    int electionCount = 2;
    Election *mockElections = new Election[electionCount];

    mockElections[0].setId("ELEC-201");
    mockElections[0].setTitle("National General Election");
    mockElections[0].setStatus(ElectionState::Published);

    mockElections[1].setId("ELEC-202");
    mockElections[1].setTitle("Provincial Assembly");
    mockElections[1].setStatus(ElectionState::Published);

    userInnerPage_Candidacy->loadPublishedElections(mockElections, electionCount);
    delete[] mockElections;
}

// ---------------------------------------------------------
// Triggered when User clicks "Submit Application" on Candidacy Page
// ---------------------------------------------------------
void MainWindow::handleCandidacyApplicationSubmit(Candidate newCandidate)
{
    // 1. Get the current user's CNIC and add it to the application
    // QString currentUserCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    QString currentUserCnic = "42101-TEST-CNIC"; // Mock CNIC for testing
    newCandidate.setUserCnic(currentUserCnic);

    // 2. Show Success Message
    QMessageBox::information(this, "Application Submitted",
                             "Your candidacy application for '" + newCandidate.getPartyName() + "' has been successfully submitted!\n\n"
                                                                                                "Please wait for admin approval. You will see your status update in the Active Elections tab once approved.");

    /*
     * WHEN YOUR BACKEND IS READY, YOU WILL UNCOMMENT THIS:
     *
     * bool success = CandidateController::getInstance().createCandidate(newCandidate);
     * if(success) {
     *     QMessageBox::information(this, "Success", "Application Submitted!");
     * } else {
     *     QMessageBox::warning(this, "Error", "Failed to submit application.");
     * }
     */

    // 3. Send the user back to the Active Elections page so they aren't stuck on the form
    ui->userContentStack->setCurrentWidget(userInnerPage_ActiveElections);
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
        originalImage.load(":/images/default_avatar.png");
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
        originalImage.load(":/images/default_avatar.png");
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
