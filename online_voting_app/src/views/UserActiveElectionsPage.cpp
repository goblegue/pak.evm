#include "UserActiveElectionsPage.h"
#include "Delegates.h"
#include <QDateTime>

UserActiveElectionsPage::UserActiveElectionsPage(QWidget *parent) : QWidget(parent) {
    electionModel = new ElectionListModel(this);
    candidateModel = new CandidateListModel(this);
    setupUi();
}

void UserActiveElectionsPage::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(25);

    // ==========================================
    // LEFT SIDE: ELECTION LIST (Fixed Width)
    // ==========================================
    // Wrapping it in a QWidget protects it from being squished!
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setMinimumWidth(350);
    leftPanel->setMaximumWidth(450); // Locks the width
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *electionLabel = new QLabel("<b>Active Elections</b>", this);
    electionLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    electionListView = new QListView(this);
    electionListView->setMouseTracking(true);
    electionListView->setModel(electionModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this));
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    electionListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Disable bottom scrollbar

    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);

    // ==========================================
    // RIGHT SIDE: STACKED WIDGET
    // ==========================================
    rightStackedWidget = new QStackedWidget(this);

    // --- PAGE 0: PLACEHOLDER (When nothing is selected) ---
    placeholderWidget = new QWidget(this);
    QVBoxLayout *phLayout = new QVBoxLayout(placeholderWidget);
    QLabel *phLabel = new QLabel("👈 Select an election from the list\nto view details and register.", this);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setStyleSheet("font-size: 18px; color: #7F8C8D; font-style: italic;");
    phLayout->addWidget(phLabel);
    rightStackedWidget->addWidget(placeholderWidget); // Index 0

    // --- PAGE 1: ELECTION DETAILS & CANDIDATES ---
    detailsContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(detailsContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    // Details Card
    QFrame *detailsCard = new QFrame(this);
    detailsCard->setStyleSheet("QFrame { background-color: white; border-radius: 8px; border: 1px solid #BDC3C7; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(detailsCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);

    detailTitleLabel = new QLabel("Select an election", this);
    detailTitleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2980B9; border: none;");

    detailStartTimeLabel = new QLabel("Starts: -", this);
    detailEndTimeLabel = new QLabel("Ends: -", this);
    QString timeStyle = "font-size: 14px; color: #7F8C8D; font-weight: bold; border: none;";
    detailStartTimeLabel->setStyleSheet(timeStyle);
    detailEndTimeLabel->setStyleSheet(timeStyle);

    cardLayout->addWidget(detailTitleLabel);
    cardLayout->addSpacing(5);
    cardLayout->addWidget(detailStartTimeLabel);
    cardLayout->addWidget(detailEndTimeLabel);

    // Generate Token Button
    generateTokenBtn = new QPushButton("Register & Generate Token", this); // Removed emoji to fix font glitch in your screenshot
    generateTokenBtn->setCursor(Qt::PointingHandCursor);
    generateTokenBtn->setFixedHeight(50);
    generateTokenBtn->setStyleSheet(
        "QPushButton { background-color: #27AE60; color: white; border-radius: 8px; font-size: 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #219653; }"
        "QPushButton:disabled { background-color: #BDC3C7; color: #7F8C8D; }"
        );

    // Candidates List
    QLabel *candidateLabel = new QLabel("<b>Approved Candidates</b>", this);
    candidateLabel->setStyleSheet("font-size: 18px; color: #2C3E50; margin-top: 10px;");

    candidateListView = new QListView(this);
    candidateListView->setMouseTracking(true);
    candidateListView->setModel(candidateModel);
    candidateListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    candidateListView->setSpacing(10);
    candidateListView->setItemDelegate(new CandidateDelegate(this));
    candidateListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    rightLayout->addWidget(detailsCard);
    rightLayout->addWidget(generateTokenBtn);
    rightLayout->addWidget(candidateLabel);
    rightLayout->addWidget(candidateListView);

    rightStackedWidget->addWidget(detailsContainer); // Index 1

    // ==========================================
    // ASSEMBLE MAIN LAYOUT
    // ==========================================
    mainLayout->addWidget(leftPanel);            // Left column (Fixed width)
    mainLayout->addWidget(rightStackedWidget);   // Right column (Takes remaining space)

    // Connections
    connect(electionListView, &QListView::clicked, this, &UserActiveElectionsPage::onElectionClicked);
    connect(generateTokenBtn, &QPushButton::clicked, this, &UserActiveElectionsPage::onGenerateTokenClicked);
    connect(candidateListView, &QListView::clicked, this, &UserActiveElectionsPage::onCandidateClicked);

}

void UserActiveElectionsPage::loadElections(Election* elections, int size) {
    electionModel->setElections(elections, size);

    // Always show the empty placeholder when fresh data loads
    rightStackedWidget->setCurrentIndex(0);
}

void UserActiveElectionsPage::loadCandidates(Candidate* candidates, int size) {
    candidateModel->setCandidates(candidates, size);
}

void UserActiveElectionsPage::onElectionClicked(const QModelIndex &index) {
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();

    detailTitleLabel->setText(selected.getTitle());
    detailStartTimeLabel->setText("Starts: " + selected.getStartTime().toString("MMMM dd, yyyy - hh:mm AP"));
    detailEndTimeLabel->setText("Ends: " + selected.getEndTime().toString("MMMM dd, yyyy - hh:mm AP"));

    if(selected.getStatus() == ElectionState::VotingClosed || selected.getStatus() == ElectionState::ResultsAnnounced) {
        generateTokenBtn->setEnabled(false);
        generateTokenBtn->setText("Voting Closed");
    } else {
        generateTokenBtn->setEnabled(true);
        generateTokenBtn->setText("Register & Generate Token");
    }

    // Swap the right panel to show the details instead of the placeholder!
    rightStackedWidget->setCurrentIndex(1);

    emit electionSelected(currentSelectedElectionId);
}

void UserActiveElectionsPage::onGenerateTokenClicked() {
    if (!currentSelectedElectionId.isEmpty()) {
        emit generateTokenRequested(currentSelectedElectionId);
    }
}

void UserActiveElectionsPage::onCandidateClicked(const QModelIndex &index) {
    Candidate selected = candidateModel->getCandidateAt(index.row());
    emit navigateToCandidateDetails(selected);
}
