#include <windows.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::cin;
using std::string;

char* data = new char[256];

void CALLBACK CompletionRoutine(DWORD errorCode, DWORD bytestransfered, LPOVERLAPPED lpOverlapped) { // функция завершения
    cout << data << endl << endl;
}

int main() {
    bool running = true;
    HANDLE pipe;
    string pipename;
    OVERLAPPED overlapped;
    BOOL isRead;

    ZeroMemory(&overlapped, sizeof(overlapped));

    pipename = "\\\\.\\pipe\\name";
    pipe = CreateFileA(pipename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // подключение к именованному каналу

    cout << "Result:\n";

    while (running) {
        isRead = ReadFileEx(pipe, data, 256, &overlapped, CompletionRoutine); // чтение данных

        if (isRead && pipe != INVALID_HANDLE_VALUE) {
            SleepEx(INFINITE, TRUE); // вызов функции завершения
        }
        else {
            cout << "Client finished\n";
            running = false;
        }
    }

    return 0;
}
