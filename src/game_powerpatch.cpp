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
#include "game_powerpatch.h"
#include "filefinder.h"
#include "utils.h"
#include "output.h"
#include "player.h"
#include "system.h"
#include "scene_load.h"
#include "scene_save.h"
#include "scene_debug.h"
#include "main_data.h"
#include "game_interpreter_map.h"
#include "game_map.h"
#include "game_switches.h"
#include "game_variables.h"
#include <lcf/lsd/reader.h>

namespace {
	void OverrideSystemMusic(lcf::rpg::Music& music, Span<std::string const>& args) {
		music.name = args[0];
		if (args.size() > 1) {
			music.volume = atoi(args[1].c_str());
		}
		if (args.size() > 2) {
			music.tempo = atoi(args[2].c_str());
		}
		if (args.size() > 3) {
			music.fadein = atoi(args[3].c_str());
		}
	}

	void StoreTimestamp(int dest_v, std::tm* tm) {
		Main_Data::game_variables->Set(dest_v, tm->tm_year + 1900);
		Main_Data::game_variables->Set(dest_v + 1, tm->tm_mon + 1);
		Main_Data::game_variables->Set(dest_v + 2, tm->tm_mday);
		Main_Data::game_variables->Set(dest_v + 3, tm->tm_wday + 1);
		Main_Data::game_variables->Set(dest_v + 4, tm->tm_hour);
		Main_Data::game_variables->Set(dest_v + 5, tm->tm_min);
		Main_Data::game_variables->Set(dest_v + 6, tm->tm_sec);
	}
}

AsyncOp Game_PowerPatch::ExecutePPC(std::string_view ppc_cmd, Span<std::string const> args) {
	auto cmd = std::find_if(PPC_commands.begin(), PPC_commands.end(), [&ppc_cmd](auto& cmd) {
		return ppc_cmd == cmd.name;
	});
	if (cmd == PPC_commands.end()) {
		Output::Warning("PPCOMP {}: unknown command", ppc_cmd);
		return {};
	}

	if (args.size() < cmd->min_args) {
		Output::Warning("PPCOMP {}: Missing required arguments (Min: {})", ppc_cmd, cmd->min_args);
		return {};
	}

	AsyncOp async_op = {};
	Output::Debug("Executing PPC: {}", ppc_cmd);
	if (!Execute(cmd->type, args, async_op)) {
		Output::Warning("PPCOMP {}: Not supported", ppc_cmd);
		return {};
	}
	return async_op;
}

