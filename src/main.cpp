#include "root/root_view.h"

#include <sodium.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    if ( sodium_init() == -1)
        return EXIT_FAILURE;

    QApplication a(argc, argv);
    RootView v;

    v.show();

    int result = a.exec();

    return result;
}
