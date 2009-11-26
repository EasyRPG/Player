#include "log.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

int verbosityLevel = VERBOSITY_ALL;
const char *stringVerbosityLevel[VERBOSITY_LEVELS] =
{
	"level error NONE:",
	"level error INFO:",
	"level error WARNING:",
	"level error ERROR:",
	"level error ALL:"
};

int setLog(int errorLevel)
{
	if (errorLevel < VERBOSITY_LEVELS)
	{
		verbosityLevel = errorLevel;
		return 1;
	}

	log(VERBOSITY_WARNING, " set unknown error level");
	return 0;
}

int log(int errorLevel, const char *error)
{
	struct tm *tm;
	time_t t;
	t = time(NULL);
	tm = localtime(&t);
	char *strTime = asctime(tm);
	*(strchr(strTime, '\n')) = '\0';

	FILE* streamLog = NULL;

	switch (errorLevel)
	{
		case ERROR_LEVEL_NONE:
			streamLog = stderr;
			break;
		case ERROR_LEVEL_INFO:
			streamLog = stdout;
			break;
		case ERROR_LEVEL_WARNING:
			streamLog = stderr;
			break;
		case ERROR_LEVEL_ERROR:
			streamLog = stderr;
			break;
		case ERROR_LEVEL_ALL:
			streamLog = stderr;
			break;
	}

	if (verbosityLevel >= errorLevel)
	{
		fprintf(streamLog, "%s: %s %s\n", strTime, stringVerbosityLevel[errorLevel], error);
		return 1;
	}

	return 0;
}
