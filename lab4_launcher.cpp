#include <windows.h>
#include <string>

using namespace std;

const int PAGES = 17;
string BASE_PATH = R"(C:\LETI\OS\LR4\)";
HANDLE* wtiteSemaphores = new HANDLE[PAGES];
HANDLE* readSemaphores = new HANDLE[PAGES];
HANDLE* activeProcesses = new HANDLE[PAGES << 1];


void createSemaphores(bool isWriter) {

    string semaphoreName;

    for (int i = 0; i < PAGES; ++i) {
        if (isWriter) {
            semaphoreName = "semaphore_" + to_string(i) + "_writer";
            wtiteSemaphores[i] = CreateSemaphoreA(NULL, 1, 1, semaphoreName.c_str());
        }
        else {
            semaphoreName = "semaphore_" + to_string(i) + "_reader";
            readSemaphores[i] = CreateSemaphoreA(NULL, 0, 1, semaphoreName.c_str());
        }
    }
}

void createProcesses(bool isWriter) {

    LPSTARTUPINFOA startupInfo = new STARTUPINFOA[PAGES];
    LPPROCESS_INFORMATION processInfo = new PROCESS_INFORMATION[PAGES];
    string processName;

    if (isWriter)
        processName = BASE_PATH + "exe\\lab4_writer.exe";
    else
        processName = BASE_PATH + "exe\\lab4_reader.exe";

    for (int i = 0; i < PAGES; ++i) {
        ZeroMemory(&startupInfo[i], sizeof(startupInfo[i]));

        CreateProcessA(processName.c_str(), NULL, NULL, NULL, TRUE, 0,
                       NULL, NULL, &startupInfo[i], &processInfo[i]);

        if (isWriter)
            activeProcesses[i] = processInfo[i].hProcess;
        else
            activeProcesses[PAGES + i] = processInfo[i].hProcess;
    }
}

int main()
{
    SYSTEM_INFO systemInfo;
    int bufferSize;

    GetSystemInfo(&systemInfo);
    bufferSize = PAGES * systemInfo.dwPageSize;

    string bufferPath, mappedName;
    HANDLE hBuffer, hMapped;
    LPVOID address;
    bool writer = true,
            reader = false;

    bufferPath = BASE_PATH + "buffer.txt";
    mappedName = "buffer";

    hBuffer = CreateFileA(bufferPath.c_str(), GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, NULL);

    hMapped = CreateFileMappingA(hBuffer, NULL, PAGE_READWRITE,
                                 0, bufferSize, mappedName.c_str());

    address = MapViewOfFile(hMapped, FILE_MAP_WRITE, 0, 0, bufferSize);

    VirtualLock(address, bufferSize);

    createSemaphores(writer);
    createSemaphores(reader);

    createProcesses(writer);
    createProcesses(reader);

    WaitForMultipleObjects(PAGES << 1, activeProcesses, TRUE, INFINITE);

    return 0;
}