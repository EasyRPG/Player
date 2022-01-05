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
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cassert>
#include <ctime>
#include "game_interpreter.h"
#include "audio.h"
#include "dynrpg.h"
#include "filefinder.h"
#include "game_map.h"
#include "game_event.h"
#include "game_enemyparty.h"
#include "game_ineluki.h"
#include "game_player.h"
#include "game_targets.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_message.h"
#include "game_pictures.h"
#include "game_screen.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_gameover.h"
#include "scene_map.h"
#include "scene_save.h"
#include "scene.h"
#include "game_clock.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include <lcf/reader_util.h>
#include <lcf/lsd/reader.h>
#include "game_battle.h"
#include "utils.h"
#include "transition.h"
#include "baseui.h"
#include "algo.h"
#include "rand.h"

enum BranchSubcommand {
	eOptionBranchElse = 1
};

constexpr int Game_Interpreter::loop_limit;
constexpr int Game_Interpreter::call_stack_limit;
constexpr int Game_Interpreter::subcommand_sentinel;

Game_Interpreter::Game_Interpreter(bool _main_flag) {
	main_flag = _main_flag;

	Clear();
}

Game_Interpreter::~Game_Interpreter() {
}

// Clear.
void Game_Interpreter::Clear() {
	_state = {};
	_keyinput = {};
	_async_op = {};
}

// Is interpreter running.
bool Game_Interpreter::IsRunning() const {
	return !_state.stack.empty();
}

// Setup.
void Game_Interpreter::Push(
	const std::vector<lcf::rpg::EventCommand>& _list,
	int event_id,
	bool started_by_decision_key
) {
	if (_list.empty()) {
		return;
	}

	if ((int)_state.stack.size() > call_stack_limit) {
		Output::Error("Call Event limit ({}) has been exceeded", call_stack_limit);
	}

	lcf::rpg::SaveEventExecFrame frame;
	frame.ID = _state.stack.size() + 1;
	frame.commands = _list;
	frame.current_command = 0;
	frame.triggered_by_decision_key = started_by_decision_key;
	frame.event_id = event_id;

	if (_state.stack.empty() && main_flag && !Game_Battle::IsBattleRunning()) {
		Main_Data::game_system->ClearMessageFace();
		Main_Data::game_player->SetMenuCalling(false);
		Main_Data::game_player->SetEncounterCalling(false);
	}

	_state.stack.push_back(std::move(frame));
}


void Game_Interpreter::KeyInputState::fromSave(const lcf::rpg::SaveEventExecState& save) {
	*this = {};

	// Maniac Patch aware check functions for parameters that handle
	// keyboard and mouse through a bitmask
	bool is_maniac = Player::IsPatchManiac();
	auto check_key = [&](auto& var) {
		if (is_maniac) {
			return (var & 1) != 0;
		} else {
			return var != 0;
		}
	};
	auto check_mouse = [&](auto& var) {
		return (var & 2) != 0;
	};

	wait = save.keyinput_wait;
	// FIXME: There is an RPG_RT bug where keyinput_variable is uint8_t
	// which we currently have to emulate. So the value from the save could be wrong.
	variable = save.keyinput_variable;

	if (save.keyinput_all_directions) {
		keys[Keys::eDown] = true;
		keys[Keys::eLeft] = true;
		keys[Keys::eRight] = true;
		keys[Keys::eUp] = true;
	} else {
		if (Player::IsRPG2k3()) {
			keys[Keys::eDown] = check_key(save.keyinput_2k3down);
			keys[Keys::eLeft] = check_key(save.keyinput_2k3left);
			keys[Keys::eRight] = check_key(save.keyinput_2k3right);
			keys[Keys::eUp] = check_key(save.keyinput_2k3up);
		} else {
			keys[Keys::eDown] = check_key(save.keyinput_2kdown_2k3operators);
			keys[Keys::eLeft] = check_key(save.keyinput_2kleft_2k3shift);
			keys[Keys::eRight] = check_key(save.keyinput_2kright);
			keys[Keys::eUp] = check_key(save.keyinput_2kup);
		}
	}

	keys[Keys::eDecision] = check_key(save.keyinput_decision);
	keys[Keys::eCancel] = check_key(save.keyinput_cancel);

	if (Player::IsRPG2k3()) {
		keys[Keys::eShift] = check_key(save.keyinput_2kleft_2k3shift);
		keys[Keys::eNumbers] = check_key(save.keyinput_2kshift_2k3numbers);
		keys[Keys::eOperators] = check_key(save.keyinput_2kdown_2k3operators);

		if (is_maniac) {
			keys[Keys::eMouseLeft] = check_mouse(save.keyinput_decision);
			keys[Keys::eMouseRight] = check_mouse(save.keyinput_cancel);
			keys[Keys::eMouseMiddle] = check_mouse(save.keyinput_2kleft_2k3shift);
			keys[Keys::eMouseScrollUp] = check_mouse(save.keyinput_2k3up);
			keys[Keys::eMouseScrollDown] = check_mouse(save.keyinput_2k3down);
		}
	} else {
		keys[Keys::eShift] = check_key(save.keyinput_2kshift_2k3numbers);
	}

	time_variable = save.keyinput_time_variable;
	timed = save.keyinput_timed;
	// FIXME: Rm2k3 has no LSD chunk for this.
	wait_frames = 0;
}

void Game_Interpreter::KeyInputState::toSave(lcf::rpg::SaveEventExecState& save) const {
	save.keyinput_wait = 0;
	save.keyinput_variable = 0;
	save.keyinput_all_directions = 0;
	save.keyinput_decision = 0;
	save.keyinput_cancel = 0;
	save.keyinput_2kshift_2k3numbers = 0;
	save.keyinput_2kdown_2k3operators = 0;
	save.keyinput_2kleft_2k3shift = 0;
	save.keyinput_2kright = 0;
	save.keyinput_2kup = 0;
	save.keyinput_time_variable = 0;
	save.keyinput_2k3down = 0;
	save.keyinput_2k3left = 0;
	save.keyinput_2k3right = 0;
	save.keyinput_2k3up = 0;
	save.keyinput_timed = 0;

	save.keyinput_wait = wait;
	// FIXME: There is an RPG_RT bug where keyinput_variable is uint8_t
	// which we currently have to emulate. So this assignment truncates.
	save.keyinput_variable = variable;

	if (keys[Keys::eDown]
			&& keys[Keys::eLeft]
			&& keys[Keys::eRight]
			&& keys[Keys::eUp]) {
		save.keyinput_all_directions = true;
	} else {
		if (Player::IsRPG2k3()) {
			save.keyinput_2k3down = keys[Keys::eDown];
			save.keyinput_2k3left = keys[Keys::eLeft];
			save.keyinput_2k3right = keys[Keys::eRight];
			save.keyinput_2k3up = keys[Keys::eUp];
		} else {
			// RM2k uses these chunks for directions.
			save.keyinput_2kdown_2k3operators = keys[Keys::eDown];
			save.keyinput_2kleft_2k3shift = keys[Keys::eLeft];
			save.keyinput_2kright = keys[Keys::eRight];
			save.keyinput_2kup = keys[Keys::eUp];
		}
	}

	save.keyinput_decision = keys[Keys::eDecision];
	save.keyinput_cancel = keys[Keys::eCancel];

	if (Player::IsRPG2k3()) {
		save.keyinput_2kleft_2k3shift = keys[Keys::eShift];
		save.keyinput_2kshift_2k3numbers = keys[Keys::eNumbers];
		save.keyinput_2kdown_2k3operators = keys[Keys::eOperators];

		if (Player::IsPatchManiac()) {
			if (keys[Keys::eMouseLeft]) {
				save.keyinput_decision |= 2;
			}
			if (keys[Keys::eMouseRight]) {
				save.keyinput_cancel |= 2;
			}
			if (keys[Keys::eMouseMiddle]) {
				save.keyinput_2kleft_2k3shift |= 2;
			}
			if (keys[Keys::eMouseScrollUp]) {
				save.keyinput_2k3up |= 2;
			}
			if (keys[Keys::eMouseScrollDown]) {
				save.keyinput_2k3down |= 2;
			}
		}
	} else {
		save.keyinput_2kshift_2k3numbers = keys[Keys::eShift];
	}

	save.keyinput_time_variable = time_variable;
	save.keyinput_timed = timed;
	// FIXME: Rm2k3 has no LSD chunk for this.
	//void = wait_frames;
}


lcf::rpg::SaveEventExecState Game_Interpreter::GetState() const {
	auto save = _state;
	_keyinput.toSave(save);
	return save;
}


void Game_Interpreter::SetupWait(int duration) {
	if (duration == 0) {
		// 0.0 waits 1 frame
		_state.wait_time = 1;
	} else {
		_state.wait_time = duration * DEFAULT_FPS / 10;
	}
}

bool Game_Interpreter::ReachedLoopLimit() const {
	return loop_count >= loop_limit;
}

int Game_Interpreter::GetThisEventId() const {
	auto event_id = GetCurrentEventId();

	if (event_id == 0 && Player::IsRPG2k3E()) {
		// RM2k3E allows "ThisEvent" commands to run from called
		// common events. It operates on the last map event in
		// the call stack.
		for (auto iter = _state.stack.rbegin()++;
				iter != _state.stack.rend(); ++iter) {
			if (iter->event_id != 0) {
				event_id = iter->event_id;
				break;
			}
		}
	}

	return event_id;
}

uint8_t& Game_Interpreter::ReserveSubcommandIndex(int indent) {
	auto& frame = GetFrame();

	auto& path = frame.subcommand_path;
	if (indent >= (int)path.size()) {
		// This fixes an RPG_RT bug where RPG_RT would resize
		// the array with uninitialized values.
		path.resize(indent + 1, subcommand_sentinel);
	}
	return path[indent];
}

void Game_Interpreter::SetSubcommandIndex(int indent, int idx) {
	ReserveSubcommandIndex(indent) = idx;
}

int Game_Interpreter::GetSubcommandIndex(int indent) const {
	auto* frame = GetFramePtr();
	if (frame == nullptr) {
		return subcommand_sentinel;
	}
	auto& path = frame->subcommand_path;
	if ((int)path.size() <= indent) {
		return subcommand_sentinel;
	}
	return path[indent];
}

// Update
void Game_Interpreter::Update(bool reset_loop_count) {
	if (reset_loop_count) {
		loop_count = 0;
	}

	// Always reset async status when we enter interpreter loop.
	_async_op = {};

	if (!IsRunning()) {
		return;
	}

	if (Input::IsTriggered(Input::DEBUG_ABORT_EVENT) && Player::debug_flag && !Game_Battle::IsBattleRunning()) {
		if (Game_Message::IsMessageActive()) {
			Game_Message::GetWindow()->FinishMessageProcessing();
		}
		if (!Main_Data::game_player->GetMoveRoute().move_commands.empty()) {
			Main_Data::game_player->CancelMoveRoute();
		}
		EndEventProcessing();
		return;
	}

	for (; loop_count < loop_limit; ++loop_count) {
		// If something is calling a menu, we're allowed to execute only 1 command per interpreter. So we pass through if loop_count == 0, and stop at 1 or greater.
		// RPG_RT compatible behavior.
		if (loop_count > 0 && Scene::instance->HasRequestedScene()) {
			break;
		}

		// Previous command triggered an async operation.
		if (IsAsyncPending()) {
			break;
		}

		if (main_flag) {
			if (Main_Data::game_player->IsBoardingOrUnboarding())
				break;

			if (Main_Data::game_player->InVehicle() && Main_Data::game_player->GetVehicle()->IsAscendingOrDescending())
				break;

			if (Game_Message::IsMessagePending())
				break;
		} else {
			if (Game_Message::IsMessageActive() && _state.show_message) {
				break;
			}
		}

		_state.show_message = false;
		_state.abort_on_escape = false;

		if (_state.wait_time > 0) {
			_state.wait_time--;
			break;
		}

		if (_state.wait_key_enter) {
			if (Game_Message::IsMessageActive()) {
				break;
			}

			if (!Input::IsTriggered(Input::DECISION)) {
				break;
			}
			_state.wait_key_enter = false;
		}

		if (_state.wait_movement) {
			if (Game_Map::IsAnyMovePending()) {
				break;
			}
			_state.wait_movement = false;
		}

		if (_keyinput.wait) {
			if (Game_Message::IsMessageActive()) {
				break;
			}

			const int key = _keyinput.CheckInput();
			Main_Data::game_variables->Set(_keyinput.variable, key);
			Game_Map::SetNeedRefresh(true);
			if (key == 0) {
				++_keyinput.wait_frames;
				break;
			}
			if (_keyinput.timed) {
				// 10 per second
				Main_Data::game_variables->Set(_keyinput.time_variable,
						(_keyinput.wait_frames * 10) / Game_Clock::GetTargetGameFps());
			}
			_keyinput.wait = false;
		}

		auto* frame = GetFramePtr();
		if (frame == nullptr) {
			break;
		}

		if (Game_Map::GetNeedRefresh()) {
			Game_Map::Refresh();
		}

		// Previous operations could have modified the stack.
		// So we need to fetch the frame again.
		frame = GetFramePtr();
		if (frame == nullptr) {
			break;
		}

		// Pop any completed stack frames
		if (frame->current_command >= (int)frame->commands.size()) {
			if (!OnFinishStackFrame()) {
				break;
			}
			continue;
		}

		// Save the frame index before we call events.
		int current_frame_idx = _state.stack.size() - 1;

		const int index_before_exec = frame->current_command;
		if (!ExecuteCommand()) {
			break;
		}

		if (Game_Battle::IsBattleRunning() && Player::IsRPG2k3() && Game_Battle::CheckWin()) {
			// Interpreter is cancelled when a win condition is fulfilled in RPG2k3 battle
			break;
		}

		// Last event command removed the frame? We're done.
		if (current_frame_idx >= (int)_state.stack.size() ) {
			continue;
		}

		// Note: In the case we executed a CallEvent command, be sure to
		// increment the old frame and not the new one we just pushed.
		frame = &_state.stack[current_frame_idx];

		// Only do auto increment if the command didn't manually
		// change the index.
		if (index_before_exec == frame->current_command) {
			frame->current_command++;
		}
	} // for

	if (loop_count > loop_limit - 1) {
		auto* frame = GetFramePtr();
		int event_id = frame ? frame->event_id : 0;
		// Executed Events Count exceeded (10000)
		Output::Debug("Event {} exceeded execution limit", event_id);
	}

	if (Game_Map::GetNeedRefresh()) {
		Game_Map::Refresh();
	}
}

// Setup Starting Event
void Game_Interpreter::Push(Game_Event* ev) {
	Push(ev->GetList(), ev->GetId(), ev->WasStartedByDecisionKey());
}

