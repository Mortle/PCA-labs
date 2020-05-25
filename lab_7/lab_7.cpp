#include <ctime>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <conio.h>

#define PORT_SERVER "COM1"
#define PORT_CLIENT "COM2"
#define TIMEOUT 500

using namespace std;

void client() {
	HANDLE pipe, sems[3];
	char buffer[20];
	int buffer_size = sizeof(buffer);
	string message, name = PORT_CLIENT;
	bool flag_success;

	sems[0] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE");
	sems[1] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE_END");
	sems[2] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE_EXIT");

	cout << "------------------------ CLIENT ------------------------\n\n";

	pipe = CreateFile((LPSTR)name.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	while (true) {
		flag_success = TRUE;
		DWORD bytes_num;
		message.clear();

		int index = WaitForMultipleObjects(3, sems, FALSE, INFINITE) - WAIT_OBJECT_0;
		if (index == 2) break;

		int blocks_num;
		if (!ReadFile(pipe, &blocks_num, sizeof(blocks_num), &bytes_num, NULL)) break;

		int size;
		if (!ReadFile(pipe, &size, sizeof(size), &bytes_num, NULL)) break;

		for (int i = 0; i < blocks_num; i++) {
			flag_success = ReadFile(pipe, buffer, buffer_size, &bytes_num, NULL);
			if (!flag_success) {
				break;
			}
			message.append(buffer, buffer_size);
		}

		if (!flag_success) break;

		message.resize(size);

		for (int i = 0; i < size; i++) {
			cout << message[i];
		}
		cout << endl;
		ReleaseSemaphore(sems[1], 1, NULL);
	}

	CloseHandle(pipe);
	CloseHandle(sems[0]);
	CloseHandle(sems[1]);
	return;
}

void server(char* path) {
	string name = PORT_SERVER;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION child_p_info;
	ZeroMemory(&child_p_info, sizeof(child_p_info));

	HANDLE handler, sems[3];
	
	char buffer[20];
	int buffer_size = sizeof(buffer);
	string message;

	cout << "------------------------ SERVER ------------------------\n\n";

	sems[0] = CreateSemaphore(NULL, 0, 1, "SEMAPHORE");
	sems[1] = CreateSemaphore(NULL, 0, 1, "SEMAPHORE_END");
	sems[2] = CreateSemaphore(NULL, 0, 1, "SEMAPHORE_EXIT");

	handler = CreateFile((LPSTR)name.c_str(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	CreateProcess((LPSTR)path, (LPSTR)" client", NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si, &child_p_info);

	SetCommMask(handler, EV_RXCHAR);
	SetupComm(handler, 1500, 1500);
	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;

	if (!SetCommTimeouts(handler, &CommTimeOuts)) {
		CloseHandle(handler);
		handler = INVALID_HANDLE_VALUE;
		return;
	}

	DCB ComDCM; // serial port structure
		memset(&ComDCM, 0, sizeof(ComDCM));
		ComDCM.DCBlength = sizeof(DCB);
		GetCommState(handler, &ComDCM);
		ComDCM.BaudRate = DWORD(9600);
		ComDCM.ByteSize = 8;
		ComDCM.Parity = NOPARITY;
		ComDCM.StopBits = ONESTOPBIT;
		ComDCM.fAbortOnError = TRUE;
		ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
		ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
		ComDCM.fBinary = TRUE;
		ComDCM.fParity = FALSE;
		ComDCM.fInX = FALSE;
		ComDCM.fOutX = FALSE;
		ComDCM.XonChar = 0;
		ComDCM.XoffChar = (unsigned char)0xFF;
		ComDCM.fErrorChar = FALSE;
		ComDCM.fNull = FALSE;
		ComDCM.fOutxCtsFlow = FALSE;
		ComDCM.fOutxDsrFlow = FALSE;
		ComDCM.XonLim = 128;
		ComDCM.XoffLim = 128;

	if (!SetCommState(handler, &ComDCM)) {
		CloseHandle(handler);
		handler = INVALID_HANDLE_VALUE;
		return;
	}

	while (true) {
		DWORD bytes_num_written;

		cout << "Enter Your Message: ";
		cin.clear();
		getline(cin, message);
		if (message == "exit") {
			ReleaseSemaphore(sems[2], 1, NULL);
			WaitForSingleObject(child_p_info.hProcess, INFINITE);
			break;
		}

		ReleaseSemaphore(sems[0], 1, NULL);

		int blocks_num = message.size() / buffer_size + 1;
		WriteFile(handler, &blocks_num, sizeof(blocks_num), &bytes_num_written, NULL);

		int size = message.size();
		WriteFile(handler, &size, sizeof(size), &bytes_num_written, NULL);

		for (int i = 0; i < blocks_num; i++) {
			message.copy(buffer, buffer_size, i * buffer_size);
			if (!WriteFile(handler, buffer, buffer_size, &bytes_num_written, NULL))
				cout << "Something went wrong...";
		}

		WaitForSingleObject(sems[1], INFINITE);
	}

	  CloseHandle(handler);
	  CloseHandle(sems[0]);
	  CloseHandle(sems[1]);
	  cout << "\n";
	  system("pause");
	  return;
}

int main(int argc, char* argv[]) {
	switch (argc) {
	case 1:
		server(argv[0]);
		break;
	default:
		client();
		break;
	}
}

