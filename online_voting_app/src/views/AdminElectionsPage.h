#ifndef ADMINELECTIONSPAGE_H
#define ADMINELECTIONSPAGE_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "./models/entities/election.h"
#include "./models/Models.h"

class AdminElectionsPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminElectionsPage(QWidget *parent = nullptr);
    void loadElections(Election* elections, int size, const QString &currentAdminId);

signals:
    
    void navigateToCreateElection();
    void electionStatusChangeRequested(QString electionId, ApprovalStatus newStatus);
    void getConfigRequested(QString electionId);

private slots:
    void showFilterMenu();
    void applyFilter(int status, QString filterName);
    void onElectionBoxClicked(const QModelIndex &proxyIndex);
    void showElectionActionMenu(const QModelIndex &proxyIndex, QPoint globalPos);
    void onGetConfigButtonClicked(const QModelIndex &proxyIndex);


private:
    QListView *electionListView;
    QPushButton *filterBtn;
    QPushButton *createElectionBtn; 
    QMenu *filterMenu;

    ManageElectionListModel *electionModel;
    ElectionFilterProxyModel *proxyModel;

    void setupUi();
};

#endif 
