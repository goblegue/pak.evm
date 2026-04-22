#include "mainwindow.h"
#include "databasemanager.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
