#include "views/AdminManagementPage.h"
#include "views/Delegates.h"
#include <QPixmap>
#include <QPainter>

AdminManagementPage::AdminManagementPage(QWidget *parent) : QWidget(parent)
{
    adminModel = new AdminListModel(this);
    proxyModel = new AdminFilterProxyModel(this);
    proxyModel->setSourceModel(adminModel);

    setupUi();
}

void AdminManagementPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ==========================================
    // HEADER: TITLE & FILTER BUTTON
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("<b>System Administrators</b>", this);
    titleLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    filterBtn = new QPushButton("Filter Status", this);
    filterBtn->setCursor(Qt::PointingHandCursor);
    filterBtn->setStyleSheet(
        "QPushButton { background-color: #2980B9; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1A5276; }");

    // Filter Menu Setup
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

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(filterBtn);

    // ==========================================
    // FULL WIDTH LIST VIEW
    // ==========================================
    adminListView = new EmptyStateListView("No administrators found.", this);
    adminListView->setMouseTracking(true);
    adminListView->setModel(proxyModel);
    adminListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    adminListView->setSpacing(10);
    adminListView->setItemDelegate(new AdminDelegate(this));
    adminListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(adminListView);

    // Connections
    connect(filterBtn, &QPushButton::clicked, this, &AdminManagementPage::showFilterMenu);
    connect(adminListView, &QListView::clicked, this, &AdminManagementPage::onAdminClicked);
    AdminDelegate *adminDelegate = new AdminDelegate(this);
    adminListView->setItemDelegate(adminDelegate);
    connect(adminDelegate, &AdminDelegate::actionButtonClicked, this, &AdminManagementPage::showActionMenu);
}

void AdminManagementPage::loadAdmins(Admin* admins, int size, const QString &currentAdminId) {
    adminModel->setCurrentAdminId(currentAdminId); // Pass it to the model!
    adminModel->setAdmins(admins, size);
}

void AdminManagementPage::onAdminClicked(const QModelIndex &proxyIndex)
{
    QModelIndex realIndex = proxyModel->mapToSource(proxyIndex);
    Admin selected = adminModel->getAdminAt(realIndex.row());
    emit adminClicked(selected);
}

void AdminManagementPage::showFilterMenu()
{
    QPoint globalPos = filterBtn->mapToGlobal(QPoint(0, filterBtn->height()));
    filterMenu->popup(globalPos);
}

void AdminManagementPage::applyFilter(int status, QString filterName)
{
    filterBtn->setText(filterName);
    proxyModel->setFilterStatus(status);
}

void AdminManagementPage::showActionMenu(const QModelIndex &proxyIndex, QPoint globalPos) {
    QModelIndex realIndex = proxyModel->mapToSource(proxyIndex);
    Admin selected = adminModel->getAdminAt(realIndex.row());

    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: white; border: 1px solid #BDC3C7; }"
                       "QMenu::item { padding: 6px 20px; color: #2C3E50; font-weight: bold; }"
                       "QMenu::item:selected { background-color: #ECF0F1; }");

    QAction *approveAct = menu.addAction("Approve");
    QAction *rejectAct  = menu.addAction("Reject");

    QAction *selectedAct = menu.exec(globalPos);

    if (selectedAct == approveAct || selectedAct == rejectAct) {
        ApprovalStatus newStatus = (selectedAct == approveAct) ? ApprovalStatus::Approved : ApprovalStatus::Rejected;

        // ==========================================
        // THIS LINE PREVENTS THE MENU FROM OPENING AGAIN!
        // ==========================================
        adminModel->setLocalVote(selected.getCnic(), 1);

        QString currentUserId = "TEST-ADMIN";
        if (AuthManager::getInstance().isLoggedIn() && AuthManager::getInstance().getCurrentUser() != nullptr) {
            currentUserId = AuthManager::getInstance().getCurrentUser()->getId();
        }

        // Send message to controller
        emit adminStatusChangeRequested(currentUserId, selected.getCnic(), newStatus);
    }
}

void AdminManagementPage::clearData()
{
    adminModel->clear();
}
