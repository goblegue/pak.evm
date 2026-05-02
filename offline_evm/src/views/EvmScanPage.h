#ifndef EVMSCANPAGE_H
#define EVMSCANPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

// Qt6 Multimedia Headers
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QMediaDevices>

class EvmScanPage : public QWidget {
    Q_OBJECT

public:
    explicit EvmScanPage(QWidget *parent = nullptr);
    ~EvmScanPage();

signals:
    void validTokenScanned(QString tokenData); // Emits when successful!

private slots:
    void simulateSuccessfulScan(); // For testing

private:
    // UI Elements
    QLabel *logoLabel;
    QVideoWidget *videoWidget;
    QLabel *scanStatusLabel;
    QPushButton *proceedBtn;

    // Camera backend
    QCamera *camera;
    QMediaCaptureSession *captureSession;

    void setupUi();
    void startCamera();
};

#endif // EVMSCANPAGE_H
