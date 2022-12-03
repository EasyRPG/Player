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
#include <vector>
#include <string>
#include <cassert>
#include <initializer_list>

/** class which represents a single command line argument and it's values */
class CmdlineArg {
public:
	/** Default construct to an invalid argument */
	CmdlineArg() = default;

	/** Construct pointing to the given argument */
	CmdlineArg(const std::string* p, int nv)
		: ptr(p), num_values(nv) {}

	/** @return true if this refers to a valid argument */
	bool IsValid() const {
		return ptr != nullptr;
	}

	/**
	 * @return the command line argument string
	 * @pre IsValid() or the result is undefined.
	 */
	const std::string& Arg() const {
		assert(IsValid());
		return ptr[0];
	}

	/** @return number of values this argument has */
	int NumValues() const {
		return num_values;
	}

	/**
	 * Gets an argument value
	 *
	 * @param i the index to the value
	 * @pre i < NumValues() or the result is undefined
	 * @return the value as a string
	 */
	const std::string& Value(int i) const {
		assert(i >= 0 && i < num_values);
		return ptr[i + 1];
	}

	/**
	 * Gets an argument value to an integer
	 *
	 * @param i the index to the value.
	 * @param value the value to write to. If the function returns true, this parameter is written, if returns false, this parameter is not touched.
	 * @return true if i < NumValues() and the string value was able to be parsed to an integer.
	 */
	bool ParseValue(int i, long& value) const;

	/**
	 * Gets an argument value to a string
	 *
	 * @param i the index to the value.
	 * @param value the value to write to. If the function returns true, this parameter is written, if returns false, this parameter is not touched.
	 * @return true if i < NumValues()
	 */
	bool ParseValue(int i, std::string& value) const;
private:
	const std::string* ptr = nullptr;
	int num_values = 0;
};

/** Commandline parser class which is used to search through command line arguments.  */
class CmdlineParser {
public:
	/** 
	 * Construct with given arguments
	 *
	 * @param arguments main() argv as vector of strings
	 */
	CmdlineParser(std::vector<std::string> arguments);

	/**
	 * Parses the next command line argument if there is a match in longval or shortval.
	 * @param arg CommandlineArg to fill in with details about argument that was parsed.
	 * @param max_values The maximum number of values of parse if they exist. The number of values in arg will be <= this value.
	 * @param longval The long argument name such as "--help"
	 * @param shortval The short argument character. 'h' for example would mean check for "-h"
	 * @return true if the argument was parsed. If returning true, the CmdlineParser advances to the next argument.
	 */
	bool ParseNext(CmdlineArg& arg, int max_values, const char* longval, char shortval = '\0');

	/**
	 * Parses the next command line argument if there is a match in longval or shortval.
	 * @param arg CommandlineArg to fill in with details about argument that was parsed.
	 * @param max_values The maximum number of values of parse if they exist. The number of values in arg will be <= this value.
	 * @param longvals A set of long argument name such as { "--help", "/?" }.
	 * @param shortval The short argument character. 'h' for example would mean check for "-h"
	 * @return true if the argument was parsed. If returning true, the CmdlineParser advances to the next argument.
	 */
	bool ParseNext(CmdlineArg& arg, int max_values, std::initializer_list<const char*> longvals, char shortval = '\0');

	/**
	 * Skips the current argument.
	 * @post advances to next command line argument
	 */
	void SkipNext();

	/** @return true if no more command line arguments */
	bool Done() const;

	/** Resets the parser back to the first argument */
	void Rewind();

	/**
	 * Rewinds the parser by the specified amount
	 * @param rewind how far to rewind
	 */
	void RewindBy(int rewind);
private:
	template <typename F>
		bool DoParseNext(CmdlineArg& arg, int max_values, F&& is_longval, char shortval);

	std::vector<std::string> args;
	int index = 0;
};

inline bool CmdlineParser::Done() const {
	return index >= static_cast<int>(args.size());
}

inline void CmdlineParser::Rewind() {
	index = 0;
}

#endif
