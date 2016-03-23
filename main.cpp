/*
 * TODO:
 * Fix Hotkeys. Use Keyboard hooks instead.
 * Try to scan memory from the end of it.
 *
 * MAKE THIS PIECE OF FUCKING GODDAMN SHIT TO WORK
 */


#include <thread>
#include <fstream>
#include <string>
#include <algorithm>
#include <chrono>

#include "processhelper.h"
#include "gamewindowutils.h"
#include "pnzutils.h"
#include "stringutils.h"

#define DEFAULT_MSECS_SLEEP_BETWEEN_CLICKS 100

LPPOINT choosePoint = 0; //position of "Choose" button
LPPOINT applyPoint = 0; //position of "Apply"

bool pause = false;
bool resetup = false;
bool autoApply = false;
bool noExtraClicks = false;
int start = -1;

unsigned power = 80;
unsigned msecs = 0;

HINSTANCE _hInstance;

std::vector<int> powers;

void setupProgram();
void getButtonsPosition();
void readSettingsFile();
void saveStatistic();
void checkTimeBetweenClicks(DWORD pid, const char* pointer);

void changePauseState(){
    pause = !pause;
    std::wcout << (pause ? L"Пауза\n" : L"Возобновление\n");
}

HHOOK hKeybHook;
CALLBACK LRESULT kHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if( nCode < 0 )
        return CallNextHookEx(hKeybHook,nCode,wParam,lParam);

    if( wParam == WM_KEYDOWN ) {
        KBDLLHOOKSTRUCT* hook = (KBDLLHOOKSTRUCT*)lParam;
        switch( hook->vkCode){
        case VK_HOME:
            if(!choosePoint){
                choosePoint = new POINT;
                GetCursorPos(choosePoint);
                std::wcout << L"Выбрать: " << choosePoint->x << 'x' << choosePoint->y << std::endl;
            } else if(pause){
                saveStatistic();
            }
            break;
        case VK_END:
            if(!applyPoint){
                applyPoint = new POINT;
                GetCursorPos(applyPoint);
                std::wcout << L"Применить: " << applyPoint->x << 'x' << applyPoint->y << std::endl;
            } else {
                changePauseState();
            }
            break;
        case 0x53: //S
            if(start == 0)
                start = 1;
            break;
        case 0x52: //R
            if(start == 0){
                resetup = true;
            }
            break;
        }
    }
    return CallNextHookEx(hKeybHook,nCode,wParam,lParam);
}

void saveStatistic(){
    std::wcout << L"Сохраняю статистику\n";
    static int countStatFiles = 0;
    std::fstream statfile;
    std::string filename = "zatocher_statistic_";
    filename += std::to_string(1) + ".txt";
    statfile.open(filename.c_str(), std::fstream::out);

    if(statfile.is_open()){
        statfile << "  | ";
        for(int i = 0; i < 10; ++i)
            statfile << i << " | ";
        statfile << "\r\n";

        for(int i = 0; i < 10; ++i){ //vertical
            statfile << i << '0' << "|";
            for(int j = 0; j < 10; ++j){ //horizontal
                statfile << (powers[i*10+j] > 100 ? "" : " ") << powers[i*10+j] << (powers[i*10+j] >= 10 ? "|" : " |");
            }
            statfile << "\r\n";
        }
        statfile << "\r\n";

        statfile << "Total clicks: " << std::accumulate(powers.begin(), powers.end(), 0) << "\r\n";
        statfile << "Total white-zone clicks: " << std::accumulate(powers.begin(), powers.begin()+39, 0) << "\r\n";
        statfile << "Total yellow-zone clicks: " << std::accumulate(powers.begin()+39, powers.begin()+74, 0) << "\r\n";
        statfile << "Total orange-zone clicks: " << std::accumulate(powers.begin()+74, powers.begin()+89, 0) << "\r\n";
        statfile << "Total red-zone clicks: " << std::accumulate(powers.begin()+89, powers.end(), 0) << "\r\n";
    } else {
        std::wcout << L"Не могу сохранить статистику.\n";
    }
}

