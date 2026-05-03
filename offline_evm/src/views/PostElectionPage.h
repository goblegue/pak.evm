#ifndef POSTELECTIONPAGE_H
#define POSTELECTIONPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton> 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class PostElectionPage : public QWidget {
    Q_OBJECT

public:
    explicit PostElectionPage(QWidget *parent = nullptr);

private slots:
    void onExportResultsClicked();
    void onDeleteElectionClicked();

private:
    void setupUi();

    QLabel *logoLabel;
    QPushButton *exportResultsBtn;
    QPushButton *deleteElectionBtn;
};

#endif // POSTELECTIONPAGE_H
