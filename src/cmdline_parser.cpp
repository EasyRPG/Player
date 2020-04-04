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

#include "cmdline_parser.h"
#include <cstring>

#include <iostream>

CmdlineParser::CmdlineParser(int* argc, char** argv) : _argc(argc), _argv(argv) {}

bool CmdlineParser::Next() {
	if (_idx < Limit()) {
		++_idx;
	}

	if (_idx < Limit()) {
		if (_consume > 0) {
			_argv[_idx] = _argv[_idx + _consume];
		}
		return true;
	}
	*_argc = Limit();
	_consume = 0;
	return false;
}

bool CmdlineParser::DoCheck(const char* longval, char shortval) {
	const char* arg = _argv[_idx];
	if (!std::strcmp(arg, longval)
			|| (shortval
				&& arg[0] == '-'
				&& arg[1] == shortval
				&& arg[2] == '\0'))
	{
		return true;
	}
	return false;
}

bool CmdlineParser::Check(const char* longval, char shortval) {
	auto rc = DoCheck(longval, shortval);
	if (rc) {
		--_idx;
		++_consume;
	}
	return rc;
}

const char* CmdlineParser::CheckValue(const char* longval, char shortval) {
	auto rc = DoCheck(longval, shortval);
	if (!rc || (_idx + 1) >= Limit()) {
		return nullptr;
	}
	--_idx;
	++_consume;
	Next();

	auto* value = _argv[_idx];
	--_idx;
	++_consume;

	return value;
}