BOOL ctrlHandler(DWORD fdwCtrlType){
    switch(fdwCtrlType){
    case CTRL_CLOSE_EVENT:
        saveStatistic();
        break;
    }
}

void getButtonsPosition() {
    std::wcout << L"Поместите курсор на кнопку \"Выбрать\" и нажмите Home.\nЗатем поместит курсор на кнопку \"Применить\" и нажмите End.\n";
    hKeybHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)kHook, _hInstance, NULL);

    choosePoint = 0;
    applyPoint = 0;
    MSG message = {0};
    while(!(choosePoint && applyPoint)){
        while( PeekMessage( &message, NULL, 0, 0, PM_NOREMOVE )) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    UnhookWindowsHookEx(hKeybHook);
}

void setupProgram(){
    std::wcout << L"Введите желаемую минимальную силу улучшений (0-100, 89+ - красная зона).\n> ";
    std::cin >> power;
    std::wcout << L"Время между кликами в мс (1000 мс = 1 секунда). 0 = по-умолчанию \n> ";
    std::cin >> msecs;
}

void waitForStart(DWORD pid, const char* pointer){
    hKeybHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)kHook, _hInstance, NULL);
    std::wcout << L"Нажмите S чтобы начать.\nЧтобы поставить программу на паузу нажмите End.\nЧтобы перенастроить программу нажмите R.\nЧтобы сохранить статистику во время паузы нажмите Home.\n";

    start = 0;
    MSG message = {0};

    while(start != 1){
        while(PeekMessage( &message, NULL, 0, 0, PM_NOREMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        if(resetup){
            UnhookWindowsHookEx(hKeybHook);
            resetup = false;
            setupProgram();
            getButtonsPosition();
            readSettingsFile();

            if(noExtraClicks){
                std::wcout << L"Проверяю время между кликами.\n";
                checkTimeBetweenClicks(pid, pointer);
            }

            std::wcout << L"Перенастройка выполнена!\n";
            waitForStart(pid, pointer);
        }
    }
    UnhookWindowsHookEx(hKeybHook);
}

void mainLoopThread(){
    HHOOK hKeybHookThread; // Hook for new thread
    hKeybHookThread = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)kHook, _hInstance, NULL);

    MSG message = {0};

    while(GetMessage( &message, NULL, 0, 0)) {
        if(message.message == WM_HOTKEY){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
}

void leftClick(){ //just left click, nothing more
    mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void checkTimeBetweenClicks(DWORD pid, const char* pointer){
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);

    std::chrono::milliseconds _new_msecs(0);
    unsigned _currentPowerPrev = GetCurrentPower(pid, pointer);
    unsigned _currentPower = _currentPowerPrev;

    SetCursorPos(choosePoint->x, choosePoint->y); //move cursor to Choose button
    Sleep(100);
    leftClick();
    auto start = std::chrono::steady_clock::now();;
    if (process) {
        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk;

        if (VirtualQueryEx(process, pointer, &info, sizeof(info)) == sizeof(info)) {
            SIZE_T bytesRead; //idk why
            chunk.resize(6); //2 bytes per symbol, max 3 symbols ("100")


            start = std::chrono::steady_clock::now();

            while(_currentPower == _currentPowerPrev){
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
                    _currentPower = atoi(percents);
                    delete percents;
                } else {
                    std::cout << "ReadProcessMemory() failed: " << GetLastError() << std::endl;
                }
            }
        }
    }

    _new_msecs = std::chrono::duration_cast<std::chrono::milliseconds>
                            (std::chrono::steady_clock::now() - start);

    if(msecs == 0){
        msecs = (_new_msecs.count() > 100 ? (int)((double)_new_msecs.count()/100)*100 : 100);
        std::wcout << L"Время между кликами установлено на " << msecs << L" мс\n";
    }
    else
        if(msecs < _new_msecs.count()){
            std::wcout << L"Слишком маленький промежуток между кликами. Рекомендуется " << (_new_msecs.count() > 100 ? (int)((double)_new_msecs.count()/100)*100 : 100) << L" \n";
            exit(0);
        }
}

