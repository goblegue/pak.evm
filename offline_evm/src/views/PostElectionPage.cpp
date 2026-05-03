#include "PostElectionPage.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

PostElectionPage::PostElectionPage(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PostElectionPage::setupUi() {
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
    if (!originalLogo.isNull()) {
        QPixmap circularLogo(50, 50);
        circularLogo.fill(Qt::transparent);
        QPainter painter(&circularLogo);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 50, 50);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, 50, 50, originalLogo);
        logoLabel->setPixmap(circularLogo);
    } else {
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

    centerLayout->addWidget(iconLabel);
    centerLayout->addWidget(titleLabel);
    centerLayout->addWidget(subTitleLabel);
    centerLayout->addWidget(footerLabel);

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}
