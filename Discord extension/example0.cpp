#include "sleepy_discord/websocketpp_websocket.h"
#include "inih/INIReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <string>
#include <vector>

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

std::string chanid; //discord channel id from ini file
std::string pipenames; //pipe name suffixes from ini file
vector<int> connectedPipes; //index array of connect pipes
//#ifdef WIN32
HANDLE pipeHandles[100]; //pipe handlers for Windows
/*#else
unsigned long pipeHandles[100]; //pipe handlers for Linux
#endif*/
char rcData[1000] = { 0 }; //received data, filter against this for send msgs to prevent sendback
std::vector<string> pnArray;

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
	void checkNamedPipeConnection(vector<string> pnArray) { //check named pipes and connect if available
		//cout << "[DEBUG] pnArray=" << pnArray.size() << " connectedPipes=" << connectedPipes.size() << endl;
		while (connectedPipes.size() < pnArray.size()) {
			for (int s = 0; s < pnArray.size(); ++s) {
#ifdef WIN32
				if (std::find(connectedPipes.begin(), connectedPipes.end(), s) != connectedPipes.end()) {
					//cout << "[DEBUG]:" << pnArray[s].c_str() << " already connected..." << endl;
					continue;
				}
				pipeHandles[s] = CreateFile(pnArray[s].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
				//cout << "trying to make pipe for " << pnArray[s].c_str() << endl;
				if (pipeHandles[s] != INVALID_HANDLE_VALUE) {
					cout << "Namedpipe " << pnArray[s].c_str() << " (pipeHandle #" << s << ") found! Connecting...";
					ConnectNamedPipe(pipeHandles[s], NULL);
					WriteFile(pipeHandles[s], "Discord extension connected", 999, &dwWritten, NULL);
					connectedPipes.push_back(s);
					cout << " DONE" << endl;
				}
				//else {
				//	cout << "[DEBUG] Named Pipe " << pnArray[s] << " is not available." << endl;
				//}
#else
				//linux code goes here
#endif
			}
		}
	}

	void makePipes() {
		char *token;
		char * writable = new char[pipenames.size()];
		std::copy(pipenames.begin(), pipenames.end(), writable);
		writable[pipenames.size()] = '\0';
		while ((token = strsep(&writable, ","))) { //build named pipe array
			char combine[255];
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "\\\\.\\pipe\\discord-%s", token); //make our real pipe name (discord-suffix) suffix from ini file pipenames
			pnArray.push_back(combine);
		}
		std::thread second(&myClientClass::checkNamedPipeConnection, this, pnArray);
		second.detach();
	}

	using SleepyDiscord::DiscordClient::DiscordClient;
	void getGameMsg() {
		while (1) {
			char data[1000] = { 0 };
			char peekBuf[1000] = { 0 };
			DWORD numRead;
			DWORD bytesAvail = 0;

			for (int i = 0; i < connectedPipes.size(); i++) {
				int cp = connectedPipes[i];
				if (pipeHandles[cp] == INVALID_HANDLE_VALUE) {
					cout << pnArray[cp] << " #" << cp << " is bad! ERR:" << GetLastError() << endl;
					continue;
				}
				if (PeekNamedPipe(pipeHandles[cp], NULL, 1000, NULL, &bytesAvail, NULL)) {
					if (bytesAvail > 0) {
						ReadFile(pipeHandles[cp], data, 1000, &numRead, NULL);
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
								//printf("[DEBUG] Recognized command: %s\n", array[0]);
								if (strstr("say", array[0]) != NULL) { //say command
									printf("[DEBUG] Sending text:%s chanid:%s\n", array[1], chanid);
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
		makePipes();
		std::thread first(&myClientClass::getGameMsg, this);
		first.detach();
	}
	//void onHeartbeat() {
		//printf("[DEBUG] heartbeat\n");
	//}
	void onMessage(SleepyDiscord::Message message) {
		//if (strstr(message.content.c_str(), rcData) != NULL) return; //prevent sendback
		char trim[999];
		_snprintf_s(trim, sizeof(trim), _TRUNCATE, "say|[DISCORD]%s: %s", message.author.username.c_str(), message.content.c_str()); //prefix [DISCORD] so we know where it came from
		for (int i = 0; i < connectedPipes.size(); i++) {
			if (pipeHandles[connectedPipes[i]] == INVALID_HANDLE_VALUE) {
				cout << "ERROR: pipeHandles #" << connectedPipes[i] << " is invalid! ERR:" << GetLastError() << endl;
			}
			else {
				WriteFile(pipeHandles[connectedPipes[i]], trim, 999, &dwWritten, NULL);
				cout << "SENDING: " << trim << endl;
			}
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
	chanid = reader.Get("settings", "channelid", "00000000000000");
	pipenames = reader.Get("settings", "pipenames", "jko,jka");
	
	myClientClass client(reader.Get("settings", "token", "NDIzNTcyMTA0Mzc0Mzg2NzAw.DYsR4A.VsdbYVBEYeVJMVyq5QXiYSRnWOM"), 2);
	client.run();
}