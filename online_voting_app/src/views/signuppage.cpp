#include "signuppage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include "ui_signuppage.h"

SignupPage::SignupPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::SignupPage)
{
    ui->setupUi(this);
    ui->newPasswordInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->confirmPasswordInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
}

SignupPage::~SignupPage()
{
    delete ui;
}

void SignupPage::on_goToLoginBtn_clicked()
{
    ui->newUsernameInput->clear();
    ui->emailInput->clear();
    ui->cnicInput->clear();
    ui->newPasswordInput->clear();
    ui->confirmPasswordInput->clear();
    emit goToLoginRequested();
}

void SignupPage::lockSignupPage()
{
    // Disable all input fields and buttons to prevent interaction
    ui->newUsernameInput->setEnabled(false);
    ui->emailInput->setEnabled(false);
    ui->cnicInput->setEnabled(false);
    ui->newPasswordInput->setEnabled(false);
    ui->confirmPasswordInput->setEnabled(false);
    ui->adminCheckBox->setEnabled(false);
    ui->signupSubmitBtn->setEnabled(false);
    ui->goToLoginBtn->setEnabled(false);
    ui->signupSubmitBtn->setText("Processing...");
}

void SignupPage::unlockSignupPage()
{
    ui->newUsernameInput->setEnabled(true);
    ui->emailInput->setEnabled(true);

    ui->cnicInput->setEnabled(true);

    ui->newPasswordInput->setEnabled(true);

    ui->confirmPasswordInput->setEnabled(true);

    ui->adminCheckBox->setEnabled(true);
    ui->signupSubmitBtn->setEnabled(true);
    ui->signupSubmitBtn->setText("Submit");
    ui->goToLoginBtn->setEnabled(true);
}
void SignupPage::on_signupSubmitBtn_clicked()
{
    lockSignupPage();
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
        unlockSignupPage();
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
        unlockSignupPage();
        return;
    }

    // 4. Validation Level 3: Email Format Check
    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(newEmail).hasMatch())
    {
        QMessageBox::warning(this, "Format Error", "Please enter a valid email address.");
        ui->emailInput->setFocus();
        unlockSignupPage();
        return;
    }

    // 5. Validation Level 4: CNIC Format Check (Pakistani Standard)
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");
    if (!cnicRegex.match(newCnic).hasMatch())
    {
        QMessageBox::warning(this, "Format Error", "Please enter a valid 13-digit CNIC.");
        ui->cnicInput->setFocus();
        unlockSignupPage();
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
            unlockSignupPage();
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
                emit goToLoginRequested();
                unlockSignupPage();
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
                    emit signupSuccessUser();
                }
                else if (signupResult == AuthManager::SignUpResult::SuccessAdminCreated)
                {
                    QMessageBox::information(
                        this,
                        "Pending Authorization",
                        "Admin request submitted. Please wait for Two-Person authorization.");
                    emit signupSuccessAdminPending();
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
    unlockSignupPage();
}