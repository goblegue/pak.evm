#define deve
#include "models/repos/DatabaseManager.h"
#include "views/mainwindow.h"
#include  "views/OfflineSetupWizard.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
#ifdef prod
    OfflineSetupWizard *wizard = new OfflineSetupWizard();


    // When wizard finishes, show main window and delete the wizard
    QObject::connect(wizard, &OfflineSetupWizard::setupComplete, [&]() {
        w.show();
        wizard->deleteLater(); // Safely delete the wizard object
    });

    wizard->show();
#endif
#ifdef deve
    w.show();
#endif
    DatabaseManager::instance().init("voting_systemdb");
    return a.exec();
}
