#ifndef OFFLINEADMINDASHBOARD_H
#define OFFLINEADMINDASHBOARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QComboBox>

class OfflineAdminDashboard : public QWidget {
    Q_OBJECT

public:
    explicit OfflineAdminDashboard(QWidget *parent = nullptr);

    // Call these to update the dashboard with live data before showing it
    void updateStats(int totalTokensScanned);
    void setCurrentEndTime(const QDateTime &currentEnd);
    void setPausedState(bool isPaused);

signals:
    // Signals to command the MainWindow State Machine
    void pauseVotingRequested(bool pause);
    void extendTimeRequested(int minutesToAdd);    void forceCloseRequested();
    void closeDashboardRequested(); // To return to the scanning screen

private slots:
    void onPauseToggled();
    void onExtendTimeClicked();
    void onForceCloseClicked();

private:
    bool m_isPaused;

    // UI Elements
    QPushButton *closeDashBtn;

    // Stats
    QLabel *tokensScannedLabel;

    // Time Management
    QLabel *currentEndTimeLabel;
    QComboBox *extensionCombo;
    QPushButton *extendTimeBtn;

    // Emergency
    QPushButton *pauseBtn;
    QPushButton *forceCloseBtn;

    void setupUi();
};

#endif // OFFLINEADMINDASHBOARD_H
