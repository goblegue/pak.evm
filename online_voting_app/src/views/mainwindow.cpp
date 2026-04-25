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

MainWindow::MainWindow(const AppConfig &config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_config(config)
{
    ui->setupUi(this);

    ui->MainStack->setCurrentIndex(4);
    this->setFocus();

    EmailService::getInstance().configure("smtp.gmail.com",
                                          465,
                                          "pak.evm.project@gmail.com",
                                          "dtgn pptc jspd vjnk");

    // 1. Create the custom page purely in C++
    m_adminInnerPage_Candidates = new AdminCandidatePage(this);
    m_adminInnerPage_Admins = new AdminManagementPage(this);
    m_adminInnerPage_Elections = new AdminElectionsPage(this);

    // 2. Add it to the Stacked Widget manually
    ui->adminContentStack->addWidget(m_adminInnerPage_Candidates);
    ui->adminContentStack->addWidget(m_adminInnerPage_Admins);
    ui->adminContentStack->addWidget(m_adminInnerPage_Elections);

    connect(m_adminInnerPage_Candidates,
            &AdminCandidatePage::electionSelected,
            this,
            &MainWindow::handleElectionSelectedForCandidates);
    connect(m_adminInnerPage_Elections, &AdminElectionsPage::navigateToCreateElection,
            this, &MainWindow::handleNavigateToCreateElection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Navigation

void MainWindow::on_goToSignupBtn_clicked()
{
    // Switch to Signup Page(Index 1)
    ui->MainStack->setCurrentIndex(1);
    this->setFocus();
}

void MainWindow::on_goToLoginBtn_clicked()
{
    // Switch to Login Page(Index 0)
    ui->MainStack->setCurrentIndex(0);
    this->setFocus();
}

// Authentication

void MainWindow::on_loginSubmitBtn_clicked()
{
    // 1. Grab the text from the UI
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    // 2. Basic Validation to ensure fields aren't empty
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter your Voter ID and Password.");
        return;
    }
    int inputType = identifyInputType(username);
    AuthManager ::LoginResult loginResult;
    if (inputType == 1)
    {
        loginResult = AuthManager::getInstance().login(password, "", username);
    }
    else if (inputType == 2)
    {
        loginResult = AuthManager::getInstance().login(password, username, "");
    }
    else
    {
        QMessageBox::warning(this, "Error", "Please enter Correct Format!");
        return;
    }

    // Handle Errors first to keep code clean
    if (loginResult == AuthManager::LoginResult::InvalidCnicOrEmail)
    {
        QMessageBox::warning(this, "Error", "Incorrect CNIC or Email!");
        return;
    }
    else if (loginResult == AuthManager::LoginResult::InvalidPassword)
    {
        QMessageBox::warning(this, "Error", "Incorrect Password!");
        return;
    }
    else if (loginResult == AuthManager::LoginResult::SystemError)
    {
        QMessageBox::warning(this, "Error", "System Error!");
        return;
    }

    QString userEmail = AuthManager::getInstance().getCurrentUser()->getEmail();

    // swapping pages for user on login result
    if (loginResult == AuthManager::LoginResult::SuccessUserLoggedIn || loginResult == AuthManager::LoginResult::SuccessAdminLoggedIn || loginResult == AuthManager::LoginResult::SuccessAdminPending)
    {
        bool otpSendSuccess = AuthManager::getInstance().requestOtp(userEmail);
        if (!otpSendSuccess)
        {
            QMessageBox::critical(this, "Network Error", "Failed to send OTP email.");
            return;
        }

        bool verified = false;

        while (!verified)
        {
            bool ok;
            QString otp = QInputDialog::getText(
                this,
                tr("2FA Verification"),
                tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                QLineEdit::Password,
                "",
                &ok);

            if (!ok)
            {
                QMessageBox::information(this,
                                         "Cancelled",
                                         "Verification cancelled. Returning to login page.");
                ui->MainStack->setCurrentIndex(StackedPages::LoginPage);
                return;
            }

            if (otp.trimmed().isEmpty())
            {
                QMessageBox::warning(this, "Error", "OTP field cannot be empty!");
                continue;
            }

            if (AuthManager::getInstance().verifyOtp(userEmail, otp))
            {
                verified = true;
            }
            else
            {
                QMessageBox::warning(this, "Error", "Incorrect OTP. Please try again.");
            }
        }

        if (loginResult == AuthManager::LoginResult::SuccessAdminLoggedIn)
        {
            // Send to Admin Dashboard
            QMessageBox::information(this, "Admin Verified", "Accessing Admin Portal...");
            ui->MainStack->setCurrentIndex(StackedPages::AdminDashPage);
        }
        else if (loginResult == AuthManager::LoginResult::SuccessUserLoggedIn)
        {
            // Send to Voter Dashboard
            QMessageBox::information(this, "Voter Verified", "Welcome to the Voting Booth.");
            ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
        }
        else if (loginResult == AuthManager::LoginResult::SuccessAdminPending)
        {
            ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
        }
    }
    else if (AuthManager::LoginResult::EmailNotVerified == loginResult)
    {
        QMessageBox::information(this,
                                 "Email Not Verified",
                                 "Your email is not verified. Please verify to continue.");
        bool otpSendSuccess = AuthManager::getInstance().requestOtp(userEmail);
        if (!otpSendSuccess)
        {
            QMessageBox::critical(this, "Network Error", "Failed to send OTP email.");
            return;
        }

        bool verified = false;

        while (!verified)
        {
            bool ok;
            QString otp = QInputDialog::getText(
                this,
                tr("Email Verification"),
                tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                QLineEdit::Password,
                "",
                &ok);

            if (!ok)
            {
                QMessageBox::information(this,
                                         "Cancelled",
                                         "Verification cancelled. Returning to login page.");
                ui->MainStack->setCurrentIndex(StackedPages::LoginPage);
                return;
            }

            if (otp.trimmed().isEmpty())
            {
                QMessageBox::warning(this, "Error", "OTP field cannot be empty!");
                continue;
            }

            if (AuthManager::getInstance().verifyOtp(userEmail, otp))
            {
                verified = true;
                QMessageBox::information(
                    this,
                    "Verification Successful",
                    "Email verified successfully! Redirecting to dashboard...");
                ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
            }
            else
            {
                QMessageBox::warning(this, "Error", "Incorrect OTP. Please try again.");
            }
        }
    }
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
                ui->MainStack->setCurrentIndex(StackedPages::LoginPage);
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

        ui->MainStack->setCurrentIndex(StackedPages::LoginPage);
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
    Election* mockElections = new Election[electionCount];

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
    mockElections[2].setStatus(ElectionState::Draft);

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

void MainWindow::handleNavigateToCreateElection()
{
    // TODO: Navigate to Create Election page when implemented
    QMessageBox::information(this, "Create Election", "Create Election functionality coming soon!");
}

// Helping Functions

int MainWindow::identifyInputType(const QString &input)
{
    if (input.isEmpty())
        return 0;

    // Define Regex patterns
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");

    if (emailRegex.match(input).hasMatch())
    {
        return 1; // It's an Email
    }

    if (cnicRegex.match(input).hasMatch())
    {
        return 2; // It's a CNIC
    }

    return 0;
}

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
