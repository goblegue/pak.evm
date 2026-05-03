#include "models/repos/DatabaseManager.h"
#include "views/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    DatabaseManager::instance().init("voting_systemdb");
    return QCoreApplication::exec();
}
