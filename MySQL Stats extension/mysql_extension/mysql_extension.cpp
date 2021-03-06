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

#include <mysql.h>

#include <inih/ini.h>
#include <inih/INIReader.h>

using namespace std;

char *dbname;
char *dbconn;
char *dbuser;
char *dbpass;
char *pipenames;

MYSQL mysql;

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

char *toChar(std::string convert) {
	char * writable = new char[convert.size()];
	std::copy(convert.begin(), convert.end(), writable);
	writable[convert.size()] = '\0';
	return writable;
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
			pipeHandles[s] = CreateFile(pnArray[s].c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			//cout << "trying to make pipe for " << pnArray[s].c_str() << endl;
			if (pipeHandles[s] != INVALID_HANDLE_VALUE) {
				cout << "Namedpipe " << pnArray[s].c_str() << " (pipeHandle #" << s << ") found! Connecting...";
				ConnectNamedPipe(pipeHandles[s], NULL);
				WriteFile(pipeHandles[s], "mysqlstats extension connected", 999, &dwWritten, NULL);
				connectedPipes.push_back(s);
				cout << " DONE" << endl;
			}
			else {
				//cout << "[DEBUG] Named Pipe " << pnArray[s] << " is not available." << endl;
			}
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
	while ((token = strsep(&pipenames, ","))) { //build named pipe array
		char combine[255];
		//cout << token << " found..." << endl;
		_snprintf_s(combine, sizeof(combine), _TRUNCATE, "\\\\.\\pipe\\mysqlstats-%s", token); //make our real pipe name (discord-suffix) suffix from ini file pipenames
		pnArray.push_back(combine);
	}
	std::thread second(checkNamedPipeConnection, pnArray);
	second.detach();
}

void sendMsg(char data[1000]) {
	DWORD dwWritten;
	for (int i = 0; i < connectedPipes.size(); i++) {
		if (pipeHandles[connectedPipes[i]] == INVALID_HANDLE_VALUE) {
			cout << "ERROR: pipeHandles #" << connectedPipes[i] << " is invalid! ERR:" << GetLastError() << endl;
		}
		else {
#ifdef _WIN32
			WriteFile(pipeHandles[connectedPipes[i]], data, 999, &dwWritten, NULL);
#else
			FILE *stream;
			stream = fdopen(connectedPipes[i], "w");
			fprintf(stream, data);
			fclose(stream);

#endif
			cout << "SENDING: " << data << endl;
		}
	}
}

void receiveMsg(char data[1000]) {
	printf("RECEIVED: %s\n", data);
	if (strstr(data, "|") != NULL) {
		char *next_token, *next_token2;
		char *token = strtok_s(data, "|", &next_token);
		char *array[2];
		char *args[25]; //for comma delimeter args
		int i = 0;
		char combine[500];
		char trim[999];

		/*if (!con->isValid())
		{
			cout << "connection is closed, reconnecting..." << endl;
			con->reconnect();
		}*/

		while (token != NULL)
		{
			if (i == 2) break;
			array[i++] = token;
			token = strtok_s(NULL, "|", &next_token);
		}
		//printf("[DEBUG] Recognized command: %s\n", array[0]);
		string t = array[1]; i = 0;
		if (t.find(",") != string::npos) {
			char *token2 = strtok_s(array[1], ",", &next_token2);
			while (token2 != NULL)
			{
				if (i == 25) break;
				args[i++] = token2;
				token2 = strtok_s(NULL, ",", &next_token2);
			}
		}
		if (strstr("login", array[0]) != NULL) { //0=gameclientid, 1=username, 2=password. Returns db user ID, fail returns 0
			if (args[0] == NULL || args[1] == NULL || args[2] == NULL)
			{
				cout << "ERROR: an arg is null" << endl;
			}
			cout << "logging in with u:" << args[1] << " p:" << args[2] << endl;
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "SELECT * FROM users WHERE username='%s' AND password='%s'", args[1], args[2]);
			cout << "combine created" << endl;
			
			if (mysql_query(&mysql, combine))
			{
				//syslog(LOG_CONS, "%s->%s", "MySQL Query Eorror:",mysql_error(&mysql)); mysql_close(&mysql);
				//exit(0);
			}
			MYSQL_RES *result;
			result = mysql_use_result(&mysql);

			if (result)
			{
				MYSQL_ROW row;
				if ((row = mysql_fetch_row(result)))
				{
					unsigned long *lengths = mysql_fetch_lengths(result);
					if (lengths[0] > 0)
					{
						cout << "found user by ID #" << row[0] << endl;
						_snprintf_s(trim, sizeof(trim), _TRUNCATE, "login|%s,%s", args[0], row[0]);
					}
				}
				mysql_free_result(result);
			}
			else //no user found
				_snprintf_s(trim, sizeof(trim), _TRUNCATE, "login|%s,%i", args[0], 0);
			sendMsg(trim);
		}
		/*if (strstr("register", array[0]) != NULL) { //0=gameclientid, 1=username, 2=password
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "INSERT INTO 'users' ('username', 'password') VALUES ('%s', '%s')", args[1], args[2]);
			stmt = con->createStatement();
			int count = stmt->execute(combine);
			if (count > 0){
				_snprintf_s(trim, sizeof(trim), _TRUNCATE, "tell|%s,%s is now registered!", args[0], args[1]);
				sendMsg(trim);
			}
			else {
				_snprintf_s(trim, sizeof(trim), _TRUNCATE, "tell|%s,%s is already a registered user.", args[0], args[1]);
				sendMsg(trim);
			}
			delete stmt;
		}
		if (strstr("getstats", array[0]) != NULL) { //0=gameclientid, 1=dbuserid, 2=gamename
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "SELECT * FROM '%s' WHERE user_id='%s'", args[2], args[1]);
			pstmt = con->prepareStatement(toChar(combine));
			res = pstmt->executeQuery();
			if (res->rowsCount() == 1) {
				_snprintf_s(trim, sizeof(trim), _TRUNCATE, "stats|%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i", args[0], res->getInt("kills"), res->getInt("deaths"), res->getInt("duel_wins"), res->getInt("duel_loses"), res->getInt("flag_captures"), res->getInt("ffa_wins"), res->getInt("ffa_loses"), res->getInt("tdm_wins"), res->getInt("tdm_loses"), res->getInt("siege_wins"), res->getInt("siege_loses"), res->getInt("ctf_wins"), res->getInt("ctf_loses"));
				sendMsg(trim);
			}
			delete res;
			delete pstmt;
		}
		if (strstr("changestats", array[0]) != NULL) { //0=gameclientid, 1=gamename, 2,3,etc=stats
			_snprintf_s(combine, sizeof(combine), _TRUNCATE, "UPDATE '%s' SET 'kills' = '%s', 'deaths' = '%s', 'duel_wins' = '%s', 'duel_loses' = '%s', 'flag_captures' = '%s', 'ffa_wins' = '%s', 'ffa_loses' = '%s', 'tdm_wins' = '%s', 'tdm_loses' = '%s', 'siege_wins' = '%s', 'siege_loses' = '%s', 'ctf_wins' = '%s', 'ctf_loses' = '%s' WHERE user_id = '%s'", args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[0]);
			stmt = con->createStatement();
			stmt->execute(combine);
			delete stmt;
		}*/
	}
}

