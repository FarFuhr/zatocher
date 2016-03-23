#ifndef GAMEWINDOWUTILS_H
#define GAMEWINDOWUTILS_H

#include <windows.h>

struct EnumData {
    DWORD dwProcessId;
    HWND hWnd;
};

HWND FindWindowFromProcessId( DWORD dwProcessId );
BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam );

#endif // GAMEWINDOWUTILS_H
