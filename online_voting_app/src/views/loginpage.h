#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include "controllers/auth_manager.h" // For LoginResult etc

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

private:
    Ui::LoginPage *ui;
    int identifyInputType(const QString &input);
};

#endif // LOGINPAGE_H
