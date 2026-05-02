#include "views/mainwindow.h"
#include "model/DatabaseManager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    DatabaseManager::instance().init("voting_systemdb");
    return QCoreApplication::exec();
}
