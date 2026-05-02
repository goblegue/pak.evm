#include "loginpage.h"
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QtConcurrent>
#include "ui_loginpage.h"

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);
}

LoginPage::~LoginPage()
{
    delete ui;
}

int LoginPage::identifyInputType(const QString &input)
{
    if (input.isEmpty())
        return 0;

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QRegularExpression cnicRegex("^\\d{5}-?\\d{7}-?\\d$");

    if (emailRegex.match(input).hasMatch())
    {
        return 1; // Email
    }

    if (cnicRegex.match(input).hasMatch())
    {
        return 2; // CNIC
    }

    return 0;
}

void LoginPage::on_goToSignupBtn_clicked()
{
    emit goToSignupRequested();
}

void LoginPage::on_loginSubmitBtn_clicked()
{
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter your CNIC or Email and Password.");
        return;
    }

    int inputType = identifyInputType(username);
    AuthManager::LoginResult loginResult;
    if (inputType == 0)
    {
        QMessageBox::warning(this, "Error", "Please enter Correct Format!");
        return;
    }

    // ... validation code above ...

    ui->loginSubmitBtn->setEnabled(false);
    ui->goToSignupBtn->setEnabled(false);
    ui->loginSubmitBtn->setText("Logging in...");

    // 1. Clear previous connections
    disconnect(&m_loginWatcher,
               &QFutureWatcher<AuthManager::LoginResult>::finished,
               nullptr,
               nullptr);

    // 2. THE FIX: ADD THE SEMICOLON HERE (Line 80ish)
    connect(&m_loginWatcher,
            &QFutureWatcher<AuthManager::LoginResult>::finished,
            this,
            &LoginPage::handleLoginFinished);

    // 3. Launch the thread (Use 'auto' to let the compiler handle the template)
    auto future = QtConcurrent::run([=]()
                                    {
        if (inputType == 1) {
            return AuthManager::getInstance().login(password, "", username);
        } else {
            return AuthManager::getInstance().login(password, username, "");
        } });

    // 4. Connect the future to the watcher
    m_loginWatcher.setFuture(future);
}

void LoginPage::handleLoginFinished()
{

    AuthManager::LoginResult loginResult = m_loginWatcher.result();
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
                                         "Verification cancelled. Please login again.");
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
            QMessageBox::information(this, "Admin Verified", "Accessing Admin Portal...");
            emit loginSuccessAdmin();
        }
        else if (loginResult == AuthManager::LoginResult::SuccessUserLoggedIn)
        {
            QMessageBox::information(this, "Voter Verified", "Welcome to the Voting Booth.");
            emit loginSuccessUser();
        }
        else if (loginResult == AuthManager::LoginResult::SuccessAdminPending)
        {
            emit loginSuccessAdminPending();
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
                                         "Verification cancelled. Please login again.");
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
                emit loginSuccessUser();
            }
            else
            {
                QMessageBox::warning(this, "Error", "Incorrect OTP. Please try again.");
            }
        }
    }
    ui->loginSubmitBtn->setEnabled(true);
    ui->goToSignupBtn->setEnabled(true);
    ui->loginSubmitBtn->setText("Login");
}