//fuck google translator
//main function for program logic
void zatochka(DWORD pid, const char* pointer){
    if(msecs == 0)
        msecs = DEFAULT_MSECS_SLEEP_BETWEEN_CLICKS;
    std::ofstream logfile("zatocher_log.txt", std::fstream::out);
    logfile << "  ";
    logfile.close();

    powers.resize(101);
    std::fill(powers.begin(), powers.end(), 0);

    bool writeStats = true;

    while(1){
        if(!pause){
            int currentPower = GetCurrentPower(pid, pointer);

            if(writeStats && currentPower != 0)
                powers[currentPower]++;

            if(currentPower >= power && currentPower <= 100 && autoApply){
                SetCursorPos(applyPoint->x, applyPoint->y); //move cursor to Apply button
                Sleep(150); //to be sure
                leftClick();
                Sleep(300); //to be sure

                if(currentPower >= power){ //to avoid extra clicks
                    std::cout << currentPower << "%\a" << std::endl;
                    keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), 0, 0);
                    keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);

                }
                Sleep(1000); //to be sure
            } else if(currentPower < power){
                SetCursorPos(choosePoint->x, choosePoint->y); //move cursor to Choose button
                leftClick();
            } else if(currentPower >= power){
                std::cout << currentPower << "%\a" << std::endl;
                changePauseState();
            }
        }
        Sleep( msecs );
    }
}

void readSettingsFile(){
    std::wcout << L"Читаю zatocher.ini\n";
    std::fstream in("zatocher.ini", std::fstream::in);
    std::string line;

    if(in.is_open()){
        while(std::getline(in, line)){
            std::vector<std::string> elems = split(line, '=');
            if(elems[0].compare("AutoApply") == 0 || elems[0].compare("autoApply") == 0 || elems[0].compare("autoapply") == 0){
                autoApply = toBool(elems[1]);
                std::wcout << L"Авто-применение (Auto-apply) " << (autoApply ? L"включено.\n" : L"отключено.\n");
            } else if(elems[0].compare("NoExtraClicks") == 0 || elems[0].compare("noExtraClicks") == 0 || elems[0].compare("noextraclicks") == 0){
                noExtraClicks = toBool(elems[1]);
                std::wcout << L"Защита от прокликиваний (NoExtraClicks) " << (noExtraClicks ? L"включена.\n" : L"отключена.\n") << L"Она может понижать скорость работы программы в 2 раза.\n";
            }
        }
    } else{
        in.close();
        std::wcout << L"Не могу прочитать .ini файл.\nСоздаю файл конфигурации zatocher.ini\n";
        in.open("zatocher.ini", std::fstream::out);
        in << "AutoApply=false\n\rNoExtraClicks=true\n\r";
        in.close();
        readSettingsFile();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    setlocale(0, "rus_rus.866");
    _hInstance = hInstance;
    pause = false;
    std::wcout << L"Ищу процесс Panzar..." << std::endl;
    DWORD pid = FindRunningProcess("PnzCl.exe");

    if(pid){
        std::cout << "PnzCl.exe: pid " << pid << std::endl;

        char* data = GetAddressOfData(pid, L"Сила улучшения", 28);

        if(data){
            setupProgram();
            readSettingsFile();
            getButtonsPosition();
            std::wcout << L"Настройка завершена!\n";

            if(noExtraClicks){
                std::wcout << L"Проверяю время между кликами.\n";
                checkTimeBetweenClicks(pid, data);
            }

            waitForStart(pid, data); //not working when out of focus
            std::wcout << L"Начинаем...\n";

            std::thread mainLoop(mainLoopThread);
            mainLoop.detach();
            zatochka(pid, data);
        } else{
            std::wcout << L"Не могу найти необходимые данные.\nСвяжитесь с разработчиком.\n";
            system("pause");
        }
    } else {
        std::wcout << L"Panzar не запущен.\n";
    }

    delete choosePoint;
    delete applyPoint;
    return 0;
}

