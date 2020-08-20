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
#include "game_ineluki.h"
#include "filefinder.h"
#include "utils.h"
#include "output.h"
#include "audio.h"
#include "input.h"
#include "player.h"

#include <lcf/inireader.h>

constexpr std::array<Game_Ineluki::Mapping, 61> Game_Ineluki::key_to_ineluki;

bool Game_Ineluki::Execute(const lcf::rpg::Sound& se) {
	if (Utils::LowerCase(se.name) == "saves.script") {
		// Support for the script written by SaveCount.dat
		// It counts the amount of savegames and outputs the result
		output_mode = OutputMode::Output;
		output_list.push_back(FileFinder::GetSavegames());
		return true;
	}

	std::string ini_file = FileFinder::FindSound(se.name);
	if (!ini_file.empty()) {
		return Execute(ini_file);
	} else {
		Output::Debug("Ineluki: Script {} not found", se.name);
	}
	return false;
}

bool Game_Ineluki::Execute(const std::string& ini_file) {
	if (functions.find(ini_file) == functions.end()) {
		if (!Parse(ini_file)) {
			return false;
		}
	}

	for (const auto& cmd : functions[ini_file]) {
		//Output::Debug("Ineluki {} {}", cmd.name, cmd.arg);

		if (cmd.name == "writetolog") {
			Output::InfoStr(cmd.arg);
		} else if (cmd.name == "execprogram") {
			// Fake execute some known programs
			if (Utils::StartsWith(cmd.arg, "exitgame") ||
					Utils::StartsWith(cmd.arg, "taskkill")) {
				Player::exit_flag = true;
			} else if (Utils::StartsWith(cmd.arg, "SaveCount.dat")) {
				// no-op, detected through saves.script access
			} else {
				Output::Warning("Ineluki ExecProgram {}: Not supported", cmd.arg);
			}
		} else if (cmd.name == "mcicommand") {
			Output::Warning("Ineluki MciProgram {}: Not supported", cmd.arg);
		} else if (cmd.name == "miditickfunction") {
			std::string arg = Utils::LowerCase(cmd.arg);
			if (arg == "original") {
				output_mode = OutputMode::Original;
			} else if (arg == "output") {
				output_mode = OutputMode::Output;
			} else if (arg == "clear") {
				output_list.clear();
			}
		} else if (cmd.name == "addoutput") {
			output_list.push_back(atoi(cmd.arg.c_str()));
		} else if (cmd.name == "enablekeysupport") {
			key_support = Utils::LowerCase(cmd.arg) == "true";
		} else if (cmd.name == "registerkeydownevent") {
			std::string arg_lower = Utils::LowerCase(cmd.arg);
			for (auto& i : key_to_ineluki) {
				if (!strcmp(arg_lower.c_str(), i.name)) {
					keylist_down.push_back({i.key, atoi(cmd.arg2.c_str())});
					break;
				}
			}
		} else if (cmd.name == "registerkeyupevent") {
			std::string arg_lower = Utils::LowerCase(cmd.arg);
			for (auto& i : key_to_ineluki) {
				if (!strcmp(cmd.arg.c_str(), i.name)) {
					keylist_up.push_back({i.key, atoi(cmd.arg2.c_str())});
					break;
				}
			}
		} else if (cmd.name == "enablemousesupport") {
			mouse_support = Utils::LowerCase(cmd.arg) == "true";
			mouse_id_prefix = atoi(cmd.arg2.c_str());
			// TODO: automatic (append mouse pos every 500ms) not implemented
		} else if (cmd.name == "getmouseposition") {
			if (!mouse_support) {
				return true;
			}

			int mouse_x;
			int mouse_y;

			Input::GetMousePosition(mouse_x, mouse_y);

			bool left = Input::IsRawKeyPressed(Input::Keys::MOUSE_LEFT);
			bool right = Input::IsRawKeyPressed(Input::Keys::MOUSE_RIGHT);

			int key = left && right ? 3 : right ? 2 : left ? 1 : 0;

			output_list.push_back(key);
			output_list.push_back(mouse_y);
			output_list.push_back(mouse_x);
			output_list.push_back(mouse_id_prefix);
		} else if (cmd.name == "setdebuglevel") {
			// no-op
		} else if (cmd.name == "registercheatevent") {
			// TODO
		}
	}

	return true;
}

