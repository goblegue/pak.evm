#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_goToSignupBtn_clicked();
    void on_goToLoginBtn_clicked();
    void on_loginSubmitBtn_clicked();
    void on_signupSubmitBtn_clicked();


    void on_adminWaitBackBtn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
