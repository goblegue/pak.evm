#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog> // For the OTP Popup
#include <QMessageBox>  // For error/success messages
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include "./src/AuthManager/auth_manager.h"
#include "./src/email/emailservice.h"
#include "./src/user.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->MainStack->setCurrentIndex(3);

    // 2. Steal the focus away from the text boxes
    // This makes sure no input field is glowing blue/selected on startup.
    this->setFocus();

    EmailService::getInstance().configure("smtp.gmail.com", 465, "pak.evm.project@gmail.com", "dtgn pptc jspd vjnk");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Navigation

void MainWindow::on_goToSignupBtn_clicked() {
    // Switch to Signup Page(Index 1)
    ui->MainStack->setCurrentIndex(1);
    this->setFocus();
}

void MainWindow::on_goToLoginBtn_clicked() {
    // Switch to Login Page(Index 0)
    ui->MainStack->setCurrentIndex(0);
    this->setFocus();
}

// Authentication

void MainWindow::on_loginSubmitBtn_clicked() {
    // 1. Grab the text from the UI
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    // 2. Basic Validation to ensure fields aren't empty
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter your Voter ID and Password.");
        return;
    }
    int inputType=identifyInputType(username);
    AuthManager :: LoginResult loginResult;
    if (inputType==1)
    {
        loginResult=AuthManager::getInstance().login(password,"",username);
    }
    else if(inputType==2)
    {
        loginResult=AuthManager::getInstance().login(password,username,"");
    }
    else
    {
        QMessageBox::warning(this,"Error","Please enter Correct Format!");
    }
    QString userEmail=AuthManager::getInstance().getCurrentUser()->getEmail();
    //swapping pages for user on login result
    if( loginResult==AuthManager::LoginResult::SuccessUserLoggedIn  ||
        loginResult==AuthManager::LoginResult::SuccessAdminLoggedIn ||
        loginResult==AuthManager::LoginResult::SuccessAdminPending)
    {
    
        //TODO handle send failure
        bool otpSendSuccess=AuthManager::getInstance().requestOtp(userEmail);

        bool ok;
        QString otp; 

        while(!ok){
            otp== QInputDialog::getText(this,
                                            tr("2FA Verification"),
                                            tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                                            QLineEdit::Password,
                                            "", &ok);
            if(!ok){
                QMessageBox::warning(this,"Error","Otp field cannot be empty");
            }
        }
        while(!AuthManager::getInstance().verifyOtp(userEmail,otp)){
            otp = QInputDialog::getText(this,
                                            tr("2FA Verification"),
                                            tr("Please enter the correct code.\n\nEnter OTP:"),
                                            QLineEdit::Password,
                                            "", &ok);
            
        }
        if (AuthManager::getInstance().verifyOtp(userEmail, otp)) {

                if (loginResult == AuthManager::LoginResult::SuccessAdminLoggedIn) {
                    // Send to Admin Dashboard
                    QMessageBox::information(this, "Admin Verified", "Accessing Admin Portal...");
                    ui->MainStack->setCurrentIndex(StackedPages::AdminDashPage);
                    this->setFocus();
                }
                else if (loginResult == AuthManager::LoginResult::SuccessUserLoggedIn) {
                    // Send to Voter Dashboard
                    QMessageBox::information(this, "Voter Verified", "Welcome to the Voting Booth.");
                    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
                    this->setFocus();
                }
                else if(loginResult== AuthManager::LoginResult::SuccessAdminPending)
                {
                    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
                    this->setFocus();
                }
        }
    }
    else if(AuthManager::LoginResult::InvalidCnicOrEmail==loginResult){
        QMessageBox::warning(this,"Error","Incorrect CNIC or Email!");
    }
    else if(AuthManager::LoginResult::InvalidPassword==loginResult){
        QMessageBox::warning(this,"Error","Incorrect Password!");
    }
    else if(AuthManager::LoginResult::SystemError==loginResult){
        QMessageBox::warning(this,"Error","System Error!");
    }
    else if(AuthManager::LoginResult::EmailNotVerified==loginResult){
            bool otpSendSuccess=AuthManager::getInstance().requestOtp(userEmail);
            bool ok;
            QString otp; 

            while(!ok){
                otp== QInputDialog::getText(this,
                                                tr("Email Verification"),
                                                tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                                                QLineEdit::Password,
                                                "", &ok);
                if(!ok){
                    QMessageBox::warning(this,"Error","Otp field cannot be empty");
                }
            }
            while(!AuthManager::getInstance().verifyOtp(userEmail,otp)){
                otp = QInputDialog::getText(this,
                                                tr("Email Verification"),
                                                tr("Please enter the correct Email!"),
                                                QLineEdit::Password,
                                                "", &ok);
            }
            if(AuthManager::getInstance().verifyOtp(userEmail,otp))
            {
                ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
                this->setFocus();
            }
    }

}

