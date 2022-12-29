#include <windows.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::cin;
using std::string;

int main() {
	bool running = true;
	HANDLE pipe;
	string pipeName, eventName;
	char* data = new char[256];
	OVERLAPPED overlapped;
	DWORD written;

	ZeroMemory(&overlapped, sizeof(overlapped));

	eventName = "eventName";
	overlapped.hEvent = CreateEventA(NULL, TRUE, FALSE, eventName.c_str()); // создание объекта "событие"

	pipeName = "\\\\.\\pipe\\name";

	pipe = CreateNamedPipeA(pipeName.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 
		PIPE_TYPE_BYTE | PIPE_WAIT, 1, 256, 256, 0, NULL); // создание именованного канала

	ConnectNamedPipe(pipe, &overlapped); // установление соединения

	cout << "Enter data (enter \"stop\" to stop)\n";

	while (running) {
		cin >> data;

		if (strcmp(data, "stop") != 0) {
			cout << endl;
			WriteFile(pipe, data, strlen(data) + 1, &written, &overlapped); // запись данных в именованный канал

			WaitForSingleObject(overlapped.hEvent, INFINITE); // ожидание завершения ввода-вывода
		}
		else {
			cout << "\nServer finished\n";
			running = false;
		}
	}

	DisconnectNamedPipe(pipe); // отключение соединения

	return 0;
}