bool Game_PowerPatch::Execute(PPC_CommandType command, Span<std::string const> args, AsyncOp& async_op) {
	using Type = PPC_CommandType;

	switch (command) {
		case Type::Debug:
			// Note: PowerPatchCompact has a few nice debug options which might
			// be neat to also have in EasyRPG's extended debug menu
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Debug>());
			break;
		case Type::Quit:
			Player::exit_flag = true;
			break;
		case Type::Restart:
			Player::reset_flag = true;
			break;
		case Type::Save: {
			int slot = args.size() >= 1 ? atoi(args[0].c_str()) : 0;
			int map = args.size() >= 2 ? atoi(args[1].c_str()) : 0;
			int map_x = args.size() >= 3 ? atoi(args[2].c_str()) : -1;
			int map_y = args.size() >= 4 ? atoi(args[3].c_str()) : -1;

			auto fs = FileFinder::Save();
			if (slot == 0) {
				slot = Game_Interpreter_Shared::GetLatestSaveSlot(fs);
			}
			if (slot <= 0) {
				Output::Warning("PowerPatch Save: Invalid save slot {}", slot);
				return true;
			}
			//TODO: map_id, map_x, map_y 
			async_op = AsyncOp::MakeSave(slot, -1);
			break;
		}
		case Type::Load: {
			int slot = args.size() >= 1 ? atoi(args[0].c_str()) : 0;

			auto fs = FileFinder::Save();
			if (slot == 0) {
				slot = Game_Interpreter_Shared::GetLatestSaveSlot(fs);
			}
			auto save = Game_Interpreter_Shared::ValidateAndLoadSave("PowerPatch Load", fs, slot);
			if (!save) {
				return true;
			}
			// In RPG_RT the loading operation happens asynchronously while the
			// current interpreter is still running. Any other Ineluki scripts might
			// be executed before the loading process is complete.
			// This breaks some games, such as "Take it cheesy"
			//   -> Here, Save01 is loaded automatically, but the interpreter still
			//      executes a few other script commands right after, which set up
			//      the mouse patch. As a result, mouse functionality will work normally
			//      in RPG_RT, but not in EasyRPG Player, which never ran the
			//      neccessary setup scripts.
			async_op = Game_Interpreter_Shared::MakeLoadParallel("PowerPatch Load", slot);
			break;
		}
		case Type::CheckSave: {
			int slot = atoi(args[0].c_str());
			int dest_sw = atoi(args[1].c_str());

			auto fs = FileFinder::Save();
			auto exists = !FileFinder::GetSaveFilename(fs, slot, true).empty();
			Main_Data::game_switches->Set(dest_sw, exists);
			break;
		}
		case Type::CopySave: {
			int slot = atoi(args[0].c_str());
			int dest_slot = atoi(args[1].c_str());

			auto fs = FileFinder::Save();
			auto save = Game_Interpreter_Shared::ValidateAndLoadSave("PowerPatch CopySave", fs, slot);
			if (!save) {
				return true;
			}
			if (dest_slot <= 0) {
				Output::Warning("PowerPatch CopySave: Invalid save number {}", dest_slot);
				return true;
			}
			//TODO: Not implemented
			break;
		}
		case Type::DeleteSave: {
			int slot = atoi(args[0].c_str());

			if (slot <= 0) {
				Output::Warning("PowerPatch DeleteSave: Invalid save number {}", slot);
				return true;
			}
			//TODO: Not implemented
			break;
		}
		case Type::GetSaveDateTime: {
			int slot = atoi(args[0].c_str());
			int dest_v = atoi(args[1].c_str());

			auto fs = FileFinder::Save();
			auto save = Game_Interpreter_Shared::ValidateAndLoadSave("PowerPatch GetSaveDateTime", fs, slot);
			if (!save) {
				return true;
			}
			std::time_t t = lcf::LSD_Reader::ToUnixTimestamp(save->title.timestamp);
			std::tm* tm = std::gmtime(&t);
			StoreTimestamp(dest_v, tm);
			break;
		}
		case Type::GetSystemDateTime: {
			int dest_v = atoi(args[0].c_str());

			std::time_t t = lcf::LSD_Reader::GenerateTimestamp();
			std::tm* tm = std::gmtime(&t);
			StoreTimestamp(dest_v, tm);
			break;
		}
		case Type::SetGlobalBrightness:
			// Not implemented.
			// In some cases, changing the Scene via ppcomp might skip on the transition
			// routine and leave the screen black in RPG_RT.
			// This command was added to the patch, to be able to manually set the
			// internal brightness back to '100'.
			break;
		case Type::CallLoadMenu:
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Load>());
			break;
		case Type::CallSaveMenu:
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Save>());
			break;
		case Type::CallGameMenu: {
			int cursor = args.size() >= 1 ? atoi(args[0].c_str()) : 0;
			//TODO: implement cursor
			Game_Map::GetInterpreter().RequestMainMenuScene();
			break;
		}
		case Type::CallTitleScreen: {
			int cursor = args.size() >= 1 ? atoi(args[0].c_str()) : 0;
			//TODO: implement cursor
			async_op = AsyncOp::MakeToTitle();
			break;
		}
		case Type::SetTitleBGM:
			OverrideSystemMusic(lcf::Data::system.title_music, args);
			break;
		case Type::SetTitleScreen:
			lcf::Data::system.title_name = lcf::DBString(args[0]);
			break;
		case Type::SetGameOverScreen:
			lcf::Data::system.gameover_name = lcf::DBString(args[0]);
			break;
		case Type::UnlockPictures: {
			int value = atoi(args[0].c_str());
			if (Player::game_config.patch_unlock_pics.IsLocked()) {
				Player::game_config.patch_unlock_pics.SetLocked(false);
			}
			Player::game_config.patch_unlock_pics.Set(value);
			break;
		}
		default:
			return false;
	}

	return true;
}
