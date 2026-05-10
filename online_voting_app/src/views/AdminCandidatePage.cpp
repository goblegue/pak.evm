#include "views/AdminCandidatePage.h"
#include "views/Delegates.h"
#include <QPixmap>
#include <QPainter>

AdminCandidatePage::AdminCandidatePage(QWidget *parent) : QWidget(parent)
{
    electionModel = new ElectionListModel(this);
    candidateModel = new CandidateListModel(this);

    
    proxyModel = new CandidateFilterProxyModel(this);
    proxyModel->setSourceModel(candidateModel);

    setupUi();
}

void AdminCandidatePage::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(25);

    
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QLabel *electionLabel = new QLabel("<b>Select Election</b>", this);
    electionLabel->setStyleSheet("font-size: 16px; color: #2C3E50;");
    electionListView = new QListView(this);
    electionListView->setMouseTracking(true);
    electionListView->setModel(electionModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this));
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);

    
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QHBoxLayout *rightHeaderLayout = new QHBoxLayout();
    QLabel *candidateLabel = new QLabel("<b>Registered Candidates</b>", this);
    candidateLabel->setStyleSheet("font-size: 16px; color: #2C3E50;");

    filterBtn = new QPushButton("Filter Status", this);
    filterBtn->setCursor(Qt::PointingHandCursor);
    filterBtn->setStyleSheet(
        "QPushButton { background-color: #2980B9; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1A5276; }");

    
    filterMenu = new QMenu(this);
    filterMenu->setStyleSheet("QMenu { background-color: white; border: 1px solid #BDC3C7; border-radius: 4px; }"
                              "QMenu::item { padding: 8px 25px 8px 20px; color: #2C3E50; font-weight: bold; }"
                              "QMenu::item:selected { background-color: #ECF0F1; }");

    
    auto createColorIcon = [](QColor color)
    {
        QPixmap pix(16, 16);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(0, 0, 16, 16, 4, 4); 
        return QIcon(pix);
    };

    
    QAction *actAll = new QAction(createColorIcon(QColor("#2980B9")), "Show All", this);
    QAction *actPending = new QAction(createColorIcon(QColor("#F39C12")), "Pending", this);
    QAction *actApproved = new QAction(createColorIcon(QColor("#27AE60")), "Approved", this);
    QAction *actRejected = new QAction(createColorIcon(QColor("#C0392B")), "Rejected", this);

    
    connect(actAll, &QAction::triggered, this, [this]()
            { applyFilter(-1, "Filter Status"); });
    connect(actPending, &QAction::triggered, this, [this]()
            { applyFilter(static_cast<int>(ApprovalStatus::Pending), "Pending"); });
    connect(actApproved, &QAction::triggered, this, [this]()
            { applyFilter(static_cast<int>(ApprovalStatus::Approved), "Approved"); });
    connect(actRejected, &QAction::triggered, this, [this]()
            { applyFilter(static_cast<int>(ApprovalStatus::Rejected), "Rejected"); });

    filterMenu->addAction(actAll);
    filterMenu->addSeparator();
    filterMenu->addAction(actPending);
    filterMenu->addAction(actApproved);
    filterMenu->addAction(actRejected);
    

    rightHeaderLayout->addWidget(candidateLabel);
    rightHeaderLayout->addStretch();
    rightHeaderLayout->addWidget(filterBtn);

    candidateListView = new QListView(this);

    
    candidateListView->setModel(proxyModel);
    candidateListView->setMouseTracking(true);
    candidateListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    candidateListView->setSpacing(10);
    candidateListView->setItemDelegate(new CandidateDelegate(this));
    candidateListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    rightLayout->addLayout(rightHeaderLayout);
    rightLayout->addWidget(candidateListView);

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 2);

    
    connect(electionListView, &QListView::clicked, this, &AdminCandidatePage::onElectionClicked);

    
    connect(candidateListView, &QListView::clicked, this, &AdminCandidatePage::onCandidateClicked);
    
    
    connect(candidateListView, &QListView::clicked, this, &AdminCandidatePage::onCandidateClicked);
}



void AdminCandidatePage::onElectionClicked(const QModelIndex &index)
{
    Election selected = electionModel->getElectionAt(index.row());
    emit electionSelected(selected.getId());
}



void AdminCandidatePage::showFilterMenu()
{
    QPoint globalPos = filterBtn->mapToGlobal(QPoint(0, filterBtn->height()));
    filterMenu->popup(globalPos);
}


void AdminCandidatePage::applyFilter(int status, QString filterName)
{
    
    filterBtn->setText(filterName);

    
    proxyModel->setFilterStatus(status);
}

void AdminCandidatePage::loadElections(Election *elections, int size)
{
    electionModel->setElections(elections, size);
}
void AdminCandidatePage::loadCandidates(Candidate *candidates, int size)
{
    candidateModel->setCandidates(candidates, size);
}

void AdminCandidatePage::onCandidateClicked(const QModelIndex &proxyIndex) {
    
    QModelIndex realIndex = proxyModel->mapToSource(proxyIndex);

    
    Candidate selected = candidateModel->getCandidateAt(realIndex.row());

    
    emit navigateToCandidateDetails(selected);
}
