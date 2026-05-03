#ifndef POSTELECTIONPAGE_H
#define POSTELECTIONPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class PostElectionPage : public QWidget {
    Q_OBJECT

public:
    explicit PostElectionPage(QWidget *parent = nullptr);

private:
    void setupUi();

    QLabel *logoLabel;
};

#endif // POSTELECTIONPAGE_H
