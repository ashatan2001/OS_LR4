#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <string>
#include <stdlib.h>

using namespace std;

const int PAGES = 17;
string BASE_PATH = R"(C:\LETI\OS\LR4\)";
HANDLE* writeSemaphores = new HANDLE[PAGES];
HANDLE* readSemaphores = new HANDLE[PAGES];

void openSemaphores() {

    string semaphoreName;

    for (int i = 0; i < PAGES; ++i) {
        semaphoreName = "semaphore_" + to_string(i) + "_writer";
        writeSemaphores[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semaphoreName.c_str());
        semaphoreName = "semaphore_" + to_string(i) + "_reader";
        readSemaphores[i] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semaphoreName.c_str());
    }
}

int main()
{
    srand(time(NULL));

    SYSTEM_INFO systemInfo;
    int bufferSize;

    GetSystemInfo(&systemInfo);
    bufferSize = PAGES * systemInfo.dwPageSize;

    string bufferPath, mappedName;
    HANDLE hMapped;
    LPVOID address;

    bufferPath = BASE_PATH + "buffer.txt";
    mappedName = "buffer";

    hMapped = OpenFileMappingA(GENERIC_READ, FALSE, mappedName.c_str());
    address = MapViewOfFile(hMapped, FILE_MAP_READ, 0, 0, bufferSize);

    FILE* logfile, * logfileExcel;
    string logfileName, logfileNameExcel;
    logfileName = BASE_PATH + "reading_logs\\" + to_string(GetCurrentProcessId()) + ".txt";
    logfile = fopen(logfileName.c_str(), "w");

    logfileNameExcel = BASE_PATH + "reading_logs_excel\\" + to_string(GetCurrentProcessId()) + ".txt";
    logfileExcel = fopen(logfileNameExcel.c_str(), "w");

    openSemaphores();

    char* data;

    DWORD finish = timeGetTime() + 15000;

    while (timeGetTime() < finish)
    {
        fprintf(logfile, "|State: WAITING|\t\t\t\t\t|Time: %d|\n", timeGetTime());
        fprintf(logfileExcel, "%d %d\n", timeGetTime(), 0);

        DWORD page = WaitForMultipleObjects(PAGES, readSemaphores, FALSE, INFINITE);

        fprintf(logfile, "|State: READING; Page: %d|\t\t\t\t|Time: %d|\n", page, timeGetTime());
        fprintf(logfileExcel, "%d %d\n", timeGetTime(), 1);

        int offset = page * systemInfo.dwPageSize;
        data = (char*)((long long)address + offset);
        Sleep(500 + rand() % 1001);

        string logname = BASE_PATH + "pages_excel\\page_" + to_string(page) + ".txt";
        FILE* log = fopen(logname.c_str(), "a");
        fprintf(log, "%d %d\n", timeGetTime(), 2);
        ReleaseSemaphore(writeSemaphores[page], 1, NULL);
        fprintf(log, "%d %d\n", timeGetTime(), 0);
        fclose(log);

        fprintf(logfile, "|State: RELEASED; Read data: %s|\t\t\t|Time: %d|\n\n", data, timeGetTime());
        fprintf(logfileExcel, "%d %d\n", timeGetTime(), 2);
    }

    return 0;
}