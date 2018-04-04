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

//#define pipename "\\\\.\\pipe\\discord"

const char *chanid; //discord channel id from ini file
std::string pipenames; //pipe name suffixes from ini file
int totalPipesNum = 0; //a total of pipes for iteration
#ifdef WIN32
HANDLE pipeHandles[100]; //pipe handlers for Windows
#else
unsigned long pipeHandles[100]; //pipe handlers for Linux
#endif

char *strsep(char **stringp, const char *delim) {
	if (*stringp == NULL) { return NULL; }
	char *token_start = *stringp;
	*stringp = strpbrk(token_start, delim);
	if (*stringp) {
		**stringp = '\0';
		(*stringp)++;
	}
	return token_start;
}

class myClientClass : public SleepyDiscord::DiscordClient {
public:
	DWORD dwWritten;
	void makePipes(const char *pipename, int pipeNum) {
		char realName[255];
		_snprintf_s(realName, sizeof(realName), _TRUNCATE, "%s%s", "\\\\.\\pipe\\", pipename);
#ifdef WIN32
		pipeHandles[pipeNum] = CreateFile(realName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (pipeHandles[pipeNum] == INVALID_HANDLE_VALUE)
			printf("Named Pipe Creation Error: %d\n", GetLastError());
		else {
			ConnectNamedPipe(pipeHandles[pipeNum], NULL);
			WriteFile(pipeHandles[pipeNum], "Discord extension connected", 999, &dwWritten, NULL);
		}
#else
#endif
		totalPipesNum = totalPipesNum + 1;
	}

	using SleepyDiscord::DiscordClient::DiscordClient;
	void getGameMsg() {
		while (1) {
			char data[1000] = { 0 };
			char peekBuf[1000] = { 0 };
			DWORD numRead;
			DWORD bytesAvail = 0;
			for (int i = 0; i < totalPipesNum; i++) {
				if (PeekNamedPipe(pipeHandles[i], NULL, 1000, NULL, &bytesAvail, NULL)) {
					if (bytesAvail > 0) {
						ReadFile(pipeHandles[i], data, 1000, &numRead, NULL);
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
			}
			sleep(1000);
		}
	}
	void onReady(std::string* jsonMessage) {
		char *token;
		int i = 0;
		char * writable = new char[pipenames.size() + 1];
		std::copy(pipenames.begin(), pipenames.end(), writable);
		writable[pipenames.size()] = '\0';
		//delete[] writable;
		while ((token = strsep(&writable, ","))) {
			char combine[255];
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "discord-%s", token); //make our real pipe name (discord-suffix) suffix from ini file pipenames
			makePipes(combine, i);
			i++;
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
		char trim[999];
		_snprintf_s(trim, sizeof(trim), _TRUNCATE, "say|[DISCORD]%s: %s", message.author.username.c_str(), message.content.c_str()); //prefix [DISCORD] so we know where it came from
		printf("SENDING: %s\n", trim);
		for (int i = 0; i < totalPipesNum; i++) {
			WriteFile(pipeHandles[i], trim, 999, &dwWritten, NULL);
		}
	}
};

int main() {
	string line;
	INIReader reader("settings.ini");

	if (reader.ParseError() < 0) {
		std::cout << "Can't load 'settings.ini'\n";
		return 1;
	}
	chanid = reader.Get("settings", "channelid", "00000000000000").c_str();
	pipenames = reader.Get("settings", "pipenames", "jko,jka");
	
	myClientClass client("NDIzNTcyMTA0Mzc0Mzg2NzAw.DYsR4A.VsdbYVBEYeVJMVyq5QXiYSRnWOM", 2);
	client.run();
}