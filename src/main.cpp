#include "utils/lock_limit.h"
#include "root/root_view.h"

#include <sodium.h>
#include <QApplication>



int main(int argc, char *argv[])
{
    if ( sodium_init() == -1)
        return EXIT_FAILURE;

    if(!increaseWindowsLockLimit( 1024 * 1024 * 256))
        return EXIT_FAILURE;

    //TODO verify that sqlcipher is actually compiled with all necessary flags

    QApplication a(argc, argv);
    RootView v;

    v.show();

    auto result = a.exec();

    return result;
}