void MainWindow::on_signupSubmitBtn_clicked() {
    // 1. Grab text from UI and use .trimmed() to remove accidental spacebars
    QString newUsername = ui->newUsernameInput->text().trimmed();
    QString newEmail = ui->emailInput->text().trimmed();
    QString newCnic = ui->cnicInput->text().trimmed();

    // Passwords should NOT be trimmed
    QString newPassword = ui->newPasswordInput->text();
    QString confirmPassword = ui->confirmPasswordInput->text();

    // 2. Validation Level 1: Check for Empty Fields
    if (newUsername.isEmpty() || newEmail.isEmpty() || newCnic.isEmpty() ||
        newPassword.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "All fields must be filled out.");
        return;
    }

    // 3. Validation Level 2: Password Match Check
    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "Security Error", "Passwords do not match. Please type them carefully.");
        ui->newPasswordInput->clear();
        ui->confirmPasswordInput->clear();
        ui->newPasswordInput->setFocus();
        return;
    }

    // 4. Validation Level 3: Email Format Check
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(newEmail).hasMatch()) {
        QMessageBox::warning(this, "Format Error", "Please enter a valid email address.");
        ui->emailInput->setFocus();
        return;
    }

    // 5. Validation Level 4: CNIC Format Check (Pakistani Standard)
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");
    if (!cnicRegex.match(newCnic).hasMatch()) {
        QMessageBox::warning(this, "Format Error", "Please enter a valid 13-digit CNIC.");
        ui->cnicInput->setFocus();
        return;
    }
    User newUser{};

    newUser.setCnic(newCnic);
    newUser.setEmail(newEmail);

    bool isAdminRegistration = ui->adminCheckBox->isChecked();
    AuthManager:: SignUpResult signupResult=AuthManager::getInstance().signUp(newUser,newPassword,isAdminRegistration);
    if(signupResult==AuthManager::SignUpResult::SuccessUserCreated || signupResult==AuthManager::SignUpResult::SuccessAdminCreated)
    {
        
            bool otpSendSuccess=AuthManager::getInstance().requestOtp(newEmail);

            bool ok;
            QString otp; 

            while(!ok){
                otp== QInputDialog::getText(this,
                                                tr("2FA Verification"),
                                                tr("A 4-digit code has been sent to your Email.\n\nEnter OTP:"),
                                                QLineEdit::Password,
                                                "", &ok);
                if(!ok){
                    QMessageBox::warning(this,"Error","Otp field cannot be empty");
                }
            }
            while(!AuthManager::getInstance().verifyOtp(newEmail,otp)){
                otp = QInputDialog::getText(this,
                                                tr("2FA Verification"),
                                                tr("Please enter the correct code.\n\nEnter OTP:"),
                                                QLineEdit::Password,
                                                "", &ok);
            }
            if (AuthManager::getInstance().verifyOtp(newEmail, otp))
            {
                if(signupResult==AuthManager::SignUpResult::SuccessUserCreated)
                {
                    QMessageBox::information(this, "Registration Successful",
                                             "Account created successfully! Welcome to the dashboard.");
                    ui->MainStack->setCurrentIndex(StackedPages::UserDashPage);
                    this->setFocus();
                }
                else if(signupResult==AuthManager::SignUpResult::SuccessAdminCreated)
                {
                    QMessageBox::information(this, "Pending Authorization",
                                             "Admin request submitted. Please wait for Two-Person authorization.");
                    ui->MainStack->setCurrentIndex(StackedPages::AdminWaitingPage);
                    this->setFocus();
                }
            }
        }


    else if(AuthManager::SignUpResult::UserAlreadyExists==signupResult){
        QMessageBox::warning(this,"Error","User Already Exists");
        // --- NORMAL VOTER FLOW ---
    }
    else if(AuthManager::SignUpResult::SystemError==signupResult){
        QMessageBox::warning(this,"Error","System Error");
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

int MainWindow::identifyInputType(const QString &input) {
    if (input.isEmpty()) return 0;

    // Define Regex patterns
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");

    if (emailRegex.match(input).hasMatch()) {
        return 1; // It's an Email
    }

    if (cnicRegex.match(input).hasMatch()) {
        return 2; // It's a CNIC
    }

    return 0;
}

void MainWindow::on_btnUserHome_clicked()
{
  ui->userContentStack->setCurrentIndex(0);
    this->setFocus();
}

void MainWindow::on_btnUserElections_Clicked() {
    ui->userContentStack->setCurrentIndex(1);
    this->setFocus();
}

void MainWindow::on_btnUserResults_clicked() {
    ui->userContentStack->setCurrentIndex(2);
    this->setFocus();
}

void MainWindow::on_btnUserLogout_clicked() {

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Logout Confirmation",
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


#include <QPainter>
#include <QPainterPath>
#include <QFile>

void MainWindow::loadUserProfile(const QString& fullName, const QString& imagePath) {
    // 1. Set the Dynamic Name
    ui->userNameLabel->setText("Hello, " + fullName);

    // 2. Load the Dynamic Image
    QPixmap originalImage;

    // Check if the user has an uploaded image path, and if the file actually exists
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        originalImage.load(imagePath);
    } else {
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
    QPixmap scaledOriginal = originalImage.scaled(50, 50, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledOriginal);

    // 4. Apply to the Button
    ui->profilePicBtn->setIcon(QIcon(circularImage));
    ui->profilePicBtn->setIconSize(QSize(50, 50));
}




