#include "views/mainwindow.h"
#include "controllers/system_bootstrapper.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "./services/email/emailservice.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    

    SystemBootstrapper bootstrapper;
    AppConfig config; // Declare outside

    try
    {
        bootstrapper.initializeSystem();
        config = bootstrapper.getConfig(); // Assign inside
    }
    catch (const std::exception &e)
    {
        return -1;
    }
    // bool success = EmailService::getInstance().sendEmail("am7862760@gmail.com", "Test Subject", "Test Message");

    // QString message = (success)?"success":"failure";
    // qInfo(message.toStdString().c_str());

    MainWindow w(config);
    // You could pass it to the window if needed:
    // MainWindow w(config.privateKey);
    w.show();

    return a.exec();
} // 'config' is deleted ONLY here (when the app closes)
