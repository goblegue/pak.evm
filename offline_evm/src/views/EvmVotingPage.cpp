#include "EvmVotingPage.h"
#include "Delegates.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

EvmVotingPage::EvmVotingPage(QWidget *parent) : QWidget(parent) {
    candidateModel = new CandidateListModel(this);
    proxyModel = new CandidateSearchProxyModel(this);
    proxyModel->setSourceModel(candidateModel);

    setupUi();
}

void EvmVotingPage::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // 1. TOP BAR (Red/Black Gradient & Clock)
    // ==========================================
    QFrame *topBar = new QFrame(this);
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #580000, stop:1 #101010); border: none; }");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 10, 20, 10);

    QLabel *logoLabel = new QLabel(this);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent;");
    QPixmap originalLogo(":/images/your_logo.png");
    if (!originalLogo.isNull()) {
        QPixmap circularLogo(50, 50); circularLogo.fill(Qt::transparent);
        QPainter painter(&circularLogo); painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path; path.addEllipse(0, 0, 50, 50);
        painter.setClipPath(path); painter.drawPixmap(0, 0, 50, 50, originalLogo);
        logoLabel->setPixmap(circularLogo);
    }

    QLabel *titleLabel = new QLabel("PAK.EVM", this);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: white; letter-spacing: 2px; background: transparent;");

    timeRemainingLabel = new QLabel("Time Remaining: 00:00:00", this);
    timeRemainingLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white; background: transparent; padding-right: 20px;");

    topBarLayout->addWidget(logoLabel);
    topBarLayout->addSpacing(15);
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(timeRemainingLabel);

    mainLayout->addWidget(topBar);

    // ==========================================
    // 2. SEARCH BAR & GRID LAYOUT
    // ==========================================
    QVBoxLayout *bodyLayout = new QVBoxLayout();
    bodyLayout->setContentsMargins(40, 20, 40, 20);
    bodyLayout->setSpacing(20);

    // Search Bar
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍 Search Candidates by Name or CNIC...");
    searchBar->setStyleSheet("QLineEdit { border: 2px solid #BDC3C7; border-radius: 25px; padding: 10px 20px; font-size: 16px; background-color: white; color: #2C3E50; }"
                             "QLineEdit:focus { border: 2px solid #7A1A1A; }");
    bodyLayout->addWidget(searchBar);

    // Candidate Grid View
    candidateListView = new QListView(this);
    candidateListView->setModel(proxyModel);
    candidateListView->setMouseTracking(true); // MUST BE TRUE FOR HOVER EFFECT!

    // ** MAGIC: Turns the List into a 2-Column Responsive Grid! **
    candidateListView->setViewMode(QListView::IconMode);
    candidateListView->setResizeMode(QListView::Adjust);
    candidateListView->setMovement(QListView::Static);
    candidateListView->setFlow(QListView::LeftToRight);
    candidateListView->setWrapping(true);
    candidateListView->setSpacing(15);

    VotingCandidateDelegate *delegate = new VotingCandidateDelegate(this);
    candidateListView->setItemDelegate(delegate);
    candidateListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    bodyLayout->addWidget(candidateListView);
    mainLayout->addLayout(bodyLayout);

    // Connections
    connect(searchBar, &QLineEdit::textChanged, this, &EvmVotingPage::onSearchTextChanged);
    connect(delegate, &VotingCandidateDelegate::voteButtonClicked, this, &EvmVotingPage::onVoteButtonClicked);
}

void EvmVotingPage::loadCandidates(Candidate* candidates, int size) {
    candidateModel->setCandidates(candidates, size);
}

void EvmVotingPage::updateTimeRemaining(const QString &timeString) {
    timeRemainingLabel->setText("Time Remaining: " + timeString);
}

void EvmVotingPage::onSearchTextChanged(const QString &text) {
    // Instantly filters the grid as they type!
    proxyModel->setFilterRegularExpression(text);
}

void EvmVotingPage::onVoteButtonClicked(const QModelIndex &proxyIndex) {
    QModelIndex realIndex = proxyModel->mapToSource(proxyIndex);
    Candidate selected = candidateModel->getCandidateAt(realIndex.row());
    emit candidateVoted(selected);
}
