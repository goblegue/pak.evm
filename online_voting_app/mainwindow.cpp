#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog> // For the OTP Popup
#include <QMessageBox>  // For error/success messages
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "./src/AuthManager/auth_manager.h"
#include "./src/email/emailservice.h"
#include "./src/user.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

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
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_goToLoginBtn_clicked() {
    // Switch to Login Page(Index 0)
    ui->stackedWidget->setCurrentIndex(0);
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
                    ui->stackedWidget->setCurrentIndex(StackedPages::AdminDashPage);
                }
                else if (loginResult == AuthManager::LoginResult::SuccessUserLoggedIn) {
                    // Send to Voter Dashboard
                    QMessageBox::information(this, "Voter Verified", "Welcome to the Voting Booth.");
                    ui->stackedWidget->setCurrentIndex(StackedPages::UserDashPage);
                }
                else if(loginResult== AuthManager::LoginResult::SuccessAdminPending)
                {
                    ui->stackedWidget->setCurrentIndex(StackedPages::AdminWaitingPage);
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
                ui->stackedWidget->setCurrentIndex(StackedPages::UserDashPage);
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
                    ui->stackedWidget->setCurrentIndex(StackedPages::UserDashPage);
                }
                else if(signupResult==AuthManager::SignUpResult::SuccessAdminCreated)
                {
                    QMessageBox::information(this, "Pending Authorization",
                                             "Admin request submitted. Please wait for Two-Person authorization.");
                    ui->stackedWidget->setCurrentIndex(StackedPages::AdminWaitingPage);
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
    ui->stackedWidget->setCurrentIndex(0);

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

    return 0; // Invalid Format
}
