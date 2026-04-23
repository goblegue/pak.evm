#include "mainwindow.h"
#include "system_bootstrapper.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SystemBootstrapper bootstrapper;
    AppConfig config; // Declare outside/above

    try
    {
        bootstrapper.initializeSystem();
        config = bootstrapper.getConfig(); // Assign inside
    }
    catch (const std::exception &e)
    {
        return -1;
    }

    MainWindow w(config);
    // You could pass it to the window if needed:
    // MainWindow w(config.privateKey);
    w.show();

    return a.exec();
} // 'config' is deleted ONLY here (when the app closes)