#ifndef EVMSCANPAGE_H
#define EVMSCANPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QImage>

// Qt6 Multimedia Headers
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QMediaDevices>
#include <QVideoSink>
#include <QVideoFrame>

class EvmScanPage : public QWidget {
    Q_OBJECT

public:
    explicit EvmScanPage(QWidget *parent = nullptr);
    ~EvmScanPage();

    void updateTimeRemaining(const QString &timeString);
    void markScanSuccessful(const QString &decryptedTokenData);
    void stopCamera();
    void resetScanner();

signals:

    void frameReadyForBackend(QString base64ImageString, QString enteredCnic);
    void proceedToVotingClicked();

private slots:

    void simulateSuccessfulScan(); // For testing
    void onVideoFrameChanged(const QVideoFrame &frame);
    void onProceedClicked();

private:
    // UI Elements
    QLabel *logoLabel;
    QVideoWidget *videoWidget;
    QLabel *scanStatusLabel;
    QPushButton *proceedBtn;
    QLineEdit *cnicInput;
    QLabel *timeRemainingLabel;

    // Camera backend
    QCamera *camera;
    QMediaCaptureSession *captureSession;

    qint64 lastProcessTime;
    bool scanAlreadySuccessful; // Stops processing once we find a good token

    void setupUi();
    void startCamera();
};

#endif // EVMSCANPAGE_H
