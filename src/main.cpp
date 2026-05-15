
#include "root/root_view.h"

#include <sodium.h>
#include <QApplication>

#ifdef _WIN32
#include "utils/lock_limit.h"
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/ptrace.h>
#include <objc/objc-runtime.h>
#include <objc/message.h>
#elif defined(__linux__)
#include <sys/prctl.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <sys/stat.h>
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
//if (IsDebuggerPresent())
    //return EXIT_FAILURE;
#elif defined(__APPLE__)
    ptrace(PT_DENY_ATTACH, 0, 0, 0);
#elif defined(__linux__)
    prctl(PR_SET_DUMPABLE, 0);
#endif

    if ( sodium_init() == -1)
        return EXIT_FAILURE;

#ifdef _WIN32
    if(!increaseWindowsLockLimit( 1024 * 1024 * 256))
        return EXIT_FAILURE;
#endif

    //TODO verify that sqlcipher is actually compiled with all necessary flags

    QApplication a(argc, argv);
    RootView v;

    v.show();

#ifdef _WIN32
    HWND id = reinterpret_cast<HWND>(v.winId());
    SetWindowDisplayAffinity(id, WDA_EXCLUDEFROMCAPTURE);
#elif defined(__APPLE__)
    id nsView = reinterpret_cast<id>(v.winId());

    SEL windowSel = sel_registerName("window");
    id nsWindow = ((id(*)(id, SEL))objc_msgSend)(nsView, windowSel);

    if (nsWindow) {
        SEL setSharingTypeSel = sel_registerName("setSharingType:");
        ((void(*)(id, SEL, NSInteger))objc_msgSend)(nsWindow, setSharingTypeSel, 0);
    }
#endif

    auto result = a.exec();

    return result;
}
