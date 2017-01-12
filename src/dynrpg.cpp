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

// Headers
#include "dynrpg.h"
#include "filefinder.h"
#include "game_actors.h"
#include "game_variables.h"
#include "main_data.h"
#include "output.h"
#include "player.h"

#include <cstring>
#include <fstream>
#include <map>

#include "dynrpg_easyrpg.h"
#include "dynrpg_textplugin.h"

enum DynRpg_ParseMode {
	ParseMode_Function,
	ParseMode_WaitForComma,
	ParseMode_WaitForArg,
	ParseMode_String,
	ParseMode_Token
};

typedef std::map<std::string, dynfunc> dyn_rpg_func;

namespace {
	bool init = false;

	// Registered DynRpg Plugins
	std::vector<std::unique_ptr<DynRpgPlugin>> plugins;

	// DynRpg Function table
	dyn_rpg_func dyn_rpg_functions;
}

void DynRpg::RegisterFunction(const std::string& name, dynfunc func) {
	dyn_rpg_functions[name] = func;
}

bool DynRpg::HasFunction(const std::string& name) {
	return dyn_rpg_functions.find(name) != dyn_rpg_functions.end();
}

// Var arg referenced by $n
std::string DynRpg::ParseVarArg(StringView func_name, dyn_arg_list args, int index, bool& parse_okay) {
	parse_okay = true;
	if (index >= static_cast<int>(args.size())) {
		parse_okay = false;
		Output::Warning("{}: Vararg {} out of range", func_name, index);
		return "";
	}

	std::string::iterator text_index, end;
	std::string text = args[index];
	text_index = text.begin();
	end = text.end();

	std::stringstream msg;

	for (; text_index != end; ++text_index) {
		char chr = *text_index;

		// Test for "" -> append "
		// otherwise end of string
		if (chr == '$' && std::distance(text_index, end) > 1) {
			char n = *std::next(text_index, 1);

			if (n == '$') {
				// $$ = $
				msg << n;
				++text_index;
			} else if (n >= '1' && n <= '9') {
				int i = (int)(n - '0');

				if (i + index < static_cast<int>(args.size())) {
					msg << args[i + index];
				}
				else {
					// $-ref out of range
					parse_okay = false;
					Output::Warning("{}: Vararg $-ref {} out of range", func_name, i);
					return "";
				}

				++text_index;
			} else {
				msg << chr;
			}
		} else {
			msg << chr;
		}
	}

	return msg.str();
}


static std::string ParseToken(const std::string& token, const std::string& function_name) {
	std::string::iterator text_index, end;
	std::string text = token;
	text_index = text.begin();
	end = text.end();

	char chr = *text_index;

	bool first = true;

	bool number_encountered = false;

	std::stringstream var_part;
	std::stringstream number_part;

	for (;;) {
		if (text_index != end) {
			chr = *text_index;
		}

		if (text_index == end) {
			// Variable reference
			std::string tmp = number_part.str();
			int number = atoi(tmp.c_str());
			tmp = var_part.str();
			if (tmp.empty()) {
				return token;
			}

			// Convert backwards
			for (std::string::reverse_iterator it = tmp.rbegin(); it != tmp.rend(); ++it) {
				if (*it == 'N') {
					if (!Main_Data::game_actors->ActorExists(number)) {
						Output::Warning("{}: Invalid actor id {} in {}", function_name, number, token);
						return "";
					}

					// N is last
					return ToString(Main_Data::game_actors->GetActor(number)->GetName());
				} else {
					// Variable
					number = Main_Data::game_variables->Get(number);
				}
			}

			number_part.str("");
			number_part << number;
			return number_part.str();
		} else if (number_encountered || (chr >= '0' && chr <= '9')) {
			number_encountered = true;
			number_part << chr;
		} else if (chr == 'N') {
			if (!first) {
				break;
			}
			var_part << chr;
		} else if (chr == 'V') {
			var_part << chr;
		} else {
			break;
		}

		++text_index;
		first = false;
	}

	// Normal token
	return Utils::LowerCase(token);
}

