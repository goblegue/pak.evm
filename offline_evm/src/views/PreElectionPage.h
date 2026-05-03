#ifndef PREELECTIONPAGE_H
#define PREELECTIONPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class PreElectionPage : public QWidget {
    Q_OBJECT
public:
    explicit PreElectionPage(QWidget *parent = nullptr);
    void updateCountdown(const QString &timeString);

private:
    QLabel *countdownLabel;
    QLabel *logoLabel;
    void setupUi();
};
#endif // PREELECTIONPAGE_H
