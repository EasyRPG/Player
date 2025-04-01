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
#include "scene_title.h"
#include "scene_menu.h"
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

	//FIXME: Move this call to game_runtime_patches.h (part of another branch)
	constexpr Input::Keys::InputKey VirtualKeyToInputKey(uint32_t key_id) {
		// see https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
		switch (key_id) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
			case 0x1: return Input::Keys::MOUSE_LEFT;
			case 0x2: return Input::Keys::MOUSE_RIGHT;
			case 0x4: return Input::Keys::MOUSE_MIDDLE;
			case 0x5: return Input::Keys::MOUSE_XBUTTON1;
			case 0x6: return Input::Keys::MOUSE_XBUTTON2;
#endif
			case 0x8: return Input::Keys::BACKSPACE;
			case 0x9: return Input::Keys::TAB;
			case 0xD: return Input::Keys::RETURN;
			case 0x10: return Input::Keys::SHIFT;
			case 0x11: return Input::Keys::CTRL;
			case 0x12: return Input::Keys::ALT;
			case 0x13: return Input::Keys::PAUSE;
			case 0x14: return Input::Keys::CAPS_LOCK;
			case 0x1B: return Input::Keys::ESCAPE;
			case 0x20: return Input::Keys::SPACE;
			case 0x21: return Input::Keys::PGUP;
			case 0x22: return Input::Keys::PGDN;
			case 0x23: return Input::Keys::ENDS;
			case 0x24: return Input::Keys::HOME;
			case 0x25: return Input::Keys::LEFT;
			case 0x26: return Input::Keys::UP;
			case 0x27: return Input::Keys::RIGHT;
			case 0x28: return Input::Keys::DOWN;
			case 0x2D: return Input::Keys::INSERT;
			case 0x2E: return Input::Keys::DEL;
			case 0x30: return Input::Keys::N0;
			case 0x31: return Input::Keys::N1;
			case 0x32: return Input::Keys::N2;
			case 0x33: return Input::Keys::N3;
			case 0x34: return Input::Keys::N4;
			case 0x35: return Input::Keys::N5;
			case 0x36: return Input::Keys::N6;
			case 0x37: return Input::Keys::N7;
			case 0x38: return Input::Keys::N8;
			case 0x39: return Input::Keys::N9;
			case 0x41: return Input::Keys::A;
			case 0x42: return Input::Keys::B;
			case 0x43: return Input::Keys::C;
			case 0x44: return Input::Keys::D;
			case 0x45: return Input::Keys::E;
			case 0x46: return Input::Keys::F;
			case 0x47: return Input::Keys::G;
			case 0x48: return Input::Keys::H;
			case 0x49: return Input::Keys::I;
			case 0x4A: return Input::Keys::J;
			case 0x4B: return Input::Keys::K;
			case 0x4C: return Input::Keys::L;
			case 0x4D: return Input::Keys::M;
			case 0x4E: return Input::Keys::N;
			case 0x4F: return Input::Keys::O;
			case 0x50: return Input::Keys::P;
			case 0x51: return Input::Keys::Q;
			case 0x52: return Input::Keys::R;
			case 0x53: return Input::Keys::S;
			case 0x54: return Input::Keys::T;
			case 0x55: return Input::Keys::U;
			case 0x56: return Input::Keys::V;
			case 0x57: return Input::Keys::W;
			case 0x58: return Input::Keys::X;
			case 0x59: return Input::Keys::Y;
			case 0x5A: return Input::Keys::Z;
			case 0x60: return Input::Keys::KP0;
			case 0x61: return Input::Keys::KP1;
			case 0x62: return Input::Keys::KP2;
			case 0x63: return Input::Keys::KP3;
			case 0x64: return Input::Keys::KP4;
			case 0x65: return Input::Keys::KP5;
			case 0x66: return Input::Keys::KP6;
			case 0x67: return Input::Keys::KP7;
			case 0x68: return Input::Keys::KP8;
			case 0x69: return Input::Keys::KP9;
			case 0x6A: return Input::Keys::KP_MULTIPLY;
			case 0x6B: return Input::Keys::KP_ADD;
			case 0x6D: return Input::Keys::KP_SUBTRACT;
			case 0x6E: return Input::Keys::KP_PERIOD;
			case 0x6F: return Input::Keys::KP_DIVIDE;
			case 0x70: return Input::Keys::F1;
			case 0x71: return Input::Keys::F2;
			case 0x72: return Input::Keys::F3;
			case 0x73: return Input::Keys::F4;
			case 0x74: return Input::Keys::F5;
			case 0x75: return Input::Keys::F6;
			case 0x76: return Input::Keys::F7;
			case 0x77: return Input::Keys::F8;
			case 0x78: return Input::Keys::F9;
			case 0x79: return Input::Keys::F10;
			case 0x7A: return Input::Keys::F11;
			case 0x7B: return Input::Keys::F12;
			case 0x90: return Input::Keys::NUM_LOCK;
			case 0x91: return Input::Keys::SCROLL_LOCK;
			case 0xA0: return Input::Keys::LSHIFT;
			case 0xA1: return Input::Keys::RSHIFT;
			case 0xA2: return Input::Keys::LCTRL;
			case 0xA3: return Input::Keys::RCTRL;

			default: return Input::Keys::NONE;
		}
	}
}

std::map<Input::Keys::InputKey, int> Game_PowerPatch::simulate_keypresses;

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
			if (args.size() >= 1) {
				if (atoi(args[0].c_str())) {
					Scene_Menu::force_cursor_index = Scene_Menu::CommandOptionType::Save;
				} else {
					Scene_Menu::force_cursor_index = Scene_Menu::CommandOptionType::Item;
				}
			}
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Menu>());
			break;
		}
		case Type::CallTitleScreen: {
			if (args.size() >= 1) {
				if (atoi(args[0].c_str())) {
					Scene_Title::force_cursor_index = Scene_Title::CommandOptionType::ContinueGame;
				} else {
					Scene_Title::force_cursor_index = Scene_Title::CommandOptionType::NewGame;
				}
			}
			//Player::force_make_to_title_flag = true;
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
		case Type::SimulateKeyPress: {
			int vk = atoi(args[0].c_str());
			auto input_key = VirtualKeyToInputKey(vk);

			if (input_key == Input::Keys::NONE) {
				Output::Debug("PowerPatch SimulateKeyPress: Unsupported keycode {}", vk);
				return true;
			}

			//TODO: Needs some proper testing
			if (Utils::LowerCase(args[1]) == "down") {
				simulate_keypresses[input_key] = 1;
			} else if (Utils::LowerCase(args[1]) == "up") {
				simulate_keypresses[input_key] = 0;
			} else {
				int duration = atoi(args[1].c_str());
				if (duration <= 0) {
					Output::Debug("PowerPatch SimulateKeyPress: Unexpected arg {} (<Action/Duration>)", duration);
					return true;
				}
				duration = DEFAULT_FPS * duration / 1000;
				if (duration == 0) {
					duration = 1;
				}
				simulate_keypresses[input_key] = duration;
			}
			break;
		}
		default:
			return false;
	}

	return true;
}

