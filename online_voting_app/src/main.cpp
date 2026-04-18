#include "mainwindow.h"
#include "databasemanager.h"
#include <QDebug>
#include <QApplication>
#include "userrepository.h"
#include "electionrepository.h"
#include "states.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try {
        DatabaseManager::getInstance().setupSchema();

        electionrepository electRepo;
        Election testElection;
        testElection.setId("ELEC001");
        testElection.setTitle("University Council 2026");
        testElection.setStatus(ElectionState::Published);
        electRepo.insertElection(testElection);

        // 2. Create a User
        userrepository userRepo;
        User testUser;
        testUser.setCnic("43101-1292397-3");
        testUser.setEmail("sabcd45123ent@fast.nu.edu.pk");
        testUser.setName("Abdullah");
        testUser.setId("USR001");

        // Dummy password to prevent the crash!
        testUser.setPassword(QByteArray("fake_binary_hash_data"), 998347);

        // Capture the return value to see if it worked
        bool isUserInserted = userRepo.insertUser(testUser);

        if (isUserInserted) {
            qDebug() << "SUCCESS! User data inserted! Check MongoDB Compass now.";
        } else {
            qDebug() << "FAILED! User was not inserted.";
        }

    } // <--- THIS WAS THE MISSING BRACE!
    catch(const std::exception & e) {
        qDebug() << "error";
        qDebug() << e.what();
    }

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
