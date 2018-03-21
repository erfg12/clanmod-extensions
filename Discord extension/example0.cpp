#include "sleepy_discord/websocketpp_websocket.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif
#ifdef __linux__
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace std;

#define pipename "\\\\.\\pipe\\discord"
const char *chanid;

class myClientClass : public SleepyDiscord::DiscordClient {
public:
	DWORD dwWritten;
	HANDLE hPipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	using SleepyDiscord::DiscordClient::DiscordClient;
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
					if (strstr("say", array[0]) != NULL) { //say command
						sendMessage(chanid, array[1]);
					}
				}
			}
			free(data);
		}
		sleep(1000);
		getGameMsg(); //loop back around
	}
	void onReady(std::string* jsonMessage) {
		if (hPipe == INVALID_HANDLE_VALUE) 
			printf("Named Pipe Creation Error: %d\n", GetLastError());
		else {
			ConnectNamedPipe(hPipe, NULL);
			WriteFile(hPipe, "Discord extension connected", 999, &dwWritten, NULL);
		}
		std::thread first(&myClientClass::getGameMsg, this);
		first.detach();
	}
	//void onHeartbeat() {
		//printf("heartbeat\n");
	//}
	void onMessage(SleepyDiscord::Message message) {
		if (hPipe == INVALID_HANDLE_VALUE)
			printf("Named Pipe Creation Error: %d\n", GetLastError());
		else {
			ConnectNamedPipe(hPipe, NULL);
		}
		char trim[999];
		char *pretrim;
		sprintf(pretrim, "[DISCORD]%s|%s", message.author.username.c_str(), message.content.c_str());
		strncpy_s(trim, pretrim, _TRUNCATE);
		printf("SENDING: %s\n", trim);
		WriteFile(hPipe, trim, 999, &dwWritten, NULL);
	}
};

int main() {
	string line;
	ifstream myfile("channelid.cfg");
	if (!myfile.is_open()) {
		printf("missing channelid.cfg file!\n");
		return 0;
	}
	getline(myfile, line);
	chanid = line.c_str();
	myfile.close();
	
	myClientClass client("NDIzNTcyMTA0Mzc0Mzg2NzAw.DYsR4A.VsdbYVBEYeVJMVyq5QXiYSRnWOM", 2);
	client.run();
}