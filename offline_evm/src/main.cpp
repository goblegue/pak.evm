#include "views/mainwindow.h"
#include  "views/OfflineSetupWizard.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    OfflineSetupWizard *wizard = new OfflineSetupWizard();

    // When wizard finishes, show main window and delete the wizard
    QObject::connect(wizard, &OfflineSetupWizard::setupFinished, [&]() {
        w.show();
        wizard->deleteLater(); // Safely delete the wizard object
    });

    wizard->show();
    return a.exec();
}
