#pragma once

#ifdef _WIN32
#include <windows.h>

inline bool increaseWindowsLockLimit(SIZE_T extra_bytes_needed) {
    SIZE_T minSize, maxSize;
    HANDLE hProcess = GetCurrentProcess();

    if (GetProcessWorkingSetSize(hProcess, &minSize, &maxSize)) {
        return SetProcessWorkingSetSize(hProcess, minSize + extra_bytes_needed, maxSize + extra_bytes_needed);
    }

    return false;
}

#endif