#include "AdminElectionsPage.h"
#include "Delegates.h"
#include <QPixmap>
#include <QPainter>

AdminElectionsPage::AdminElectionsPage(QWidget *parent) : QWidget(parent) {
    electionModel = new ManageElectionListModel(this);
    proxyModel = new ElectionFilterProxyModel(this);
    proxyModel->setSourceModel(electionModel);

    setupUi();
}

void AdminElectionsPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ==========================================
    // HEADER: TITLE & FILTER
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("<b>Election Management</b>", this);
    titleLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    filterBtn = new QPushButton("Filter Status", this);
    filterBtn->setCursor(Qt::PointingHandCursor);
    filterBtn->setStyleSheet("QPushButton { background-color: #2980B9; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; } QPushButton:hover { background-color: #1A5276; }");

    filterMenu = new QMenu(this);
    filterMenu->setStyleSheet("QMenu { background-color: white; border: 1px solid #BDC3C7; border-radius: 4px; } QMenu::item { padding: 8px 25px 8px 20px; font-weight: bold; } QMenu::item:selected { background-color: #ECF0F1; }");

    auto createColorIcon =[](QColor color) {
        QPixmap pix(16, 16); pix.fill(Qt::transparent);
        QPainter painter(&pix); painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(color); painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(0, 0, 16, 16, 4, 4); return QIcon(pix);
    };

    QAction *actAll = new QAction(createColorIcon(QColor("#2C3E50")), "Show All", this);
    QAction *actDraft = new QAction(createColorIcon(QColor("#95A5A6")), "Draft", this);
    QAction *actRejected = new QAction(createColorIcon(QColor("#E74C3C")), "Rejected", this);
    QAction *actPublished = new QAction(createColorIcon(QColor("#3498DB")), "Published", this);
    QAction *actOpen = new QAction(createColorIcon(QColor("#2ECC71")), "Voting Open", this);
    QAction *actClosed = new QAction(createColorIcon(QColor("#F39C12")), "Voting Closed", this);
    QAction *actResults = new QAction(createColorIcon(QColor("#9B59B6")), "Results Announced", this);

    connect(actAll, &QAction::triggered, this, [this](){ applyFilter(-1, "Filter Status"); });
    connect(actDraft, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::Drafted), "Draft"); });
    connect(actRejected, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::Rejected), "Rejected"); });
    connect(actPublished, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::Published), "Published"); });
    connect(actOpen, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::VotingOpen), "Voting Open"); });
    connect(actClosed, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::VotingClosed), "Voting Closed"); });
    connect(actResults, &QAction::triggered, this, [this](){ applyFilter(static_cast<int>(ElectionState::ResultsAnnounced), "Results Announced"); });

    filterMenu->addAction(actAll); filterMenu->addSeparator();
    filterMenu->addAction(actDraft); filterMenu->addAction(actRejected); filterMenu->addAction(actPublished);
    filterMenu->addAction(actOpen); filterMenu->addAction(actClosed); filterMenu->addAction(actResults);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(filterBtn);

    // ==========================================
    // ACCORDION LIST VIEW
    // ==========================================
    electionListView = new QListView(this);
    electionListView->setMouseTracking(true);
    electionListView->setModel(proxyModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);

    ElectionAccordionDelegate *accordionDelegate = new ElectionAccordionDelegate(this);
    electionListView->setItemDelegate(accordionDelegate);
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    // ==========================================
    // FLOATING '+' BUTTON
    // ==========================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    createElectionBtn = new QPushButton("+", this);
    createElectionBtn->setFixedSize(60, 60);
    createElectionBtn->setCursor(Qt::PointingHandCursor);
    createElectionBtn->setStyleSheet(
        "QPushButton { background-color: #27AE60; color: white; border-radius: 30px; font-size: 30px; font-weight: bold; }"
        "QPushButton:hover { background-color: #219653; box-shadow: 2px 2px 5px rgba(0,0,0,0.3); }"
        );

    bottomLayout->addStretch(); // Push button to the right
    bottomLayout->addWidget(createElectionBtn);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(electionListView);
    mainLayout->addLayout(bottomLayout);

    // Connections
    connect(filterBtn, &QPushButton::clicked, this, &AdminElectionsPage::showFilterMenu);
    connect(createElectionBtn, &QPushButton::clicked, this, &AdminElectionsPage::navigateToCreateElection);

    // Connect the delegate's click to our expand function
    connect(accordionDelegate, &ElectionAccordionDelegate::electionClicked, this, &AdminElectionsPage::onElectionBoxClicked);
}

void AdminElectionsPage::loadElections(Election* elections, int size) {
    electionModel->setElections(elections, size);
}

void AdminElectionsPage::onElectionBoxClicked(const QModelIndex &proxyIndex) {
    QModelIndex realIndex = proxyModel->mapToSource(proxyIndex);

    // Extract ID and tell the model to toggle its expanded state
    QString electionId = electionModel->data(realIndex, Qt::DisplayRole).toString(); // Wait, DisplayRole is Title. Let's get the election directly.
    // Better way:
    QString actualId = electionModel->getElectionAt(realIndex.row()).getId();

    electionModel->toggleExpanded(actualId);
}

void AdminElectionsPage::showFilterMenu() {
    QPoint globalPos = filterBtn->mapToGlobal(QPoint(0, filterBtn->height()));
    filterMenu->popup(globalPos);
}

void AdminElectionsPage::applyFilter(int status, QString filterName) {
    filterBtn->setText(filterName);
    proxyModel->setFilterStatus(status);
}
