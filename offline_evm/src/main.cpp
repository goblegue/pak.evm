#define prod
#include "controllers/system_bootloader.h" // [NEW]
#include "models/repos/DatabaseManager.h"
#include "views/OfflineSetupWizard.h"
#include "views/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // [NEW] Bootstrap the backend BEFORE loading the UI!
    SystemBootLoader bootloader;
    bootloader.initializeSystem();

    MainWindow w;
#ifdef prod
    OfflineSetupWizard *wizard = new OfflineSetupWizard();

    QObject::connect(wizard, &OfflineSetupWizard::setupComplete, [&]() {
        w.show();
        wizard->deleteLater();
    });

    // Check if system is already configured!
    // If it is, skip the wizard. If not, show it.
    // (For now, we just show it)
    wizard->show();
#endif

#ifdef deve
    w.show();
#endif

    return a.exec();
}