#include "gamewindowutils.h"

/*You'll ask me: wtf?
 * And I will answer:
 * Fuck it. I don't know.
 * I <3 stackoverflow
 * I hate winapi.
 */

BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam ) {
    EnumData& ed = *(EnumData*)lParam;
    DWORD dwProcessId = 0x0;
    GetWindowThreadProcessId( hWnd, &dwProcessId );
    if ( ed.dwProcessId == dwProcessId ) {
        ed.hWnd = hWnd;
        SetLastError( ERROR_SUCCESS );
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowFromProcessId( DWORD dwProcessId ) {
    EnumData ed = { dwProcessId };
    if ( !EnumWindows( EnumProc, (LPARAM)&ed ) &&
         ( GetLastError() == ERROR_SUCCESS ) ) {
        return ed.hWnd;
    }
    return NULL;
}