bool Game_Ineluki::ExecuteAutorunScript() {
	auto is = FileFinder::OpenInputStream(FileFinder::FindDefault("autorun.script"));

	if (!is) {
		return false;
	}

	Output::Debug("Ineluki: Processing autostart script");

	std::string line = Utils::ReadLine(is);
	while (!is.eof()) {
		if (!line.empty()) {
			Execute(FileFinder::FindDefault(line));
		}
		line = Utils::ReadLine(is);
	}

	return true;
}

bool Game_Ineluki::Parse(const std::string& ini_file) {
	auto is = FileFinder::OpenInputStream(ini_file);
	lcf::INIReader ini(is);
	if (ini.ParseError() == -1) {
		return false;
	}

	Output::Debug("Ineluki: Parsing script {}", FileFinder::GetPathInsideGamePath(ini_file));

	command_list commands;
	std::string section = "execute";

	do {
		InelukiCommand cmd;
		cmd.name = Utils::LowerCase(ini.Get(section, "action", std::string()));
		bool valid = true;

		if (cmd.name == "writetolog") {
			cmd.arg = ini.Get(section, "text", std::string());
		} else if (cmd.name == "execprogram") {
			cmd.arg = ini.Get(section, "command", std::string());
		} else if (cmd.name == "mcicommand") {
			cmd.arg = ini.Get(section, "command", std::string());
		} else if (cmd.name == "miditickfunction") {
			cmd.arg = ini.Get(section, "command", std::string());
			if (cmd.arg.empty()) {
				cmd.arg = ini.Get(section, "value", std::string());
			}
		} else if (cmd.name == "addoutput") {
			cmd.arg = ini.Get(section, "value", std::string());
		} else if (cmd.name == "enablekeysupport") {
			cmd.arg = ini.Get(section, "enable", std::string());
		} else if (cmd.name == "registerkeydownevent") {
			cmd.arg = ini.Get(section, "key", std::string());
			cmd.arg2 = ini.Get(section, "value", std::string());
		} else if (cmd.name == "registerkeyupevent") {
			cmd.arg = ini.Get(section, "key", std::string());
			cmd.arg2 = ini.Get(section, "value", std::string());
		} else if (cmd.name == "enablemousesupport") {
			cmd.arg = ini.Get(section, "enable", std::string());
			cmd.arg2 = ini.Get(section, "id", std::string());
			cmd.arg3 = ini.Get(section, "automatic", std::string());
		} else if (cmd.name == "getmouseposition") {
			// no args
		} else if (cmd.name == "setdebuglevel") {
			cmd.arg = ini.Get(section, "level", std::string());
		} else if (cmd.name == "registercheatevent") {
			cmd.arg = ini.Get(section, "cheat", std::string());
			cmd.arg2 = ini.Get(section, "value", std::string());
		} else {
			valid = false;
		}

		if (valid) {
			commands.push_back(cmd);
		}

		section = ini.Get(section, "next", std::string());
	} while (!section.empty());

	functions[ini_file] = commands;

	return true;
}

int Game_Ineluki::GetMidiTicks() {
	if (output_mode == OutputMode::Original) {
		return Audio().BGM_GetTicks();
	} else {
		int val = -1;
		if (!output_list.empty()) {
			val = output_list.back();
			output_list.pop_back();
		}
		return val;
	}
}

void Game_Ineluki::Update() {
	if (!key_support) {
		return;
	}

	for (const auto& key : keylist_down) {
		if (Input::IsRawKeyTriggered(key.key)) {
			//Output::Debug("Key Down: {}", key.key, key.value);
			output_list.push_back(key.value);
		}
	}

	for (const auto& key : keylist_up) {
		if (Input::IsRawKeyReleased(key.key)) {
			//Output::Debug("Key Up: {}", key.key, key.value);
			output_list.push_back(key.value);
		}
	}
}