int main()
{
#ifdef _DEBUG
	dbname = toChar("newage_clanmod");
	dbconn = toChar("newagesoftware.net");
	dbuser = toChar("newage_jake");
	dbpass = toChar("Cl4nm0d!2");
	pipenames = toChar("jka,jko");
#else
	INIReader reader("settings.ini");

	if (reader.ParseError() < 0) {
		std::cout << "Can't load 'settings.ini'\n";
		return 1;
	}

	dbname = toChar(reader.Get("settings", "dbname", "clanmod"));
	dbconn = toChar(reader.Get("settings", "dbconn", "tcp://127.0.0.1:3306"));
	dbuser = toChar(reader.Get("settings", "dbuser", "root"));
	dbpass = toChar(reader.Get("settings", "dbpass", "MyP@ssw0rd!"));
	pipenames = toChar(reader.Get("settings", "pipenames", "jko,jka"));
#endif

	cout << "mysql connecting with db:" << dbname << " c:" << dbconn << " u:" << dbuser << " p:**********" << endl;
	mysql_init(&mysql);
	if (mysql_real_connect(&mysql, dbconn, dbuser, dbpass, dbname, 0, NULL, 0))
		cout << "mysql connected!" << endl;
	else
		cout << "mysql connection failed! ERR:" << mysql_error(&mysql) << endl;
	my_bool reconnect = 1;
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, &reconnect);

	makePipes();

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
						receiveMsg(data);
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
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    return 1;
}

