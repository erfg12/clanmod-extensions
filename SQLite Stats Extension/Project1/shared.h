#ifndef _SHARED_H_
#define _SHARED_H_

#include <iostream>
#include "sqlite3/sqlite3.h"
#include <string.h>
#include <thread>

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

#define pipename "\\\\.\\pipe\\sqlstats"

extern HANDLE hPipe;
extern DWORD dwWritten;

extern void sqliteRegisterUser(char *SQLStmnt, ...);
extern int sqliteSelectUserID(char *SQLStmnt, ...);
extern char *sqliteGetName(char *SQLStmnt, ...);
extern void sqliteUpdateStats(char *SQLStmnt, ...);
extern char *SHA1ThisPass(const unsigned char *myPass);
#endif