void create_all_plugins() {
	plugins.emplace_back(new DynRpg::EasyRpgPlugin());
	plugins.emplace_back(new DynRpg::TextPlugin());
	
	for (auto& plugin : plugins) {
		plugin->RegisterFunctions();
	}

	init = true;
}

std::string DynRpg::ParseCommand(const std::string& command, std::vector<std::string>& args) {
	if (command.empty()) {
		// Not a DynRPG function (empty comment)
		return "";
	}

	std::string::iterator text_index, end;
	std::string text = command;
	text_index = text.begin();
	end = text.end();

	char chr = *text_index;

	if (chr != '@') {
		// Not a DynRPG function, normal comment
		return "";
	}

	DynRpg_ParseMode mode = ParseMode_Function;
	std::string function_name;
	std::string tmp;
	std::stringstream token;

	++text_index;

	// Parameters can be of type Token, Number or String
	// Strings are in "", a "-literal is represented by ""
	// Number is a valid float number
	// Tokens are Strings without "" and with Whitespace stripped o_O
	// If a token is (regex) N?V+[0-9]+ it is resolved to a var or an actor

	// All arguments are passed as string to the DynRpg functions and are
	// converted to int or float on demand.

	for (;;) {
		if (text_index != end) {
			chr = *text_index;
		}

		if (text_index == end) {
			switch (mode) {
				case ParseMode_Function:
					// End of function token
					function_name = Utils::LowerCase(token.str());
					if (function_name.empty()) {
						// empty function name
						Output::Warning("Empty DynRPG function name");
						return "";
					}
					break;
				case ParseMode_WaitForComma:
					// no-op
					break;
				case ParseMode_WaitForArg:
					if (!args.empty()) {
						// Found , but no token -> empty arg
						args.emplace_back("");
					}
					break;
				case ParseMode_String:
					// Unterminated literal, handled like a terminated literal
					args.emplace_back(token.str());
					break;
				case ParseMode_Token:
					tmp = ParseToken(token.str(), function_name);
					args.emplace_back(tmp);
					break;
			}

			break;
		} else if (chr == ' ') {
			switch (mode) {
				case ParseMode_Function:
					// End of function token
					function_name = Utils::LowerCase(token.str());
					if (function_name.empty()) {
						// empty function name
						Output::Warning("Empty DynRPG function name");
						return "";
					}
					token.str("");

					mode = ParseMode_WaitForArg;
					break;
				case ParseMode_WaitForComma:
				case ParseMode_WaitForArg:
					// no-op
					break;
				case ParseMode_String:
					token << chr;
					break;
				case ParseMode_Token:
					// Skip whitespace
					break;
			}
		} else if (chr == ',') {
			switch (mode) {
				case ParseMode_Function:
					// End of function token
					function_name = Utils::LowerCase(token.str());
					if (function_name.empty()) {
						// empty function name
						Output::Warning("Empty DynRPG function name");
						return "";
					}
					token.str("");
					// Empty arg
					args.emplace_back("");
					mode = ParseMode_WaitForArg;
					break;
				case ParseMode_WaitForComma:
					mode = ParseMode_WaitForArg;
					break;
				case ParseMode_WaitForArg:
					// Empty arg
					args.emplace_back("");
					break;
				case ParseMode_String:
					token << chr;
					break;
				case ParseMode_Token:
					tmp = ParseToken(token.str(), function_name);
					args.emplace_back(tmp);
					// already on a comma
					mode = ParseMode_WaitForArg;
					token.str("");
					break;
			}
		} else {
			// Anything else that isn't special purpose
			switch (mode) {
				case ParseMode_Function:
					token << chr;
					break;
				case ParseMode_WaitForComma:
					Output::Warning("{}: Expected \",\", got token", function_name);
					return "";
				case ParseMode_WaitForArg:
					if (chr == '"') {
						mode = ParseMode_String;
						// begin of string
					}
					else {
						mode = ParseMode_Token;
						token << chr;
					}
					break;
				case ParseMode_String:
					if (chr == '"') {
						// Test for "" -> append "
						// otherwise end of string
						if (std::distance(text_index, end) > 1 && *std::next(text_index, 1) == '"') {
							token << '"';
							++text_index;
						}
						else {
							// End of string
							args.emplace_back(token.str());

							mode = ParseMode_WaitForComma;
							token.str("");
						}
					}
					else {
						token << chr;
					}
					break;
				case ParseMode_Token:
					token << chr;
					break;
			}
		}

		++text_index;
	}

	return function_name;
}

