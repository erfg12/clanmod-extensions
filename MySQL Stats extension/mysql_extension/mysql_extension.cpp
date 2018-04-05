// mysql_extension.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>

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

#include <inih/ini.h>
#include <inih/INIReader.h>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

std::string dbname;
std::string dbconn;
std::string dbuser;
std::string dbpass;

using namespace std;

std::string pipenames;
vector<int> connectedPipes; //index array of connect pipes
#ifdef WIN32
HANDLE pipeHandles[100]; //pipe handlers for Windows
#else
std::vector<string> linuxPipeHandles[100]; //pipe handlers for Linux
#endif
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

void checkNamedPipeConnection(vector<string> pnArray) { //check named pipes and connect if available
	//cout << "[DEBUG] pnArray=" << pnArray.size() << " connectedPipes=" << connectedPipes.size() << endl;
	DWORD dwWritten;
	while (connectedPipes.size() < pnArray.size()) {
		for (int s = 0; s < pnArray.size(); ++s) {
#ifdef WIN32
			if (std::find(connectedPipes.begin(), connectedPipes.end(), s) != connectedPipes.end()) {
				//cout << "[DEBUG]:" << pnArray[s].c_str() << " already connected..." << endl;
				continue;
			}
			LPCTSTR lp = (LPCTSTR)pnArray[s].c_str();
			pipeHandles[s] = CreateFile(lp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			//cout << "trying to make pipe for " << pnArray[s].c_str() << endl;
			if (pipeHandles[s] != INVALID_HANDLE_VALUE) {
				cout << "Namedpipe " << pnArray[s].c_str() << " (pipeHandle #" << s << ") found! Connecting...";
				ConnectNamedPipe(pipeHandles[s], NULL);
				WriteFile(pipeHandles[s], "mysqlstats extension connected", 999, &dwWritten, NULL);
				connectedPipes.push_back(s);
				cout << " DONE" << endl;
			}
			//else {
			//	cout << "[DEBUG] Named Pipe " << pnArray[s] << " is not available." << endl;
			//}
#else
			char combine[255];
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "/tmp/%s", pnArray[s].erase(pnArray[s].begin() + 9));
			linuxPipeHandles.push_back(combine);
			FILE *stream;
			stream = fdopen(combine, "w");
			fprintf(stream, "mysqlstats extension connected");
			fclose(stream);
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
		_snprintf_s(combine, sizeof(combine), _TRUNCATE, "\\\\.\\pipe\\mysqlstats-%s", token); //make our real pipe name (discord-suffix) suffix from ini file pipenames
		pnArray.push_back(combine);
	}
	std::thread second(checkNamedPipeConnection, pnArray);
	second.detach();
}

void sendMsg(char data[1000]) {
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
		if (strstr("login", array[0]) != NULL) {
			//cmd here
		}
		if (strstr("register", array[0]) != NULL) {
			//cmd here
		}
		if (strstr("getstats", array[0]) != NULL) {
			//cmd here
		}
		if (strstr("changestats", array[0]) != NULL) {
			//cmd here
		}
	}
}

void getGameMsg() {
	while (1) {
		char data[1000] = { 0 };
		char peekBuf[1000] = { 0 };
#ifdef _WIN32
		DWORD numRead;
		DWORD bytesAvail = 0;
#endif

		for (int i = 0; i < connectedPipes.size(); i++) {
			int cp = connectedPipes[i];
			if (pipeHandles[cp] == INVALID_HANDLE_VALUE) {
				cout << pnArray[cp] << " #" << cp << " is bad! ERR:" << GetLastError() << endl;
				continue;
			}
#ifdef _WIN32
			if (PeekNamedPipe(pipeHandles[cp], NULL, 1000, NULL, &bytesAvail, NULL)) {
				if (bytesAvail > 0) {
					ReadFile(pipeHandles[cp], data, 1000, &numRead, NULL);
					if (numRead > 0) {
						sendMsg(data);
					}
				}
			}
#else
			FILE *stream;
			stream = fdopen(linuxPipeHandles[cp], "r");
			while ((c = fgetc(stream)) != EOF) {
				sendMsg(c);
			}
			fclose(stream);
#endif
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

int main()
{
	INIReader reader("settings.ini");

	if (reader.ParseError() < 0) {
		std::cout << "Can't load 'settings.ini'\n";
		return 1;
	}

	dbname = reader.Get("settings", "dbname", "clanmod");
	dbconn = reader.Get("settings", "dbconn", "tcp://127.0.0.1:3306");
	dbuser = reader.Get("settings", "dbuser", "root");
	dbpass = reader.Get("settings", "dbpass", "MyP@ssw0rd!");

	makePipes();
	std::thread first(getGameMsg);
	first.detach();

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		driver = get_driver_instance();
		con = driver->connect(dbconn, dbuser, dbpass);
		con->setSchema(dbname);
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
    return 0;
}