void Game_Interpreter::Push(Game_Event* ev, const lcf::rpg::EventPage* page, bool triggered_by_decision_key) {
	Push(page->event_commands, ev->GetId(), triggered_by_decision_key);
}

void Game_Interpreter::Push(Game_CommonEvent* ev) {
	Push(ev->GetList(), 0, false);
}

bool Game_Interpreter::CheckGameOver() {
	if (!Game_Battle::IsBattleRunning() && !Main_Data::game_party->IsAnyActive()) {
		// Empty party is allowed
		if (Main_Data::game_party->GetBattlerCount() > 0) {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Gameover>());
			return true;
		}
	}
	return false;
}

void Game_Interpreter::SkipToNextConditional(std::initializer_list<Cmd> codes, int indent) {
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	if (index >= static_cast<int>(list.size())) {
		return;
	}

	for (++index; index < static_cast<int>(list.size()); ++index) {
		const auto& com = list[index];
		if (com.indent > indent) {
			continue;
		}
		if (std::find(codes.begin(), codes.end(), static_cast<Cmd>(com.code)) != codes.end()) {
			break;
		}
	}
}

int Game_Interpreter::DecodeInt(lcf::DBArray<int32_t>::const_iterator& it) {
	int value = 0;

	for (;;) {
		int x = *it++;
		value <<= 7;
		value |= x & 0x7F;
		if (!(x & 0x80))
			break;
	}

	return value;
}

const std::string Game_Interpreter::DecodeString(lcf::DBArray<int32_t>::const_iterator& it) {
	std::ostringstream out;
	int len = DecodeInt(it);

	for (int i = 0; i < len; i++)
		out << (char)*it++;

	std::string result = lcf::ReaderUtil::Recode(out.str(), Player::encoding);

	return result;
}

lcf::rpg::MoveCommand Game_Interpreter::DecodeMove(lcf::DBArray<int32_t>::const_iterator& it) {
	lcf::rpg::MoveCommand cmd;
	cmd.command_id = *it++;

	switch (cmd.command_id) {
	case 32:	// Switch ON
	case 33:	// Switch OFF
		cmd.parameter_a = DecodeInt(it);
		break;
	case 34:	// Change Graphic
		cmd.parameter_string = lcf::DBString(DecodeString(it));
		cmd.parameter_a = DecodeInt(it);
		break;
	case 35:	// Play Sound Effect
		cmd.parameter_string = lcf::DBString(DecodeString(it));
		cmd.parameter_a = DecodeInt(it);
		cmd.parameter_b = DecodeInt(it);
		cmd.parameter_c = DecodeInt(it);
		break;
	}

	return cmd;
}

// Execute Command.
bool Game_Interpreter::ExecuteCommand() {
	auto& frame = GetFrame();
	const auto& com = frame.commands[frame.current_command];

	switch (static_cast<Cmd>(com.code)) {
		case Cmd::ShowMessage:
			return CommandShowMessage(com);
		case Cmd::MessageOptions:
			return CommandMessageOptions(com);
		case Cmd::ChangeFaceGraphic:
			return CommandChangeFaceGraphic(com);
		case Cmd::ShowChoice:
			return CommandShowChoices(com);
		case Cmd::ShowChoiceOption:
			return CommandShowChoiceOption(com);
		case Cmd::ShowChoiceEnd:
			return CommandShowChoiceEnd(com);
		case Cmd::InputNumber:
			return CommandInputNumber(com);
		case Cmd::ControlSwitches:
			return CommandControlSwitches(com);
		case Cmd::ControlVars:
			return CommandControlVariables(com);
		case Cmd::TimerOperation:
			return CommandTimerOperation(com);
		case Cmd::ChangeGold:
			return CommandChangeGold(com);
		case Cmd::ChangeItems:
			return CommandChangeItems(com);
		case Cmd::ChangePartyMembers:
			return CommandChangePartyMember(com);
		case Cmd::ChangeExp:
			return CommandChangeExp(com);
		case Cmd::ChangeLevel:
			return CommandChangeLevel(com);
		case Cmd::ChangeParameters:
			return CommandChangeParameters(com);
		case Cmd::ChangeSkills:
			return CommandChangeSkills(com);
		case Cmd::ChangeEquipment:
			return CommandChangeEquipment(com);
		case Cmd::ChangeHP:
			return CommandChangeHP(com);
		case Cmd::ChangeSP:
			return CommandChangeSP(com);
		case Cmd::ChangeCondition:
			return CommandChangeCondition(com);
		case Cmd::FullHeal:
			return CommandFullHeal(com);
		case Cmd::SimulatedAttack:
			return CommandSimulatedAttack(com);
		case Cmd::Wait:
			return CommandWait(com);
		case Cmd::PlayBGM:
			return CommandPlayBGM(com);
		case Cmd::FadeOutBGM:
			return CommandFadeOutBGM(com);
		case Cmd::PlaySound:
			return CommandPlaySound(com);
		case Cmd::EndEventProcessing:
			return CommandEndEventProcessing(com);
		case Cmd::Comment:
		case Cmd::Comment_2:
			return CommandComment(com);
		case Cmd::GameOver:
			return CommandGameOver(com);
		case Cmd::ChangeHeroName:
			return CommandChangeHeroName(com);
		case Cmd::ChangeHeroTitle:
			return CommandChangeHeroTitle(com);
		case Cmd::ChangeSpriteAssociation:
			return CommandChangeSpriteAssociation(com);
		case Cmd::ChangeActorFace:
			return CommandChangeActorFace(com);
		case Cmd::ChangeVehicleGraphic:
			return CommandChangeVehicleGraphic(com);
		case Cmd::ChangeSystemBGM:
			return CommandChangeSystemBGM(com);
		case Cmd::ChangeSystemSFX:
			return CommandChangeSystemSFX(com);
		case Cmd::ChangeSystemGraphics:
			return CommandChangeSystemGraphics(com);
		case Cmd::ChangeScreenTransitions:
			return CommandChangeScreenTransitions(com);
		case Cmd::MemorizeLocation:
			return CommandMemorizeLocation(com);
		case Cmd::SetVehicleLocation:
			return CommandSetVehicleLocation(com);
		case Cmd::ChangeEventLocation:
			return CommandChangeEventLocation(com);
		case Cmd::TradeEventLocations:
			return CommandTradeEventLocations(com);
		case Cmd::StoreTerrainID:
			return CommandStoreTerrainID(com);
		case Cmd::StoreEventID:
			return CommandStoreEventID(com);
		case Cmd::EraseScreen:
			return CommandEraseScreen(com);
		case Cmd::ShowScreen:
			return CommandShowScreen(com);
		case Cmd::TintScreen:
			return CommandTintScreen(com);
		case Cmd::FlashScreen:
			return CommandFlashScreen(com);
		case Cmd::ShakeScreen:
			return CommandShakeScreen(com);
		case Cmd::WeatherEffects:
			return CommandWeatherEffects(com);
		case Cmd::ShowPicture:
			return CommandShowPicture(com);
		case Cmd::MovePicture:
			return CommandMovePicture(com);
		case Cmd::ErasePicture:
			return CommandErasePicture(com);
		case Cmd::SpriteTransparency:
			return CommandSpriteTransparency(com);
		case Cmd::MoveEvent:
			return CommandMoveEvent(com);
		case Cmd::MemorizeBGM:
			return CommandMemorizeBGM(com);
		case Cmd::PlayMemorizedBGM:
			return CommandPlayMemorizedBGM(com);
		case Cmd::KeyInputProc:
			return CommandKeyInputProc(com);
		case Cmd::ChangeMapTileset:
			return CommandChangeMapTileset(com);
		case Cmd::ChangePBG:
			return CommandChangePBG(com);
		case Cmd::ChangeEncounterRate:
			return CommandChangeEncounterRate(com);
		case Cmd::TileSubstitution:
			return CommandTileSubstitution(com);
		case Cmd::TeleportTargets:
			return CommandTeleportTargets(com);
		case Cmd::ChangeTeleportAccess:
			return CommandChangeTeleportAccess(com);
		case Cmd::EscapeTarget:
			return CommandEscapeTarget(com);
		case Cmd::ChangeEscapeAccess:
			return CommandChangeEscapeAccess(com);
		case Cmd::ChangeSaveAccess:
			return CommandChangeSaveAccess(com);
		case Cmd::ChangeMainMenuAccess:
			return CommandChangeMainMenuAccess(com);
		case Cmd::ConditionalBranch:
			return CommandConditionalBranch(com);
		case Cmd::Label:
			return true;
		case Cmd::JumpToLabel:
			return CommandJumpToLabel(com);
		case Cmd::Loop:
			return CommandLoop(com);
		case Cmd::BreakLoop:
			return CommandBreakLoop(com);
		case Cmd::EndLoop:
			return CommandEndLoop(com);
		case Cmd::EraseEvent:
			return CommandEraseEvent(com);
		case Cmd::CallEvent:
			return CommandCallEvent(com);
		case Cmd::ReturntoTitleScreen:
			return CommandReturnToTitleScreen(com);
		case Cmd::ChangeClass:
			return CommandChangeClass(com);
		case Cmd::ChangeBattleCommands:
			return CommandChangeBattleCommands(com);
		case Cmd::ElseBranch:
			return CommandElseBranch(com);
		case Cmd::EndBranch:
			return CommandEndBranch(com);
		case Cmd::ExitGame:
			return CommandExitGame(com);
		case Cmd::ToggleFullscreen:
			return CommandToggleFullscreen(com);
		case Cmd::Maniac_GetSaveInfo:
			return CommandManiacGetSaveInfo(com);
		case Cmd::Maniac_Load:
			return CommandManiacLoad(com);
		case Cmd::Maniac_Save:
			return CommandManiacSave(com);
		case Cmd::Maniac_EndLoadProcess:
			return CommandManiacEndLoadProcess(com);
		case Cmd::Maniac_GetMousePosition:
			return CommandManiacGetMousePosition(com);
		case Cmd::Maniac_SetMousePosition:
			return CommandManiacSetMousePosition(com);
		case Cmd::Maniac_ShowStringPicture:
			return CommandManiacShowStringPicture(com);
		case Cmd::Maniac_GetPictureInfo:
			return CommandManiacGetPictureInfo(com);
		case Cmd::Maniac_ControlVarArray:
			return CommandManiacControlVarArray(com);
		case Cmd::Maniac_KeyInputProcEx:
			return CommandManiacKeyInputProcEx(com);
		case Cmd::Maniac_RewriteMap:
			return CommandManiacRewriteMap(com);
		case Cmd::Maniac_ControlGlobalSave:
			return CommandManiacControlGlobalSave(com);
		case Cmd::Maniac_ChangePictureId:
			return CommandManiacChangePictureId(com);
		case Cmd::Maniac_SetGameOption:
			return CommandManiacSetGameOption(com);
		case Cmd::Maniac_CallCommand:
			return CommandManiacCallCommand(com);
		default:
			return true;
	}
}

bool Game_Interpreter::OnFinishStackFrame() {
	auto& frame = GetFrame();

	const bool is_base_frame = _state.stack.size() == 1;

	if (main_flag && is_base_frame && !Game_Battle::IsBattleRunning()) {
		Main_Data::game_system->ClearMessageFace();
	}

	int event_id = frame.event_id;

	if (is_base_frame && event_id > 0) {
		Game_Event* evnt = Game_Map::GetEvent(event_id);
		if (!evnt) {
			Output::Error("Call stack finished with invalid event id {}. This can be caused by a vehicle teleport?", event_id);
		} else if (main_flag) {
			evnt->OnFinishForegroundEvent();
		}
	}

	if (!main_flag && is_base_frame) {
		// Parallel events will never clear the base stack frame. Instead we just
		// reset the index back to 0 and wait for a frame.
		// This not only optimizes away copying event code, it's also RPG_RT compatible.
		frame.current_command = 0;
	} else {
		// If a called frame, or base frame of foreground interpreter, pop the stack.
		_state.stack.pop_back();
	}

	return !is_base_frame;
}

std::vector<std::string> Game_Interpreter::GetChoices(int max_num_choices) {
	const auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	// Let's find the choices
	int current_indent = list[index + 1].indent;
	std::vector<std::string> s_choices;
	for (int index_temp = index + 1; index_temp < static_cast<int>(list.size()); ++index_temp) {
		const auto& com = list[index_temp];
		if (com.indent != current_indent) {
			continue;
		}

		if (static_cast<Cmd>(com.code) == Cmd::ShowChoiceOption && com.parameters.size() > 0 && com.parameters[0] < max_num_choices) {
			// Choice found
			s_choices.push_back(ToString(list[index_temp].string));
		}

		if (static_cast<Cmd>(com.code) == Cmd::ShowChoiceEnd) {
			break;
		}
	}
	return s_choices;
}

bool Game_Interpreter::CommandOptionGeneric(lcf::rpg::EventCommand const& com, int option_sub_idx, std::initializer_list<Cmd> next) {
	const auto sub_idx = GetSubcommandIndex(com.indent);
	if (sub_idx == option_sub_idx) {
		// Executes this option, so clear the subidx to skip all other options.
		SetSubcommandIndex(com.indent, subcommand_sentinel);
	} else {
		SkipToNextConditional(next, com.indent);
	}
	return true;
}

bool Game_Interpreter::CommandShowMessage(lcf::rpg::EventCommand const& com) { // code 10110
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	auto pm = PendingMessage();
	pm.SetIsEventMessage(true);

	// Set first line
	pm.PushLine(ToString(com.string));

	++index;

	// Check for continued lines via ShowMessage_2
	while (index < static_cast<int>(list.size()) && static_cast<Cmd>(list[index].code) == Cmd::ShowMessage_2) {
		// Add second (another) line
		pm.PushLine(ToString(list[index].string));
		++index;
	}

	// Handle Choices or number
	if (index < static_cast<int>(list.size())) {
		// If next event command is show choices
		if (static_cast<Cmd>(list[index].code) == Cmd::ShowChoice) {
			std::vector<std::string> s_choices = GetChoices(4);
			// If choices fit on screen
			if (static_cast<int>(s_choices.size()) <= (4 - pm.NumLines())) {
				pm.SetChoiceCancelType(list[index].parameters[0]);
				SetupChoices(s_choices, com.indent, pm);
				++index;
			}
		} else if (static_cast<Cmd>(list[index].code) == Cmd::InputNumber) {
			// If next event command is input number
			// If input number fits on screen
			if (pm.NumLines() < 4) {
				int digits = list[index].parameters[0];
				int variable_id = list[index].parameters[1];
				pm.PushNumInput(variable_id, digits);
				++index;
			}
		}
	}

	Game_Message::SetPendingMessage(std::move(pm));
	_state.show_message = true;

	return true;
}

