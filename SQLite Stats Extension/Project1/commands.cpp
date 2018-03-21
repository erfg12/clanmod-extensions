#include "shared.h"

void login(char *name, const unsigned char *pass) {
	char s[1024];
	snprintf(s, 1024, "SELECT * FROM users WHERE user = '%s' AND pass = '%s'", name, SHA1ThisPass(pass));
	int userID = sqliteSelectUserID(s);

	if (userID > 0) {
		printf("You are now logged in.\n");
	}
	else
		printf("User not found\n");
}

void updateStats(int sql_kills, int	sql_deaths, int	sql_flagcaps, int sql_duelwins, int	sql_duelloses, int sql_ffawins, int	sql_ffaloses, int sql_tdmwins, int sql_tdmloses, int sql_siegewins, int sql_siegeloses, int sql_ctfwins, int sql_ctfloses, const char *userID) {
	char s[1024];
	snprintf(s, 1024, "UPDATE stats SET kills=kills+%i, deaths=deaths+%i, duel_wins=duel_wins+%i, duel_loses=duel_loses+%i, flag_captures=flag_captures+%i, ffa_wins=ffa_wins+%i, ffa_loses=ffa_loses+%i, tdm_wins=tdm_wins+%i, tdm_loses=tdm_loses+%i, siege_wins=siege_wins+%i, siege_loses=siege_loses+%i, ctf_wins=ctf_wins+%i, ctf_loses=ctf_loses+%i WHERE user_id = '%i'", sql_kills, sql_deaths, sql_duelwins, sql_duelloses, sql_flagcaps, sql_ffawins, sql_ffaloses, sql_tdmwins, sql_tdmloses, sql_siegewins, sql_siegeloses, sql_ctfwins, sql_ctfloses, atoi(userID));
	sqliteUpdateStats(s);
	printf("stats have been updated");
}

void sendResponse(char *msg) {
	char trim[999];
	strncpy_s(trim, msg, _TRUNCATE);
	printf("SENDING: %s\n", trim);
	WriteFile(hPipe, trim, 999, &dwWritten, NULL);
}