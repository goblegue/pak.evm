#ifndef OFFLINEADMINAUTHPAGE_H
#define OFFLINEADMINAUTHPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class OfflineAdminAuthPage : public QWidget {
    Q_OBJECT

public:
    explicit OfflineAdminAuthPage(QWidget *parent = nullptr);
    void resetForm(); // Clears passwords when page opens

signals:
    void backToScanRequested();
    void authSuccessful(QString adminId); // Emits when login succeeds!

private slots:
    void onLoginClicked();

private:
    QLabel *logoLabel;
    QPushButton *backBtn;
    QLineEdit *cnicInput;
    QLineEdit *passwordInput;
    QPushButton *loginBtn;

    void setupUi();
};

#endif // OFFLINEADMINAUTHPAGE_H
