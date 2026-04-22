#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "system_bootstrapper.h"

enum StackedPages{
    LoginPage,
    SignupPage,
    AdminWaitingPage,
    UserDashPage,
    AdminDashPage
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const AppConfig &config, QWidget *parent = nullptr);
    ~MainWindow();
    void loadUserProfile(const QString& fullName, const QString& imagePath);
private slots:
    void on_goToSignupBtn_clicked();
    void on_goToLoginBtn_clicked();
    void on_loginSubmitBtn_clicked();
    void on_signupSubmitBtn_clicked();


    void on_adminWaitBackBtn_clicked();

    void on_btnUserHome_clicked();
    void on_btnUserElections_Clicked();
    void on_btnUserResults_clicked();
    void on_btnUserLogout_clicked();



private:
    Ui::MainWindow *ui;
    AppConfig m_config;
    // This function returns: 0 = Invalid, 1 = Email, 2 = CNIC
    int identifyInputType(const QString &input);
};
#endif // MAINWINDOW_H
