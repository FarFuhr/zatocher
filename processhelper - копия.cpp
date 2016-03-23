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

std::vector<char*> GetAddressesOfData(DWORD pid, const wchar_t *data, size_t len) {
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);

        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk; //chunk of data read from memory
        std::vector<char*> pointersToDataFound; //pointers to all found values

        char* p = 0;
        while (p < si.lpMaximumApplicationAddress) { //from begging to end of process memory
            if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info)) {
                p = (char*)info.BaseAddress;
                chunk.resize(info.RegionSize);
                SIZE_T bytesRead;
                if (ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead)) {
                    for (size_t i = 0; i < (bytesRead - len); ++i) { //Search for the required data in the read chunk
                        if (memcmp(data, &chunk[i], len) == 0) {
                            if(chunk[i+30] != '{'){
                                pointersToDataFound.push_back((char*)p + i + 30); //15 symbols * 2 bit/symbol = 30 bit offset
                                //std::cout << "Data found: " << (void*)pointersToDataFound.back() << std::endl;
                            }
                        }
                    }
                }
                p += info.RegionSize;
            }
        }

        return pointersToDataFound;
    } else{
        std::cout << "OpenProcess faild: " << GetLastError() << std::endl;
    }
    return std::vector<char*>();
}