bool Game_Interpreter::CommandMessageOptions(lcf::rpg::EventCommand const& com) { //code 10120
	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	Main_Data::game_system->SetMessageTransparent(com.parameters[0] != 0);
	Main_Data::game_system->SetMessagePosition(com.parameters[1]);
	Main_Data::game_system->SetMessagePositionFixed(com.parameters[2] == 0);
	Main_Data::game_system->SetMessageContinueEvents(com.parameters[3] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeFaceGraphic(lcf::rpg::EventCommand const& com) { // Code 10130
	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	Main_Data::game_system->SetMessageFaceName(ToString(com.string));
	Main_Data::game_system->SetMessageFaceIndex(com.parameters[0]);
	Main_Data::game_system->SetMessageFaceRightPosition(com.parameters[1] != 0);
	Main_Data::game_system->SetMessageFaceFlipped(com.parameters[2] != 0);
	return true;
}

void Game_Interpreter::SetupChoices(const std::vector<std::string>& choices, int indent, PendingMessage& pm) {
	// Set choices to message text
	pm.SetChoiceResetColors(false);
	for (int i = 0; i < 4 && i < static_cast<int>(choices.size()); i++) {
		pm.PushChoice(choices[i]);
	}

	pm.SetChoiceContinuation([this, indent](int choice_result) {
		SetSubcommandIndex(indent, choice_result);
		return AsyncOp();
	});

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(indent);
}

bool Game_Interpreter::CommandShowChoices(lcf::rpg::EventCommand const& com) { // code 10140
	auto& index = GetFrame().current_command;

	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	auto pm = PendingMessage();
	pm.SetIsEventMessage(true);

	// Choices setup
	std::vector<std::string> choices = GetChoices(4);
	pm.SetChoiceCancelType(com.parameters[0]);
	SetupChoices(choices, com.indent, pm);

	Game_Message::SetPendingMessage(std::move(pm));
	_state.show_message = true;

	++index;
	return false;
}


bool Game_Interpreter::CommandShowChoiceOption(lcf::rpg::EventCommand const& com) { //code 20140
	const auto opt_sub_idx = com.parameters[0];
	return CommandOptionGeneric(com, opt_sub_idx, {Cmd::ShowChoiceOption, Cmd::ShowChoiceEnd});
}

bool Game_Interpreter::CommandShowChoiceEnd(lcf::rpg::EventCommand const& /* com */) { //code 20141
	return true;
}


bool Game_Interpreter::CommandInputNumber(lcf::rpg::EventCommand const& com) { // code 10150
	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	auto pm = PendingMessage();
	pm.SetIsEventMessage(true);

	int variable_id = com.parameters[1];
	int digits = com.parameters[0];

	pm.PushNumInput(variable_id, digits);

	Game_Message::SetPendingMessage(std::move(pm));
	_state.show_message = true;

	// Continue
	return true;
}

bool Game_Interpreter::CommandControlSwitches(lcf::rpg::EventCommand const& com) { // code 10210
	if (com.parameters[0] >= 0 && com.parameters[0] <= 2) {
		// Param0: 0: Single, 1: Range, 2: Indirect
		// For Range set end to param 2, otherwise to start, this way the loop runs exactly once

		int start = com.parameters[0] == 2 ? Main_Data::game_variables->Get(com.parameters[1]) : com.parameters[1];
		int end = com.parameters[0] == 1 ? com.parameters[2] : start;
		int val = com.parameters[3];

		if (start == end) {
			if (val < 2) {
				Main_Data::game_switches->Set(start, val == 0);
			} else {
				Main_Data::game_switches->Flip(start);
			}
		} else {
			if (val < 2) {
				Main_Data::game_switches->SetRange(start, end, val == 0);
			} else {
				Main_Data::game_switches->FlipRange(start, end);
			}
		}

		Game_Map::SetNeedRefresh(true);
	}

	return true;
}

bool Game_Interpreter::CommandControlVariables(lcf::rpg::EventCommand const& com) { // code 10220
	int value = 0;

	Game_Actor* actor = nullptr;
	Game_Character* character = nullptr;

	switch (com.parameters[4]) {
		case 0:
			// Constant
			value = com.parameters[5];
			break;
		case 1:
			// Var A ops B
			value = Main_Data::game_variables->Get(com.parameters[5]);
			break;
		case 2:
			// Number of var A ops B
			value = Main_Data::game_variables->Get(Main_Data::game_variables->Get(com.parameters[5]));
			break;
		case 3: {
			// Random between range
			int rmax = max(com.parameters[5], com.parameters[6]);
			int rmin = min(com.parameters[5], com.parameters[6]);
			value = Rand::GetRandomNumber(rmin, rmax);
			break;
		}
		case 4:
			// Items
			switch (com.parameters[6]) {
				case 0:
					// Number of items posessed
					value = Main_Data::game_party->GetItemCount(com.parameters[5]);
					break;
				case 1:
					// How often the item is equipped
					value = Main_Data::game_party->GetEquippedItemCount(com.parameters[5]);
					break;
			}
			break;
		case 5:
			// Hero
			actor = Main_Data::game_actors->GetActor(com.parameters[5]);

			if (!actor) {
				Output::Warning("ControlVariables: Invalid actor ID {}", com.parameters[5]);
				return true;
			}

			switch (com.parameters[6]) {
				case 0:
					// Level
					value = actor->GetLevel();
					break;
				case 1:
					// Experience
					value = actor->GetExp();
					break;
				case 2:
					// Current HP
					value = actor->GetHp();
					break;
				case 3:
					// Current MP
					value = actor->GetSp();
					break;
				case 4:
					// Max HP
					value = actor->GetMaxHp();
					break;
				case 5:
					// Max MP
					value = actor->GetMaxSp();
					break;
				case 6:
					// Attack
					value = actor->GetAtk();
					break;
				case 7:
					// Defense
					value = actor->GetDef();
					break;
				case 8:
					// Intelligence
					value = actor->GetSpi();
					break;
				case 9:
					// Agility
					value = actor->GetAgi();
					break;
				case 10:
					// Weapon ID
					value = actor->GetWeaponId();
					break;
				case 11:
					// Shield ID
					value = actor->GetShieldId();
					break;
				case 12:
					// Armor ID
					value = actor->GetArmorId();
					break;
				case 13:
					// Helmet ID
					value = actor->GetHelmetId();
					break;
				case 14:
					// Accesory ID
					value = actor->GetAccessoryId();
					break;
			}
			break;
		case 6:
			// Characters
			character = GetCharacter(com.parameters[5]);
			if (character != NULL) {
				int event_id = com.parameters[5];
				switch (com.parameters[6]) {
					case 0:
						// Map ID
						if (!Player::IsRPG2k()
								|| event_id == Game_Character::CharPlayer
								|| event_id == Game_Character::CharBoat
								|| event_id == Game_Character::CharShip
								|| event_id == Game_Character::CharAirship) {
							value = character->GetMapId();
						} else {
							// This is an RPG_RT bug for 2k only. Requesting the map id of an event always returns 0.
							value = 0;
						}
						break;
					case 1:
						// X Coordinate
						value = character->GetX();
						break;
					case 2:
						// Y Coordinate
						value = character->GetY();
						break;
					case 3:
						// Orientation
						int dir;
						dir = character->GetFacing();
						value = dir == 0 ? 8 :
								dir == 1 ? 6 :
								dir == 2 ? 2 : 4;
						break;
					case 4:
						// Screen X
						value = character->GetScreenX();
						break;
					case 5:
						// Screen Y
						value = character->GetScreenY();
				}
			}
			break;
		case 7:
			// More
			switch (com.parameters[5]) {
				case 0:
					// Gold
					value = Main_Data::game_party->GetGold();
					break;
				case 1:
					// Timer 1 remaining time
					value = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer1);
					break;
				case 2:
					// Number of heroes in party
					value = Main_Data::game_party->GetActors().size();
					break;
				case 3:
					// Number of saves
					value = Main_Data::game_system->GetSaveCount();
					break;
				case 4:
					// Number of battles
					value = Main_Data::game_party->GetBattleCount();
					break;
				case 5:
					// Number of wins
					value = Main_Data::game_party->GetWinCount();
					break;
				case 6:
					// Number of defeats
					value = Main_Data::game_party->GetDefeatCount();
					break;
				case 7:
					// Number of escapes (aka run away)
					value = Main_Data::game_party->GetRunCount();
					break;
				case 8:
					// MIDI play position
					value = Main_Data::game_ineluki->GetMidiTicks();
					break;
				case 9:
					// Timer 2 remaining time
					value = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer2);
					break;
				case 10:
					// Current date (YYMMDD)
					if (Player::IsPatchManiac()) {
						std::time_t t = std::time(nullptr);
						std::tm* tm = std::localtime(&t);
						value = atoi(Utils::FormatDate(tm, Utils::DateFormat_YYMMDD).c_str());
					}
					break;
				case 11:
					// Current time (HHMMSS)
					if (Player::IsPatchManiac()) {
						std::time_t t = std::time(nullptr);
						std::tm* tm = std::localtime(&t);
						value = atoi(Utils::FormatDate(tm, Utils::DateFormat_HHMMSS).c_str());
					}
					break;
				case 12:
					// Frames
					if (Player::IsPatchManiac()) {
						value = Main_Data::game_system->GetFrameCounter();
					}
					break;
				case 13:
					// Patch version
					if (Player::IsPatchManiac()) {
						// Latest version before the engine rewrite
						value = 200128;
					}
					break;
			}
			break;
		case 8:
			// Battle related
			if (Main_Data::game_enemyparty.get()->GetBattlerCount() < com.parameters[5]) {
				break;
			}

			switch (com.parameters[6]) {
				case 0:
					// Enemy HP
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetHp();
					break;
				case 1:
					// Enemy SP
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetSp();
					break;
				case 2:
					// Enemy MaxHP
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetMaxHp();
					break;
				case 3:
					// Enemy MaxSP
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetMaxSp();
					break;
				case 4:
					// Enemy Attack
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetAtk();
					break;
				case 5:
					// Enemy Defense
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetDef();
					break;
				case 6:
					// Enemy Spirit
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetSpi();
					break;
				case 7:
					// Enemy Agility
					value = (*Main_Data::game_enemyparty)[com.parameters[5]].GetAgi();
					break;
			}
		default:
			;
	}

	if (com.parameters[0] >= 0 && com.parameters[0] <= 2) {
		// Param0: 0: Single, 1: Range, 2: Indirect
		// For Range set end to param 2, otherwise to start, this way the loop runs exactly once

		int start = com.parameters[0] == 2 ? Main_Data::game_variables->Get(com.parameters[1]) : com.parameters[1];
		int end = com.parameters[0] == 1 ? com.parameters[2] : start;

		if (start == end) {
			// Single variable case - if this is random value, we already called the RNG earlier.
			switch (com.parameters[3]) {
				case 0:
					Main_Data::game_variables->Set(start, value);
					break;
				case 1:
					Main_Data::game_variables->Add(start, value);
					break;
				case 2:
					Main_Data::game_variables->Sub(start, value);
					break;
				case 3:
					Main_Data::game_variables->Mult(start, value);
					break;
				case 4:
					Main_Data::game_variables->Div(start, value);
					break;
				case 5:
					Main_Data::game_variables->Mod(start, value);
					break;
			}
		} else if (com.parameters[4] == 1) {
			// Multiple variables - Direct variable lookup
			int var_id = com.parameters[5];
			switch (com.parameters[3]) {
				case 0:
					Main_Data::game_variables->SetRangeVariable(start, end, var_id);
					break;
				case 1:
					Main_Data::game_variables->AddRangeVariable(start, end, var_id);
					break;
				case 2:
					Main_Data::game_variables->SubRangeVariable(start, end, var_id);
					break;
				case 3:
					Main_Data::game_variables->MultRangeVariable(start, end, var_id);
					break;
				case 4:
					Main_Data::game_variables->DivRangeVariable(start, end, var_id);
					break;
				case 5:
					Main_Data::game_variables->ModRangeVariable(start, end, var_id);
					break;
			}
		} else if (com.parameters[4] == 2) {
			// Multiple variables - Indirect variable lookup
			int var_id = com.parameters[5];
			switch (com.parameters[3]) {
				case 0:
					Main_Data::game_variables->SetRangeVariableIndirect(start, end, var_id);
					break;
				case 1:
					Main_Data::game_variables->AddRangeVariableIndirect(start, end, var_id);
					break;
				case 2:
					Main_Data::game_variables->SubRangeVariableIndirect(start, end, var_id);
					break;
				case 3:
					Main_Data::game_variables->MultRangeVariableIndirect(start, end, var_id);
					break;
				case 4:
					Main_Data::game_variables->DivRangeVariableIndirect(start, end, var_id);
					break;
				case 5:
					Main_Data::game_variables->ModRangeVariableIndirect(start, end, var_id);
					break;
			}
		} else if (com.parameters[4] == 3) {
			// Multiple variables - random
			int rmax = max(com.parameters[5], com.parameters[6]);
			int rmin = min(com.parameters[5], com.parameters[6]);
			switch (com.parameters[3]) {
				case 0:
					Main_Data::game_variables->SetRangeRandom(start, end, rmin, rmax);
					break;
				case 1:
					Main_Data::game_variables->AddRangeRandom(start, end, rmin, rmax);
					break;
				case 2:
					Main_Data::game_variables->SubRangeRandom(start, end, rmin, rmax);
					break;
				case 3:
					Main_Data::game_variables->MultRangeRandom(start, end, rmin, rmax);
					break;
				case 4:
					Main_Data::game_variables->DivRangeRandom(start, end, rmin, rmax);
					break;
				case 5:
					Main_Data::game_variables->ModRangeRandom(start, end, rmin, rmax);
					break;
			}
		} else {
			// Multiple variables - constant
			switch (com.parameters[3]) {
				case 0:
					Main_Data::game_variables->SetRange(start, end, value);
					break;
				case 1:
					Main_Data::game_variables->AddRange(start, end, value);
					break;
				case 2:
					Main_Data::game_variables->SubRange(start, end, value);
					break;
				case 3:
					Main_Data::game_variables->MultRange(start, end, value);
					break;
				case 4:
					Main_Data::game_variables->DivRange(start, end, value);
					break;
				case 5:
					Main_Data::game_variables->ModRange(start, end, value);
					break;
			}
		}

		Game_Map::SetNeedRefresh(true);
	}

	return true;
}

int Game_Interpreter::OperateValue(int operation, int operand_type, int operand) {
	int value = 0;

	if (operand_type == 0) {
		value = operand;
	} else {
		value = Main_Data::game_variables->Get(operand);
	}

	// Reverse sign of value if operation is substract
	if (operation == 1) {
		value = -value;
	}

	return value;
}

