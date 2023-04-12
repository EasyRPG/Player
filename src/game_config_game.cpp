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
		if (cp.ParseNext(arg, 0, "--new-game")) {
			new_game.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-new-game")) {
			new_game.Set(false);
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
			patch_override = true;
			continue;
		}
		if (cp.ParseNext(arg, 6, "--patch")) {
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
}
