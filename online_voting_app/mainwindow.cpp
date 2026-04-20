#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog> // For the OTP Popup
#include <QMessageBox>  // For error/success messages
#include <QRegularExpression>
#include <QRegularExpressionMatch>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    // 2. Steal the focus away from the text boxes
    // This makes sure no input field is glowing blue/selected on startup.
    this->setFocus();

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

    // add database working here

    // 3. Trigger the OTP Popup
    bool ok;
    QString otp = QInputDialog::getText(this,
                                        tr("2FA Verification"),
                                        tr("Enter the 4-digit OTP sent to your device:"),
                                        QLineEdit::Password, // Hides the OTP as it's typed
                                        "",
                                        &ok);

    // 4. Verify the OTP
    if (ok && !otp.isEmpty()) {
        if (otp == "1234") { // Hardcoded "1234" for testing purposes
            QMessageBox::information(this, "Success", "Identity Verified! Welcome to the polls.");

            // Clear the password field for security before moving on
            ui->passwordInput->clear();

            // Later route the user to the Voting Page here:
            // ui->stackedWidget->setCurrentIndex(2);
        } else {
            QMessageBox::critical(this, "Security Alert", "Invalid OTP.");
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

    // 6. Check the Checkbox State (Is this an Admin or Voter?)
    // Make sure your checkbox in Qt Designer is named 'adminCheckBox'
    bool isAdminRegistration = ui->adminCheckBox->isChecked();

    // ==========================================
    // DATABASE WORKING HERE (MongoDB Integration)
    // Here you will hash the password, generate OTP,
    // and push the User Object to the DB.
    // Example: userDoc.append("role", isAdminRegistration ? "PENDING_ADMIN" : "VOTER");
    // ==========================================

    // 7. Security Cleanup: Clear all inputs so the next person can't see them
    ui->newUsernameInput->clear();
    ui->emailInput->clear();
    ui->cnicInput->clear();
    ui->newPasswordInput->clear();
    ui->confirmPasswordInput->clear();

    // CRITICAL: Uncheck the box so it doesn't stay checked for the next user!
    ui->adminCheckBox->setChecked(false);

    // 8. Dynamic Routing Logic
    if (isAdminRegistration) {
        // --- ADMIN FLOW ---
        QMessageBox::information(this, "Pending Authorization",
                                 "Admin request submitted. Please wait for Two-Person authorization.");

        // Route to Page 3 (Wait for Admin Approval) -> Index 2
        ui->stackedWidget->setCurrentIndex(2);

    } else {
        // --- NORMAL VOTER FLOW ---
        QMessageBox::information(this, "Registration Successful",
                                 "Account created successfully! Welcome to the dashboard.");

        // Route to Page 4 (Voter Dashboard) -> Index 3
        ui->stackedWidget->setCurrentIndex(3);
    }
}

void MainWindow::on_adminWaitBackBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

}

