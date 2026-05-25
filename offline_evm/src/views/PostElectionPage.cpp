#include "PostElectionPage.h"
#include "controllers/audit_controller.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "controllers/audit_controller.h"
#include "controllers/election_controller.h"
#include "models/repos/DatabaseManager.h"
#include "controllers/auth_manager.h"

PostElectionPage::PostElectionPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void PostElectionPage::setupUi()
{
    this->setObjectName("PostElectionPage");
    this->setStyleSheet("#ScanPageBG { background-color: #f5f7fb; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Flush to edges
    mainLayout->setSpacing(0);

    // ==========================================
    // 1. TOP BAR (Red/Black Gradient & Logo)
    // ==========================================
    QFrame *topBar = new QFrame(this);
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #101010); border: none; }");

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 10, 20, 10);

    // Circular Logo (Matches Scan Page)
    logoLabel = new QLabel(this);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent;");

    QPixmap originalLogo(":/resource/pak.evm-logo.png"); // Make sure your resource path is correct here!
    if (!originalLogo.isNull())
    {
        QPixmap circularLogo(50, 50);
        circularLogo.fill(Qt::transparent);
        QPainter painter(&circularLogo);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 50, 50);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, 50, 50, originalLogo);
        logoLabel->setPixmap(circularLogo);
    }
    else
    {
        logoLabel->setStyleSheet("background-color: white; border-radius: 25px;");
    }

    QLabel *appTitleLabel = new QLabel("PAK.EVM", this);
    appTitleLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: white; letter-spacing: 2px; background: transparent;");

    topBarLayout->addWidget(logoLabel);
    topBarLayout->addSpacing(15);
    topBarLayout->addWidget(appTitleLabel);
    topBarLayout->addStretch();

    mainLayout->addWidget(topBar);

    // ==========================================
    // 2. CENTER CONTENT (Election Closed Message)
    // ==========================================
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setSpacing(20);

    QLabel *iconLabel = new QLabel("🔒", this);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 80px; color: #2C3E50; background: transparent;");

    QLabel *titleLabel = new QLabel("Voting is Closed", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 48px; font-weight: 900; color: #E74C3C; background: transparent;");

    QLabel *subTitleLabel = new QLabel("The election time period has officially ended.\nNo further ballots can be cast on this terminal.", this);
    subTitleLabel->setAlignment(Qt::AlignCenter);
    subTitleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #34495E; background: transparent; line-height: 1.5;");

    QLabel *footerLabel = new QLabel("Please wait for the Election Commission to announce the official results.", this);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet("font-size: 16px; color: #7F8C8D; background: transparent; margin-top: 30px;");

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(30);
    buttonsLayout->setAlignment(Qt::AlignCenter);

    // EXPORT BUTTON (Left side of the gradient: Red -> Dark Red)
    exportResultsBtn = new QPushButton("⬇ Get Election Results", this);
    exportResultsBtn->setCursor(Qt::PointingHandCursor);
    exportResultsBtn->setFixedSize(250, 50);
    exportResultsBtn->setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #340808); "
        "   color: white; border-radius: 8px; font-size: 16px; font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7A1A1A, stop:1 #4A1212); "
        "}");

    // DELETE BUTTON (Right side of the gradient: Dark Red -> Black)
    deleteElectionBtn = new QPushButton("🗑 Delete Election Data", this);
    deleteElectionBtn->setCursor(Qt::PointingHandCursor);
    deleteElectionBtn->setFixedSize(250, 50);
    deleteElectionBtn->setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #340808, stop:1 #101010); "
        "   color: white; border-radius: 8px; font-size: 16px; font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4A1212, stop:1 #2C2C2C); "
        "}");

    buttonsLayout->addWidget(exportResultsBtn);
    buttonsLayout->addWidget(deleteElectionBtn);

    centerLayout->addWidget(iconLabel);
    centerLayout->addWidget(titleLabel);
    centerLayout->addWidget(subTitleLabel);
    centerLayout->addWidget(footerLabel);
    centerLayout->addSpacing(30);
    centerLayout->addLayout(buttonsLayout);

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    connect(exportResultsBtn, &QPushButton::clicked, this, &PostElectionPage::onExportResultsClicked);
    connect(deleteElectionBtn, &QPushButton::clicked, this, &PostElectionPage::onDeleteElectionClicked);
}

void PostElectionPage::onExportResultsClicked()
{
    // 1. Open the OS File Dialog so the user can pick where to save the file
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Election Results",
        "Election_Final_Results.json", // Default file name
        "JSON Files (*.json)");

    // 2. If the user clicks "Cancel" on the dialog, stop here.
    if (filePath.isEmpty())
    {
        return;
    }

    bool success = AuditController::getInstance().exportFinalResults(filePath);

    if (success)
    {
        QMessageBox::information(this, "Export Successful", "The election results and cryptographic proofs have been securely saved to:\n" + filePath);
    }
    else
    {
        QMessageBox::critical(this, "Export Failed", "The forensic audit failed or the file could not be written.");
    }
}

void PostElectionPage::onDeleteElectionClicked()
{
    // 1. Double-check warning! You don't want them to delete before exporting.
    QMessageBox::StandardButton reply = QMessageBox::warning(this, "CRITICAL WARNING",
                                                             "Are you absolutely sure you want to PERMANENTLY wipe all election data from this machine?\n\n"
                                                             "Ensure you have exported the final results first. This action CANNOT be undone.",
                                                             QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes)
    {

        // 2. Open a secure dialog to ask for the Master Password
        bool ok;
        QString masterKey = QInputDialog::getText(this,
                                                  "Master Authorization Required",
                                                  "Enter the Master Password to confirm system wipe:",
                                                  QLineEdit::Password,
                                                  "", &ok);

        // 3. Check if they clicked "OK" and actually typed something
        if (ok)
        {
            if (masterKey.isEmpty())
            {
                QMessageBox::warning(this, "Error", "Master Password cannot be empty.");
                return;
            }

            /*
             * =======================================================
             * WHEN YOUR BACKEND IS READY, UNCOMMENT THIS REAL LOGIC:
             * =======================================================
             *
             * // A. Try to close the election using the Master Password
             * bool authorized = ElectionController::getInstance().closeElection(masterKey);
             *
             * if (authorized) {
             *     // B. If authorized, wipe the database!
             *     if (DatabaseManager::getInstance().cleanupForNewElection()) {
             *         QMessageBox::information(this, "System Wiped", "All election records, candidates, and tokens have been securely wiped.\n\nThe application will now close.");
             *         QApplication::quit(); // Close the app, ready for next election cycle
             *     } else {
             *         QMessageBox::critical(this, "Database Error", "Failed to clear database tables.");
             *     }
             * } else {
             *     QMessageBox::critical(this, "Access Denied", "Incorrect Master Password. Wipe aborted.");
             * }
             */

            // TEMPORARY MOCK FOR UI TESTING:

            bool success = AuthManager::getInstance().unlockMasterAuthority(masterKey);
            if (masterKey == "admin123")
            {
                QMessageBox::information(this, "System Wiped", "Simulation: System wiped successfully! Exiting app...");
                // QApplication::quit();
            }
            else
            {
                QMessageBox::critical(this, "Access Denied", "Simulation: Wrong Password!");
            }
        }
    }
}
