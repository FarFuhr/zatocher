#include "pnzutils.h"

//Returns current power of enchanting (?)
int GetCurrentPower(DWORD pid, const char* pointer){
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process) {
        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk;

        if (VirtualQueryEx(process, pointer, &info, sizeof(info)) == sizeof(info)) {
            SIZE_T bytesRead; //idk why
            chunk.resize(6); //2 bytes per symbol, max 3 symbols ("100")

            if (ReadProcessMemory(process, pointer, &chunk[0], 6, &bytesRead)) {
                char* percents = new char[4]; //0-100 and '\0'
                int j = 0;
                for(size_t i = 0; i < chunk.size(); ++i){
                    if(chunk[i] != 0 && chunk[i] != '%'){ //2 bytes per symbol, symbols are numbers ==> 1, 3 and 5 bytes are 0x00 or 0x04
                        percents[j] = chunk[i];
                        ++j;
                    }
                    percents[3] = '\0'; //just to be sure
                }
                return atoi(percents);
            } else {
                std::cout << "ReadProcessMemory() failed: " << GetLastError() << std::endl;
            }
        }
    }

    return -1;
}
