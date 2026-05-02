#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "controllers/system_bootstrapper.h"
#include "views/loginpage.h"
#include "views/AdminCandidatePage.h"
#include "views/AdminManagementPage.h"
#include "views/AdminElectionsPage.h"
#include "views/AdminCandidateDetailsPage.h"
#include "views/AdminCreateElectionPage.h"
#include "views/UserActiveElectionsPage.h"
#include "views/UserMyTokensPage.h"
#include "views/UserCandidacyPage.h"

enum StackedPages { Login_Page, SignupPage, AdminWaitingPage, UserDashPage, AdminDashPage };

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
    void handleLoginSuccessUser();
    void handleLoginSuccessAdmin();
    void handleLoginSuccessAdminPending();
    void handleGoToSignupRequested();

    void on_goToLoginBtn_clicked();
    void on_signupSubmitBtn_clicked();

    void on_adminWaitBackBtn_clicked();

    void on_btnUserHome_clicked();
    void on_btnUserResults_clicked();
    void on_btnUserLogout_clicked();
    // admin
    void on_adminSidebarCandidatesBtn_clicked();
    void on_adminSidebarAdminsBtn_clicked();
    void on_adminSidebarElectionsBtn_clicked();
    void handleElectionSelectedForCandidates(QString);
    void handleNavigateToCandidateDetails(Candidate selectedCandidate);
    void handleBackToCandidateList();
    void handleCandidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus);
    void handleNavigateToCreateElection();
    void handleCreateElectionSubmit(QString title, QDateTime publishTime, QDateTime startTime, QDateTime endTime);
    void handleBackToElectionList();
    void on_btnAdminLogout_clicked();
    // user
    void on_userSidebarActiveElectionsBtn_clicked();
    void on_userSidebarMyTokensBtn_clicked();
    void handleUserElectionSelected(QString electionId);
    void handleGenerateTokenRequested(QString electionId);
    void handleEmailTokenRequested(Token selectedToken);
    void handleUserNavigateToCandidateDetails(Candidate selectedCandidate);
    void handleUserBackToActiveElections();
    void on_userSidebarLocateStationBtn_clicked();
    void on_userSidebarCandidacyBtn_clicked();
    void handleCandidacyApplicationSubmit(Candidate newCandidate);

private:
    Ui::MainWindow *ui;
    AppConfig m_config;
    // admin
    LoginPage *m_loginPage;
    AdminCandidatePage *m_adminInnerPage_Candidates;
    AdminManagementPage *m_adminInnerPage_Admins;
    AdminElectionsPage *m_adminInnerPage_Elections;
    AdminCandidateDetailsPage *m_adminInnerPage_CandidateDetails;
    AdminCreateElectionPage *m_adminInnerPage_CreateElection;
    // user
    UserActiveElectionsPage *userInnerPage_ActiveElections;
    UserMyTokensPage *userInnerPage_MyTokens;
    AdminCandidateDetailsPage *userInnerPage_CandidateDetails;
    UserCandidacyPage *userInnerPage_Candidacy;
};
#endif // MAINWINDOW_H
