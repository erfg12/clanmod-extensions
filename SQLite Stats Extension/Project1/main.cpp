#include "shared.h"

HANDLE hPipe;
DWORD dwWritten;

void getGameMsg() {
	char data[1000];
	DWORD numRead;
	char *token;
	DWORD bytesAvail = 0;
	BOOL isOK = PeekNamedPipe(hPipe, NULL, 0, NULL, &bytesAvail, NULL);
	if (bytesAvail > 0) {
		ReadFile(hPipe, data, 1000, &numRead, NULL);
		if (numRead > 0) {
			printf("RECEIVED: %s\n", data);
			if (strstr(data, "|") != NULL) {
				char *token = strtok(data, "|");
				char *array[3];
				int i = 0;
				while (token != NULL)
				{
					array[i++] = token;
					token = strtok(NULL, "|");
				}
				if (strstr("register", array[0]) != NULL) {
					//sqliteRegisterUser
				}
				if (strstr("login", array[0]) != NULL) {
					//login
				}
				if (strstr("findname", array[0]) != NULL) {
					//sqliteGetName
				}
				if (strstr("update", array[0]) != NULL) {
					//updateStats
				}
			}
		}
		free(data);
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	getGameMsg(); //loop back around
}

int main()
{
	hPipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hPipe == INVALID_HANDLE_VALUE)
		printf("Named Pipe Creation Error: %d\n", GetLastError());
	else {
		ConnectNamedPipe(hPipe, NULL);
		WriteFile(hPipe, "SQLite Stats extension connected", 999, &dwWritten, NULL);
	}
	std::thread first(getGameMsg);
	first.detach();
}