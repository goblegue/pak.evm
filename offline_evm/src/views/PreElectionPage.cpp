#include "PreElectionPage.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

PreElectionPage::PreElectionPage(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PreElectionPage::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. TOP BAR (Same as Scan Page)
    QFrame *topBar = new QFrame(this);
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #101010); border: none; }");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 10, 20, 10); // Matches the other pages

    // --- Circular Logo ---
    logoLabel = new QLabel(this);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent;");

    QPixmap originalLogo(":/resource/pak.evm-logo.png"); // Use your actual image path!
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

    // --- Title ---
    QLabel *titleLabel = new QLabel("PAK.EVM", this);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: white; letter-spacing: 2px; background: transparent;");

    // Add to layout
    topBarLayout->addWidget(logoLabel);
    topBarLayout->addSpacing(15);
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();

    mainLayout->addWidget(topBar);

    // 2. CENTER COUNTDOWN
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setAlignment(Qt::AlignCenter);

    QLabel *waitingLabel = new QLabel("Voting Begins In:", this);
    waitingLabel->setAlignment(Qt::AlignCenter);
    waitingLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #2C3E50;background: transparent;");

    countdownLabel = new QLabel("00:00:00", this);
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setStyleSheet("font-size: 80px; font-weight: 900; color: #E74C3C; background-color: white; border: 4px solid #BDC3C7; border-radius: 15px; padding: 20px;");

    centerLayout->addWidget(waitingLabel);
    centerLayout->addSpacing(20);
    centerLayout->addWidget(countdownLabel);

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}

void PreElectionPage::updateCountdown(const QString &timeString) {
    countdownLabel->setText(timeString);
}
