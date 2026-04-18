#include "mainwindow.h"
#include "databasemanager.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // try{

    //     DatabaseManager::getInstance().setupSchema();

    // }catch(const std::exception & e){
    //     qDebug()<<"error";
    //     qDebug()<<e.what();
    // }

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
