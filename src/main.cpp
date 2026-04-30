#include <QApplication>
#include "mainapp.h"
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainApp w;

    w.show();

    int result = a.exec();
    DatabaseManager::instance().closeDatabase();

    return result;
}
