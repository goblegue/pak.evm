#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QFutureWatcher>
#include "controllers/auth_manager.h" 

namespace Ui
{
    class LoginPage;
}

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

signals:
    void goToSignupRequested();
    void loginSuccessUser();
    void loginSuccessAdmin();
    void loginSuccessAdminPending();

private slots:
    void on_loginSubmitBtn_clicked();
    void on_goToSignupBtn_clicked();
    void handleLoginFinished();

private:
    Ui::LoginPage *ui;
    int identifyInputType(const QString &input);
    void resetLoginButton();
    QFutureWatcher<AuthManager::LoginResult> m_loginWatcher;
};

#endif 