bool DynRpg::Invoke(const std::string& command) {
	if (!init) {
		create_all_plugins();
	}

	std::vector<std::string> args;
	std::string function_name = ParseCommand(command, args);

	if (function_name.empty()) {
		return true;
	}

	return Invoke(function_name, args);
}

bool DynRpg::Invoke(const std::string& func, dyn_arg_list args) {
	if (!init) {
		create_all_plugins();
	}

	if (!DynRpg::HasFunction(func)) {
		// Not a supported function
		Output::Warning("Unsupported DynRPG function: {}", func);
		return true;
	}

	return dyn_rpg_functions[func](args);
}

std::string get_filename(int slot) {
	auto fs = FileFinder::Save();

	std::string filename = std::string("Save") + (slot <= 9 ? "0" : "") + std::to_string(slot) + ".dyn";

	std::string found = fs.FindFile(filename);

	if (found.empty()) {
		found = filename;
	}

	return found;
}

void DynRpg::Load(int slot) {
	if (!Player::IsPatchDynRpg()) {
		return;
	}

	if (!init) {
		create_all_plugins();
	}

	std::string filename = get_filename(slot);

	auto in = FileFinder::Save().OpenInputStream(filename);

	if (!in) {
		Output::Warning("Couldn't read DynRPG save: {}", filename);
	}

	std::vector<uint8_t> in_buffer;
	in_buffer.resize(8);

	in.read((char*)in_buffer.data(), 8);

	if (strncmp((char*)in_buffer.data(), "DYNSAVE1", 8) != 0) {
		Output::Warning("Corrupted DynRPG save: {}", filename);
		return;
	}

	while (!(in.eof() || in.fail())) {
		// Read header length followed by header (Plugin Identifier)

		uint32_t len;
		in.read((char *) &len, 4);
		Utils::SwapByteOrder(len);

		in_buffer.resize(len);
		in.read((char*)in_buffer.data(), len);

		// Find a plugin that feels responsible
		bool have_one = false;

		for (auto &plugin : plugins) {
			if (strncmp((char*)in_buffer.data(), plugin->GetIdentifier().c_str(), len) == 0) {
				// Chunk length
				in.read((char *) &len, 4);
				Utils::SwapByteOrder(len);

				if (len > 0) {
					// Read chunk
					in_buffer.resize(len);
					in.read((char*)in_buffer.data(), len);

					plugin->Load(in_buffer);
				}

				have_one = true;
				break;
			}
		}

		if (!have_one) {
			// Skip this chunk, no plugin found
			in.read((char *) &len, 4);
			Utils::SwapByteOrder(len);

			in.seekg(len, std::ios::cur);
		}
	}
}

void DynRpg::Save(int slot) {
	if (!Player::IsPatchDynRpg()) {
		return;
	}

	if (!init) {
		create_all_plugins();
	}

	std::string filename = get_filename(slot);

	auto out = FileFinder::Save().OpenOutputStream(filename);

	if (!out) {
		Output::Warning("Couldn't write DynRPG save: {}", filename);
		return;
	}

	std::string header = "DYNSAVE1";

	out.write(header.c_str(), 8);

	for (auto &plugin : plugins) {
		uint32_t len = plugin->GetIdentifier().size();
		Utils::SwapByteOrder(len);

		out.write((char*)&len, 4);
		out.write(plugin->GetIdentifier().c_str(), len);

		std::vector<uint8_t> data = plugin->Save();
		len = data.size();
		Utils::SwapByteOrder(len);

		out.write((char*)&len, 4);
		out.write((char*)data.data(), data.size());
	}
}

void DynRpg::Update() {
	for (auto& plugin : plugins) {
		plugin->Update();
	}
}

void DynRpg::Reset() {
	init = false;
	dyn_rpg_functions.clear();
	plugins.clear();
}
