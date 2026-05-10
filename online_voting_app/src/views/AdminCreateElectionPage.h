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

    // Call this every time before showing the page so the dates are fresh!
    void resetForm();

signals:
    void backBtnClicked();

    // Emits the data to MainWindow to send to ElectionController
    void createElectionRequested(QString title, QDateTime publishTime, QDateTime startTime, QDateTime endTime);

private slots:
    void onSubmitClicked();

    // These dynamically update the constraints of the next pickers
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

#endif // ADMINCREATEELECTIONPAGE_H
