#include "OfflineSetupWizard.h"
#include <QFormLayout>
#include <QStyledItemDelegate>
#include <QPainter>

#include "controllers/auth_manager.h"
#include "controllers/election_controller.h"
#include "controllers/system_controller.h"
#include "models/repos/configrepo.h"
#include "services/crypto/crypto_engine.h"

// ==========================================
// CUSTOM DELEGATE TO DRAW THE ADMIN BOXES
// ==========================================
class LocalAdminDelegate : public QStyledItemDelegate
{
public:
    explicit LocalAdminDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), 60); // Height of the box
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect.adjusted(2, 2, -2, -2); // Padding around box

        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor("#7A1A1A"), 2)); // Burgundy border
        painter->drawRoundedRect(rect, 6, 6);

        painter->setPen(QColor("#2C3E50")); // Dark text for Username
        QFont font = option.font;
        font.setBold(true);
        font.setPointSize(14);
        painter->setFont(font);

        QString username = "🧑‍🦰 " + index.data().toString(); // Add a little icon
        painter->drawText(rect.adjusted(15, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, username);

        painter->restore();
    }
};

OfflineSetupWizard::OfflineSetupWizard(QWidget *parent) : QMainWindow(parent)
{
    setupUi();

    this->setFixedSize(1300, 650);
}

void OfflineSetupWizard::setupUi()
{
    this->setStyleSheet("background-color: #FFFFFF;");

    // 1. Create a "container" widget
    QWidget *centralContainer = new QWidget(this);

    // 2. Set this container as the heart of the QMainWindow
    this->setCentralWidget(centralContainer);

    // 3. Apply your layout to the CONTAINER, not 'this'
    QHBoxLayout *mainLayout = new QHBoxLayout(centralContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    buildSidebar(mainLayout);

    wizardStack = new QStackedWidget(this);
    mainLayout->addWidget(wizardStack, 1);

    buildWelcomePage();
    buildMasterSetupPage();
    buildAdminSetupPage();
    buildLoadElectionPage();

    wizardStack->setCurrentIndex(0);
    step1Btn->setChecked(true);
}

void OfflineSetupWizard::buildSidebar(QHBoxLayout *mainLayout)
{
    QFrame *sidebar = new QFrame(this);
    sidebar->setFixedWidth(250);

    // 1. GIVE IT A UNIQUE ID
    sidebar->setObjectName("WizardSidebar");

    // 2. TARGET ONLY THAT ID IN THE STYLESHEET
    sidebar->setStyleSheet("QFrame#WizardSidebar { background:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #101010, stop:1 #580000); border: none; }");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 20, 0, 0);

    QLabel *titleLabel = new QLabel("<b>PAK.EVM SETUP</b>", sidebar);
    // Add 'background: transparent;' so the main gradient shines through the text perfectly
    titleLabel->setStyleSheet("color: white; font-size: 18px; padding: 15px; background: transparent; margin-bottom: 10px;");
    sideLayout->addWidget(titleLabel);

    QString btnStyle = "QPushButton { background-color: transparent; color: #e8eef7; text-align: left; padding: 15px 20px; font-size: 15px; font-weight: bold; border: none; }"
                       "QPushButton:checked { background-color: #7A1A1A; color: white; border-left: 5px solid #ffffff; }";

    step1Btn = new QPushButton("1. Welcome", sidebar);
    step2Btn = new QPushButton("2. Key Setup", sidebar);
    step3Btn = new QPushButton("3. Create Admins", sidebar);
    step4Btn = new QPushButton("4. Load Election Data", sidebar);

    QList<QPushButton *> steps = {step1Btn, step2Btn, step3Btn, step4Btn};
    for (QPushButton *btn : steps)
    {
        btn->setStyleSheet(btnStyle);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setAttribute(Qt::WA_TransparentForMouseEvents);
        btn->setFocusPolicy(Qt::NoFocus);
        sideLayout->addWidget(btn);
    }

    sideLayout->addStretch();
    mainLayout->addWidget(sidebar);
}
void OfflineSetupWizard::buildWelcomePage()
{
    welcomePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(welcomePage);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *title = new QLabel("Welcome to the Secure Voting Terminal", welcomePage);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #2C3E50;");

    QLabel *desc = new QLabel(
        "This wizard will guide you through the initial commissioning of this offline Electronic Voting Machine (EVM).\n\n"
        "During this process, you will generate the cryptographic Master Key required to secure local ballot data. "
        "Ensure you are in a secure environment before proceeding.",
        welcomePage);
    desc->setStyleSheet("font-size: 16px; color: #34495E; line-height: 1.5;");
    desc->setWordWrap(true);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    welcomeNextBtn = new QPushButton("Next: Setup Master Key ➔", welcomePage);
    welcomeNextBtn->setCursor(Qt::PointingHandCursor);
    welcomeNextBtn->setFixedSize(250, 45);
    welcomeNextBtn->setStyleSheet("QPushButton { background-color: #580000; /* Dark reddish color to match the sidebar bottom */color: #FFFFFF;padding: 12px 24px;border: none;border-radius: 8px;font-weight: bold;position: absolute;bottom: 40px;right: 40px; }"
                                  "QPushButton:hover { background-color: #7A1A1A; }");

    bottomLayout->addStretch();
    bottomLayout->addWidget(welcomeNextBtn);

    layout->addWidget(title);
    layout->addSpacing(20);
    layout->addWidget(desc);
    layout->addStretch();
    layout->addLayout(bottomLayout);

    wizardStack->addWidget(welcomePage);

    connect(welcomeNextBtn, &QPushButton::clicked, this, &OfflineSetupWizard::goToMasterSetup);
}

