#include "sleepy_discord/websocketpp_websocket.h"
#include "inih/INIReader.h"
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
		while (1) {
			char data[1000] = { 0 };
			char peekBuf[1000] = { 0 };
			DWORD numRead;
			char *token;
			DWORD bytesAvail = 0;
			if (PeekNamedPipe(hPipe, NULL, 1000, NULL, &bytesAvail, NULL)) {
				if (bytesAvail > 0) {
					ReadFile(hPipe, data, 1000, &numRead, NULL);
					if (numRead > 0) {
						printf("RECEIVED: %s\n", data);
						if (strstr(data, "|") != NULL) {
							char *next_token;
							char *token = strtok_s(data, "|", &next_token);
							char *array[2];
							int i = 0;
							while (token != NULL)
							{
								if (i == 2) break;
								array[i++] = token;
								token = strtok_s(NULL, "|", &next_token);
							}
							//printf("Recognized command: %s\n", array[0]);
							if (strstr("say", array[0]) != NULL) { //say command
								//printf("Sending text: %s\n", array[1]);
								sendMessage(chanid, array[1]);
							}
						}
					}
				}
			}
			sleep(1000);
		}
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
		if (strstr(message.content.c_str(), ": [") != NULL) { //dont re-send messages
			return;
		}
		if (hPipe == INVALID_HANDLE_VALUE)
			printf("Named Pipe Creation Error: %d\n", GetLastError());
		else {
			ConnectNamedPipe(hPipe, NULL);
		}
		char trim[999];
		_snprintf_s(trim, sizeof(trim), _TRUNCATE, "say|[DISCORD]%s: %s", message.author.username.c_str(), message.content.c_str()); //prefix [DISCORD] so we know where it came from
		printf("SENDING: %s\n", trim);
		WriteFile(hPipe, trim, 999, &dwWritten, NULL);
	}
};

int main() {
	string line;
	/*ifstream myfile("channelid.cfg");
	if (!myfile.is_open()) {
		printf("missing channelid.cfg file!\n");
		return 0;
	}
	getline(myfile, line);
	myfile.close();*/

	INIReader reader("settings.ini");

	if (reader.ParseError() < 0) {
		std::cout << "Can't load 'settings.ini'\n";
		return 1;
	}
	chanid = reader.Get("settings", "channelid", "00000000000000").c_str();
	
	myClientClass client("NDIzNTcyMTA0Mzc0Mzg2NzAw.DYsR4A.VsdbYVBEYeVJMVyq5QXiYSRnWOM", 2);
	client.run();
}