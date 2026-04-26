#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "controllers/system_bootstrapper.h"
#include "views/AdminCandidatePage.h"
#include "views/AdminManagementPage.h"
#include "views/AdminElectionsPage.h"
#include "views/AdminCandidateDetailsPage.h"
#include "views/AdminCreateElectionPage.h"

enum StackedPages
{
    LoginPage,
    SignupPage,
    AdminWaitingPage,
    UserDashPage,
    AdminDashPage
};

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
    MainWindow(const AppConfig &config, QWidget *parent = nullptr);
    ~MainWindow();
    void loadUserProfile(const QString &fullName, const QString &imagePath);
    void loadAdminProfile(const QString &fullName, const QString &imagePath);
private slots:
    void on_goToSignupBtn_clicked();
    void on_goToLoginBtn_clicked();
    void on_loginSubmitBtn_clicked();
    void on_signupSubmitBtn_clicked();

    void on_adminWaitBackBtn_clicked();

    void on_btnUserHome_clicked();
    void on_btnUserResults_clicked();
    void on_btnUserLogout_clicked();

    void on_adminSidebarCandidatesBtn_clicked();
    void handleElectionSelectedForCandidates(QString);
    void on_adminSidebarAdminsBtn_clicked();
    void on_adminSidebarElectionsBtn_clicked();
    void handleNavigateToCandidateDetails(Candidate selectedCandidate);
    void handleBackToCandidateList();
    //Handles the Approve/Reject button clicks from the Details Page
    void handleCandidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus);
    void handleNavigateToCreateElection();
    void handleCreateElectionSubmit(QString title, QDateTime publishTime, QDateTime startTime, QDateTime endTime);
    void handleBackToElectionList();

private:
    Ui::MainWindow *ui;
    AppConfig m_config;
    // This function returns: 0 = Invalid, 1 = Email, 2 = CNIC
    int identifyInputType(const QString &input);
    AdminCandidatePage *m_adminInnerPage_Candidates;
    AdminManagementPage *m_adminInnerPage_Admins;
    AdminElectionsPage *m_adminInnerPage_Elections;
    AdminCandidateDetailsPage *m_adminInnerPage_CandidateDetails;
     AdminCreateElectionPage *m_adminInnerPage_CreateElection;
};
#endif // MAINWINDOW_H
