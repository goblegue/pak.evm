#ifndef ADMINMANAGEMENTPAGE_H
#define ADMINMANAGEMENTPAGE_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "user_management/admin/admin.h"
#include "Models.h"
#include "user_management/AuthManager/auth_manager.h"

class AdminManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminManagementPage(QWidget *parent = nullptr);
    void loadAdmins(Admin* admins, int size);

signals:
    // Future-proofing: When you click an admin to approve/reject them
    void adminClicked(Admin selectedAdmin); 
    void adminStatusChangeRequested(QString currentUserId, QString targetCnic, ApprovalStatus newStatus);

private slots:
    void showFilterMenu();
    void applyFilter(int status, QString filterName);
    void onAdminClicked(const QModelIndex &proxyIndex);
    void showActionMenu(const QModelIndex &proxyIndex, QPoint globalPos);
private:
    QListView *adminListView;
    QPushButton *filterBtn;
    QMenu *filterMenu;
    
    AdminListModel *adminModel;
    AdminFilterProxyModel *proxyModel;

    void setupUi();
};

#endif // ADMINMANAGEMENTPAGE_H
