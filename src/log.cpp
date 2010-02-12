#include "log.h"
#include <cstdio>
#include <ctime>
#include <cstring>

#ifdef WIN32
#include <windows.h>
SDL_SysWMinfo wmInfo;
SDL_VERSION(&wmInfo.version);
SDL_GetWMInfo(&wmInfo);
HWND hWnd = wmInfo.window;
#endif

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#endif

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

#ifndef WIN32
	FILE* streamLog = NULL;
	
	switch (errorLevel)
	{
		case ERROR_LEVEL_INFO:
			streamLog = stdout;
			break;
		case ERROR_LEVEL_NONE:
		case ERROR_LEVEL_WARNING:
		case ERROR_LEVEL_ERROR:
		case ERROR_LEVEL_ALL:
			streamLog = stderr;
			break;
	}
#endif

	if (verbosityLevel >= errorLevel)
	{
#if !defined WIN32 && !defined __APPLE__
		fprintf(streamLog, "%s: %s %s\n", strTime, stringVerbosityLevel[errorLevel], error);
#endif
#ifdef WIN32
		MessageBox(hWnd, error, "Error", MB_ICONERROR);
#endif
#ifdef __APPLE__
		//Objective-Crap(TM) needed for Cocoa's NSAlert. Yeah, 64-bit OSX doesn't have Carbon anymore.
		//TODO: totally untested yet
		NSString *nsTitle = [NSString stringWithUTF8String:"Error"];
		NSString *nsMessage = [NSString stringWithUTF8String:error];
		NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
			defaultButton:@"OK"
			alternateButton:nil
			otherButton:nil
			informativeTextWithFormat:nsMessage];
		[alert runModal];
#endif
		return 1;
	}

	return 0;
}
