#define prod
#include <QApplication>

#include "controllers/election_controller.h"
#include "controllers/system_bootloader.h"
#include "models/repos/DatabaseManager.h"
#include "views/OfflineSetupWizard.h"
#include "views/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. Initialize the system and database first!
    SystemBootLoader bootstrapper;
    bootstrapper.initializeSystem();

    // 2. Create the main kiosk window (but don't show it yet)
    MainWindow *w = new MainWindow();

#ifdef prod
    // 3. Check the current state of the Election
    ElectionState currentState = ElectionController::getInstance().getCurrentState();

    // If it's a completely fresh DB (Setup) OR an old election was finished (Closed)
    if (currentState == ElectionState::Setup || currentState == ElectionState::Closed) {
        // We need to show the Setup Wizard
        OfflineSetupWizard *wizard = new OfflineSetupWizard();

        // When wizard finishes, show main window and safely delete the wizard
        QObject::connect(wizard, &OfflineSetupWizard::setupComplete, [w, wizard]() {
            w->show();
            wizard->deleteLater();
        });

        wizard->show();
    } else {
        // System is already configured (ReadyWaiting, Open, or Paused).
        // Skip the wizard entirely and jump straight into Kiosk Mode!
        w->show();
    }
#endif

#ifdef deve
    w->show();
#endif

    int result = a.exec();

    // Clean up
    delete w;
    return result;
}