void OfflineSetupWizard::buildMasterSetupPage()
{
    masterSetupPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(masterSetupPage);
    layout->setContentsMargins(40, 40, 40, 40);

    // --- 1. Title ---
    QLabel *title = new QLabel("Key Setup", masterSetupPage);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #2C3E50;");

    // --- 2. Form Layout ---
    QFormLayout *form = new QFormLayout();
    form->setVerticalSpacing(20); // Slightly tighter spacing so everything fits nicely

    QString pwdStyle = "QLineEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px; font-size: 16px; background-color: white; color: #2C3E50; }"
                       "QLineEdit:focus { border: 2px solid #7A1A1A; }";

    QString pubKeyStyle = "QLineEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px; font-size: 16px; background-color: white; color: #2C3E50; margin-top:50px }"
                          "QLineEdit:focus { border: 2px solid #7A1A1A; }";
    // Inputs
    passInput = new QLineEdit(masterSetupPage);
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setStyleSheet(pwdStyle);
    passInput->setPlaceholderText("Enter Master Password");

    confirmPassInput = new QLineEdit(masterSetupPage);
    confirmPassInput->setEchoMode(QLineEdit::Password);
    confirmPassInput->setStyleSheet(pwdStyle);
    confirmPassInput->setPlaceholderText("Confirm Master Password");

    publicKeyInput = new QLineEdit(masterSetupPage);
    publicKeyInput->setStyleSheet(pubKeyStyle);
    publicKeyInput->setPlaceholderText("Enter the Commission's Public Key (e.g., Base64 String)");
    publicKeyInput->setFont(QFont("Courier", 12));

    // Labels
    QString pwdLabelStyle = "font-size: 16px; font-weight: bold; color: #2C3E50;";
    QString pubKeyLabelStyle = "font-size: 16px; font-weight: bold; color: #2C3E50; margin-top:50px";

    QLabel *l1 = new QLabel("Master Password:", masterSetupPage);
    l1->setStyleSheet(pwdLabelStyle);
    QLabel *l2 = new QLabel("Confirm Password:", masterSetupPage);
    l2->setStyleSheet(pwdLabelStyle);
    QLabel *l3 = new QLabel("Commission Public Key:", masterSetupPage);
    l3->setStyleSheet(pubKeyLabelStyle);

    // Add Password Rows
    form->addRow(l1, passInput);
    form->addRow(l2, confirmPassInput);

    QLabel *desc = new QLabel(
        "Enter the Master Password. This password acts as the Root of Trust for the entire system.\n"
        "Warning: Do not share this password with any Unauthorized person!",
        masterSetupPage);

    // Styled red, with a top margin to push it away from the Confirm Password box
    desc->setStyleSheet("font-size: 15px; color: #E74C3C; font-weight: bold; margin-top: 25px; margin-bottom: 20px;");
    desc->setWordWrap(true);

    // Inserting it into the form automatically spans it across the whole width
    form->addRow(desc);
    // ==========================================

    // Add Public Key Row underneath the red text
    form->addRow(l3, publicKeyInput);
    QLabel *desc_2 = new QLabel("Enter the Public key  that has been sent to your Email Address!", masterSetupPage);
    desc_2->setStyleSheet("font-size: 15px; color: #E74C3C; font-weight: bold; margin-top: 10px; margin-bottom: 10px;");
    desc_2->setWordWrap(true);
    form->addRow(desc_2);

    // Styled red, with a top margin to push it away from the Confirm Password box
    desc->setStyleSheet("font-size: 15px; color: #E74C3C; font-weight: bold; margin-top: 20px; margin-bottom: 30px;");
    desc->setWordWrap(true);

    // --- 4. Bottom Next/Back Buttons ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    masterBackBtn = new QPushButton("⬅ Back", masterSetupPage);
    masterBackBtn->setCursor(Qt::PointingHandCursor);
    masterBackBtn->setFixedSize(120, 45);
    masterBackBtn->setStyleSheet("QPushButton { background-color: transparent; color: #7A1A1A; border: 2px solid #7A1A1A; border-radius: 6px; font-size: 16px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #FFE5E5; }");

    masterNextBtn = new QPushButton("Next: Create Admins ➔", masterSetupPage);
    masterNextBtn->setCursor(Qt::PointingHandCursor);
    masterNextBtn->setFixedSize(250, 45);
    masterNextBtn->setStyleSheet("QPushButton { background-color: #580000; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #7A1A1A; }");

    bottomLayout->addWidget(masterBackBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(masterNextBtn);

    // --- 5. Assemble the Page ---
    layout->addWidget(title);
    layout->addSpacing(30);
    layout->addLayout(form);
    layout->addStretch();
    layout->addLayout(bottomLayout);

    wizardStack->addWidget(masterSetupPage);

    // Connections
    connect(masterBackBtn, &QPushButton::clicked, this, &OfflineSetupWizard::goBackToWelcome);
    connect(masterNextBtn, &QPushButton::clicked, this, &OfflineSetupWizard::processMasterSetup);
}

void OfflineSetupWizard::goToMasterSetup()
{
    wizardStack->setCurrentIndex(1);
    step2Btn->setChecked(true);
    this->setFocus();
}

void OfflineSetupWizard::goBackToWelcome()
{
    wizardStack->setCurrentIndex(0);
    step1Btn->setChecked(true);
}

void OfflineSetupWizard::buildAdminSetupPage()
{
    adminSetupPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(adminSetupPage);
    layout->setContentsMargins(40, 40, 40, 40);

    // --- Title ---
    QLabel *title = new QLabel("Create Local Administrators", adminSetupPage);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");

    QLabel *desc = new QLabel("Register the polling officers who will manage this machine. You must create at least one admin to proceed.", adminSetupPage);
    desc->setStyleSheet("font-size: 15px; color: #34495E; margin-bottom: 15px;");
    desc->setWordWrap(true);

    // --- Split Layout (Form on Left, List on Right) ---
    QHBoxLayout *splitLayout = new QHBoxLayout();
    splitLayout->setSpacing(40);

    // LEFT SIDE: Form
    QVBoxLayout *formContainer = new QVBoxLayout();
    QFormLayout *form = new QFormLayout();
    form->setVerticalSpacing(15);

    QString inputStyle = "QLineEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px; font-size: 14px; background-color: white; }"
                         "QLineEdit:focus { border: 2px solid #7A1A1A; }";
    QString labelStyle = "font-size: 15px; font-weight: bold; color: #2C3E50;";

    adminUserIn = new QLineEdit(adminSetupPage);
    adminUserIn->setStyleSheet(inputStyle);
    adminCnicIn = new QLineEdit(adminSetupPage);
    adminCnicIn->setStyleSheet(inputStyle);
    adminPassIn = new QLineEdit(adminSetupPage);
    adminPassIn->setStyleSheet(inputStyle);
    adminPassIn->setEchoMode(QLineEdit::Password);
    adminConfirmIn = new QLineEdit(adminSetupPage);
    adminConfirmIn->setStyleSheet(inputStyle);
    adminConfirmIn->setEchoMode(QLineEdit::Password);

    QLabel *l1 = new QLabel("Username:", adminSetupPage);
    l1->setStyleSheet(labelStyle);
    QLabel *l2 = new QLabel("CNIC:", adminSetupPage);
    l2->setStyleSheet(labelStyle);
    QLabel *l3 = new QLabel("Password:", adminSetupPage);
    l3->setStyleSheet(labelStyle);
    QLabel *l4 = new QLabel("Confirm Password:", adminSetupPage);
    l4->setStyleSheet(labelStyle);

    form->addRow(l1, adminUserIn);
    form->addRow(l2, adminCnicIn);
    form->addRow(l3, adminPassIn);
    form->addRow(l4, adminConfirmIn);

    // Floating (+) Button under the form, aligned to the right!
    QHBoxLayout *addBtnLayout = new QHBoxLayout();
    addAdminBtn = new QPushButton("Add Admin", adminSetupPage);
    addAdminBtn->setCursor(Qt::PointingHandCursor);
    addAdminBtn->setFixedSize(150, 40);
    addAdminBtn->setStyleSheet("QPushButton { background-color: #580000; color: white; border-radius: 20px; font-weight: bold; font-size: 14px; }"
                               "QPushButton:hover { background-color: #7A1A1A; }");
    addBtnLayout->addStretch();
    addBtnLayout->addWidget(addAdminBtn);

    formContainer->addLayout(form);
    formContainer->addLayout(addBtnLayout);
    formContainer->addStretch(); // Push form to top

    // RIGHT SIDE: The List of Created Admins
    QVBoxLayout *listContainer = new QVBoxLayout();
    QLabel *listTitle = new QLabel("<b>Added Administrators</b>", adminSetupPage);
    listTitle->setStyleSheet("font-size: 16px; color: #2C3E50;");

    adminListWidget = new QListWidget(adminSetupPage);
    adminListWidget->setItemDelegate(new LocalAdminDelegate(this));
    adminListWidget->setStyleSheet("QListWidget { border: none; background: transparent; outline: none; }");
    adminListWidget->setSpacing(5);

    listContainer->addWidget(listTitle);
    listContainer->addWidget(adminListWidget);

    splitLayout->addLayout(formContainer, 1); // 1 part width
    splitLayout->addLayout(listContainer, 1); // 1 part width

    // --- Bottom Next/Back Buttons ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    adminBackBtn = new QPushButton("⬅ Back", adminSetupPage);
    adminBackBtn->setCursor(Qt::PointingHandCursor);
    adminBackBtn->setFixedSize(120, 45);
    adminBackBtn->setStyleSheet("QPushButton { background-color: transparent; color: #7A1A1A; border: 2px solid #7A1A1A; border-radius: 6px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #FFE5E5; }");

    adminNextBtn = new QPushButton("Next: Load Election ➔", adminSetupPage);
    adminNextBtn->setCursor(Qt::PointingHandCursor);
    adminNextBtn->setFixedSize(250, 45);
    adminNextBtn->setEnabled(false); // Disabled until at least 1 admin is added!
    adminNextBtn->setStyleSheet("QPushButton { background-color: #580000; color: white; border-radius: 8px; font-size: 16px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #7A1A1A; }"
                                "QPushButton:disabled { background-color: #BDC3C7; color: #ECF0F1; }");

    bottomLayout->addWidget(adminBackBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(adminNextBtn);

    // --- Assemble ---
    layout->addWidget(title);
    layout->addWidget(desc);
    layout->addLayout(splitLayout);
    layout->addLayout(bottomLayout);

    wizardStack->addWidget(adminSetupPage);

    // Connections
    connect(addAdminBtn, &QPushButton::clicked, this, &OfflineSetupWizard::onAddAdminClicked);
    connect(adminBackBtn, &QPushButton::clicked, this, &OfflineSetupWizard::goBackToMasterSetup);
    connect(adminNextBtn, &QPushButton::clicked, this, &OfflineSetupWizard::processAdminSetup);
}

// ---------------------------------------------------------
// NEW LOGIC & VALIDATION FUNCTIONS
// ---------------------------------------------------------

// Change processMasterSetup() to swap to the new page instead of ending!
void OfflineSetupWizard::processMasterSetup()
{
    QString pass = passInput->text();
    QString confirm = confirmPassInput->text();
    QString pubKey = publicKeyInput->text().trimmed();

    if (pass.isEmpty() || confirm.isEmpty() || pubKey.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Fields cannot be empty.");
        return;
    }
    if (pass != confirm)
    {
        QMessageBox::critical(this, "Security Error", "Passwords do not match!");
        passInput->clear();
        confirmPassInput->clear();
        passInput->setFocus();
        return;
    }

    // [BACKEND INTEGRATION: CRYPTOGRAPHY]
    if (!CryptoEngine::getInstance().generateAndStoreKeyPair(pass))
    {
        QMessageBox::critical(this, "Crypto Error", "Failed to generate local security keys!");
        return;
    }

    // [BACKEND INTEGRATION: SAVE PUBLIC KEY]
    if (!SystemController::getInstance().saveSystemPublicKey(pubKey))
    {
        QMessageBox::critical(this, "Database Error", "Failed to save the Commission's Public Key!");
        return;
    }

    wizardStack->setCurrentIndex(2);
    step3Btn->setChecked(true);
    this->setFocus();
}

void OfflineSetupWizard::goBackToMasterSetup()
{
    wizardStack->setCurrentIndex(1);
    step2Btn->setChecked(true);
    this->setFocus();
}
void OfflineSetupWizard::onAddAdminClicked()
{
    QString user = adminUserIn->text().trimmed();
    QString cnic = adminCnicIn->text().trimmed();
    QString pass = adminPassIn->text();
    QString conf = adminConfirmIn->text();

    if (user.isEmpty() || cnic.isEmpty() || pass.isEmpty() || conf.isEmpty())
    {
        QMessageBox::warning(this, "Error", "All fields must be filled out.");
        return;
    }
    if (pass != conf)
    {
        QMessageBox::warning(this, "Error", "Passwords do not match.");
        return;
    }

    // [BACKEND INTEGRATION: CREATE WORKER]
    bool success = AuthManager::getInstance().createOperationalWorker(user, pass);
    if (!success)
    {
        QMessageBox::critical(this, "Database Error", "Failed to save the Poll Worker.");
        return;
    }

    // [CRITICAL FIX: LOGIN THE FIRST WORKER AUTOMATICALLY]
    // The ElectionController requires a logged-in worker to load the USB data!
    if (!AuthManager::getInstance().isWorkerLoggedIn())
    {
        AuthManager::getInstance().loginWorker(user, pass);
    }

    adminListWidget->addItem(user);
    adminUserIn->clear();
    adminCnicIn->clear();
    adminPassIn->clear();
    adminConfirmIn->clear();
    adminUserIn->setFocus();
    adminNextBtn->setEnabled(true);
}

void OfflineSetupWizard::buildLoadElectionPage()
{
    loadElectionPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(loadElectionPage);
    layout->setContentsMargins(40, 40, 40, 40);

    // --- Title ---
    QLabel *title = new QLabel("Load Election Configuration", loadElectionPage);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #2C3E50;");

    QLabel *desc = new QLabel("Please browse for the official Election Configuration File (.json) provided by the Election Commission. This file contains the candidates, rules, and cryptographic public keys.", loadElectionPage);
    desc->setStyleSheet("font-size: 15px; color: #34495E; margin-bottom: 15px;");
    desc->setWordWrap(true);

    // --- Browse Section ---
    QHBoxLayout *browseLayout = new QHBoxLayout();
    browseFileBtn = new QPushButton("Browse File...", loadElectionPage);
    browseFileBtn->setCursor(Qt::PointingHandCursor);
    browseFileBtn->setFixedSize(180, 40);
    browseFileBtn->setStyleSheet("QPushButton { background-color: #580000; color: white; border-radius: 6px; font-size: 14px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #7A1A1A; }");

    fileNameLabel = new QLabel("No file selected.", loadElectionPage);
    fileNameLabel->setStyleSheet("font-size: 14px; color: #7F8C8D; font-style: italic; margin-left:5px;");

    browseLayout->addWidget(browseFileBtn);
    browseLayout->addWidget(fileNameLabel);
    browseLayout->addStretch();

    // --- File Content Display Area ---
    fileDataDisplay = new QTextEdit(loadElectionPage);
    fileDataDisplay->setReadOnly(true); // User cannot type in here!
    fileDataDisplay->setStyleSheet("QTextEdit { background-color: white; border: 2px solid #BDC3C7; border-radius: 6px; padding: 15px; font-family: 'Courier New', monospace; font-size: 14px; color: #2C3E50; }");
    fileDataDisplay->setPlaceholderText("File contents will appear here...");

    // --- Bottom Next/Back Buttons ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    loadBackBtn = new QPushButton("⬅ Back", loadElectionPage);
    loadBackBtn->setCursor(Qt::PointingHandCursor);
    loadBackBtn->setFixedSize(120, 45);
    loadBackBtn->setStyleSheet("QPushButton { background-color: transparent; color: #7A1A1A; border: 2px solid #7A1A1A; border-radius: 6px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #FFE5E5; }");

    loadNextBtn = new QPushButton("Next: Finalize Setup ➔", loadElectionPage);
    loadNextBtn->setCursor(Qt::PointingHandCursor);
    loadNextBtn->setFixedSize(250, 45);
    loadNextBtn->setEnabled(false); // Locked until file is loaded!
    loadNextBtn->setStyleSheet("QPushButton { background-color: #580000; color: white; border-radius: 8px; font-size: 16px; font-weight: bold; }"
                               "QPushButton:hover { background-color: #7A1A1A; }"
                               "QPushButton:disabled { background-color: #BDC3C7; color: #ECF0F1; }");

    bottomLayout->addWidget(loadBackBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(loadNextBtn);

    // --- Assemble ---
    layout->addWidget(title);
    layout->addWidget(desc);
    layout->addLayout(browseLayout);
    layout->addSpacing(10);
    layout->addWidget(fileDataDisplay, 1); // 1 = stretch to fill available vertical space
    layout->addSpacing(20);
    layout->addLayout(bottomLayout);

    wizardStack->addWidget(loadElectionPage);

    // Connections
    connect(browseFileBtn, &QPushButton::clicked, this, &OfflineSetupWizard::onBrowseFileClicked);
    connect(loadBackBtn, &QPushButton::clicked, this, &OfflineSetupWizard::goBackToAdminSetup);
    connect(loadNextBtn, &QPushButton::clicked, this, &OfflineSetupWizard::processLoadElection);
}

// ==========================================
// NEW NAVIGATION & FILE LOGIC
// ==========================================

void OfflineSetupWizard::processAdminSetup()
{
    // Go from Page 3 (Admins) to Page 4 (Load File)
    wizardStack->setCurrentIndex(3);
    step4Btn->setChecked(true);
}

void OfflineSetupWizard::goBackToAdminSetup()
{
    // Go back to Page 3
    wizardStack->setCurrentIndex(2);
    step3Btn->setChecked(true);
}

void OfflineSetupWizard::onBrowseFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select Election Configuration File",
                                                    "",
                                                    "JSON Files (*.json);;All Files (*.*)");
    if (filePath.isEmpty())
        return;

    // [NEW] Save the path for processLoadElection()
    loadedFilePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "File Error", "Could not read the selected file.");
        return;
    }

    QTextStream in(&file);
    fileDataDisplay->setText(in.readAll());
    file.close();

    QFileInfo fileInfo(filePath);
    fileNameLabel->setText("Loaded: " + fileInfo.fileName());
    fileNameLabel->setStyleSheet("font-size: 14px; color: #27AE60; font-weight: bold;");

    loadNextBtn->setEnabled(true);
}

void OfflineSetupWizard::processLoadElection()
{
    if (loadedFilePath.isEmpty())
        return;

    //[BACKEND INTEGRATION: LOAD USB JSON]
    bool success = ElectionController::getInstance().loadElectionDataFromUSB(loadedFilePath);

    if (!success)
    {
        QMessageBox::critical(this,
                              "Data Retrieval Error",
                              "Failed to load election configuration from the file. Please check the log for details ");
        return;
    }

    QMessageBox::information(this,
                             "Success",
                             "Terminal Configured Successfully!\n\nEntering Kiosk Mode...");
    emit setupComplete();
}
/*void OfflineSetupWizard::finishSetup() {
    QMessageBox::information(this, "Setup Complete", "The machine has been successfully configured and local administrators have been registered!\n\nLaunching Kiosk Mode...");

    emit setupFinished(); // Tell the world we are done!
    this->close();        // Close this window
}
*/
