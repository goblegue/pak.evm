#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "OfflineSetupWizard.h"
#include <QTimer>
#include <QDateTime>
#include "PreElectionPage.h"
#include "PostElectionPage.h"
#include "EvmScanPage.h"
#include "EvmVotingPage.h"
#include "OfflineAdminDashboard.h"
#include "OfflineAdminAuthPage.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void processCameraString(QString base64ImageString, QString cnic);
    void onHeartbeatTick();
    void handleProceedToVoting();

    void handleEmergencyPause(bool pause);
    void handleTimeExtension(int minutesToAdd);
    void handleEmergencyForceClose();
    void handleCloseAdminDashboard();
    void handleSecretKnockDetected();
    void handleAdminAuthSuccess(QString adminCnic);
    void handleAdminAuthBack();

private:
    Ui::MainWindow *ui;
    void setupKioskUi();
    QStackedWidget *mainKioskStack;
    QTimer *kioskHeartbeat;
    QDateTime currentElectionStartTime;
    QDateTime currentElectionEndTime;

    PreElectionPage *preElectionPage;
    PostElectionPage *postElectionPage;
    EvmVotingPage *votingPage;
    OfflineAdminDashboard *adminDashboard;
    OfflineAdminAuthPage *adminAuthPage;


    bool m_systemIsPaused;

    void verifyScannedToken(QString cnic, QString qrPayload);
    QString formatTime(qint64 totalSeconds);


    EvmScanPage *scanPage;
};
#endif // MAINWINDOW_H