std::vector<Game_Actor*> Game_Interpreter::GetActors(int mode, int id) {
	std::vector<Game_Actor*> actors;
	Game_Actor* actor;

	switch (mode) {
	case 0:
		// Party
		actors = Main_Data::game_party->GetActors();
		break;
	case 1:
		// Hero
		actor = Main_Data::game_actors->GetActor(id);

		if (!actor) {
			Output::Warning("Invalid actor ID {}", id);
			return actors;
		}

		actors.push_back(actor);
		break;
	case 2:
		// Var hero
		actor = Main_Data::game_actors->GetActor(Main_Data::game_variables->Get(id));
		if (!actor) {
			Output::Warning("Invalid actor ID {}", Main_Data::game_variables->Get(id));
			return actors;
		}

		actors.push_back(actor);
		break;
	}

	return actors;
}

Game_Character* Game_Interpreter::GetCharacter(int event_id) const {
	if (event_id == Game_Character::CharThisEvent) {
		event_id = GetThisEventId();
		// Is a common event
		if (event_id == 0) {
			// With no map parent
			Output::Warning("Can't use ThisEvent in common event: Not called from a map event");
			return nullptr;
		}
	}

	Game_Character* ch = Game_Character::GetCharacter(event_id, event_id);
	if (!ch) {
		Output::Warning("Unknown event with id {}", event_id);
	}
	return ch;
}

bool Game_Interpreter::CommandTimerOperation(lcf::rpg::EventCommand const& com) { // code 10230
	int timer_id = (com.parameters.size() <= 5) ? 0 : com.parameters[5];
	int seconds;
	bool visible, battle;

	switch (com.parameters[0]) {
	case 0:
		seconds = ValueOrVariable(com.parameters[1],
			com.parameters[2]);
		Main_Data::game_party->SetTimer(timer_id, seconds);
		break;
	case 1:
		visible = com.parameters[3] != 0;
		battle = com.parameters[4] != 0;
		Main_Data::game_party->StartTimer(timer_id, visible, battle);
		break;
	case 2:
		Main_Data::game_party->StopTimer(timer_id);
		break;
	default:
		return false;
	}
	return true;
}

bool Game_Interpreter::CommandChangeGold(lcf::rpg::EventCommand const& com) { // Code 10310
	int value;
	value = OperateValue(
		com.parameters[0],
		com.parameters[1],
		com.parameters[2]
	);

	Main_Data::game_party->GainGold(value);

	// Continue
	return true;
}

bool Game_Interpreter::CommandChangeItems(lcf::rpg::EventCommand const& com) { // Code 10320
	int value;
	value = OperateValue(
		com.parameters[0],
		com.parameters[3],
		com.parameters[4]
	);

	// Add item can't be used to remove an item and
	// remove item can't be used to add one
	if (com.parameters[0] == 1) {
		// Substract
		if (value > 0) {
			return true;
		}
	} else {
		// Add
		if (value < 0) {
			return true;
		}
	}

	if (com.parameters[1] == 0) {
		// Item by const number
		Main_Data::game_party->AddItem(com.parameters[2], value);
	} else {
		// Item by variable
		Main_Data::game_party->AddItem(
			Main_Data::game_variables->Get(com.parameters[2]),
			value
		);
	}
	Game_Map::SetNeedRefresh(true);
	// Continue
	return true;
}

bool Game_Interpreter::CommandChangePartyMember(lcf::rpg::EventCommand const& com) { // Code 10330
	Game_Actor* actor;
	int id;

	if (com.parameters[1] == 0) {
		id = com.parameters[2];
	} else {
		id = Main_Data::game_variables->Get(com.parameters[2]);
	}

	actor = Main_Data::game_actors->GetActor(id);

	if (!actor) {
		Output::Warning("ChangePartyMember: Invalid actor ID {}", id);
		return true;
	}

	if (com.parameters[0] == 0) {
		// Add members
		Main_Data::game_party->AddActor(id);

	} else {
		// Remove members
		Main_Data::game_party->RemoveActor(id);
	}

	CheckGameOver();
	Game_Map::SetNeedRefresh(true);

	// Continue
	return true;
}

void Game_Interpreter::ForegroundTextPush(PendingMessage pm) {
	auto& fg_interp = GetForegroundInterpreter();
	fg_interp._state.show_message = true;

	Game_Message::SetPendingMessage(std::move(pm));
}

bool Game_Interpreter::CommandChangeExp(lcf::rpg::EventCommand const& com) { // Code 10410
	bool show_msg = com.parameters[5];

	if (show_msg && !Game_Message::CanShowMessage(true)) {
		return false;
	}
	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	PendingMessage pm;
	pm.SetEnableFace(false);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeExp(actor->GetExp() + value, show_msg ? &pm : nullptr);
	}

	if (CheckGameOver()) {
		return true;
	}

	if (show_msg) {
		ForegroundTextPush(std::move(pm));
	}
	return true;
}

bool Game_Interpreter::CommandChangeLevel(lcf::rpg::EventCommand const& com) { // Code 10420
	bool show_msg = com.parameters[5];

	if (show_msg && !Game_Message::CanShowMessage(true)) {
		return false;
	}

	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	PendingMessage pm;
	pm.SetEnableFace(false);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeLevel(actor->GetLevel() + value, show_msg ? &pm : nullptr);
	}

	if (CheckGameOver()) {
		return true;
	}

	if (show_msg && pm.IsActive()) {
		ForegroundTextPush(std::move(pm));
	}
	return true;
}

int Game_Interpreter::ValueOrVariable(int mode, int val) {
	if (mode == 0) {
		return val;
	} else if (mode == 1) {
		return Main_Data::game_variables->Get(val);
	} else if (Player::IsPatchManiac()) {
		// Maniac Patch does not implement all modes for all commands
		// For simplicity it is enabled for all here
		if (mode == 2) {
			// Variable indirect
			return Main_Data::game_variables->GetIndirect(val);
		} else if (mode == 3) {
			// Switch (F = 0, T = 1)
			return Main_Data::game_switches->GetInt(val);
		} else if (mode == 4) {
			// Switch through Variable (F = 0, T = 1)
			return Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(val));
		}
	}
	return -1;
}

bool Game_Interpreter::CommandChangeParameters(lcf::rpg::EventCommand const& com) { // Code 10430
	int value = OperateValue(
		com.parameters[2],
		com.parameters[4],
		com.parameters[5]
	);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		switch (com.parameters[3]) {
		case 0:
			// Max HP
			actor->SetBaseMaxHp(actor->GetBaseMaxHp() + value);
			break;
		case 1:
			// Max MP
			actor->SetBaseMaxSp(actor->GetBaseMaxSp() + value);
			break;
		case 2:
			// Attack
			actor->SetBaseAtk(actor->GetBaseAtk() + value);
			break;
		case 3:
			// Defense
			actor->SetBaseDef(actor->GetBaseDef() + value);
			break;
		case 4:
			// Spirit
			actor->SetBaseSpi(actor->GetBaseSpi() + value);
			break;
		case 5:
			// Agility
			actor->SetBaseAgi(actor->GetBaseAgi() + value);
			break;
		}
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandChangeSkills(lcf::rpg::EventCommand const& com) { // Code 10440
	bool remove = com.parameters[2] != 0;
	int skill_id = ValueOrVariable(com.parameters[3], com.parameters[4]);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		if (remove)
			actor->UnlearnSkill(skill_id);
		else
			actor->LearnSkill(skill_id, nullptr);
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandChangeEquipment(lcf::rpg::EventCommand const& com) { // Code 10450
	int item_id;
	int slot;
	const lcf::rpg::Item* item;

	switch (com.parameters[2]) {
		case 0:
			item_id = ValueOrVariable(com.parameters[3],
									  com.parameters[4]);
			item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
			if (!item) {
				Output::Warning("ChangeEquipment: Invalid item ID {}", item_id);
				return true;
			}

			switch (item->type) {
				case lcf::rpg::Item::Type_weapon:
				case lcf::rpg::Item::Type_shield:
				case lcf::rpg::Item::Type_armor:
				case lcf::rpg::Item::Type_helmet:
				case lcf::rpg::Item::Type_accessory:
					slot = item->type;
					break;
				default:
					return true;
			}
			break;
		case 1:
			item_id = 0;
			slot = com.parameters[3] + 1;
			break;
		default:
			return false;
	}

	if (slot == 6) {
		for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
			actor->RemoveWholeEquipment();
		}
	} else {
		for (const auto &actor : GetActors(com.parameters[0], com.parameters[1])) {
			if (actor->HasTwoWeapons() && slot == lcf::rpg::Item::Type_shield && item_id != 0) {
				continue;
			}

			if (Main_Data::game_party->GetItemCount(item_id) == 0 && !actor->IsEquipped(item_id)) {
				Main_Data::game_party->AddItem(item_id, 1);
			}

			if (actor->HasTwoWeapons() && slot == lcf::rpg::Item::Type_weapon && item_id != 0) {
				lcf::rpg::Item* new_equipment = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
				lcf::rpg::Item* equipment1 = lcf::ReaderUtil::GetElement(lcf::Data::items, actor->GetWeaponId());
				lcf::rpg::Item* equipment2 = lcf::ReaderUtil::GetElement(lcf::Data::items, actor->GetShieldId());

				if (equipment1 && !equipment2 && !equipment1->two_handed && !new_equipment->two_handed) {
					// Assign to 2nd weapon slot when empty
					actor->ChangeEquipment(slot + 1, item_id);
					continue;
				}
			}

			actor->ChangeEquipment(slot, item_id);
		}
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandChangeHP(lcf::rpg::EventCommand const& com) { // Code 10460
	bool remove = com.parameters[2] != 0;
	int amount = ValueOrVariable(com.parameters[3],
								 com.parameters[4]);
	bool lethal = com.parameters[5] != 0;

	if (remove)
		amount = -amount;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeHp(amount, lethal);

		auto& scene = Scene::instance;
		if (scene) {
			scene->OnEventHpChanged(actor, amount);
		}

		if (actor->IsDead() && actor->GetActorBattleSprite()) {
			actor->GetActorBattleSprite()->DetectStateChange();
		}
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandChangeSP(lcf::rpg::EventCommand const& com) { // Code 10470
	bool remove = com.parameters[2] != 0;
	int amount = ValueOrVariable(com.parameters[3], com.parameters[4]);

	if (remove)
		amount = -amount;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		int sp = actor->GetSp() + amount;
		if (sp < 0)
			sp = 0;
		actor->SetSp(sp);
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandChangeCondition(lcf::rpg::EventCommand const& com) { // Code 10480
	bool remove = com.parameters[2] != 0;
	int state_id = com.parameters[3];

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		if (remove) {
			// RPG_RT: On the map, will remove battle states even if actor has
			// state inflicted by equipment.
			actor->RemoveState(state_id, !Game_Battle::IsBattleRunning());
		} else {
			// RPG_RT always adds states from event commands, even battle states.
			actor->AddState(state_id, true);
		}
		if (actor->GetActorBattleSprite()) {
			actor->GetActorBattleSprite()->DetectStateChange();
		}
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandFullHeal(lcf::rpg::EventCommand const& com) { // Code 10490
	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->FullHeal();
		if (actor->GetActorBattleSprite()) {
			actor->GetActorBattleSprite()->DetectStateChange();
		}
	}

	CheckGameOver();

	return true;
}

bool Game_Interpreter::CommandSimulatedAttack(lcf::rpg::EventCommand const& com) { // code 10500
	int atk = com.parameters[2];
	int def = com.parameters[3];
	int spi = com.parameters[4];
	int var = com.parameters[5];

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		int result = atk;
		result -= (actor->GetDef() * def) / 400;
		result -= (actor->GetSpi() * spi) / 800;
		result = std::max(result, 0);
		result = Algo::VarianceAdjustEffect(result, var);

		result = std::max(0, result);
		actor->ChangeHp(-result, true);

		if (com.parameters[6] != 0) {
			Main_Data::game_variables->Set(com.parameters[7], result);
			Game_Map::SetNeedRefresh(true);
		}
	}

	CheckGameOver();
	return true;
}

bool Game_Interpreter::CommandWait(lcf::rpg::EventCommand const& com) { // code 11410
	auto& index = GetFrame().current_command;

	// Wait a given time
	if (com.parameters.size() <= 1 ||
		(com.parameters.size() > 1 && com.parameters[1] == 0)) {
		SetupWait(com.parameters[0]);
		return true;
	}

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	// Wait until decision key pressed, but skip the first frame so that
	// it ignores keys that were pressed before this command started.
	// FIXME: Is this behavior correct?
	_state.wait_key_enter = true;
	++index;
	return false;
}

bool Game_Interpreter::CommandPlayBGM(lcf::rpg::EventCommand const& com) { // code 11510
	lcf::rpg::Music music;
	music.name = ToString(com.string);
	music.fadein = com.parameters[0];
	music.volume = com.parameters[1];
	music.tempo = com.parameters[2];
	music.balance = com.parameters[3];
	Main_Data::game_system->BgmPlay(music);
	return true;
}

bool Game_Interpreter::CommandFadeOutBGM(lcf::rpg::EventCommand const& com) { // code 11520
	int fadeout = com.parameters[0];
	Main_Data::game_system->BgmFade(fadeout);
	return true;
}

bool Game_Interpreter::CommandPlaySound(lcf::rpg::EventCommand const& com) { // code 11550
	lcf::rpg::Sound sound;
	sound.name = ToString(com.string);
	sound.volume = com.parameters[0];
	sound.tempo = com.parameters[1];
	sound.balance = com.parameters[2];
	Main_Data::game_system->SePlay(sound, true);
	return true;
}

bool Game_Interpreter::CommandEndEventProcessing(lcf::rpg::EventCommand const& /* com */) { // code 12310
	EndEventProcessing();
	return true;
}

bool Game_Interpreter::CommandComment(const lcf::rpg::EventCommand &com) {
	if (Player::IsPatchDynRpg()) {
		if (com.string.empty() || com.string[0] != '@') {
			// Not a DynRPG command
			return true;
		}

		auto& frame = GetFrame();
		const auto& list = frame.commands;
		auto& index = frame.current_command;

		std::string command = ToString(com.string);
		// Concat everything that is not another command or a new comment block
		for (size_t i = index + 1; i < list.size(); ++i) {
			const auto& cmd = list[i];
			if (cmd.code == static_cast<uint32_t>(Cmd::Comment_2) &&
					!cmd.string.empty() && cmd.string[0] != '@') {
				command += ToString(cmd.string);
			} else {
				break;
			}
		}

		return DynRpg::Invoke(command);
	}
	return true;
}

void Game_Interpreter::EndEventProcessing() {
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	index = static_cast<int>(list.size());
}

bool Game_Interpreter::CommandGameOver(lcf::rpg::EventCommand const& /* com */) { // code 12420
	auto& index = GetFrame().current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Gameover>());
	++index;
	return false;
}

bool Game_Interpreter::CommandChangeHeroName(lcf::rpg::EventCommand const& com) { // code 10610
	Game_Actor* actor = Main_Data::game_actors->GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeHeroName: Invalid actor ID {}", com.parameters[0]);
		return true;
	}

	actor->SetName(ToString(com.string));
	return true;
}

bool Game_Interpreter::CommandChangeHeroTitle(lcf::rpg::EventCommand const& com) { // code 10620
	Game_Actor* actor = Main_Data::game_actors->GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeHeroTitle: Invalid actor ID {}", com.parameters[0]);
		return true;
	}

	actor->SetTitle(ToString(com.string));
	return true;
}

bool Game_Interpreter::CommandChangeSpriteAssociation(lcf::rpg::EventCommand const& com) { // code 10630
	Game_Actor* actor = Main_Data::game_actors->GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeSpriteAssociation: Invalid actor ID {}", com.parameters[0]);
		return true;
	}

	auto file = ToString(com.string);
	int idx = com.parameters[1];
	bool transparent = com.parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	Main_Data::game_player->ResetGraphic();
	return true;
}

