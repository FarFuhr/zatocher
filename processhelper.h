#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

DWORD FindRunningProcess(std::string process);
char* GetAddressOfData(DWORD pid, const wchar_t *data, size_t len);

#endif // PROCESSHELPER_H
