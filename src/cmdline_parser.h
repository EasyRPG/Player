/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_CMDLINE_PARSER_H
#define EP_CMDLINE_PARSER_H
#include <cstring>

/** Commandline parser class which is used to search through command line arguments. 
 * Each matched commandline argument is removed from argc/argv.
 *
 * Usage:
 * CmdLineParser p(&argc, argv);
 * while (p.Next()) {
 *     if (p.Check("--foo", "f)) {
 *         // Handle foo
 *         continue;
 *     }
 * }
 */
class CmdlineParser {
	public:
		CmdlineParser(int* argc, char** argv);

		/* Iterate to the next argument */
		bool Next();

		bool Check(const char* longval, char shortval = '\0');

		const char* CheckValue(const char* longval, char shortval = '\0');

	private:
		int Limit() const { return *_argc - _consume; }

		bool DoCheck(const char* longval, char shortval);

		int* _argc = nullptr;
		char** _argv = nullptr;
		int _consume = 0;
		int _idx = -1;
};

#endif
