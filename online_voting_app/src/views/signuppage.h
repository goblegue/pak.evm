#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>
#include "controllers/auth_manager.h"

namespace Ui {
class SignupPage;
}

class SignupPage : public QWidget
{
    Q_OBJECT

public:
    explicit SignupPage(QWidget *parent = nullptr);
    ~SignupPage();

signals:
    void goToLoginRequested();
    void signupSuccessUser();
    void signupSuccessAdminPending();

private slots:
    void on_signupSubmitBtn_clicked();
    void on_goToLoginBtn_clicked();

private:
    Ui::SignupPage *ui;
    void lockSignupPage();
    void unlockSignupPage();
};

#endif // SIGNUPPAGE_H