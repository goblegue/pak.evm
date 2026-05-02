#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "EvmScanPage.h"

#include <QMainWindow>
#include "OfflineSetupWizard.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    EvmScanPage *scanPage;
};
#endif // MAINWINDOW_H