bool Game_Interpreter::CommandChangeActorFace(lcf::rpg::EventCommand const& com) { // code 10640
	Game_Actor* actor = Main_Data::game_actors->GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("CommandChangeActorFace: Invalid actor ID {}", com.parameters[0]);
		return true;
	}

	actor->SetFace(ToString(com.string), com.parameters[1]);
	return true;
}

bool Game_Interpreter::CommandChangeVehicleGraphic(lcf::rpg::EventCommand const& com) { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0] + 1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

	if (!vehicle) {
		Output::Warning("ChangeVehicleGraphic: Invalid vehicle ID {}", vehicle_id);
		return true;
	}

	const std::string& name = ToString(com.string);
	int vehicle_index = com.parameters[1];

	vehicle->SetSpriteGraphic(name, vehicle_index);
	vehicle->SetOrigSpriteGraphic(name, vehicle_index);

	return true;
}

bool Game_Interpreter::CommandChangeSystemBGM(lcf::rpg::EventCommand const& com) { //code 10660
	lcf::rpg::Music music;
	int context = com.parameters[0];
	music.name = ToString(com.string);
	music.fadein = com.parameters[1];
	music.volume = com.parameters[2];
	music.tempo = com.parameters[3];
	music.balance = com.parameters[4];
	Main_Data::game_system->SetSystemBGM(context, std::move(music));
	return true;
}

bool Game_Interpreter::CommandChangeSystemSFX(lcf::rpg::EventCommand const& com) { //code 10670
	lcf::rpg::Sound sound;
	int context = com.parameters[0];
	sound.name = ToString(com.string);
	sound.volume = com.parameters[1];
	sound.tempo = com.parameters[2];
	sound.balance = com.parameters[3];
	Main_Data::game_system->SetSystemSE(context, std::move(sound));
	return true;
}

bool Game_Interpreter::CommandChangeSystemGraphics(lcf::rpg::EventCommand const& com) { // code 10680
	Main_Data::game_system->SetSystemGraphic(ToString(com.string),
			static_cast<lcf::rpg::System::Stretch>(com.parameters[0]),
			static_cast<lcf::rpg::System::Font>(com.parameters[1]));

	return true;
}

bool Game_Interpreter::CommandChangeScreenTransitions(lcf::rpg::EventCommand const& com) { // code 10690
	Main_Data::game_system->SetTransition(com.parameters[0], com.parameters[1]);
	return true;
}

bool Game_Interpreter::CommandMemorizeLocation(lcf::rpg::EventCommand const& com) { // code 10820
	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	Main_Data::game_variables->Set(var_map_id, Game_Map::GetMapId());
	Main_Data::game_variables->Set(var_x, player->GetX());
	Main_Data::game_variables->Set(var_y, player->GetY());
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter::CommandSetVehicleLocation(lcf::rpg::EventCommand const& com) { // code 10850
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0] + 1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

	if (!vehicle) {
		// SetVehicleLocation moves the party, too, when she is in the referenced
		// vehicle. In RPG_RT a party that is in no vehicle has the vehicle_id -1.
		// Due to this implementation detail passing -1 as vehicle_id will move the
		// party instead.
		if (vehicle_id == 0) {
			// 0 because we adjust all vehicle IDs by +1 to match the lcf values
			Output::Debug("SetVehicleLocation: Party referenced");
		} else {
			Output::Warning("SetVehicleLocation: Invalid vehicle ID {}", vehicle_id);
			return true;
		}
	}

	int map_id = ValueOrVariable(com.parameters[1], com.parameters[2]);
	int x = ValueOrVariable(com.parameters[1], com.parameters[3]);
	int y = ValueOrVariable(com.parameters[1], com.parameters[4]);

	// Check if the party is in the current vehicle and transfer the party together with it
	if (Main_Data::game_player->GetVehicle() == vehicle) {
		if (map_id == Game_Map::GetMapId()) {
			if (vehicle) {
				vehicle->MoveTo(map_id, x, y);
			}
			Main_Data::game_player->MoveTo(map_id, x, y);
			return true;
		};

		// This implements a bug in RPG_RT which allows moving the party to a new map while boarded (or when using -1)
		// without doing a teleport + transition.
		// In player we implement this as an async "Quick Teleport" which immediately switches to
		// the other map with no transition and no change in screen effects such as pictures and
		// battle animations.

		if (vehicle) {
			vehicle->MoveTo(map_id, x, y);
		}

		auto event_id = GetOriginalEventId();
		if (!main_flag && event_id != 0) {
			Output::Error("VehicleTeleport not allowed from parallel map event! Id={}", event_id);
		}

		_async_op = AsyncOp::MakeQuickTeleport(map_id, x, y);
	} else if (vehicle) {
		vehicle->MoveTo(map_id, x, y);
	}

	return true;
}

bool Game_Interpreter::CommandChangeEventLocation(lcf::rpg::EventCommand const& com) { // Code 10860
	int event_id = com.parameters[0];
	Game_Character *event = GetCharacter(event_id);
	if (event != NULL) {
		const auto x = ValueOrVariable(com.parameters[1], com.parameters[2]);
		const auto y = ValueOrVariable(com.parameters[1], com.parameters[3]);
		event->MoveTo(event->GetMapId(), x, y);

		// RPG2k3 feature
		int direction = com.parameters.size() > 4 ? com.parameters[4] - 1 : -1;
		// Only for the constant case, not for variables
		if (com.parameters[1] == 0 && direction != -1) {
			event->SetDirection(direction);
			event->UpdateFacing();
		}
	}
	return true;
}

bool Game_Interpreter::CommandTradeEventLocations(lcf::rpg::EventCommand const& com) { // Code 10870
	int event1_id = com.parameters[0];
	int event2_id = com.parameters[1];

	Game_Character *event1 = GetCharacter(event1_id);
	Game_Character *event2 = GetCharacter(event2_id);

	if (event1 != NULL && event2 != NULL) {
		auto m1 = event1->GetMapId();
		auto x1 = event1->GetX();
		auto y1 = event1->GetY();

		auto m2 = event2->GetMapId();
		auto x2 = event2->GetX();
		auto y2 = event2->GetY();

		event1->MoveTo(m2, x2, y2);
		event2->MoveTo(m1, x1, y1);
	}

	return true;
}

