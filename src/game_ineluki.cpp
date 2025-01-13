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
#include "async_handler.h"
#include "filefinder.h"
#include "utils.h"
#include "output.h"
#include "audio.h"
#include "input.h"
#include "player.h"
#include "system.h"

#include <lcf/inireader.h>

namespace {
#if defined(SUPPORT_KEYBOARD)
	void mask_kb(bool mask) {
		constexpr std::array<Input::Keys::InputKey, 4> keys = {
				Input::Keys::W,
				Input::Keys::S,
				Input::Keys::A,
				Input::Keys::D
		};

		auto keymask = Input::GetMask();
		for (auto k : keys) {
			keymask[k] = mask;
		}
		Input::SetMask(keymask);
	}
#endif
}

Game_Ineluki::~Game_Ineluki() {
#if defined(SUPPORT_KEYBOARD)
	if (key_support) {
		mask_kb(false);
	}
#endif
}

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

bool Game_Ineluki::Execute(StringView ini_file) {
	auto ini_file_s = ToString(ini_file);

	if (functions.find(ini_file_s) == functions.end()) {
		if (!Parse(ini_file)) {
			return false;
		}
	}

	for (const auto& cmd : functions[ini_file_s]) {
		//Output::Debug("Ineluki {} {}", cmd.name, cmd.arg);

		if (cmd.name == "writetolog") {
			Output::InfoStr(cmd.arg);
		} else if (cmd.name == "execprogram") {
			// Fake execute some known programs
			if (StringView(cmd.arg).starts_with("exitgame") ||
					StringView(cmd.arg).starts_with("taskkill")) {
				Player::exit_flag = true;
			} else if (StringView(cmd.arg).starts_with("SaveCount.dat")) {
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
			bool prev_key_support = key_support;
			key_support = Utils::LowerCase(cmd.arg) == "true";

#if !defined(SUPPORT_KEYBOARD)
			if (key_support) {
				Output::Warning("Ineluki: Keyboard input is not supported on this platform");
			}
#else
			if (prev_key_support != key_support) {
				Output::Debug("Ineluki: Key support is now {}", key_support ? "Enabled" : "Disabled");
			}

			mask_kb(key_support);
#endif
		} else if (cmd.name == "registerkeydownevent") {
			std::string arg_lower = Utils::LowerCase(cmd.arg);
			auto it = std::find_if(key_to_ineluki.begin(), key_to_ineluki.end(), [&](const auto& k) {
				return !strcmp(arg_lower.c_str(), k.name);
			});
			if (it != key_to_ineluki.end()) {
				keylist_down.push_back({it->key, atoi(cmd.arg2.c_str())});
			}
		} else if (cmd.name == "registerkeyupevent") {
			std::string arg_lower = Utils::LowerCase(cmd.arg);
			auto it = std::find_if(key_to_ineluki.begin(), key_to_ineluki.end(), [&](const auto& k) {
				return !strcmp(arg_lower.c_str(), k.name);
			});
			if (it != key_to_ineluki.end()) {
				keylist_up.push_back({it->key, atoi(cmd.arg2.c_str())});
			}
		} else if (cmd.name == "enablemousesupport") {
			bool prev_mouse_support = mouse_support;
			mouse_support = Utils::LowerCase(cmd.arg) == "true";
			mouse_id_prefix = atoi(cmd.arg2.c_str());
			// TODO: automatic (append mouse pos every 500ms) not implemented
#if !defined(USE_MOUSE_OR_TOUCH) || !defined(SUPPORT_MOUSE_OR_TOUCH)
			if (mouse_support) {
				Output::Warning("Ineluki: Mouse input is not supported on this platform");
			}
#endif
			if (prev_mouse_support != mouse_support) {
				Output::Debug("Ineluki: Mouse support is now {}", mouse_support ? "Enabled" : "Disabled");
			}
		} else if (cmd.name == "getmouseposition") {
			if (!mouse_support) {
				return true;
			}

			Point mouse_pos = Input::GetMousePosition();

			bool left = Input::IsPressed(Input::MOUSE_LEFT);
			bool right = Input::IsPressed(Input::MOUSE_RIGHT);
			int key = left && right ? 3 : right ? 2 : left ? 1 : 0;

			output_list.push_back(key);
			output_list.push_back(mouse_pos.y);
			output_list.push_back(mouse_pos.x);
			output_list.push_back(mouse_id_prefix);
		} else if (cmd.name == "setdebuglevel") {
			// no-op
		} else if (cmd.name == "registercheatevent") {
			cheatlist.emplace_back(Utils::LowerCase(cmd.arg), atoi(cmd.arg2.c_str()));
		}
	}

	return true;
}

bool Game_Ineluki::ExecuteScriptList(StringView list_file) {
	auto is = FileFinder::Game().OpenInputStream(ToString(list_file));
	assert(async_scripts.empty());

	if (!is) {
		return false;
	}

	Output::Debug("Ineluki: Processing script list {}", FileFinder::GetPathInsideGamePath(ToString(list_file)));

	std::string line;
	std::vector<FileRequestAsync*> requests;
	while (Utils::ReadLine(is, line)) {
		if (!line.empty()) {
			FileRequestAsync* request = AsyncHandler::RequestFile(line);
			auto binding = request->Bind(&Game_Ineluki::OnScriptFileReady, this);
			async_scripts.emplace_back(binding, line);
			requests.push_back(request);
		}
	}

	for (auto& r: requests) {
		r->SetImportantFile(true);
		r->Start();
	}

	return true;
}

bool Game_Ineluki::Parse(StringView ini_file) {
	auto ini_file_s = ToString(ini_file);

	auto is = FileFinder::Game().OpenInputStream(ini_file_s);
	if (!is) {
		return false;
	}

	lcf::INIReader ini(is);
	if (ini.ParseError() == -1) {
		return false;
	}

	Output::Debug("Ineluki: Parsing script {}", FileFinder::GetPathInsideGamePath(ini_file_s));

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
			Output::Debug("Ineluki: Unknown command {}", cmd.name);
			valid = false;
		}

		if (valid) {
			commands.push_back(cmd);
		}

		section = ini.Get(section, "next", std::string());
	} while (!section.empty());

	functions[ini_file_s] = commands;

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
			output_list.push_back(key.value);
		}
	}

	for (const auto& key : keylist_up) {
		if (Input::IsRawKeyReleased(key.key)) {
			output_list.push_back(key.value);
		}
	}

	for (auto& cheat: cheatlist) {
		if (cheat.keys.empty()) {
			continue;
		}

		if (Input::IsRawKeyPressed(cheat.keys[cheat.index])) {
			++cheat.index;
			if (cheat.index >= static_cast<int>(cheat.keys.size())) {
				output_list.push_back(cheat.value);
				cheat.index = 0;
			}
		} else if (cheat.index > 0) {
			auto pressed = Input::GetAllRawPressed();
			// Don't reset when the previous cheat key is (still) pressed
			pressed[cheat.keys[cheat.index - 1]] = false;
			if (pressed.any()) {
				cheat.index = 0;
			}
		}
	}
}

void Game_Ineluki::OnScriptFileReady(FileRequestResult* result) {
	auto it = std::find_if(async_scripts.begin(), async_scripts.end(), [&](const auto& a) {
		return a.script_name == result->file;
	});
	assert(it != async_scripts.end());
	it->invoked = true;

	if (std::all_of(async_scripts.begin(), async_scripts.end(), [](const auto& a) {
		return a.invoked;
	})) {
		std::for_each(async_scripts.begin(), async_scripts.end(), [this](const auto& a) {
			Execute(FileFinder::Game().FindFile(a.script_name));
		});
		async_scripts.clear();
	}
}

Game_Ineluki::CheatItem::CheatItem(const std::string& code, int value) : value(value) {
	for (char c: code) {
		auto it = std::find_if(key_to_ineluki.begin(), key_to_ineluki.end(), [&](const auto& k) {
			return c == k.name[0];
		});
		if (it != key_to_ineluki.end()) {
			keys.push_back(it->key);
		}
	}
}
