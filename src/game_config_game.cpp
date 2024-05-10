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

#include "game_config_game.h"
#include "cmdline_parser.h"
#include "directory_tree.h"
#include "filefinder.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include <cstring>

Game_ConfigGame Game_ConfigGame::Create(CmdlineParser& cp) {
	Game_ConfigGame cfg;

	auto cli_config = FileFinder::Game().OpenFile(EASYRPG_INI_NAME);
	if (cli_config) {
		cfg.LoadFromStream(cli_config);
	}

	cfg.LoadFromArgs(cp);

	cfg.engine = Player::EngineNone;
	if (!cfg.engine_str.Get().empty()) {
		std::string v = cfg.engine_str.Get();
		if (v == "rpg2k" || v == "2000") {
			cfg.engine = Player::EngineRpg2k;
		}
		else if (v == "rpg2kv150" || v == "2000v150") {
			cfg.engine = Player::EngineRpg2k | Player::EngineMajorUpdated;
		}
		else if (v == "rpg2ke" || v == "2000e") {
			cfg.engine = Player::EngineRpg2k | Player::EngineMajorUpdated | Player::EngineEnglish;
		}
		else if (v == "rpg2k3" || v == "2003") {
			cfg.engine = Player::EngineRpg2k3;
		}
		else if (v == "rpg2k3v105" || v == "2003v105") {
			cfg.engine = Player::EngineRpg2k3 | Player::EngineMajorUpdated;
		}
		else if (v == "rpg2k3e" || v == "2003e") {
			cfg.engine = Player::EngineRpg2k3 | Player::EngineMajorUpdated | Player::EngineEnglish;
		}
	}

	return cfg;
}

void Game_ConfigGame::LoadFromArgs(CmdlineParser& cp) {
	cp.Rewind();

	while (!cp.Done()) {
		CmdlineArg arg;
		long li_value = 0;

		if (cp.ParseNext(arg, 0, {"--new-game", "--no-new-game"})) {
			new_game.Set(arg.ArgIsOn());
			continue;
		}
		if (cp.ParseNext(arg, 1, "--engine")) {
			if (arg.NumValues() > 0) {
				const auto& v = arg.Value(0);
				engine_str.Set(v);
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-patch")) {
			patch_support.Set(false);
			patch_dynrpg.Lock(false);
			patch_maniac.Lock(false);
			patch_unlock_pics.Lock(false);
			patch_common_this_event.Lock(false);
			patch_key_patch.Lock(false);
			patch_rpg2k3_commands.Lock(false);
			patch_anti_lag_switch.Lock(0);
			patch_direct_menu.Lock(0);
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-easyrpg", "--no-patch-easyrpg"})) {
			patch_easyrpg.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-dynrpg", "--no-patch-dynrpg"})) {
			patch_dynrpg.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 1, {"--patch-maniac", "--no-patch-maniac"})) {
			patch_maniac.Set(arg.ArgIsOn());

			if (arg.ArgIsOn() && arg.ParseValue(0, li_value)) {
				patch_maniac.Set(li_value);
			}

			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-common-this", "--no-patch-common-this"})) {
			patch_common_this_event.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-pic-unlock", "--no-patch-pic-unlock"})) {
			patch_unlock_pics.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-key-patch", "--no-patch-key-patch"})) {
			patch_key_patch.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--patch-rpg2k3-cmds", "--patch-rpg2k3-commands", "--no-patch-rpg2k3-cmds", "--no-patch-rpg2k3-commands"})) {
			patch_rpg2k3_commands.Set(arg.ArgIsOn());
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 1, {"--patch-antilag-switch", "--no-patch-antilag-switch"})) {
			if (arg.ArgIsOn() && arg.ParseValue(0, li_value)) {
				patch_anti_lag_switch.Set(li_value);
				patch_override = true;
			}

			if (arg.ArgIsOff()) {
				patch_anti_lag_switch.Set(0);
				patch_override = true;
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, {"--patch-direct-menu", "--no-patch-direct-menu"})) {
			if (arg.ArgIsOn() && arg.ParseValue(0, li_value)) {
				patch_direct_menu.Set(li_value);
				patch_override = true;
			}

			if (arg.ArgIsOff()) {
				patch_direct_menu.Set(0);
				patch_override = true;
			}
			continue;
		}
		if (cp.ParseNext(arg, 6, "--patch")) {
			// For backwards compatibility only
			for (int i = 0; i < arg.NumValues(); ++i) {
				const auto& v = arg.Value(i);
				if (v == "dynrpg") {
					patch_dynrpg.Set(true);
				} else if (v == "maniac") {
					patch_maniac.Set(true);
				} else if (v == "common-this") {
					patch_common_this_event.Set(true);
				} else if (v == "pic-unlock") {
					patch_unlock_pics.Set(true);
				} else if (v == "key-patch") {
					patch_key_patch.Set(true);
				} else if (v == "rpg2k3-cmds" || v == "rpg2k3-commands") {
					patch_rpg2k3_commands.Set(true);
				}
			}
			patch_override = true;

			continue;
		}

		cp.SkipNext();
	}
}

void Game_ConfigGame::LoadFromStream(Filesystem_Stream::InputStream& is) {
	lcf::INIReader ini(is);

	if (ini.ParseError()) {
		Output::Debug("Failed to parse ini config file {}", is.GetName());
		return;
	}

	new_game.FromIni(ini);
	engine_str.FromIni(ini);
	fake_resolution.FromIni(ini);

	if (patch_easyrpg.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_dynrpg.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_maniac.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_common_this_event.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_unlock_pics.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_key_patch.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_rpg2k3_commands.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_anti_lag_switch.FromIni(ini)) {
		patch_override = true;
	}

	if (patch_direct_menu.FromIni(ini)) {
		patch_override = true;
	}
}

void Game_ConfigGame::PrintActivePatches() {
	std::vector<std::string> patches;

	auto add_bool = [&](auto& patch) {
		if (patch.Get()) {
			patches.push_back(ToString(patch.GetName()));
		}
	};

	add_bool(patch_easyrpg);
	add_bool(patch_dynrpg);
	add_bool(patch_common_this_event);
	add_bool(patch_unlock_pics);
	add_bool(patch_key_patch);
	add_bool(patch_rpg2k3_commands);

	auto add_int = [&](auto& patch) {
		if (patch.Get() > 0) {
			patches.push_back(fmt::format("{} ({})", patch.GetName(), patch.Get()));
		}
	};

	add_int(patch_maniac);
	add_int(patch_anti_lag_switch);
	add_int(patch_direct_menu);

	if (patches.empty()) {
		Output::Debug("Patch configuration: None");
	} else {
		std::string out = "Patch configuration: ";
		bool first = true;
		for (const auto& s: patches) {
			if (!first) {
				out += ", ";
			}
			out += s;
			first = false;
		}
		Output::DebugStr(out);
	}
}
