#ifndef OFFLINESETUPWIZARD_H
#define OFFLINESETUPWIZARD_H

#include <QWidget>
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QListWidget>

class OfflineSetupWizard : public QMainWindow {
    Q_OBJECT

public:
    explicit OfflineSetupWizard(QWidget *parent = nullptr);

signals:
    void setupComplete();

private slots:
    // Navigation Logic
    void goToMasterSetup();
    void goBackToWelcome();
    void processMasterSetup();
    void goBackToMasterSetup();
    void onAddAdminClicked();

    void processAdminSetup();
    void goBackToAdminSetup();
    void onBrowseFileClicked();
    void processLoadElection();

private:
    // Layout Elements
    QStackedWidget *wizardStack;

    // Sidebar Tracker Buttons
    QPushButton *step1Btn;
    QPushButton *step2Btn;
    QPushButton *step3Btn;
    QPushButton *step4Btn;

    // Page 1: Welcome
    QWidget *welcomePage;
    QPushButton *welcomeNextBtn;

    // Page 2: Master Setup
    QWidget *masterSetupPage;
    QLineEdit *passInput;
    QLineEdit *confirmPassInput;
    QLineEdit *publicKeyInput;
    QPushButton *masterBackBtn;
    QPushButton *masterNextBtn;

    // PAGE 3: ADMIN CREATION VARIABLES
    QWidget *adminSetupPage;
    QLineEdit *adminUserIn;
    QLineEdit *adminCnicIn;
    QLineEdit *adminPassIn;
    QLineEdit *adminConfirmIn;
    QPushButton *addAdminBtn;     // The (+) Button
    QListWidget *adminListWidget; // The List of created admins
    QPushButton *adminBackBtn;
    QPushButton *adminNextBtn;

    // PAGE 4: LOAD ELECTION VARIABLES
    QWidget *loadElectionPage;
    QPushButton *browseFileBtn;
    QLabel *fileNameLabel;
    QTextEdit *fileDataDisplay;
    QPushButton *loadBackBtn;
    QPushButton *loadNextBtn;


    // UI Setup Functions
    void setupUi();
    void buildSidebar(QHBoxLayout *mainLayout);
    void buildWelcomePage();
    void buildMasterSetupPage();
    void buildAdminSetupPage();
    void buildLoadElectionPage();
    //void finishSetup();
};

#endif // OFFLINESETUPWIZARD_H