bool Game_Interpreter::CommandStoreTerrainID(lcf::rpg::EventCommand const& com) { // code 10820
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	Main_Data::game_variables->Set(var_id, Game_Map::GetTerrainTag(x, y));
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter::CommandStoreEventID(lcf::rpg::EventCommand const& com) { // code 10920
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	auto* ev = Game_Map::GetEventAt(x, y, false);
	Main_Data::game_variables->Set(var_id, ev ? ev->GetId() : 0);
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter::CommandEraseScreen(lcf::rpg::EventCommand const& com) { // code 11010
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	// Emulates RPG_RT behavior where any transition out is skipped when these scenes are pending.
	auto st = Scene::instance->GetRequestedSceneType();
	if (st == Scene::Battle || st == Scene::Gameover) {
		return true;
	}

	// Transition commands in battle have glitchy behavior in RPG_RT, but they don't affect the map.
	// We disable in them in Player.
	if (Game_Battle::IsBattleRunning()) {
		return true;
	}

	int tt = Transition::TransitionNone;

	switch (com.parameters[0]) {
	case -1:
		tt = Main_Data::game_system->GetTransition(Main_Data::game_system->Transition_TeleportErase);
		break;
	case 0:
		tt = Transition::TransitionFadeOut;
		break;
	case 1:
		tt = Transition::TransitionRandomBlocks;
		break;
	case 2:
		tt = Transition::TransitionRandomBlocksDown;
		break;
	case 3:
		tt = Transition::TransitionRandomBlocksUp;
		break;
	case 4:
		tt = Transition::TransitionBlindClose;
		break;
	case 5:
		tt = Transition::TransitionVerticalStripesOut;
		break;
	case 6:
		tt = Transition::TransitionHorizontalStripesOut;
		break;
	case 7:
		tt = Transition::TransitionBorderToCenterOut;
		break;
	case 8:
		tt = Transition::TransitionCenterToBorderOut;
		break;
	case 9:
		tt = Transition::TransitionScrollUpOut;
		break;
	case 10:
		tt = Transition::TransitionScrollDownOut;
		break;
	case 11:
		tt = Transition::TransitionScrollLeftOut;
		break;
	case 12:
		tt = Transition::TransitionScrollRightOut;
		break;
	case 13:
		tt = Transition::TransitionVerticalDivision;
		break;
	case 14:
		tt = Transition::TransitionHorizontalDivision;
		break;
	case 15:
		tt = Transition::TransitionCrossDivision;
		break;
	case 16:
		tt = Transition::TransitionZoomIn;
		break;
	case 17:
		tt = Transition::TransitionMosaicOut;
		break;
	case 18:
		tt = Transition::TransitionWaveOut;
		break;
	case 19:
		tt = Transition::TransitionCutOut;
		break;
	default:
		tt = Transition::TransitionNone;
		break;
	}

	_async_op = AsyncOp::MakeEraseScreen(tt);

	return true;
}

bool Game_Interpreter::CommandShowScreen(lcf::rpg::EventCommand const& com) { // code 11020
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	// Transition commands in battle have glitchy behavior in RPG_RT, but they don't affect the map.
	// We disable in them in Player.
	if (Game_Battle::IsBattleRunning()) {
		return true;
	}

	int tt = Transition::TransitionNone;

	switch (com.parameters[0]) {
	case -1:
		tt = Main_Data::game_system->GetTransition(Main_Data::game_system->Transition_TeleportShow);
		break;
	case 0:
		tt = Transition::TransitionFadeIn;
		break;
	case 1:
		tt = Transition::TransitionRandomBlocks;
		break;
	case 2:
		tt = Transition::TransitionRandomBlocksDown;
		break;
	case 3:
		tt = Transition::TransitionRandomBlocksUp;
		break;
	case 4:
		tt = Transition::TransitionBlindOpen;
		break;
	case 5:
		tt = Transition::TransitionVerticalStripesIn;
		break;
	case 6:
		tt = Transition::TransitionHorizontalStripesIn;
		break;
	case 7:
		tt = Transition::TransitionBorderToCenterIn;
		break;
	case 8:
		tt = Transition::TransitionCenterToBorderIn;
		break;
	case 9:
		tt = Transition::TransitionScrollUpIn;
		break;
	case 10:
		tt = Transition::TransitionScrollDownIn;
		break;
	case 11:
		tt = Transition::TransitionScrollLeftIn;
		break;
	case 12:
		tt = Transition::TransitionScrollRightIn;
		break;
	case 13:
		tt = Transition::TransitionVerticalCombine;
		break;
	case 14:
		tt = Transition::TransitionHorizontalCombine;
		break;
	case 15:
		tt = Transition::TransitionCrossCombine;
		break;
	case 16:
		tt = Transition::TransitionZoomOut;
		break;
	case 17:
		tt = Transition::TransitionMosaicIn;
		break;
	case 18:
		tt = Transition::TransitionWaveIn;
		break;
	case 19:
		tt = Transition::TransitionCutIn;
		break;
	default:
		tt = Transition::TransitionNone;
		break;
	}

	_async_op = AsyncOp::MakeShowScreen(tt);
	return true;
}

bool Game_Interpreter::CommandTintScreen(lcf::rpg::EventCommand const& com) { // code 11030
	Game_Screen* screen = Main_Data::game_screen.get();
	int r = com.parameters[0];
	int g = com.parameters[1];
	int b = com.parameters[2];
	int s = com.parameters[3];
	int tenths = com.parameters[4];
	bool wait = com.parameters[5] != 0;

	screen->TintScreen(r, g, b, s, tenths * DEFAULT_FPS / 10);

	if (wait)
		SetupWait(tenths);

	return true;
}

bool Game_Interpreter::CommandFlashScreen(lcf::rpg::EventCommand const& com) { // code 11040
	Game_Screen* screen = Main_Data::game_screen.get();
	int r = com.parameters[0];
	int g = com.parameters[1];
	int b = com.parameters[2];
	int s = com.parameters[3];
	int tenths = com.parameters[4];
	bool wait = com.parameters[5] != 0;

	if (com.parameters.size() <= 6) {
		screen->FlashOnce(r, g, b, s, tenths * DEFAULT_FPS / 10);
		if (wait)
			SetupWait(tenths);
	} else {
		switch (com.parameters[6]) {
		case 0:
			screen->FlashOnce(r, g, b, s, tenths * DEFAULT_FPS / 10);
			if (wait)
				SetupWait(tenths);
			break;
		case 1:
			screen->FlashBegin(r, g, b, s, tenths * DEFAULT_FPS / 10);
			break;
		case 2:
			screen->FlashEnd();
			break;
		}
	}

	return true;
}

bool Game_Interpreter::CommandShakeScreen(lcf::rpg::EventCommand const& com) { // code 11050
	Game_Screen* screen = Main_Data::game_screen.get();
	int strength = com.parameters[0];
	int speed = com.parameters[1];
	int tenths = com.parameters[2];
	bool wait = com.parameters[3] != 0;
	// params array is size 4 in 2k and 2k games ported to 2k3.
	int shake_cmd = com.parameters.size() > 4 ?
		com.parameters[4] : 0;

	switch (shake_cmd) {
		case 0:
			screen->ShakeOnce(strength, speed, tenths * DEFAULT_FPS / 10);
			if (wait) {
				SetupWait(tenths);
			}
			break;
		case 1:
			screen->ShakeBegin(strength, speed);
			break;
		case 2:
			screen->ShakeEnd();
			break;
	}

	return true;
}

bool Game_Interpreter::CommandWeatherEffects(lcf::rpg::EventCommand const& com) { // code 11070
	Game_Screen* screen = Main_Data::game_screen.get();
	int type = com.parameters[0];
	int str = com.parameters[1];
	// Few games use a greater strength value to achieve more intense but glichty weather
	int strength = std::min(str, 2);
	screen->SetWeatherEffect(type, strength);
	return true;
}


// PicPointerPatch handling.
// See http://cherrytree.at/cms/download/?did=19
namespace PicPointerPatch {

	static void AdjustId(int& pic_id) {
		if (pic_id > 10000) {
			int new_id;
			if (pic_id > 50000) {
				new_id = Main_Data::game_variables->Get(pic_id - 50000);
			} else {
				new_id = Main_Data::game_variables->Get(pic_id - 10000);
			}

			if (new_id > 0) {
				Output::Debug("PicPointer: ID {} replaced with ID {}", pic_id, new_id);
				pic_id = new_id;
			}
		}
	}

	static void AdjustParams(Game_Pictures::Params& params) {
		if (params.magnify > 10000) {
			int new_magnify = Main_Data::game_variables->Get(params.magnify - 10000);
			Output::Debug("PicPointer: Zoom {} replaced with {}", params.magnify, new_magnify);
			params.magnify = new_magnify;
		}

		if (params.top_trans > 10000) {
			int new_top_trans = Main_Data::game_variables->Get(params.top_trans - 10000);
			Output::Debug("PicPointer: Top transparency {} replaced with {}", params.top_trans, new_top_trans);
			params.top_trans = new_top_trans;
		}

		if (params.bottom_trans > 10000) {
			int new_bottom_trans = Main_Data::game_variables->Get(params.bottom_trans - 10000);
			Output::Debug("PicPointer: Bottom transparency {} replaced with {}", params.bottom_trans, new_bottom_trans);
			params.bottom_trans = new_bottom_trans;
		}
	}

	static std::string ReplaceName(const std::string& str, int value, int digits) {
		// Replace last 4 characters with 0-padded pic_num
		std::u32string u_pic_name = Utils::DecodeUTF32(str);

		// Out of bounds test
		if ((int)u_pic_name.length() < digits) {
			return str;
		}

		// No substitution required
		if (digits == 0) {
			return str;
		}

		std::string new_pic_name = Utils::EncodeUTF(u_pic_name.substr(0, u_pic_name.size() - digits));
		std::stringstream ss;
		ss << new_pic_name << std::setfill('0') << std::setw(digits) << value;
		new_pic_name = ss.str();

		if (!Player::IsRPG2k3E()) {
			// Prevent debug messages because this function is used by ShowPicture of RPG2k3E
			Output::Debug("PicPointer: File {} replaced with {}", str, new_pic_name);
		}
		return new_pic_name;
	}

	static void AdjustShowParams(int& pic_id, Game_Pictures::ShowParams& params) {
		// Adjust name
		if (pic_id >= 50000) {
			// Name substitution is pic_id + 1
			int pic_num = Main_Data::game_variables->Get(pic_id - 50000 + 1);

			if (pic_num >= 0) {
				params.name = ReplaceName(params.name, pic_num, 4);
			}
		}

		AdjustId(pic_id);
		AdjustParams(params);
	}

	static void AdjustMoveParams(int& pic_id, Game_Pictures::MoveParams& params) {
		AdjustId(pic_id);
		AdjustParams(params);

		if (params.duration > 10000) {
			int new_duration = Main_Data::game_variables->Get(params.duration - 10000);
			Output::Debug("PicPointer: Move duration {} replaced with {}", params.duration, new_duration);
			params.duration = new_duration;
		}
	}

}

bool Game_Interpreter::CommandShowPicture(lcf::rpg::EventCommand const& com) { // code 11110
	// Older versions of RPG_RT block pictures when message active.
	if (!Player::IsEnglish() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	Game_Pictures::ShowParams params = {};
	params.name = ToString(com.string);
	// Maniac Patch uses the upper bits for X/Y origin, mask it away
	params.position_x = ValueOrVariable(com.parameters[1] & 0xFF, com.parameters[2]);
	params.position_y = ValueOrVariable(com.parameters[1] & 0xFF, com.parameters[3]);
	params.fixed_to_map = com.parameters[4] > 0;
	params.magnify = com.parameters[5];
	params.use_transparent_color = com.parameters[7] > 0;
	params.top_trans = com.parameters[6];
	params.red = com.parameters[8];
	params.green = com.parameters[9];
	params.blue = com.parameters[10];
	params.saturation = com.parameters[11];
	params.effect_mode = com.parameters[12];
	params.effect_power = com.parameters[13];

	size_t param_size = com.parameters.size();

	if (param_size > 14) {
		// RPG2k3 sets this chunk. Versions < 1.12 let you specify separate top and bottom
		// transparency. >= 1.12 Editor only let you set one transparency field but it affects
		// both chunks here.
		// Maniac Patch uses the upper bits for flags, mask it away
		params.bottom_trans = com.parameters[14] & 0xFF;
	} else if (Player::IsRPG2k3() && !Player::IsRPG2k3E()) {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		params.bottom_trans = params.top_trans;
	}

	if (param_size > 16) {
		// Handling of RPG2k3 1.12 chunks
		pic_id = ValueOrVariable(com.parameters[17], pic_id);
		if (com.parameters[19] != 0) {
			int var = 0;
			if (Main_Data::game_variables->IsValid(com.parameters[19])) {
				var = Main_Data::game_variables->Get(com.parameters[19]);
			}
			params.name = PicPointerPatch::ReplaceName(params.name, var, com.parameters[18]);
		}
		params.magnify = ValueOrVariable(com.parameters[20], params.magnify);
		params.top_trans = ValueOrVariable(com.parameters[21], params.top_trans);
		if (com.parameters[22] > 0) {
			// If spritesheet is enabled
			params.spritesheet_cols = com.parameters[22];
			params.spritesheet_rows = com.parameters[23];

			// Animate and index selection are exclusive
			if (com.parameters[24] == 2) {
				params.spritesheet_speed = com.parameters[25];
				params.spritesheet_play_once = com.parameters[26];
			} else {
				// Picture data / LSD data frame number is 0 based, while event parameter counts from 1.
				params.spritesheet_frame = ValueOrVariable(com.parameters[24], com.parameters[25]) - 1;
			}
		}

		params.map_layer = com.parameters[27];
		params.battle_layer = com.parameters[28];
		params.flags = com.parameters[29];

		if (Player::IsPatchManiac()) {
			int flags = com.parameters[14] >> 8;
			int blend_mode = flags & 3;
			if (blend_mode == 1) {
				params.blend_mode = (int)Bitmap::BlendMode::Multiply;
			} else if (blend_mode == 2) {
				params.blend_mode = (int)Bitmap::BlendMode::Additive;
			} else if (blend_mode == 3) {
				params.blend_mode = (int)Bitmap::BlendMode::Overlay;
			}
			params.flip_x = (flags & 16) == 16;
			params.flip_y = (flags & 32) == 32;

			if ((com.parameters[1] >> 8) != 0) {
				Output::Warning("Maniac ShowPicture: X/Y origin not supported");
			}

			if (params.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
				Output::Warning("Maniac ShowPicture: Fixed angle not supported");
				params.effect_mode = lcf::rpg::SavePicture::Effect_none;
			}
		}
	}

	PicPointerPatch::AdjustShowParams(pic_id, params);

	// Sanitize input
	params.magnify = std::max(0, std::min(params.magnify, 2000));
	params.top_trans = std::max(0, std::min(params.top_trans, 100));
	params.bottom_trans = std::max(0, std::min(params.bottom_trans, 100));

	if (pic_id <= 0) {
		Output::Error("ShowPicture: Requested invalid picture id ({})", pic_id);
	}

	// RPG_RT will crash if you ask for a picture id greater than the limit that
	// version of the engine allows. We allow an arbitrary number of pictures in Player.

	Main_Data::game_pictures->Show(pic_id, params);

	return true;
}

bool Game_Interpreter::CommandMovePicture(lcf::rpg::EventCommand const& com) { // code 11120
	// Older versions of RPG_RT block pictures when message active.
	if (!Player::IsEnglish() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	Game_Pictures::MoveParams params;
	// Maniac Patch uses the upper bits for X/Y origin, mask it away
	params.position_x = ValueOrVariable(com.parameters[1] & 0xFF, com.parameters[2]);
	params.position_y = ValueOrVariable(com.parameters[1] & 0xFF, com.parameters[3]);
	params.magnify = com.parameters[5];
	params.top_trans = com.parameters[6];
	params.red = com.parameters[8];
	params.green = com.parameters[9];
	params.blue = com.parameters[10];
	params.saturation = com.parameters[11];
	params.effect_mode = com.parameters[12];
	params.effect_power = com.parameters[13];
	params.duration = com.parameters[14];

	bool wait = com.parameters[15] != 0;

	size_t param_size = com.parameters.size();

	if (Player::IsRPG2k() || Player::IsRPG2k3E()) {
		if (param_size > 17) {
			// Handling of RPG2k3 1.12 chunks
			pic_id = ValueOrVariable(com.parameters[17], pic_id);
			// Currently unused by RPG Maker
			//int chars_to_replace = com.parameters[18];
			//int replace_with = com.parameters[19];
			params.magnify = ValueOrVariable(com.parameters[20], params.magnify);
			params.top_trans = ValueOrVariable(com.parameters[21], params.top_trans);
		}

		// RPG2k and RPG2k3 1.10 do not support this option
		params.bottom_trans = params.top_trans;

		if (Player::IsPatchManiac()) {
			int flags = com.parameters[16] >> 8;
			int blend_mode = flags & 3;
			if (blend_mode == 1) {
				params.blend_mode = (int)Bitmap::BlendMode::Multiply;
			} else if (blend_mode == 2) {
				params.blend_mode = (int)Bitmap::BlendMode::Additive;
			} else if (blend_mode == 3) {
				params.blend_mode = (int)Bitmap::BlendMode::Overlay;
			}
			params.flip_x = (flags & 16) == 16;
			params.flip_y = (flags & 32) == 32;

			if ((com.parameters[1] >> 8) != 0) {
				Output::Warning("Maniac MovePicture: X/Y origin not supported");
			}

			if (params.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
				Output::Warning("Maniac MovePicture: Fixed angle not supported");
				params.effect_mode = lcf::rpg::SavePicture::Effect_none;
			}
		}
	} else {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		params.bottom_trans = param_size > 16 ? com.parameters[16] : params.top_trans;
	}

	PicPointerPatch::AdjustMoveParams(pic_id, params);

	// Sanitize input
	params.magnify = std::max(0, std::min(params.magnify, 2000));
	params.top_trans = std::max(0, std::min(params.top_trans, 100));
	params.bottom_trans = std::max(0, std::min(params.bottom_trans, 100));
	params.duration = std::max(0, std::min(params.duration, 10000));

	if (pic_id <= 0) {
		Output::Error("MovePicture: Requested invalid picture id ({})", pic_id);
	}

	Main_Data::game_pictures->Move(pic_id, params);

	if (wait)
		SetupWait(params.duration);

	return true;
}

bool Game_Interpreter::CommandErasePicture(lcf::rpg::EventCommand const& com) { // code 11130
	// Older versions of RPG_RT block pictures when message active.
	if (!Player::IsEnglish() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	if (com.parameters.size() > 1) {
		// Handling of RPG2k3 1.12 chunks
		int id_type = com.parameters[1];

		int pic_id_max;
		switch (id_type) {
			case 0:
				// Erase single picture specified by constant
			case 1:
				// Erase single picture referenced by variable
				pic_id = ValueOrVariable(id_type, pic_id);
				pic_id_max = pic_id;
				break;
			case 2:
				// Erase [Arg0, Arg2]
				pic_id_max = com.parameters[2];
				break;
			case 3:
				// Erase [V[Arg0], V[Arg2]]
				if (!Player::IsPatchManiac()) {
					return true;
				}
				pic_id = Main_Data::game_variables->Get(pic_id);
				pic_id_max = Main_Data::game_variables->Get(com.parameters[2]);
				break;
			case 4:
				// Erase single picture referenced by variable indirect
				if (!Player::IsPatchManiac()) {
					return true;
				}
				pic_id = Main_Data::game_variables->GetIndirect(pic_id);
				pic_id_max = pic_id;
				break;
			case 5:
				// Erase all pictures
				if (!Player::IsPatchManiac()) {
					return true;
				}
				Main_Data::game_pictures->EraseAll();
				return true;
			default:
				return true;
		}

		for (int i = pic_id; i <= pic_id_max; ++i) {
			if (i <= 0) {
				Output::Error("ErasePicture: Requested invalid picture id ({})", i);
			}

			Main_Data::game_pictures->Erase(i);
		}
	} else {
		PicPointerPatch::AdjustId(pic_id);

		if (pic_id <= 0) {
			Output::Error("ErasePicture: Requested invalid picture id ({})", pic_id);
		}

		Main_Data::game_pictures->Erase(pic_id);
	}

	return true;
}

bool Game_Interpreter::CommandSpriteTransparency(lcf::rpg::EventCommand const& com) { // code 11310
	bool hidden = (com.parameters[0] == 0);
	Game_Character* player = Main_Data::game_player.get();
	player->SetSpriteHidden(hidden);
	// RPG_RT does this here.
	player->ResetThrough();

	return true;
}

bool Game_Interpreter::CommandMoveEvent(lcf::rpg::EventCommand const& com) { // code 11330
	int event_id = com.parameters[0];
	Game_Character* event = GetCharacter(event_id);
	if (event != NULL) {
		// If the event is a vehicle in use, push the commands to the player instead
		if (event_id >= Game_Character::CharBoat && event_id <= Game_Character::CharAirship)
			if (static_cast<Game_Vehicle*>(event)->IsInUse())
				event = Main_Data::game_player.get();

		lcf::rpg::MoveRoute route;
		int move_freq = com.parameters[1];

		if (move_freq <= 0 || move_freq > 8) {
			// Invalid values
			move_freq = 6;
		}

		route.repeat = com.parameters[2] != 0;
		route.skippable = com.parameters[3] != 0;

		for (auto it = com.parameters.begin() + 4; it < com.parameters.end(); ) {
			route.move_commands.push_back(DecodeMove(it));
		}

		event->ForceMoveRoute(route, move_freq);
	}
	return true;
}

bool Game_Interpreter::CommandMemorizeBGM(lcf::rpg::EventCommand const& /* com */) { // code 11530
	Main_Data::game_system->MemorizeBGM();
	return true;
}

bool Game_Interpreter::CommandPlayMemorizedBGM(lcf::rpg::EventCommand const& /* com */) { // code 11540
	Main_Data::game_system->PlayMemorizedBGM();
	return true;
}

int Game_Interpreter::KeyInputState::CheckInput() const {
	auto check = wait ? Input::IsTriggered : Input::IsPressed;

#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
	// FIXME: Refactor input system to make mouse buttons individual keys
	auto check_raw = wait ? Input::IsRawKeyTriggered : Input::IsRawKeyPressed;

	// Mouse buttons checked first (Maniac checks them last) to prevent conflict
	// with DECISION that is mapped to MOUSE_LEFT
	// The order of checking matches the Maniac behaviour
	if (keys[Keys::eMouseScrollDown]) {
		if (check_raw(Input::Keys::MOUSE_SCROLLDOWN)) {
			return 1001;
		}
	}

	if (keys[Keys::eMouseScrollUp]) {
		if (check_raw(Input::Keys::MOUSE_SCROLLUP)) {
			return 1004;
		}
	}

	if (keys[Keys::eMouseMiddle]) {
		if (check_raw(Input::Keys::MOUSE_MIDDLE)) {
			return 1007;
		}
	}

	if (keys[Keys::eMouseRight]) {
		if (check_raw(Input::Keys::MOUSE_RIGHT)) {
			return 1006;
		}
	}

	if (keys[Keys::eMouseLeft]) {
		if (check_raw(Input::Keys::MOUSE_LEFT)) {
			return 1005;
		}
	}
#endif

	// RPG processes keys from highest variable value to lowest.
	if (keys[Keys::eOperators]) {
		for (int i = 5; i > 0;) {
			--i;
			if (check((Input::InputButton)(Input::PLUS + i))) {
				return 20 + i;
			}
		}
	}
	if (keys[Keys::eNumbers]) {
		for (int i = 10; i > 0;) {
			--i;
			if (check((Input::InputButton)(Input::N0 + i))) {
				return 10 + i;
			}
		}
	}

	if (keys[Keys::eShift] && check(Input::SHIFT)) {
		return 7;
	}
	if (keys[Keys::eCancel] && check(Input::CANCEL)) {
		return 6;
	}
	if (keys[Keys::eDecision] && check(Input::DECISION)) {
		return 5;
	}
	if (keys[Keys::eUp] && check(Input::UP)) {
		return 4;
	}
	if (keys[Keys::eRight] && check(Input::RIGHT)) {
		return 3;
	}
	if (keys[Keys::eLeft] && check(Input::LEFT)) {
		return 2;
	}
	if (keys[Keys::eDown] && check(Input::DOWN)) {
		return 1;
	}

	return 0;
}

bool Game_Interpreter::CommandKeyInputProc(lcf::rpg::EventCommand const& com) { // code 11610
	int var_id = com.parameters[0];
	bool wait = com.parameters[1] != 0;

	if (wait) {
		// While waiting the variable is reset to 0 each frame.
		Main_Data::game_variables->Set(var_id, 0);
		Game_Map::SetNeedRefresh(true);
	}

	if (wait && Game_Message::IsMessageActive()) {
		return false;
	}

	_keyinput = {};
	_keyinput.wait = wait;
	_keyinput.variable = var_id;

	const size_t param_size = com.parameters.size();

	// Maniac Patch aware check functions for parameters that handle
	// keyboard and mouse through a bitmask
	bool is_maniac = Player::IsPatchManiac();
	auto check_key = [&](auto idx, bool handle_maniac = false) {
		if (param_size <= idx) {
			return false;
		}
		if (handle_maniac && is_maniac) {
			return (com.parameters[idx] & 1) != 0;
		} else {
			return com.parameters[idx] != 0;
		}
	};

	_keyinput.keys[Keys::eDecision] = check_key(3, true);
	_keyinput.keys[Keys::eCancel] = check_key(4, true);

	// All engines support older versions of the command depending on the
	// length of the parameter list
	if (Player::IsRPG2k()) {
		if (param_size < 6 || Player::IsRPG2kLegacy()) {
			// For Rpg2k <1.50 (later versions got individual key checks)
			if (com.parameters[2] != 0) {
				_keyinput.keys[Keys::eDown] = true;
				_keyinput.keys[Keys::eLeft] = true;
				_keyinput.keys[Keys::eRight] = true;
				_keyinput.keys[Keys::eUp] = true;
			}
		} else {
			// For Rpg2k >=1.50
			_keyinput.keys[Keys::eShift] = com.parameters[5] != 0;
			_keyinput.keys[Keys::eDown] = check_key(6);
			_keyinput.keys[Keys::eLeft] = check_key(7);
			_keyinput.keys[Keys::eRight] = check_key(8);
			_keyinput.keys[Keys::eUp] = check_key(9);
		}
	} else {
		if (param_size != 10 || Player::IsRPG2k3Legacy()) {
			if ((param_size < 10 || Player::IsRPG2k3Legacy()) && com.parameters[2] != 0) {
				// For RPG2k3 <1.05 (later versions got individual key checks)
				_keyinput.keys[Keys::eDown] = true;
				_keyinput.keys[Keys::eLeft] = true;
				_keyinput.keys[Keys::eRight] = true;
				_keyinput.keys[Keys::eUp] = true;
			}
			_keyinput.keys[Keys::eNumbers] = check_key(5);
			_keyinput.keys[Keys::eOperators] = check_key(6);
			_keyinput.time_variable = param_size > 7 ? com.parameters[7] : 0; // Attention: int, not bool
			_keyinput.timed = check_key(8);
			if (param_size > 10 && Player::IsMajorUpdatedVersion()) {
				// For Rpg2k3 >=1.05
				// ManiacPatch Middle & Wheel only handled for 2k3 Major Updated,
				// the only version that has this patch
				_keyinput.keys[Keys::eShift] = check_key(9, true);
				_keyinput.keys[Keys::eDown] = check_key(10, true);
				_keyinput.keys[Keys::eLeft] = check_key(11);
				_keyinput.keys[Keys::eRight] = check_key(12);
				_keyinput.keys[Keys::eUp] = check_key(13, true);
			}
		} else {
			// Since RPG2k3 1.05
			// Support for RPG2k >=1.50 games imported into RPG2k3
			_keyinput.keys[Keys::eShift] = com.parameters[5] != 0;
			_keyinput.keys[Keys::eDown] = com.parameters[6] != 0;
			_keyinput.keys[Keys::eLeft] = com.parameters[7] != 0;
			_keyinput.keys[Keys::eRight] = com.parameters[8] != 0;
			_keyinput.keys[Keys::eUp] = com.parameters[9] != 0;
		}
	}

	if (is_maniac) {
		auto check_mouse = [&](auto idx) {
			if (param_size <= idx) {
				return false;
			}

			bool result = (com.parameters[idx] & 2) != 0;
#if !defined(USE_MOUSE) || !defined(SUPPORT_MOUSE)
			if (result) {
				Output::Warning("ManiacPatch: Mouse input is not supported on this platform");
				result = false;
			}
#endif
			return result;
		};
		_keyinput.keys[Keys::eMouseLeft] = check_mouse(3);
		_keyinput.keys[Keys::eMouseRight] = check_mouse(4);
		_keyinput.keys[Keys::eMouseMiddle] = check_mouse(9);
		_keyinput.keys[Keys::eMouseScrollDown] = check_mouse(10);
		_keyinput.keys[Keys::eMouseScrollUp] = check_mouse(13);
	}

	if (_keyinput.wait) {
		// RPG_RT will reset all trigger key states when a waiting key input proc command is executed,
		// which means we always wait at least 1 frame to continue. Keys which are held down are not reset.
		// This also prevents player actions for this frame such as summoning the menu or triggering events.
		Input::ResetTriggerKeys();
		return true;
	}

	int key = _keyinput.CheckInput();
	Main_Data::game_variables->Set(_keyinput.variable, key);
	Game_Map::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandChangeMapTileset(lcf::rpg::EventCommand const& com) { // code 11710
	int chipset_id = com.parameters[0];

	if (chipset_id == Game_Map::GetChipset()) {
		return true;
	}

	Game_Map::SetChipset(chipset_id);

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();

	if (!scene)
		return true;

	scene->spriteset->ChipsetUpdated();

	return true;
}

bool Game_Interpreter::CommandChangePBG(lcf::rpg::EventCommand const& com) { // code 11720
	Game_Map::Parallax::Params params;
	params.name = ToString(com.string);
	params.scroll_horz = com.parameters[0] != 0;
	params.scroll_vert = com.parameters[1] != 0;
	params.scroll_horz_auto = com.parameters[2] != 0;
	params.scroll_horz_speed = com.parameters[3];
	params.scroll_vert_auto = com.parameters[4] != 0;
	params.scroll_vert_speed = com.parameters[5];

	Game_Map::Parallax::ChangeBG(params);

	if (!params.name.empty()) {
		_async_op = AsyncOp::MakeYield();
	}

	return true;
}

bool Game_Interpreter::CommandChangeEncounterRate(lcf::rpg::EventCommand const& com) { // code 11740
	int steps = com.parameters[0];

	Game_Map::SetEncounterRate(steps);

	return true;
}

bool Game_Interpreter::CommandTileSubstitution(lcf::rpg::EventCommand const& com) { // code 11750
	bool upper = com.parameters[0] != 0;
	int old_id = com.parameters[1];
	int new_id = com.parameters[2];
	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();
	if (!scene)
		return true;

	if (upper)
		scene->spriteset->SubstituteUp(old_id, new_id);
	else
		scene->spriteset->SubstituteDown(old_id, new_id);

	return true;
}

bool Game_Interpreter::CommandTeleportTargets(lcf::rpg::EventCommand const& com) { // code 11810
	int map_id = com.parameters[1];

	if (com.parameters[0] != 0) {
		Main_Data::game_targets->RemoveTeleportTarget(map_id);
		return true;
	}

	int x = com.parameters[2];
	int y = com.parameters[3];
	bool switch_on = static_cast<bool>(com.parameters[4]);
	int switch_id = com.parameters[5];
	Main_Data::game_targets->AddTeleportTarget(map_id, x, y, switch_on, switch_id);
	return true;
}

bool Game_Interpreter::CommandChangeTeleportAccess(lcf::rpg::EventCommand const& com) { // code 11820
	Main_Data::game_system->SetAllowTeleport(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandEscapeTarget(lcf::rpg::EventCommand const& com) { // code 11830
	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];
	bool switch_on = static_cast<bool>(com.parameters[3]);
	int switch_id = com.parameters[4];
	Main_Data::game_targets->SetEscapeTarget(map_id, x, y, switch_on, switch_id);
	return true;
}

bool Game_Interpreter::CommandChangeEscapeAccess(lcf::rpg::EventCommand const& com) { // code 11840
	Main_Data::game_system->SetAllowEscape(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeSaveAccess(lcf::rpg::EventCommand const& com) { // code 11930
	Main_Data::game_system->SetAllowSave(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeMainMenuAccess(lcf::rpg::EventCommand const& com) { // code 11960
	Main_Data::game_system->SetAllowMenu(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandConditionalBranch(lcf::rpg::EventCommand const& com) { // Code 12010
	const auto& frame = GetFrame();

	bool result = false;
	int value1, value2;
	int actor_id;
	Game_Actor* actor;
	Game_Character* character;

	switch (com.parameters[0]) {
	case 0:
		// Switch
		result = Main_Data::game_switches->Get(com.parameters[1]) == (com.parameters[2] == 0);
		break;
	case 1:
		// Variable
		value1 = Main_Data::game_variables->Get(com.parameters[1]);
		if (com.parameters[2] == 0) {
			value2 = com.parameters[3];
		} else {
			value2 = Main_Data::game_variables->Get(com.parameters[3]);
		}
		switch (com.parameters[4]) {
		case 0:
			// Equal to
			result = (value1 == value2);
			break;
		case 1:
			// Greater than or equal
			result = (value1 >= value2);
			break;
		case 2:
			// Less than or equal
			result = (value1 <= value2);
			break;
		case 3:
			// Greater than
			result = (value1 > value2);
			break;
		case 4:
			// Less than
			result = (value1 < value2);
			break;
		case 5:
			// Different
			result = (value1 != value2);
			break;
		}
		break;
	case 2:
		value1 = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer1);
		value2 = com.parameters[1];
		switch (com.parameters[2]) {
		case 0:
			result = (value1 >= value2);
			break;
		case 1:
			result = (value1 <= value2);
			break;
		}
		break;
	case 3:
		// Gold
		if (com.parameters[2] == 0) {
			// Greater than or equal
			result = (Main_Data::game_party->GetGold() >= com.parameters[1]);
		} else {
			// Less than or equal
			result = (Main_Data::game_party->GetGold() <= com.parameters[1]);
		}
		break;
	case 4:
		// Item
		if (com.parameters[2] == 0) {
			// Having
			result = Main_Data::game_party->GetItemCount(com.parameters[1])
				+ Main_Data::game_party->GetEquippedItemCount(com.parameters[1]) > 0;
		} else {
			// Not having
			result = Main_Data::game_party->GetItemCount(com.parameters[1])
				+ Main_Data::game_party->GetEquippedItemCount(com.parameters[1]) == 0;
		}
		break;
	case 5:
		// Hero
		actor_id = com.parameters[1];
		actor = Main_Data::game_actors->GetActor(actor_id);

		if (!actor) {
			Output::Warning("ConditionalBranch: Invalid actor ID {}", actor_id);
			// Use Else Branch
			SetSubcommandIndex(com.indent, 1);
			SkipToNextConditional({Cmd::ElseBranch, Cmd::EndBranch}, com.indent);
			return true;
		}

		switch (com.parameters[2]) {
		case 0:
			// Is actor in party
			result = Main_Data::game_party->IsActorInParty(actor_id);
			break;
		case 1:
			// Name
			result = (actor->GetName() == com.string);
			break;
		case 2:
			// Higher or equal level
			result = (actor->GetLevel() >= com.parameters[3]);
			break;
		case 3:
			// Higher or equal HP
			result = (actor->GetHp() >= com.parameters[3]);
			break;
		case 4:
			// Is skill learned
			result = (actor->IsSkillLearned(com.parameters[3]));
			break;
		case 5:
			// Equipped object
			result = (
				(actor->GetShieldId() == com.parameters[3]) ||
				(actor->GetArmorId() == com.parameters[3]) ||
				(actor->GetHelmetId() == com.parameters[3]) ||
				(actor->GetAccessoryId() == com.parameters[3]) ||
				(actor->GetWeaponId() == com.parameters[3])
				);
			break;
		case 6:
			// Has state
			result = (actor->HasState(com.parameters[3]));
			break;
		default:
			;
		}
		break;
	case 6:
		// Orientation of char
		character = GetCharacter(com.parameters[1]);
		if (character != NULL) {
			result = character->GetFacing() == com.parameters[2];
		}
		break;
	case 7: {
		// Vehicle in use
		Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[1] + 1);
		Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

		if (!vehicle) {
			Output::Warning("ConditionalBranch: Invalid vehicle ID {}", vehicle_id);
			return true;
		}

		result = vehicle->IsInUse();
		break;
	}
	case 8:
		// Key decision initiated this event
		result = frame.triggered_by_decision_key;
		break;
	case 9:
		// BGM looped at least once
		result = Audio().BGM_PlayedOnce();
		break;
	case 10:
		value1 = Main_Data::game_party->GetTimerSeconds(Main_Data::game_party->Timer2);
		value2 = com.parameters[1];
		switch (com.parameters[2]) {
		case 0:
			result = (value1 >= value2);
			break;
		case 1:
			result = (value1 <= value2);
			break;
		}
		break;
	case 11:
		// RPG Maker 2003 v1.11 features
		switch (com.parameters[1]) {
		case 0:
			// Any savestate available
			result = FileFinder::HasSavegame();
			break;
		case 1:
			// Is Test Play mode?
			result = Player::debug_flag;
			break;
		case 2:
			// Is ATB wait on?
			result = Main_Data::game_system->GetAtbMode() == lcf::rpg::SaveSystem::AtbMode_atb_wait;
			break;
		case 3:
			// Is Fullscreen active?
			result = DisplayUi->IsFullscreen();
			break;
		}
		break;
	default:
		Output::Warning("ConditionalBranch: Branch {} unsupported", com.parameters[0]);
	}


	int sub_idx = subcommand_sentinel;
	if (!result) {
		sub_idx = eOptionBranchElse;
		SkipToNextConditional({Cmd::ElseBranch, Cmd::EndBranch}, com.indent);
	}

	SetSubcommandIndex(com.indent, sub_idx);
	return true;
}


bool Game_Interpreter::CommandElseBranch(lcf::rpg::EventCommand const& com) { //code 22010
	return CommandOptionGeneric(com, eOptionBranchElse, {Cmd::EndBranch});
}

bool Game_Interpreter::CommandEndBranch(lcf::rpg::EventCommand const& /* com */) { //code 22011
	return true;
}

bool Game_Interpreter::CommandJumpToLabel(lcf::rpg::EventCommand const& com) { // code 12120
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	int label_id = com.parameters[0];

	for (int idx = 0; (size_t)idx < list.size(); idx++) {
		if (static_cast<Cmd>(list[idx].code) != Cmd::Label)
			continue;
		if (list[idx].parameters[0] != label_id)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandLoop(lcf::rpg::EventCommand const& com) { // code 12210
	if (!Player::IsPatchManiac() || com.parameters.empty() || com.parameters[0] == 0) {
		// Infinite Loop
		return true;
	}

	Output::Warning("Maniac CommandLoop: Conditional loops unsupported");
	return true;
}

bool Game_Interpreter::CommandBreakLoop(lcf::rpg::EventCommand const& /* com */) { // code 12220
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	// BreakLoop will jump to the end of the event if there is no loop.

	//FIXME: This emulates an RPG_RT bug where break loop ignores scopes and
	//unconditionally jumps to the next EndLoop command.
	auto pcode = static_cast<Cmd>(list[index].code);

	for (++index; index < (int)list.size(); ++index) {
		if (pcode == Cmd::EndLoop) {
			break;
		}
		pcode = static_cast<Cmd>(list[index].code);
	}

	return true;
}

bool Game_Interpreter::CommandEndLoop(lcf::rpg::EventCommand const& com) { // code 22210
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	int indent = com.indent;

	for (int idx = index; idx >= 0; idx--) {
		if (list[idx].indent > indent)
			continue;
		if (list[idx].indent < indent)
			return false;
		if (static_cast<Cmd>(list[idx].code) != Cmd::Loop)
			continue;
		index = idx;
		break;
	}

	// Jump past the Cmd::Loop to the first command.
	if (index < (int)frame.commands.size()) {
		++index;
	}

	return true;
}

bool Game_Interpreter::CommandEraseEvent(lcf::rpg::EventCommand const& /* com */) { // code 12320
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	auto event_id = GetThisEventId();

	// When a common event and not RPG2k3E engine ignore the call, otherwise
	// operate on last map_event
	if (event_id == 0)
		return true;

	Game_Event* evnt = Game_Map::GetEvent(event_id);
	if (evnt) {
		evnt->SetActive(false);

		// Parallel map events shall stop immediately
		if (!main_flag) {
			// When the event was called indirectly from a parallel process
			// event execution continues after a yield.
			// Increment the index for this special case.
			index++;
			return false;
		}
	}

	return true;
}

bool Game_Interpreter::CommandCallEvent(lcf::rpg::EventCommand const& com) { // code 12330
	int evt_id;
	int event_page;

	switch (com.parameters[0]) {
	case 0: { // Common Event
		evt_id = com.parameters[1];
		Game_CommonEvent* common_event = lcf::ReaderUtil::GetElement(Game_Map::GetCommonEvents(), evt_id);
		if (!common_event) {
			Output::Warning("CallEvent: Can't call invalid common event {}", evt_id);
			return true;
		}

		Push(common_event);

		return true;
	}
	case 1: // Map Event
		evt_id = com.parameters[1];
		event_page = com.parameters[2];
		break;
	case 2: // Indirect
		evt_id = Main_Data::game_variables->Get(com.parameters[1]);
		event_page = Main_Data::game_variables->Get(com.parameters[2]);
		break;
	default:
		return false;
	}

	Game_Event* event = static_cast<Game_Event*>(GetCharacter(evt_id));
	if (!event) {
		Output::Warning("CallEvent: Can't call non-existant event {}", evt_id);
		return false;
	}

	const lcf::rpg::EventPage* page = event->GetPage(event_page);
	if (!page) {
		Output::Warning("CallEvent: Can't call non-existant page {} of event {}", event_page, evt_id);
		return false;
	}

	Push(page->event_commands, event->GetId(), false);

	return true;
}

bool Game_Interpreter::CommandReturnToTitleScreen(lcf::rpg::EventCommand const& /* com */) { // code 12510
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	_async_op = AsyncOp::MakeToTitle();
	return true;
}

bool Game_Interpreter::CommandChangeClass(lcf::rpg::EventCommand const& com) { // code 1008
	int class_id = com.parameters[2]; // 0: No class, 1+: Specific class
	bool level1 = com.parameters[3] > 0;
	int skill_mode = com.parameters[4]; // no change, replace, add
	int param_mode = com.parameters[5]; // no change, halve, level 1, current level
	bool show_msg = com.parameters[6] > 0;

	if (show_msg && !Game_Message::CanShowMessage(true)) {
		return false;
	}

	PendingMessage pm;
	pm.SetEnableFace(false);

	const lcf::rpg::Class* cls = lcf::ReaderUtil::GetElement(lcf::Data::classes, class_id);
	if (!cls && class_id != 0) {
		Output::Warning("ChangeClass: Can't change class. Class {} is invalid", class_id);
		return true;
	}

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		int level = level1 ? 1 : actor->GetLevel();
		actor->ChangeClass(class_id, level,
				static_cast<Game_Actor::ClassChangeSkillMode>(skill_mode),
				static_cast<Game_Actor::ClassChangeParamMode>(param_mode),
				show_msg ? &pm : nullptr
				);
	}

	if (CheckGameOver()) {
		return true;
	}

	if (show_msg && pm.IsActive()) {
		ForegroundTextPush(std::move(pm));
	}

	return true;
}

bool Game_Interpreter::CommandChangeBattleCommands(lcf::rpg::EventCommand const& com) { // code 1009
	int cmd_id = com.parameters[2];
	bool add = com.parameters[3] != 0;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeBattleCommands(add, cmd_id);
	}

	return true;
}

bool Game_Interpreter::CommandExitGame(lcf::rpg::EventCommand const& /* com */) {
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	_async_op = AsyncOp::MakeExitGame();
	return true;
}

bool Game_Interpreter::CommandToggleFullscreen(lcf::rpg::EventCommand const& /* com */) {
	DisplayUi->ToggleFullscreen();
	return true;
}

bool Game_Interpreter::CommandManiacGetSaveInfo(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int save_number = ValueOrVariable(com.parameters[0], com.parameters[1]);

	// Error case, set to YYMMDD later on success
	Main_Data::game_variables->Set(com.parameters[2], 0);

	if (save_number <= 0) {
		Output::Debug("ManiacGetSaveInfo: Invalid save number {}", save_number);
		return true;
	}

	auto savefs = FileFinder::Save();
	std::string save_name = Scene_Save::GetSaveFilename(savefs, save_number);
	auto save = lcf::LSD_Reader::Load(save_name, Player::encoding);

	if (!save) {
		Output::Debug("ManiacGetSaveInfo: Save not found {}", save_number);
		return true;
	}

	std::time_t t = lcf::LSD_Reader::ToUnixTimestamp(save->title.timestamp);
	std::tm* tm = std::gmtime(&t);

	Main_Data::game_variables->Set(com.parameters[2], atoi(Utils::FormatDate(tm, Utils::DateFormat_YYMMDD).c_str()));
	Main_Data::game_variables->Set(com.parameters[3], atoi(Utils::FormatDate(tm, Utils::DateFormat_HHMMSS).c_str()));
	Main_Data::game_variables->Set(com.parameters[4], save->title.hero_level);
	Main_Data::game_variables->Set(com.parameters[5], save->title.hero_hp);
	Game_Map::SetNeedRefresh(true);

	auto face_ids = Utils::MakeArray(save->title.face1_id, save->title.face2_id, save->title.face3_id, save->title.face4_id);
	auto face_names = Utils::MakeArray(save->title.face1_name, save->title.face2_name, save->title.face3_name, save->title.face4_name);

	for (int i = 0; i <= 3; ++i) {
		const int param = 8 + i;

		int pic_id = ValueOrVariable(com.parameters[7], com.parameters[param]);
		if (pic_id <= 0) {
			continue;
		}

		if (face_names[i].empty()) {
			Main_Data::game_pictures->Erase(pic_id);
			continue;
		}

		// When the picture exists: Data is reused and effects finish immediately
		// When not: Default data is used
		// New features (spritesheets etc.) are always set due to how the patch works
		// We are incompatible here and only set name and spritesheet and reuse stuff like the layer
		Game_Pictures::ShowParams params;
		auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
		if (pic.Exists()) {
			params = pic.GetShowParams();
		} else {
			params.map_layer = 7;
			params.battle_layer = 7;
		}
		params.name = FileFinder::MakePath("..\\FaceSet", face_names[i]);
		params.spritesheet_cols = 4;
		params.spritesheet_rows = 4;
		params.spritesheet_frame = face_ids[i];
		params.spritesheet_speed = 0;
		Main_Data::game_pictures->Show(pic_id, params);
	}

	return true;
}

bool Game_Interpreter::CommandManiacSave(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int slot = ValueOrVariable(com.parameters[0], com.parameters[1]);
	if (slot <= 0) {
		Output::Debug("ManiacSave: Invalid save slot {}", slot);
		return true;
	}

	int out_var = com.parameters[2] != 0 ? com.parameters[3] : -1;

	// Maniac Patch saves directly and game data could be in an undefined state
	// We yield first to the Update loop and then do a save.
	_async_op = AsyncOp::MakeSave(slot, out_var);

	return true;
}

bool Game_Interpreter::CommandManiacLoad(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int slot = ValueOrVariable(com.parameters[0], com.parameters[1]);
	if (slot <= 0) {
		Output::Debug("ManiacLoad: Invalid save slot {}", slot);
		return true;
	}

	// Not implemented (kinda useless feature):
	// When com.parameters[2] is 1 the check whether the file exists is skipped
	// When skipped and missing RPG_RT will crash
	auto savefs = FileFinder::Save();
	std::string save_name = Scene_Save::GetSaveFilename(savefs, slot);
	auto save = lcf::LSD_Reader::Load(save_name, Player::encoding);

	if (!save) {
		Output::Debug("ManiacLoad: Save not found {}", slot);
		return true;
	}

	// FIXME: In Maniac the load causes a blackscreen followed by a fade-in that can be cancelled by a transition event
	// This is not implemented yet, the loading is instant without fading
	_async_op = AsyncOp::MakeLoad(slot);

	return true;
}

bool Game_Interpreter::CommandManiacEndLoadProcess(lcf::rpg::EventCommand const&) {
	// no-op
	return true;
}

bool Game_Interpreter::CommandManiacGetMousePosition(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

#if !defined(USE_MOUSE) || !defined(SUPPORT_MOUSE)
	static bool warned = false;
	if (!warned) {
		// This command is polled, prevent excessive spam
		Output::Warning("Maniac Patch: Mouse input is not supported on this platform");
		warned = true;
	}
	return true;
#endif

	Point mouse_pos = Input::GetMousePosition();

	Main_Data::game_variables->Set(com.parameters[0], mouse_pos.x);
	Main_Data::game_variables->Set(com.parameters[1], mouse_pos.y);

	Game_Map::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandManiacSetMousePosition(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command SetMousePosition not supported");
	return true;
}

bool Game_Interpreter::CommandManiacShowStringPicture(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command ShowStringPicture not supported");
	return true;
}

bool Game_Interpreter::CommandManiacGetPictureInfo(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command GetPictureInfo not supported");
	return true;
}

bool Game_Interpreter::CommandManiacControlVarArray(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command ControlVarArray not supported");
	return true;
}

bool Game_Interpreter::CommandManiacKeyInputProcEx(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command KeyInputProcEx not supported");
	return true;
}

bool Game_Interpreter::CommandManiacRewriteMap(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command RewriteMap not supported");
	return true;
}

bool Game_Interpreter::CommandManiacControlGlobalSave(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command ControlGlobalSave not supported");
	return true;
}

bool Game_Interpreter::CommandManiacChangePictureId(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command ChangePictureId not supported");
	return true;
}

bool Game_Interpreter::CommandManiacSetGameOption(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command SetGameOption not supported");
	return true;
}

bool Game_Interpreter::CommandManiacCallCommand(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command CallCommand not supported");
	return true;
}

Game_Interpreter& Game_Interpreter::GetForegroundInterpreter() {
	return Game_Battle::IsBattleRunning()
		? Game_Battle::GetInterpreter()
		: Game_Map::GetInterpreter();
}

bool Game_Interpreter::IsWaitingForWaitCommand() const {
	return (_state.wait_time > 0) || _state.wait_key_enter;
}
