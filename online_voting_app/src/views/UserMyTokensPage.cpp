#include "UserMyTokensPage.h"
#include "Delegates.h"
#include <QFontDatabase>

UserMyTokensPage::UserMyTokensPage(QWidget *parent) : QWidget(parent) {
    tokenModel = new TokenListModel(this);
    setupUi();
}

void UserMyTokensPage::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(25);

    // ==========================================
    // LEFT SIDE: TOKEN LIST (Fixed Width)
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setMinimumWidth(350);
    leftPanel->setMaximumWidth(400);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("<b>My Digital Tokens</b>", this);
    titleLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    tokenListView = new QListView(this);
    tokenListView->setModel(tokenModel);
    tokenListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenListView->setSpacing(10);
    tokenListView->setItemDelegate(new TokenDelegate(this));
    tokenListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    tokenListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(tokenListView);

    // ==========================================
    // RIGHT SIDE: THE SECURE TICKET
    // ==========================================
    rightStackedWidget = new QStackedWidget(this);

    // --- PAGE 0: PLACEHOLDER ---
    placeholderWidget = new QWidget(this);
    QVBoxLayout *phLayout = new QVBoxLayout(placeholderWidget);
    QLabel *phLabel = new QLabel("👈 Select a token from your wallet to view details.", this);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setStyleSheet("font-size: 18px; color: #7F8C8D; font-style: italic;");
    phLayout->addWidget(phLabel);
    rightStackedWidget->addWidget(placeholderWidget); // Index 0

    // --- PAGE 1: DIGITAL TICKET ---
    ticketContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(ticketContainer);
    rightLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // The Ticket Frame (Looks like a boarding pass)
    QFrame *ticketFrame = new QFrame(this);
    ticketFrame->setFixedWidth(500);
    ticketFrame->setStyleSheet("QFrame { background-color: white; border-radius: 12px; border: 2px solid #2980B9; }");
    QVBoxLayout *ticketLayout = new QVBoxLayout(ticketFrame);
    ticketLayout->setContentsMargins(30, 30, 30, 30);
    ticketLayout->setSpacing(20);

    // Ticket Header
    QLabel *ticketHeader = new QLabel("OFFICIAL VOTING TOKEN", this);
    ticketHeader->setAlignment(Qt::AlignCenter);
    ticketHeader->setStyleSheet("font-size: 20px; font-weight: bold; color: #2980B9; border: none; border-bottom: 2px dashed #BDC3C7; padding-bottom: 10px;");

    // Ticket Details
    ticketElectionIdLabel = new QLabel("Election: -", this);
    ticketIssueDateLabel = new QLabel("Issued: -", this);
    ticketStationLabel = new QLabel("Station: -", this);
    QString detailStyle = "font-size: 16px; color: #34495E; font-weight: bold; border: none;";
    ticketElectionIdLabel->setStyleSheet(detailStyle);
    ticketIssueDateLabel->setStyleSheet(detailStyle);
    ticketStationLabel->setStyleSheet(detailStyle);

    // The Cryptographic Hash (Monospaced Hacker Font)
    QLabel *hashTitle = new QLabel("SECURE CRYPTOGRAPHIC SIGNATURE:", this);
    hashTitle->setStyleSheet("font-size: 12px; color: #7F8C8D; font-weight: bold; border: none; margin-top: 15px;");

    hashStringLabel = new QLabel("...", this);
    hashStringLabel->setWordWrap(true);
    hashStringLabel->setAlignment(Qt::AlignCenter);

    // Use a fixed-width (monospace) font for the hash to make it look cryptographic
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(11);
    hashStringLabel->setFont(monoFont);
    hashStringLabel->setStyleSheet("background-color: #F4F6F6; color: #E74C3C; border: 1px solid #BDC3C7; border-radius: 4px; padding: 15px;");

    // QR Code Placeholder Text
    QLabel *qrHint = new QLabel("Present this secure token at the physical voting station.", this);
    qrHint->setAlignment(Qt::AlignCenter);
    qrHint->setStyleSheet("font-size: 14px; color: #27AE60; font-weight: bold; border: none; margin-top: 10px;");

    // Assemble the Ticket
    ticketLayout->addWidget(ticketHeader);
    ticketLayout->addWidget(ticketElectionIdLabel);
    ticketLayout->addWidget(ticketStationLabel);
    ticketLayout->addWidget(ticketIssueDateLabel);
    ticketLayout->addWidget(hashTitle);
    ticketLayout->addWidget(hashStringLabel);
    ticketLayout->addWidget(qrHint);

    rightLayout->addSpacing(40); // Push ticket down a bit from the top
    rightLayout->addWidget(ticketFrame);

    rightStackedWidget->addWidget(ticketContainer); // Index 1

    // ==========================================
    // ASSEMBLE MAIN LAYOUT
    // ==========================================
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightStackedWidget);

    connect(tokenListView, &QListView::clicked, this, &UserMyTokensPage::onTokenClicked);
}

void UserMyTokensPage::loadTokens(Token* tokens, int size) {
    tokenModel->setTokens(tokens, size);
    rightStackedWidget->setCurrentIndex(0); // Reset to placeholder
}

void UserMyTokensPage::onTokenClicked(const QModelIndex &index) {
    Token selectedToken = tokenModel->getTokenAt(index.row());

    // Update Ticket Details
    ticketElectionIdLabel->setText("Election ID: " + selectedToken.getElectionId());
    ticketStationLabel->setText("Assigned Station: " + selectedToken.getAssignedStationId());
    ticketIssueDateLabel->setText("Issued: " + selectedToken.getIssuedAt().toString("MMM dd, yyyy - hh:mm AP"));

    // Set the Cryptographic Signature string
    hashStringLabel->setText(selectedToken.getTokenSignature());

    // Swap to the Ticket view
    rightStackedWidget->setCurrentIndex(1);
}
