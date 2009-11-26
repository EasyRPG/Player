/*
 * log.h
 *
 *  Created on: 19-nov-2009
 *      Author: md
 */

#ifndef LOG_H_
#define LOG_H_

enum
{
	VERBOSITY_NONE = 0,
	VERBOSITY_INFO,
	VERBOSITY_WARNING,
	VERBOSITY_ERROR,
	VERBOSITY_ALL,
	VERBOSITY_LEVELS
};

enum
{
	ERROR_LEVEL_NONE = 0,
	ERROR_LEVEL_INFO,
	ERROR_LEVEL_WARNING,
	ERROR_LEVEL_ERROR,
	ERROR_LEVEL_ALL,
	ERROR_LEVEL_LEVELS
};

extern int verbosityLevel;

int setLog(int errorLevel);
int log(int errorLevel, const char *error);

#endif /* LOG_H_ */
