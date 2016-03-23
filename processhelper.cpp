#include "processhelper.h"

//returns pid by process name, code from stackoverflow with a few corrections
DWORD FindRunningProcess(std::string process) {
    DWORD pid = 0; //return value

    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    } else{
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if(Process32First(hProcessSnap, &pe32)) {
            if(process.compare(pe32.szExeFile) == 0) {
                pid = pe32.th32ProcessID;
            } else {
                while(Process32Next(hProcessSnap, &pe32)) {
                    if (process.compare(pe32.szExeFile) == 0) {
                        pid = pe32.th32ProcessID;
                        break;
                    }
                }
            }
            CloseHandle(hProcessSnap);
        }
    }

    return pid;
}

char* GetAddressOfData(DWORD pid, const wchar_t *data, size_t len) {
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);

        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk; //chunk of data read from memory

        for (char* p = (char*)0x10000000;
            p < si.lpMaximumApplicationAddress;
            p += info.RegionSize)
        { //from begging to end of process memory
            if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT &&
                (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE))
            {
                p = (char*)info.BaseAddress;
                chunk.resize(info.RegionSize);
                SIZE_T bytesRead;
                if (ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead)) {
                    chunk.resize(bytesRead);
                    for (size_t i = 0; i < (bytesRead - len); ++i) { //Search for the required data in the chunk read
                        if (memcmp(data, &chunk[i], len) == 0) {
                            if(chunk[i+30] != '{'){
                                return p+i+30;
                            }
                        }
                    }
                }
            }
        }

        return 0;
    } else{
        std::cout << "OpenProcess() failed: " << GetLastError() << std::endl;
    }
    return NULL;
}
