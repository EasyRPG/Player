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
#include "utils.h"
#include "output.h"
#include <cstring>

#include <iostream>
#include <sstream>

bool CmdlineArg::ParseValue(int i, std::string& value) const {
	if (i >= NumValues()) {
		return false;
	}
	value = Value(i);
	return true;
}

bool CmdlineArg::ParseValue(int i, long& value) const {
	if (i >= NumValues()) {
		return false;
	}
	auto& s = Value(i);

	auto* p = s.c_str();
	auto* e = p + s.size();
	long v = strtol(p, const_cast<char**>(&e), 10);
	if (p == e) {
		return false;
	}
	value = v;
	return true;
}

CmdlineParser::CmdlineParser(std::vector<std::string> arguments)
{
	size_t argc = arguments.size();
	if (argc > 0) {
		args.reserve(argc - 1);
		copy(arguments.begin() + 1, arguments.end(), back_inserter(args));
	}
}

template <typename F>
bool CmdlineParser::DoParseNext(CmdlineArg& arg, int max_values, F&& is_longval, char shortval) {
	const auto nargs = static_cast<int>(args.size());

	if (index >= nargs) {
		return false;
	}

	if (!(is_longval(args[index])
			|| (shortval
				&& args[index][0] == '-'
				&& args[index][1] == shortval
				&& args[index][2] == '\0')))
	{
		return false;
	}

	auto* ptr = args.data() + index;
	++index;

	int nv = 0;
	while (nv < max_values) {
		if (index >= nargs) {
			break;
		}

		auto& next = args[index];
		if (!next.empty() && next.front() == '-') {
			break;
		}

		++nv;
		++index;
	}

	arg = CmdlineArg(ptr, nv);

	return true;
}

bool CmdlineParser::ParseNext(CmdlineArg& arg, int max_values, const char* longval, char shortval) {
	return DoParseNext(arg, max_values,
			[=](auto& s) { return Utils::StrICmp(longval, s.c_str()) == 0; },
			shortval);
}

bool CmdlineParser::ParseNext(CmdlineArg& arg, int max_values, std::initializer_list<const char*> longvals, char shortval) {
	return DoParseNext(arg, max_values,
			[=](auto& s) {
				for (auto& lv: longvals) {
					if (Utils::StrICmp(lv, s.c_str()) == 0) {
						return true;
					}
				}
				return false;
			},
			shortval);
}

void CmdlineParser::SkipNext() {
	assert(index < static_cast<int>(args.size()));
	++index;
}

void CmdlineParser::RewindBy(int rewind) {
	assert(rewind >= 0);
	index -= rewind;
	assert(index >= 0);
}
