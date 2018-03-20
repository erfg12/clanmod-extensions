#include "shared.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

char *SHA1ThisPass(const unsigned char *myPass) {
	SHA1Context sha;
	int i;
	char *balance[5];

	SHA1Reset(&sha);
	SHA1Input(&sha, myPass, strlen(reinterpret_cast<const char*>(myPass)));

	if (!SHA1Result(&sha))
		fprintf(stderr, "ERROR-- could not compute message digest\n");
	else
	{
		for (i = 0; i < 5; i++)
		{
			sscanf(balance[i], "%d", &sha.Message_Digest[i]);
		}
	}
	char * s = (char*)malloc(snprintf(NULL, 0, "%s%s%s%s%s", balance[0], balance[1], balance[2], balance[3], balance[4]) + 1);
	sprintf(s, "%s%s%s%s%s", balance[0], balance[1], balance[2], balance[3], balance[4]);

	return s;
}

void sqliteRegisterUser(char *SQLStmnt, ...) {
	va_list		argptr;
	char		text[1024];

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_open("clanmod.db", &db);
	if (rc) //error connecting
		sqlite3_close(db);

	va_start(argptr, SQLStmnt);
	vsprintf(text, SQLStmnt, argptr);
	va_end(argptr);
	rc = sqlite3_exec(db, text, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
	}
	else {
		int lastid = sqlite3_last_insert_rowid(db);
		char sql[1024];
		sprintf(sql, "INSERT INTO stats (user_id) VALUES ('%i')", lastid);
		rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			sqlite3_close(db);
			printf("Database error\n");
		}
		else {
			sqlite3_close(db);
			printf("Database insert success\n");
		}
	}
}

void sqliteUpdateStats(char *SQLStmnt, ...) {
	va_list		argptr;
	char		text[1024];

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_open("clanmod.db", &db);
	if (rc) { //error connecting
		sqlite3_close(db);
		printf("ERROR: Can't connecting to clanmod.db is the file in your GameData folder?\n");
	}

	va_start(argptr, SQLStmnt);
	vsprintf(text, SQLStmnt, argptr);
	va_end(argptr);
	rc = sqlite3_exec(db, text, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		printf("Database error\n");
	}
	else {
		sqlite3_close(db);
		printf("Database insert success\n");
	}
}

int sqliteSelectUserID(char *SQLStmnt, ...) {
	sqlite3_stmt *stmt;
	sqlite3 *db;
	char *zErrMsg = 0;
	va_list		argptr;
	char		text[1024];

	va_start(argptr, SQLStmnt);
	vsprintf(text, SQLStmnt, argptr);
	va_end(argptr);

	if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
	{
		int rc = sqlite3_prepare_v2(db, text, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
			fprintf(stderr, "SQL error: %s\n", zErrMsg);

		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			printf("user ID not found\n");
			return 0;
		}

		if (rc == SQLITE_DONE) {
			sqlite3_finalize(stmt);
			printf("user ID not found\n");
			return 0;
		}

		int id = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return id;
	}
	else
		return 0;
}

char *sqliteGetName(char *SQLStmnt, ...) {
	sqlite3_stmt *stmt;
	sqlite3 *db;
	char *zErrMsg = 0;
	va_list		argptr;
	char		text[1024];

	va_start(argptr, SQLStmnt);
	vsprintf(text, SQLStmnt, argptr);
	va_end(argptr);

	if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
	{
		int rc = sqlite3_prepare_v2(db, text, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
			fprintf(stderr, "SQL error: %s\n", zErrMsg);

		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			printf("user name not found\n");
			return 0;
		}

		if (rc == SQLITE_DONE) {
			sqlite3_finalize(stmt);
			printf("user name not found\n");
			return 0;
		}

		char data[1024];
		sprintf(data, reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		sqlite3_finalize(stmt);
		return data;
	}
	else
		return NULL;
}