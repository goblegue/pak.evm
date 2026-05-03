#include "OfflineAdminAuthPage.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QFormLayout>

OfflineAdminAuthPage::OfflineAdminAuthPage(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void OfflineAdminAuthPage::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // 1. TOP BAR (Red/Black Gradient & Logo)
    // ==========================================
    QFrame *topBar = new QFrame(this);
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #101010); border: none; }");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 10, 20, 10);

    // Circular Logo
    logoLabel = new QLabel(this);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent;");
    QPixmap originalLogo(":/resource/pak.evm-logo.png");
    if (!originalLogo.isNull()) {
        QPixmap circularLogo(50, 50); circularLogo.fill(Qt::transparent);
        QPainter painter(&circularLogo); painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path; path.addEllipse(0, 0, 50, 50);
        painter.setClipPath(path); painter.drawPixmap(0, 0, 50, 50, originalLogo);
        logoLabel->setPixmap(circularLogo);
    }

    QLabel *appTitleLabel = new QLabel("PAK.EVM", this);
    appTitleLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: white; letter-spacing: 2px; background: transparent;");

    topBarLayout->addWidget(logoLabel);
    topBarLayout->addSpacing(15);
    topBarLayout->addWidget(appTitleLabel);
    topBarLayout->addStretch();
    mainLayout->addWidget(topBar);

    // ==========================================
    // 2. BACK BUTTON (Top Left under Top Bar)
    // ==========================================
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setContentsMargins(20, 20, 20, 0);

    backBtn = new QPushButton("⬅ Back", this);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedSize(180, 45);

    // YOUR REQUESTED STYLE: Red Text, White Background, Red Border, Light Red Hover
    backBtn->setStyleSheet(
        "QPushButton { background-color: white; color: #7A1A1A; border: 2px solid #7A1A1A; border-radius: 6px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #FFE5E5; }" // Light red hover
        );

    navLayout->addWidget(backBtn);
    navLayout->addStretch();
    mainLayout->addLayout(navLayout);

    // ==========================================
    // 3. CENTER LOGIN CARD
    // ==========================================
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setAlignment(Qt::AlignCenter);

    QFrame *authCard = new QFrame(this);
    authCard->setFixedSize(500, 350);
    authCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #BDC3C7; border-radius: 12px; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(authCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *title = new QLabel("Admin Authorization", authCard);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #2C3E50; border: none;");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subTitle = new QLabel("Please enter your Poll Worker credentials.", authCard);
    subTitle->setStyleSheet("font-size: 14px; color: #7F8C8D; border: none; margin-bottom: 20px;");
    subTitle->setAlignment(Qt::AlignCenter);

    // YOUR REQUESTED STYLE: Red borders on inputs
    QString inputStyle = "QLineEdit { border: 2px solid #7A1A1A; border-radius: 6px; padding: 10px; font-size: 16px; color: #2C3E50; background-color: white; }"
                         "QLineEdit:focus { background-color: #FDF2F2; }"; // Slight red tint on focus

    cnicInput = new QLineEdit(authCard);
    cnicInput->setPlaceholderText("Admin CNIC");
    cnicInput->setStyleSheet(inputStyle);

    passwordInput = new QLineEdit(authCard);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet(inputStyle);

    loginBtn = new QPushButton("Verify & Login", authCard);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setFixedHeight(45);
    // YOUR REQUESTED STYLE: Red border (Solid red background to match inputs)
    loginBtn->setStyleSheet(
        "QPushButton { background-color: #7A1A1A; color: white; border: 2px solid #7A1A1A; border-radius: 6px; font-size: 16px; font-weight: bold; margin-top: 15px; }"
        "QPushButton:hover { background-color: #580000; border: 2px solid #580000; }"
        );

    cardLayout->addWidget(title);
    cardLayout->addWidget(subTitle);
    cardLayout->addWidget(cnicInput);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(passwordInput);
    cardLayout->addWidget(loginBtn);

    centerLayout->addWidget(authCard);
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    // Connections
    connect(backBtn, &QPushButton::clicked, this, [this](){ emit backToScanRequested(); });
    connect(loginBtn, &QPushButton::clicked, this, &OfflineAdminAuthPage::onLoginClicked);
}

void OfflineAdminAuthPage::resetForm() {
    cnicInput->clear();
    passwordInput->clear();
    cnicInput->setFocus();
}

void OfflineAdminAuthPage::onLoginClicked() {
    QString cnic = cnicInput->text().trimmed();
    QString pass = passwordInput->text();

    if (cnic.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter both CNIC and Password.");
        return;
    }

    // ==========================================
    // MOCK AUTHENTICATION
    // ==========================================
    // Your backend developer will replace this with: AuthManager::getInstance().login(...)
    if (cnic == "admin" && pass == "123") {
        emit authSuccessful(cnic); // SUCCESS!
    } else {
        QMessageBox::critical(this, "Access Denied", "Invalid Admin credentials.");
        passwordInput->clear();
        passwordInput->setFocus();
    }
}
