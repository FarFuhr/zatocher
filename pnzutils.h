#ifndef PNZUTILS_H
#define PNZUTILS_H

#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <iostream>
#include <cstring>

int GetCurrentPower(DWORD pid, const char* pointer);

#endif // PNZUTILS_H
