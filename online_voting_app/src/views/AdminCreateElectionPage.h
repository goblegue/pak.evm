#ifndef ADMINCREATEELECTIONPAGE_H
#define ADMINCREATEELECTIONPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

class AdminCreateElectionPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminCreateElectionPage(QWidget *parent = nullptr);

    
    void resetForm();

signals:
    void backBtnClicked();

    
    void createElectionRequested(QString title, QDateTime publishTime, QDateTime startTime, QDateTime endTime);

private slots:
    void onSubmitClicked();

    
    void onPublishTimeChanged(const QDateTime &newDateTime);
    void onStartTimeChanged(const QDateTime &newDateTime);

private:
    QPushButton *backBtn;
    QLineEdit *titleInput;
    QDateTimeEdit *publishTimeEdit;
    QDateTimeEdit *startTimeEdit;
    QDateTimeEdit *endTimeEdit;
    QPushButton *submitBtn;

    void setupUi();
};

#endif 
