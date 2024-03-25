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
#include <ctime>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <cassert>
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
#include "game_interpreter_control_variables.h"
#include "game_windows.h"
#include "maniac_patch.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_gameover.h"
#include "scene_map.h"
#include "scene_save.h"
#include "scene_settings.h"
#include "scene.h"
#include "game_clock.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include <lcf/reader_util.h>
#include <lcf/lsd/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/writer_lcf.h>
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
	std::vector<lcf::rpg::EventCommand> _list,
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
	frame.commands = std::move(_list);
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

void Game_Interpreter::SetupWaitFrames(int duration) {
	if (duration == 0) {
		// 0.0 waits 1 frame
		_state.wait_time = 1;
	} else {
		_state.wait_time = duration;
	}
}

bool Game_Interpreter::ReachedLoopLimit() const {
	return loop_count >= loop_limit;
}

int Game_Interpreter::GetThisEventId() const {
	auto event_id = GetCurrentEventId();

	if (event_id == 0 && (Player::IsRPG2k3E() || Player::game_config.patch_common_this_event.Get())) {
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
			if (_async_op.GetType() == AsyncOp::Type::eYieldRepeat) {
				// This will cause an incorrect execution when the yielding
				// command changed the index.
				// Only use YieldRepeat for commands that do not do this!
				auto& frame = GetFrame();
				--frame.current_command;
			}
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
	return ExecuteCommand(com);
}

bool Game_Interpreter::ExecuteCommand(lcf::rpg::EventCommand const& com) {
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
		case Cmd::PlayerVisibility:
			return CommandPlayerVisibility(com);
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
		case Cmd::ChangeEncounterSteps:
			return CommandChangeEncounterSteps(com);
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
		case Cmd::OpenVideoOptions:
			return CommandOpenVideoOptions(com);
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
		case Cmd::Maniac_ControlStrings:
			return CommandManiacControlStrings(com);
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

	PendingMessage pm(Game_Message::CommandCodeInserter);
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

	// TODO: Maniac Patch Message Box Size and Font not implemented
	// see https://jetrotal.github.io/CSA/#Display%20Text%20Options

	return true;
}

bool Game_Interpreter::CommandChangeFaceGraphic(lcf::rpg::EventCommand const& com) { // Code 10130
	if (!Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	Main_Data::game_system->SetMessageFaceName(ToString(CommandStringOrVariableBitfield(com, 3, 0, 4)));
	Main_Data::game_system->SetMessageFaceIndex(ValueOrVariableBitfield(com, 3, 1, 0));
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

	PendingMessage pm(Game_Message::CommandCodeInserter);
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

	PendingMessage pm(Game_Message::CommandCodeInserter);
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
	int operand = com.parameters[4];

	if (EP_UNLIKELY(operand >= 9 && !Player::IsPatchManiac())) {
		Output::Warning("ControlVariables: Unsupported operand {}", operand);
		return true;
	}

	switch (operand) {
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
			value = Main_Data::game_variables->GetIndirect(com.parameters[5]);
			break;
		case 3: {
			// Random between range
			int32_t arg1 = com.parameters[5];
			int32_t arg2 = com.parameters[6];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				arg1 = ValueOrVariableBitfield(com.parameters[7], 0, arg1);
				arg2 = ValueOrVariableBitfield(com.parameters[7], 1, arg2);
			}

			value = ControlVariables::Random(arg1, arg2);
			break;
		}
		case 4: {
			// Items
			int item = com.parameters[5];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				item = ValueOrVariable(com.parameters[7], item);
			}

			value = ControlVariables::Item(com.parameters[6], item);
			break;
		}
		case 5: { // Hero
			int actor_id = com.parameters[5];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				actor_id = ValueOrVariable(com.parameters[7], actor_id);
			}
			value = ControlVariables::Actor(com.parameters[6], actor_id);
			break;
		}
		case 6: {
			// Characters
			int event_id = com.parameters[5];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				event_id = ValueOrVariable(com.parameters[7], event_id);
			}
			value = ControlVariables::Event(com.parameters[6], event_id, *this);
			break;
		}
		case 7:
			// More
			value = ControlVariables::Other(com.parameters[5]);
			break;
		case 8: {
			int enemy_id = com.parameters[5];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				enemy_id = ValueOrVariable(com.parameters[7], enemy_id);
			}

			// Battle related
			value = ControlVariables::Enemy(com.parameters[6], enemy_id);
			break;
		}
		case 9: { // Party Member (Maniac)
			int party_idx = com.parameters[5];
			if (Player::IsPatchManiac() && com.parameters.size() >= 8) {
				party_idx = ValueOrVariable(com.parameters[7], party_idx);
			}
			value = ControlVariables::Party(com.parameters[6], party_idx);
			break;
		}
		case 10: {
			// Switch (Maniac)
			value = com.parameters[5];
			if (com.parameters[6] == 1) {
				value = Main_Data::game_switches->GetInt(value);
			} else {
				value = Main_Data::game_switches->GetInt(Main_Data::game_variables->Get(value));
			}
			break;
		}
		case 11: {
			// Pow (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[7], 1, com.parameters[6]);
			value = ControlVariables::Pow(arg1, arg2);
			break;
		}
		case 12: {
			// Sqrt (Maniac)
			int arg = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int mul = com.parameters[6];
			value = ControlVariables::Sqrt(arg, mul);
			break;
		}
		case 13: {
			// Sin (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[7], 1, com.parameters[8]);
			float mul = static_cast<float>(com.parameters[6]);
			value = ControlVariables::Sin(arg1, arg2, mul);
			break;
		}
		case 14: {
			// Cos (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[7], 1, com.parameters[8]);
			int mul = com.parameters[6];
			value = ControlVariables::Cos(arg1, arg2, mul);
			break;
		}
		case 15: {
			// Atan2 (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[8], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[8], 1, com.parameters[6]);
			int mul = com.parameters[7];
			value = ControlVariables::Atan2(arg1, arg2, mul);
			break;
		}
		case 16: {
			// Min (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[7], 1, com.parameters[6]);
			value = ControlVariables::Min(arg1, arg2);
			break;
		}
		case 17: {
			// Max (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[7], 0, com.parameters[5]);
			int arg2 = ValueOrVariableBitfield(com.parameters[7], 1, com.parameters[6]);
			value = ControlVariables::Max(arg1, arg2);
			break;
		}
		case 18: {
			// Abs (Maniac)
			int arg = ValueOrVariableBitfield(com.parameters[6], 0, com.parameters[5]);
			value = ControlVariables::Abs(arg);
			break;
		}
		case 19: {
			// Binary (Maniac)
			int arg1 = ValueOrVariableBitfield(com.parameters[8], 0, com.parameters[6]);
			int arg2 = ValueOrVariableBitfield(com.parameters[8], 1, com.parameters[7]);
			value = ControlVariables::Binary(com.parameters[5], arg1, arg2);
			break;
		}
		case 20: {
			// Ternary (Maniac)
			int mode = com.parameters[10];
			int arg1 = ValueOrVariableBitfield(mode, 0, com.parameters[6]);
			int arg2 = ValueOrVariableBitfield(mode, 1, com.parameters[7]);
			int op = com.parameters[5];
			if (CheckOperator(arg1, arg2, op)) {
				value = ValueOrVariableBitfield(mode, 2, com.parameters[8]);
			} else {
				value = ValueOrVariableBitfield(mode, 3, com.parameters[9]);
			}
			break;
		}
		case 21:
			// Expression (Maniac)
			value = ManiacPatch::ParseExpression(MakeSpan(com.parameters).subspan(6, com.parameters[5]), *this);
			break;
		default:
			Output::Warning("ControlVariables: Unsupported operand {}", operand);
			return true;
	}

	int target = com.parameters[0];
	if (target >= 0 && target <= 4) {
		// For Range set end to param 2, otherwise to start, this way the loop runs exactly once

		int start, end;
		if (target == 0) {
			// Single
			start = com.parameters[1];
			end = start;
		} else if (target == 1) {
			// Range
			start = com.parameters[1];
			end = com.parameters[2];
		} else if (target == 2) {
			// Indirect
			start = Main_Data::game_variables->Get(com.parameters[1]);
			end = start;
		} else if (target == 3 && Player::IsPatchManiac()) {
			// Range Indirect (Maniac)
			start = Main_Data::game_variables->Get(com.parameters[1]);
			end = Main_Data::game_variables->Get(com.parameters[2]);
		} else if (target == 4 && Player::IsPatchManiac()) {
			// Expression (Maniac)
			int idx = com.parameters[1];
			start = ManiacPatch::ParseExpression(MakeSpan(com.parameters).subspan(idx + 1, com.parameters[idx]), *this);
			end = start;
		} else {
			return true;
		}

		if (Player::IsPatchManiac() && end < start) {
			// Vanilla does not support end..start, Maniac does
			std::swap(start, end);
		}

		int operation = com.parameters[3];
		if (EP_UNLIKELY(operation >= 6 && !Player::IsPatchManiac())) {
			Output::Warning("ControlVariables: Unsupported operation {}", operation);
			return true;
		}

		if (start == end) {
			// Single variable case - if this is random value, we already called the RNG earlier.
			switch (operation) {
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
				case 6:
					Main_Data::game_variables->BitOr(start, value);
					break;
				case 7:
					Main_Data::game_variables->BitAnd(start, value);
					break;
				case 8:
					Main_Data::game_variables->BitXor(start, value);
					break;
				case 9:
					Main_Data::game_variables->BitShiftLeft(start, value);
					break;
				case 10:
					Main_Data::game_variables->BitShiftRight(start, value);
					break;
			}
		} else if (com.parameters[4] == 1) {
			// Multiple variables - Direct variable lookup
			int var_id = com.parameters[5];
			switch (operation) {
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
				case 6:
					Main_Data::game_variables->BitOrRangeVariable(start, end, var_id);
					break;
				case 7:
					Main_Data::game_variables->BitAndRangeVariable(start, end, var_id);
					break;
				case 8:
					Main_Data::game_variables->BitXorRangeVariable(start, end, var_id);
					break;
				case 9:
					Main_Data::game_variables->BitShiftLeftRangeVariable(start, end, var_id);
					break;
				case 10:
					Main_Data::game_variables->BitShiftRightRangeVariable(start, end, var_id);
					break;
			}
		} else if (com.parameters[4] == 2) {
			// Multiple variables - Indirect variable lookup
			int var_id = com.parameters[5];
			switch (operation) {
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
				case 6:
					Main_Data::game_variables->BitOrRangeVariableIndirect(start, end, var_id);
					break;
				case 7:
					Main_Data::game_variables->BitAndRangeVariableIndirect(start, end, var_id);
					break;
				case 8:
					Main_Data::game_variables->BitXorRangeVariableIndirect(start, end, var_id);
					break;
				case 9:
					Main_Data::game_variables->BitShiftLeftRangeVariableIndirect(start, end, var_id);
					break;
				case 10:
					Main_Data::game_variables->BitShiftRightRangeVariableIndirect(start, end, var_id);
					break;
			}
		} else if (com.parameters[4] == 3) {
			// Multiple variables - random
			int rmax = max(com.parameters[5], com.parameters[6]);
			int rmin = min(com.parameters[5], com.parameters[6]);
			switch (operation) {
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
				case 6:
					Main_Data::game_variables->BitOrRangeRandom(start, end, rmin, rmax);
					break;
				case 7:
					Main_Data::game_variables->BitAndRangeRandom(start, end, rmin, rmax);
					break;
				case 8:
					Main_Data::game_variables->BitXorRangeRandom(start, end, rmin, rmax);
					break;
				case 9:
					Main_Data::game_variables->BitShiftLeftRangeRandom(start, end, rmin, rmax);
					break;
				case 10:
					Main_Data::game_variables->BitShiftRightRangeRandom(start, end, rmin, rmax);
					break;
			}
		} else {
			// Multiple variables - constant
			switch (operation) {
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
				case 6:
					Main_Data::game_variables->BitOrRange(start, end, value);
					break;
				case 7:
					Main_Data::game_variables->BitAndRange(start, end, value);
					break;
				case 8:
					Main_Data::game_variables->BitXorRange(start, end, value);
					break;
				case 9:
					Main_Data::game_variables->BitShiftLeftRange(start, end, value);
					break;
				case 10:
					Main_Data::game_variables->BitShiftRightRange(start, end, value);
					break;
			}
		}

		Game_Map::SetNeedRefresh(true);
	}

	return true;
}

int Game_Interpreter::OperateValue(int operation, int operand_type, int operand) {
	int value = ValueOrVariable(operand_type, operand);

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
	int timer_id = 0;

	if (com.parameters.size() > 5 && Player::IsRPG2k3Commands()) {
		timer_id = com.parameters[5];
	}

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
	int id = ValueOrVariable(com.parameters[1], com.parameters[2]);

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

	PendingMessage pm(Game_Message::CommandCodeInserter);
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

	PendingMessage pm(Game_Message::CommandCodeInserter);
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

int Game_Interpreter::ValueOrVariableBitfield(int mode, int shift, int val) {
	return ValueOrVariable((mode & (0xF << shift * 4)) >> shift * 4, val);
}

int Game_Interpreter::ValueOrVariableBitfield(lcf::rpg::EventCommand const& com, int mode_idx, int shift, int val_idx) {
	assert(com.parameters.size() > val_idx);

	if (!Player::IsPatchManiac()) {
		return com.parameters[val_idx];
	}

	assert(mode_idx != val_idx);

	if (com.parameters.size() > std::max(mode_idx, val_idx)) {
		int mode = com.parameters[mode_idx];
		return ValueOrVariableBitfield(com.parameters[mode_idx], shift, com.parameters[val_idx]);
	}

	return com.parameters[val_idx];
}

StringView Game_Interpreter::CommandStringOrVariable(lcf::rpg::EventCommand const& com, int mode_idx, int val_idx) {
	if (!Player::IsPatchManiac()) {
		return com.string;
	}

	assert(mode_idx != val_idx);

	if (com.parameters.size() > std::max(mode_idx, val_idx)) {
		return Main_Data::game_strings->GetWithMode(ToString(com.string), com.parameters[mode_idx], com.parameters[val_idx], *Main_Data::game_variables);
	}

	return com.string;
}

StringView Game_Interpreter::CommandStringOrVariableBitfield(lcf::rpg::EventCommand const& com, int mode_idx, int shift, int val_idx) {
	if (!Player::IsPatchManiac()) {
		return com.string;
	}

	assert(mode_idx != val_idx);

	if (com.parameters.size() >= std::max(mode_idx, val_idx) + 1) {
		int mode = com.parameters[mode_idx];
		return Main_Data::game_strings->GetWithMode(ToString(com.string), (mode & (0xF << shift * 4)) >> shift * 4, com.parameters[val_idx], *Main_Data::game_variables);
	}

	return com.string;
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

	bool maniac = Player::IsPatchManiac();

	// Wait a given time
	if (com.parameters.size() <= 1 || (!maniac && !Player::IsRPG2k3Commands())) {
		SetupWait(com.parameters[0]);
		return true;
	}

	if (!maniac && com.parameters.size() > 1 && com.parameters[1] == 0) {
		SetupWait(com.parameters[0]);
		return true;
	}

	if (maniac && com.parameters.size() > 1 && com.parameters[1] != 1) {
		int wait_type = com.parameters[1];
		int mode = 0;

		if (com.parameters.size() > 2) {
			mode = com.parameters[2];
		}

		int duration = ValueOrVariable(mode, com.parameters[0]);

		if (wait_type == 256) {
			SetupWaitFrames(duration);
		} else {
			SetupWait(duration);
		}

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
	music.name = ToString(CommandStringOrVariableBitfield(com, 4, 0, 5));

	music.fadein = ValueOrVariableBitfield(com, 4, 1, 0);
	music.volume = ValueOrVariableBitfield(com, 4, 2, 1);
	music.tempo = ValueOrVariableBitfield(com, 4, 3, 2);
	music.balance = ValueOrVariableBitfield(com, 4, 4, 3);

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
	sound.name = ToString(CommandStringOrVariableBitfield(com, 3, 0, 4));

	sound.volume = ValueOrVariableBitfield(com, 3, 1, 0);
	sound.tempo = ValueOrVariableBitfield(com, 3, 2, 1);
	sound.balance = ValueOrVariableBitfield(com, 3, 3, 2);

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
	int id = ValueOrVariableBitfield(com, 1, 0, 0);
	Game_Actor* actor = Main_Data::game_actors->GetActor(id);

	if (!actor) {
		Output::Warning("ChangeHeroName: Invalid actor ID {}", id);
		return true;
	}

	actor->SetName(ToString(CommandStringOrVariableBitfield(com, 1, 1, 2)));
	return true;
}

bool Game_Interpreter::CommandChangeHeroTitle(lcf::rpg::EventCommand const& com) { // code 10620
	int id = ValueOrVariableBitfield(com, 1, 0, 0);
	Game_Actor* actor = Main_Data::game_actors->GetActor(id);

	if (!actor) {
		Output::Warning("ChangeHeroTitle: Invalid actor ID {}", id);
		return true;
	}

	actor->SetTitle(ToString(CommandStringOrVariableBitfield(com, 1, 1, 2)));
	return true;
}

bool Game_Interpreter::CommandChangeSpriteAssociation(lcf::rpg::EventCommand const& com) { // code 10630
	int id = ValueOrVariableBitfield(com, 3, 0, 0);
	Game_Actor* actor = Main_Data::game_actors->GetActor(id);

	if (!actor) {
		Output::Warning("ChangeSpriteAssociation: Invalid actor ID {}", id);
		return true;
	}

	auto file = ToString(CommandStringOrVariableBitfield(com, 3, 1, 4));
	int idx = ValueOrVariableBitfield(com, 3, 2, 1);
	bool transparent = com.parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	Main_Data::game_player->ResetGraphic();
	return true;
}

bool Game_Interpreter::CommandChangeActorFace(lcf::rpg::EventCommand const& com) { // code 10640
	int id = ValueOrVariableBitfield(com, 2, 0, 0);
	Game_Actor* actor = Main_Data::game_actors->GetActor(id);

	if (!actor) {
		Output::Warning("CommandChangeActorFace: Invalid actor ID {}", id);
		return true;
	}

	actor->SetFace(
			ToString(CommandStringOrVariableBitfield(com, 2, 1, 3)),
			ValueOrVariableBitfield(com, 2, 2, 1));
	return true;
}

bool Game_Interpreter::CommandChangeVehicleGraphic(lcf::rpg::EventCommand const& com) { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0] + 1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

	if (!vehicle) {
		Output::Warning("ChangeVehicleGraphic: Invalid vehicle ID {}", static_cast<int>(vehicle_id));
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
	Main_Data::game_system->SetSystemGraphic(ToString(CommandStringOrVariable(com, 2, 3)),
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
			Output::Warning("SetVehicleLocation: Invalid vehicle ID {}", static_cast<int>(vehicle_id));
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
	if (event != nullptr) {
		const auto x = ValueOrVariable(com.parameters[1], com.parameters[2]);
		const auto y = ValueOrVariable(com.parameters[1], com.parameters[3]);
		event->MoveTo(event->GetMapId(), x, y);

		// RPG2k3 feature
		int direction = -1;
		if (Player::IsRPG2k3Commands() && com.parameters.size() > 4) {
			direction = com.parameters[4] - 1;
		}

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

	if (event1 != nullptr && event2 != nullptr) {
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

	if (com.parameters.size() <= 6 || !Player::IsRPG2k3Commands()) {
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
	int shake_cmd = 0;
	if (com.parameters.size() > 4 && Player::IsRPG2k3Commands()) {
		shake_cmd = com.parameters[4];
	}

	switch (shake_cmd) {
		case 0:
			if (tenths > 0) {
				screen->ShakeOnce(strength, speed, tenths * DEFAULT_FPS / 10);
				if (wait) {
					SetupWait(tenths);
				}
			} else {
				screen->ShakeEnd();
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
	// Few games use a greater strength value to achieve more intense but glitchy weather
	int strength = std::min(str, 2);

	if (!Player::IsRPG2k3Commands() && type > 2) {
		type = 0;
	}

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

		if (!Player::IsRPG2k3ECommands()) {
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
	if (!Player::IsEnglish() && !Player::game_config.patch_unlock_pics.Get() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	Game_Pictures::ShowParams params = {};
	params.name = ToString(com.string);
	// Maniac Patch uses the upper bits for X/Y origin, mask it away
	int pos_mode = ManiacBitmask(com.parameters[1], 0xFF);
	params.position_x = ValueOrVariable(pos_mode, com.parameters[2]);
	params.position_y = ValueOrVariable(pos_mode, com.parameters[3]);
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
		params.bottom_trans = ManiacBitmask(com.parameters[14], 0xFF);
	} else if (Player::IsRPG2k3() && !Player::IsRPG2k3E()) {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		params.bottom_trans = params.top_trans;
	}

	if (param_size > 16 && (Player::IsRPG2k3ECommands() || Player::IsPatchManiac())) {
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
			params.origin = com.parameters[1] >> 8;

			if (params.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
				params.effect_power = ValueOrVariableBitfield(com.parameters[16], 0, params.effect_power);
				int divisor = ValueOrVariableBitfield(com.parameters[16], 1, com.parameters[15]);
				if (divisor == 0) {
					divisor = 1;
				}
				params.effect_power /= divisor;
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

	if (Main_Data::game_pictures->Show(pic_id, params)) {
		if (params.origin > 0) {
			auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
			if (pic.IsRequestPending()) {
				pic.MakeRequestImportant();
				_async_op = AsyncOp::MakeYield();
			}
		}
	}

	return true;
}

bool Game_Interpreter::CommandMovePicture(lcf::rpg::EventCommand const& com) { // code 11120
	// Older versions of RPG_RT block pictures when message active.
	if (!Player::IsEnglish() && !Player::game_config.patch_unlock_pics.Get() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	Game_Pictures::MoveParams params;
	// Maniac Patch uses the upper bits for X/Y origin, mask it away
	int pos_mode = ManiacBitmask(com.parameters[1], 0xFF);
	params.position_x = ValueOrVariable(pos_mode, com.parameters[2]);
	params.position_y = ValueOrVariable(pos_mode, com.parameters[3]);
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

	if (Player::IsRPG2k() || Player::IsRPG2k3E() || Player::IsPatchManiac()) {
		if (param_size > 17 && (Player::IsRPG2k3ECommands() || Player::IsPatchManiac())) {
			// Handling of RPG2k3 1.12 chunks
			// Maniac Patch uses the upper bits for "wait is variable", mask it away
			pic_id = ValueOrVariable(ManiacBitmask(com.parameters[17], 0xFF), pic_id);
			// Currently unused by RPG Maker
			//int chars_to_replace = com.parameters[18];
			//int replace_with = com.parameters[19];
			params.magnify = ValueOrVariable(com.parameters[20], params.magnify);
			params.top_trans = ValueOrVariable(com.parameters[21], params.top_trans);
		}

		// RPG2k and RPG2k3 1.10 do not support this option
		params.bottom_trans = params.top_trans;

		if (Player::IsPatchManiac() && param_size > 16) {
			int flags = com.parameters[16] >> 8;
			int blend_mode = flags & 3;
			if (blend_mode == 1) {
				params.blend_mode = (int)Bitmap::BlendMode::Multiply;
			} else if (blend_mode == 2) {
				params.blend_mode = (int)Bitmap::BlendMode::Additive;
			} else if (blend_mode == 3) {
				params.blend_mode = (int)Bitmap::BlendMode::Overlay;
			}

			if (param_size > 17) {
				params.duration = ValueOrVariableBitfield(com.parameters[17], 2, params.duration);
			}
			params.flip_x = (flags & 16) == 16;
			params.flip_y = (flags & 32) == 32;
			params.origin = com.parameters[1] >> 8;

			if (params.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
				params.effect_power = ValueOrVariableBitfield(com.parameters[16], 0, params.effect_power);
				int divisor = ValueOrVariableBitfield(com.parameters[16], 1, com.parameters[15]);
				if (divisor == 0) {
					divisor = 1;
				}
				params.effect_power /= divisor;
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
	params.duration = std::max(Player::IsPatchManiac() ? -10000 : 0, std::min(params.duration, 10000));

	if (pic_id <= 0) {
		Output::Error("MovePicture: Requested invalid picture id ({})", pic_id);
	}

	Main_Data::game_pictures->Move(pic_id, params);

	if (params.origin > 0) {
		auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
		if (pic.IsRequestPending()) {
			pic.MakeRequestImportant();
			_async_op = AsyncOp::MakeYield();
		}
	}

	if (wait)
		SetupWait(params.duration);

	return true;
}

bool Game_Interpreter::CommandErasePicture(lcf::rpg::EventCommand const& com) { // code 11130
	// Older versions of RPG_RT block pictures when message active.
	if (!Player::IsEnglish() && !Player::game_config.patch_unlock_pics.Get() && Game_Message::IsMessageActive()) {
		return false;
	}

	int pic_id = com.parameters[0];

	if (com.parameters.size() > 1 && Player::IsRPG2k3ECommands()) {
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

bool Game_Interpreter::CommandPlayerVisibility(lcf::rpg::EventCommand const& com) { // code 11310
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

	// Mouse buttons checked first (Maniac checks them last) to prevent conflict
	// when DECISION is mapped to MOUSE_LEFT
	// The order of checking matches the Maniac behaviour
	if (keys[Keys::eMouseScrollDown] && check(Input::SCROLL_DOWN)) {
		return 1001;
	}

	if (keys[Keys::eMouseScrollUp] && check(Input::SCROLL_UP)) {
		return 1004;
	}

	if (keys[Keys::eMouseMiddle] && check(Input::MOUSE_MIDDLE)) {
		return 1007;
	}

	if (keys[Keys::eMouseRight] && check(Input::MOUSE_RIGHT)) {
		return 1006;
	}

	if (keys[Keys::eMouseLeft] && check(Input::MOUSE_LEFT)) {
		return 1005;
	}

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

	_keyinput.keys[Keys::eDecision] = check_key(3u, true);
	_keyinput.keys[Keys::eCancel] = check_key(4u, true);

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
			_keyinput.keys[Keys::eDown] = check_key(6u);
			_keyinput.keys[Keys::eLeft] = check_key(7u);
			_keyinput.keys[Keys::eRight] = check_key(8u);
			_keyinput.keys[Keys::eUp] = check_key(9u);
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
			_keyinput.keys[Keys::eNumbers] = check_key(5u);
			_keyinput.keys[Keys::eOperators] = check_key(6u);
			_keyinput.time_variable = param_size > 7 ? com.parameters[7] : 0; // Attention: int, not bool
			_keyinput.timed = check_key(8u);
			if (param_size > 10 && Player::IsMajorUpdatedVersion()) {
				// For Rpg2k3 >=1.05
				// ManiacPatch Middle & Wheel only handled for 2k3 Major Updated,
				// the only version that has this patch
				_keyinput.keys[Keys::eShift] = check_key(9u, true);
				_keyinput.keys[Keys::eDown] = check_key(10u, true);
				_keyinput.keys[Keys::eLeft] = check_key(11u);
				_keyinput.keys[Keys::eRight] = check_key(12u);
				_keyinput.keys[Keys::eUp] = check_key(13u, true);
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
#if !defined(USE_MOUSE_OR_TOUCH) || !defined(SUPPORT_MOUSE_OR_TOUCH)
			if (result) {
				Output::Warning("ManiacPatch: Mouse input is not supported on this platform");
				result = false;
			}
#endif
			return result;
		};
		_keyinput.keys[Keys::eMouseLeft] = check_mouse(3u);
		_keyinput.keys[Keys::eMouseRight] = check_mouse(4u);
		_keyinput.keys[Keys::eMouseMiddle] = check_mouse(9u);
		_keyinput.keys[Keys::eMouseScrollDown] = check_mouse(10u);
		_keyinput.keys[Keys::eMouseScrollUp] = check_mouse(13u);
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
		if (!AsyncHandler::RequestFile("Panorama", params.name)->IsReady()) {
			_async_op = AsyncOp::MakeYield();
		}
	}

	return true;
}

bool Game_Interpreter::CommandChangeEncounterSteps(lcf::rpg::EventCommand const& com) { // code 11740
	int steps = com.parameters[0];

	Game_Map::SetEncounterSteps(steps);

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
		value2 = ValueOrVariable(com.parameters[2], com.parameters[3]);
		result = CheckOperator(value1, value2, com.parameters[4]);
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
			Output::Warning("ConditionalBranch: Invalid vehicle ID {}", static_cast<int>(vehicle_id));
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
		result = Main_Data::game_system->BgmPlayedOnce();
		break;
	case 10:
		if (Player::IsRPG2k3Commands()) {
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
		}
		break;
	case 11:
		// RPG Maker 2003 v1.11 features
		if (Player::IsRPG2k3ECommands()) {
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
		}
		break;
	case 12:
		// Maniac: Other
		if (Player::IsPatchManiac()) {
			switch (com.parameters[1]) {
				case 0:
					result = Main_Data::game_system->IsLoadedThisFrame();
					break;
				case 1:
					// Joypad is active (We always read from Controller so simply report 'true')
#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
					result = true;
#else
					result = false;
#endif
					break;
				case 2:
					// FIXME: Window has focus. Needs function exposed in DisplayUi
					// Assuming 'true' as Player usually suspends when loosing focus
					result = true;
					break;
			}
		}
		break;
	case 13:
		// Maniac: Switch through Variable
		if (Player::IsPatchManiac()) {
			result = Main_Data::game_switches->Get(Main_Data::game_variables->Get(com.parameters[1])) == (com.parameters[2] == 0);
		}
		break;
	case 14:
		// Maniac: Variable indirect
		if (Player::IsPatchManiac()) {
			value1 = Main_Data::game_variables->GetIndirect(com.parameters[1]);
			value2 = ValueOrVariable(com.parameters[2], com.parameters[3]);
			result = CheckOperator(value1, value2, com.parameters[4]);
		}
		break;
	case 15:
		// Maniac: string comparison
		if (Player::IsPatchManiac()) {
			int modes[] = {
				(com.parameters[1]     ) & 15, //str_l mode: 0 = direct, 1 = indirect
				(com.parameters[1] >> 4) & 15, //str_r mode: 0 = literal, 1 = direct, 2 = indirect
			};

			int op = com.parameters[4] & 3;
			int ignoreCase = com.parameters[4] >> 8 & 1;

			std::string str_param = ToString(com.string);
			StringView str_l = Main_Data::game_strings->GetWithMode(str_param, modes[0]+1, com.parameters[2], *Main_Data::game_variables);
			StringView str_r = Main_Data::game_strings->GetWithMode(str_param, modes[1], com.parameters[3], *Main_Data::game_variables);
			result = ManiacPatch::CheckString(str_l, str_r, op, ignoreCase);
		}
		break;
	case 16:
		// Maniac: Expression
		result = ManiacPatch::ParseExpression(MakeSpan(com.parameters).subspan(6), *this);
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
	if (!Player::IsPatchManiac() || com.parameters.size() < 5 || com.parameters[0] == 0) {
		// Infinite loop
		return true;
	}

	int type = com.parameters[0];

	auto& frame = GetFrame();
	auto& index = frame.current_command;
	frame.maniac_loop_info.resize((com.indent + 1) * 2);
	frame.maniac_loop_info_size = static_cast<int32_t>(frame.maniac_loop_info.size() / 2);

	int32_t& begin_loop_val = frame.maniac_loop_info[frame.maniac_loop_info.size() - 2];
	int32_t& end_loop_val = frame.maniac_loop_info[frame.maniac_loop_info.size() - 1];
	begin_loop_val = 0;
	end_loop_val = 0;

	int begin_arg = ValueOrVariableBitfield(com.parameters[1], 0, com.parameters[2]);
	int end_arg = ValueOrVariableBitfield(com.parameters[1], 1, com.parameters[3]);
	int op = com.parameters[1] >> 8;

	switch (type) {
		case 1: // X times
			end_loop_val = begin_arg - 1;
			break;
		case 2: // Count up
		case 3: // Count down
			begin_loop_val = begin_arg;
			end_loop_val = end_arg;
			break;
		case 4: // While
		case 5: // Do While
			break;
		default:
			SkipToNextConditional({Cmd::EndLoop}, com.indent);
			++index;
			return true;
	}

	int check_beg = begin_loop_val;
	int check_end = end_loop_val;
	if (type == 4) { // While
		check_beg = ValueOrVariableBitfield(com.parameters[1], 0, com.parameters[2]);
		check_end = ValueOrVariableBitfield(com.parameters[1], 1, com.parameters[3]);
	}

	// Do While (5) always runs the loop at least once
	if (type != 5 && !ManiacCheckContinueLoop(check_beg, check_end, type, op)) {
		SkipToNextConditional({Cmd::EndLoop}, com.indent);
		++index;
	}

	int loop_count_var = com.parameters[4];
	if (loop_count_var > 0) {
		Main_Data::game_variables->Set(loop_count_var, begin_loop_val);
		Game_Map::SetNeedRefresh(true);
	}

	return true;
}

bool Game_Interpreter::CommandBreakLoop(lcf::rpg::EventCommand const& /* com */) { // code 12220
	auto& frame = GetFrame();
	const auto& list = frame.commands;
	auto& index = frame.current_command;

	// BreakLoop will jump to the end of the event if there is no loop.

	bool has_bug = !Player::IsPatchManiac();
	if (!has_bug) {
		SkipToNextConditional({ Cmd::EndLoop }, list[index].indent - 1);
		++index;
		return true;
	}

	// This emulates an RPG_RT bug where break loop ignores scopes and
	// unconditionally jumps to the next EndLoop command.
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

	if (Player::IsPatchManiac() && com.parameters.size() >= 5 && com.parameters[0] != 0) {
		int type = com.parameters[0];
		int offset = com.indent * 2;

		if (static_cast<int>(frame.maniac_loop_info.size()) < (offset + 1) * 2) {
			frame.maniac_loop_info.resize((offset + 1) * 2);
			frame.maniac_loop_info_size = frame.maniac_loop_info.size() / 2;
		}

		int32_t& cur_loop_val = frame.maniac_loop_info[offset];
		int32_t& end_loop_val = frame.maniac_loop_info[offset + 1];

		switch (type) {
			case 1: // X times
			case 2: // Count up
			case 4: // While
			case 5: // Do While
				++cur_loop_val;
				break;
			case 3: // Count down
				--cur_loop_val;
				break;
			default:
				++index;
				return true;
		}

		int check_cur = cur_loop_val;
		int check_end = end_loop_val;
		if (type >= 4) {
			// While (4) and Do While (5) fetch variables each loop
			// For the others it is constant
			check_cur = ValueOrVariableBitfield(com.parameters[1], 0, com.parameters[2]);
			check_end = ValueOrVariableBitfield(com.parameters[1], 1, com.parameters[3]);
		}
		int op = com.parameters[1] >> 8;
		if (!ManiacCheckContinueLoop(check_cur, check_end, type, op)) {
			// End loop
			frame.maniac_loop_info.resize(offset);
			frame.maniac_loop_info_size = offset / 2;
			++index;
			return true;
		}

		int loop_count_var = com.parameters[4];
		if (loop_count_var > 0) {
			Main_Data::game_variables->Set(loop_count_var, cur_loop_val);
			Game_Map::SetNeedRefresh(true);
		}
	}

	// Restart the loop
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
	case 0:
	case 3:
	case 4: { // Common Event
		if (com.parameters[0] == 0) {
			evt_id = com.parameters[1];
		} else if (com.parameters[0] == 3 && Player::IsPatchManiac()) {
			evt_id = Main_Data::game_variables->Get(com.parameters[1]);
		} else if (com.parameters[0] == 4 && Player::IsPatchManiac()) {
			evt_id = Main_Data::game_variables->GetIndirect(com.parameters[1]);
		} else {
			return true;
		}

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
		return true;
	}

	Game_Event* event = static_cast<Game_Event*>(GetCharacter(evt_id));
	if (!event) {
		Output::Warning("CallEvent: Can't call non-existent event {}", evt_id);
		return true;
	}

	const lcf::rpg::EventPage* page = event->GetPage(event_page);
	if (!page) {
		Output::Warning("CallEvent: Can't call non-existent page {} of event {}", event_page, evt_id);
		return true;
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
	if (!Player::IsRPG2k3Commands()) {
		return true;
	}

	int class_id = com.parameters[2]; // 0: No class, 1+: Specific class
	bool level1 = com.parameters[3] > 0;
	int skill_mode = com.parameters[4]; // no change, replace, add
	int param_mode = com.parameters[5]; // no change, halve, level 1, current level
	bool show_msg = com.parameters[6] > 0;

	if (show_msg && !Game_Message::CanShowMessage(true)) {
		return false;
	}

	PendingMessage pm(Game_Message::CommandCodeInserter);
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
	if (!Player::IsRPG2k3Commands()) {
		return true;
	}

	int cmd_id = com.parameters[2];
	bool add = com.parameters[3] != 0;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeBattleCommands(add, cmd_id);
	}

	return true;
}

bool Game_Interpreter::CommandExitGame(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	_async_op = AsyncOp::MakeExitGame();
	return true;
}

bool Game_Interpreter::CommandToggleFullscreen(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	DisplayUi->ToggleFullscreen();
	return true;
}

bool Game_Interpreter::CommandOpenVideoOptions(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	// Don't interrupt other pending game scenes for the settings menu.
	if (Scene::instance->HasRequestedScene()) {
		return false;
	}

	// FIXME: RPG_RT pops up an immediate modal dialog, while this
	// does a normal scene call. This could result in interpreter timing differences.
	Scene::instance->SetRequestedScene(std::make_shared<Scene_Settings>());
	++index;
	return false;
}

bool Game_Interpreter::CommandManiacGetSaveInfo(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int save_number = ValueOrVariable(com.parameters[0], com.parameters[1]);

	// Error case, set later on success
	Main_Data::game_variables->Set(com.parameters[2], 0);
	Main_Data::game_variables->Set(com.parameters[3], 0);
	Main_Data::game_variables->Set(com.parameters[4], 0);
	Main_Data::game_variables->Set(com.parameters[5], 0);

	if (save_number <= 0) {
		Output::Debug("ManiacGetSaveInfo: Invalid save number {}", save_number);
		return true;
	}

	auto savefs = FileFinder::Save();
	std::string save_name = Scene_Save::GetSaveFilename(savefs, save_number);
	auto save_stream = FileFinder::Save().OpenInputStream(save_name);

	if (!save_stream) {
		Output::Debug("ManiacGetSaveInfo: Save not found {}", save_number);
		return true;
	}

	auto save = lcf::LSD_Reader::Load(save_stream, Player::encoding);
	if (!save) {
		Output::Debug("ManiacGetSaveInfo: Save corrupted {}", save_number);
		// Maniac Patch writes this for whatever reason
		Main_Data::game_variables->Set(com.parameters[2], 8991230);
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
			params.top_trans = 100;
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
	auto save_stream = FileFinder::Save().OpenInputStream(save_name);
	std::unique_ptr<lcf::rpg::Save> save = lcf::LSD_Reader::Load(save_stream, Player::encoding);

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

#if !defined(USE_MOUSE_OR_TOUCH) || !defined(SUPPORT_MOUSE_OR_TOUCH)
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

bool Game_Interpreter::CommandManiacShowStringPicture(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int pic_id = ValueOrVariableBitfield(com.parameters[0], 0, com.parameters[1]);

	if (pic_id <= 0) {
		Output::Error("ShowStringPic: Requested invalid picture id ({})", pic_id);
		return true;
	}

	Game_Windows::WindowParams params = {};
	Game_Windows::WindowText text;

	params.position_x = ValueOrVariableBitfield(com.parameters[0], 1, com.parameters[2]);
	params.position_y = ValueOrVariableBitfield(com.parameters[0], 1, com.parameters[3]);
	params.magnify = ValueOrVariableBitfield(com.parameters[0], 2, com.parameters[4]);
	params.top_trans = ValueOrVariableBitfield(com.parameters[0], 3, com.parameters[5]);
	params.red = com.parameters[6];
	params.green = com.parameters[7];
	params.blue = com.parameters[8];
	params.saturation = com.parameters[9];
	params.effect_mode = com.parameters[10];
	params.effect_power = com.parameters[11];

	params.map_layer = com.parameters[15];
	params.battle_layer = com.parameters[16];
	params.flags = com.parameters[17];

	int flags = com.parameters[12];
	int blend_mode = flags & 0xF;
	if (blend_mode == 1) {
		params.blend_mode = (int)Bitmap::BlendMode::Multiply;
	} else if (blend_mode == 2) {
		params.blend_mode = (int)Bitmap::BlendMode::Additive;
	} else if (blend_mode == 3) {
		params.blend_mode = (int)Bitmap::BlendMode::Overlay;
	}
	params.flip_x = (flags & 16) == 16;
	params.flip_y = (flags & 32) == 32;
	params.origin = (com.parameters[0] & (0xF << 24)) >> 24;

	flags = com.parameters[13];
	if (params.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
		params.effect_power = ValueOrVariableBitfield(flags, 6, params.effect_power);
		int divisor = ValueOrVariableBitfield(flags, 7, com.parameters[21]);
		if (divisor == 0) {
			divisor = 1;
		}
		params.effect_power /= divisor;
	}

	params.fixed_to_map = (flags & 0xFF) > 0;
	params.message_stretch = (flags & 0xF00) >> 8;

	// Shifting values around to match System::Stretch enum
	if (params.message_stretch == 0) {
		params.message_stretch = 2;
	} else {
		params.message_stretch -= 1;
	}

	params.draw_frame = (flags & (1 << 12)) == 0;
	text.draw_gradient = (flags & (1 << 13)) == 0;
	text.draw_shadow = (flags & (1 << 14)) == 0;
	text.bold = (flags & (1 << 15)) > 0;
	params.border_margin = (flags & (1 << 16)) == 0;

	params.width = ValueOrVariableBitfield(com.parameters[0], 4, com.parameters[18]);
	params.height = ValueOrVariableBitfield(com.parameters[0], 4, com.parameters[19]);

	if (params.width < 0 || params.height < 0) {
		Output::Warning("ShowStringPic: Invalid window dimension {}x{} (id={})", params.width, params.height, pic_id);
		return true;
	}

	flags = com.parameters[14];
	params.use_transparent_color = (flags & 0xFF) > 0;
	text.letter_spacing = (flags & (0xFF << 8)) >> 8;
	text.line_spacing = (flags & (0xFF << 16)) >> 16;
	text.font_size = ValueOrVariableBitfield(com.parameters[0], 5, com.parameters[20]);


	// maniacs stores string parsing modes in the delimiters
	// x01 -> literal string
	// x02 -> direct reference
	// x03 -> indirect reference
	// for the displayed string, the id argument is in com.parameters[22]
	// here we are capturing all the delimiters, but currently only need to support reading the first one
	int i = 0;
	std::vector<int> delims;
	auto components = Utils::Tokenize(com.string, [p = &delims, &i](char32_t ch) {
		if (ch == '\x01' || ch == '\x02' || ch == '\x03') {
			p->push_back(static_cast<int>(ch));
			return true;
		}
		return false;
	});

	if (components.size() < 4) {
		Output::Warning("ShowStringPic: Bad text arg (id={})", pic_id);
		return true;
	}

	if (com.parameters.size() >= 23) {
		text.text = ToString(Main_Data::game_strings->GetWithMode(
			components[1],
			delims[0] - 1,
			com.parameters[22],
			*Main_Data::game_variables
		));
	} else {
		text.text = components[1];
	}

	params.system_name = components[2];
	text.font_name = components[3];

	params.texts = {text};

	bool async_wait;
	Main_Data::game_windows->Create(pic_id, params, async_wait);

	if (async_wait) {
		_async_op = AsyncOp::MakeYield();
	}

	return true;
}

bool Game_Interpreter::CommandManiacGetPictureInfo(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int pic_id = ValueOrVariable(com.parameters[0], com.parameters[3]);
	auto& pic = Main_Data::game_pictures->GetPicture(pic_id);

	if (pic.IsRequestPending()) {
		// Cannot do anything useful here without the dimensions
		pic.MakeRequestImportant();
		_async_op = AsyncOp::MakeYieldRepeat();
		return true;
	}

	const auto& data = pic.data;

	int x = 0;
	int y = 0;
	int width = pic.sprite ? pic.sprite->GetWidth() : 0;
	int height = pic.sprite ? pic.sprite->GetHeight() : 0;

	switch (com.parameters[1]) {
		case 0:
			x = Utils::RoundTo<int>(data.current_x);
			y = Utils::RoundTo<int>(data.current_y);
			break;
		case 1:
			x = Utils::RoundTo<int>(data.current_x);
			y = Utils::RoundTo<int>(data.current_y);
			width = Utils::RoundTo<int>(width * data.current_magnify / 100.0);
			height = Utils::RoundTo<int>(height * data.current_magnify / 100.0);
			break;
		case 2:
			x = Utils::RoundTo<int>(data.finish_x);
			y = Utils::RoundTo<int>(data.finish_y);
			width = Utils::RoundTo<int>(width * data.finish_magnify / 100.0);
			height = Utils::RoundTo<int>(height * data.finish_magnify / 100.0);
			break;
	}

	switch (com.parameters[2]) {
		case 1:
			// X/Y is top-left corner
			x -= (width / 2);
			y -= (height / 2);
			break;
		case 2: {
			// Left, Top, Right, Bottom
			x -= (width / 2);
			y -= (height / 2);
			width += x;
			height += y;
			break;
		}
	}

	Main_Data::game_variables->Set(com.parameters[4], x);
	Main_Data::game_variables->Set(com.parameters[5], y);
	Main_Data::game_variables->Set(com.parameters[6], width);
	Main_Data::game_variables->Set(com.parameters[7], height);

	Game_Map::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandManiacControlVarArray(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int op = com.parameters[0];
	int mode = com.parameters[1];

	int target_a = ValueOrVariableBitfield(mode, 0, com.parameters[2]);
	int length = ValueOrVariableBitfield(mode, 1, com.parameters[3]);
	int target_b = ValueOrVariableBitfield(mode, 2, com.parameters[4]);
	int last_target_a = target_a + length - 1;

	if (target_a < 1 || length <= 0) {
		return true;
	}

	switch (op) {
		case 0: {
			// Copy, assigns left to right, the others apply right to left
			int last_target_b = target_b + length - 1;
			Main_Data::game_variables->SetArray(target_b, last_target_b, target_a);
			break;
		}
		case 1:
			// Swap
			Main_Data::game_variables->SwapArray(target_a, last_target_a, target_b);
			break;
		case 2:
			// Sort asc
			Main_Data::game_variables->SortRange(target_a, last_target_a, true);
			break;
		case 3:
			// Sort desc
			Main_Data::game_variables->SortRange(target_a, last_target_a, false);
			break;
		case 4:
			// Shuffle
			Main_Data::game_variables->ShuffleRange(target_a, last_target_a);
			break;
		case 5:
			// Enumerate (target_b is a single value here, not an array)
			Main_Data::game_variables->EnumerateRange(target_a, last_target_a, target_b);
			break;
		case 6:
			// Add
			Main_Data::game_variables->AddArray(target_a, last_target_a, target_b);
			break;
		case 7:
			// Sub
			Main_Data::game_variables->SubArray(target_a, last_target_a, target_b);
			break;
		case 8:
			// Mul
			Main_Data::game_variables->MultArray(target_a, last_target_a, target_b);
			break;
		case 9:
			// Div
			Main_Data::game_variables->DivArray(target_a, last_target_a, target_b);
			break;
		case 10:
			// Mod
			Main_Data::game_variables->ModArray(target_a, last_target_a, target_b);
			break;
		case 11:
			// OR
			Main_Data::game_variables->BitOrArray(target_a, last_target_a, target_b);
			break;
		case 12:
			// AND
			Main_Data::game_variables->BitAndArray(target_a, last_target_a, target_b);
			break;
		case 13:
			// XOR
			Main_Data::game_variables->BitXorArray(target_a, last_target_a, target_b);
			break;
		case 14:
			// Shift left
			Main_Data::game_variables->BitShiftLeftArray(target_a, last_target_a, target_b);
			break;
		case 15:
			// Shift right
			Main_Data::game_variables->BitShiftRightArray(target_a, last_target_a, target_b);
			break;
		default:
			Output::Warning("ManiacControlVarArray: Unknown operation {}", op);
	}

	Game_Map::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandManiacKeyInputProcEx(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

#if !defined(SUPPORT_KEYBOARD)
	Output::Warning("Maniac KeyInputProc: Keyboard input is not supported on this platform");
	// Fallthrough: Variables are still set to 0
#endif

	int operation = com.parameters[0];
	int start_var_id = com.parameters[1];

	if (operation == 0 || operation == 1) {
		// The difference between 0 and 1 is whether the Joypad is checked
		// We do not implement the Joypad code so both behave the same
		auto keys = ManiacPatch::GetKeyRange();

		for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
			Main_Data::game_variables->Set(start_var_id + i, keys[i] ? 1 : 0);
		}
	} else if (operation == 2) {
		int key_id = ValueOrVariable(com.parameters[2], com.parameters[3]);
		bool key_state = ManiacPatch::GetKeyState(key_id);
		Main_Data::game_variables->Set(start_var_id, key_state ? 1 : 0);
	} else {
		Output::Warning("Maniac KeyInputProcEx: Joypad not supported");
	}

	Game_Map::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandManiacRewriteMap(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command RewriteMap not supported");
	return true;
}

bool Game_Interpreter::CommandManiacControlGlobalSave(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int operation = com.parameters[0];

	static bool was_loaded = false; // FIXME

	if (operation == 0 || (!was_loaded && (operation == 4 || operation == 5))) {
		was_loaded = true;
		// Load
		auto lgs = FileFinder::Save().OpenFile("Save.lgs");
		if (!lgs) {
			return true;
		}

		lcf::LcfReader reader(lgs);
		std::string header;
		reader.ReadString(header, reader.ReadInt());
		if (header.length() != 13 || header != "LcfGlobalSave") {
			Output::Debug("This is not a valid global save.");
			return true;
		}

		lcf::LcfReader::Chunk chunk;

		while (!reader.Eof()) {
			chunk.ID = reader.ReadInt();
			chunk.length = reader.ReadInt();
			switch (chunk.ID) {
				case 1: {
					Game_Switches::Switches_t switches;
					reader.Read(switches, chunk.length);
					Main_Data::game_switches_global->SetData(std::move(switches));
					break;
				}
				case 2: {
					Game_Variables::Variables_t variables;
					reader.Read(variables, chunk.length);
					Main_Data::game_variables_global->SetData(std::move(variables));
					break;
				}
				default:
					reader.Skip(chunk, "CommandManiacControlGlobalSave");
			}
		}
	}


	if (operation == 0 || operation == 1) {
		// Open / Close (no-op)
	} else if (operation == 2 || operation == 3) {
		// 2: Save (write to file)
		// 3: Save and Close
		if (!was_loaded) {
			return true;
		}

		auto savelgs_name = FileFinder::Save().FindFile("Save.lgs");
		if (savelgs_name.empty()) {
			savelgs_name = "Save.lgs";
		}

		auto lgs_out = FileFinder::Save().OpenOutputStream(savelgs_name);
		if (!lgs_out) {
			Output::Warning("Maniac ControlGlobalSave: Saving failed");
			return true;
		}

		lcf::LcfWriter writer(lgs_out, lcf::EngineVersion::e2k3);
		writer.WriteInt(13);
		const std::string header = "LcfGlobalSave";
		writer.Write(header);
		writer.WriteInt(1);
		writer.WriteInt(Main_Data::game_switches_global->GetSize());
		writer.Write(Main_Data::game_switches_global->GetData());
		writer.WriteInt(2);
		writer.WriteInt(Main_Data::game_variables_global->GetSize() * sizeof(int32_t));
		writer.Write(Main_Data::game_variables_global->GetData());
	} else if (operation == 4 || operation == 5) {
		int type = com.parameters[2];
		int game_state_idx = ValueOrVariableBitfield(com.parameters[1], 0, com.parameters[3]);
		int global_save_idx = ValueOrVariableBitfield(com.parameters[1], 1, com.parameters[4]);
		int length = ValueOrVariableBitfield(com.parameters[1], 2, com.parameters[5]);

		if (operation == 4) {
			// Copy from global save to game state
			for (int i = 0; i < length; ++i) {
				if (type == 0) {
					Main_Data::game_switches->Set(game_state_idx, Main_Data::game_switches_global->Get(global_save_idx));
				} else if (type == 1) {
					Main_Data::game_variables->Set(game_state_idx, Main_Data::game_variables_global->Get(global_save_idx));
				}
				++game_state_idx;
				++global_save_idx;
			}

			Game_Map::SetNeedRefresh(true);
		} else {
			// Copy from game state to global save
			for (int i = 0; i < length; ++i) {
				if (type == 0) {
					Main_Data::game_switches_global->Set(global_save_idx, Main_Data::game_switches->Get(game_state_idx));
				} else if (type == 1) {
					Main_Data::game_variables_global->Set(global_save_idx, Main_Data::game_variables->Get(game_state_idx));
				}
				++game_state_idx;
				++global_save_idx;
			}
		}
	}

	return true;
}

bool Game_Interpreter::CommandManiacChangePictureId(lcf::rpg::EventCommand const&) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	Output::Warning("Maniac Patch: Command ChangePictureId not supported");
	return true;
}

bool Game_Interpreter::CommandManiacSetGameOption(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int operation = com.parameters[1];
	//int value = ValueOrVariable(com.parameters[0], com.parameters[2]);

	switch (operation) {
		case 2: // Change Picture Limit (noop, we support arbitrary amount of pictures)
			break;
		default:
			Output::Warning("Maniac SetGameOption: Operation {} not supported", operation);
	}

	return true;
}

bool Game_Interpreter::CommandManiacControlStrings(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}
	//*parameter 0 - Modifier of the operation members
	//	-bits 0..3:
	//		Refers to the String mode: t[x], t[x..y], t[v[x]], t[v[x]..v[y]]
	//	-bits 4..7:
	//		Refers to the first argument, which can change depending on the type of operation:  x, v[x], v[v[x]] | string, t[x], t[v[x]]
	//	-bits 8..11:
	//		Refers to the second argument, which can change depending on the type of operation: x, v[x], v[v[x]] | string, t[x], t[v[x]]
	//	-bits 12..15:
	//		Refers to the third argument, which can change depending on the type of operation:  x, v[x], v[v[x]] | string, t[x], t[v[x]]
	//	-bits 16..19:
	//		Refers to the fourth argument, which can change depending on the type of operation: x, v[x], v[v[x]] | string, t[x], t[v[x]] (edge case for exMatch)
	//
	//*parameter 1 - String index 0
	//
	//*parameter 2 - String index 1 (optional for ranges)
	//
	//*parameter 3 - general flags
	//	-byte0:
	//		Refers to the type of operation: asg, cat, toNum...
	//	-byte1:
	//		It is a flag that indicates sub-operation: actor[x].name, .actor[x].desc, ins, rep, subs, join... Used only in asg and cat operations
	//	-byte2:
	//		Flags, such as: extract, hex... There is also an edge case where the last argument of exRep is here
	//
	//*parameters 4..n - arguments
	int string_mode = com.parameters[0] & 15;
	int string_id_0 = com.parameters[1];
	int string_id_1 = com.parameters[2]; //for ranges

	int is_range = string_mode & 1;

	if (string_mode >= 2) {
		string_id_0 = Main_Data::game_variables->Get(string_id_0);
	}
	if (string_mode == 3) {
		string_id_1 = Main_Data::game_variables->Get(string_id_1);
	}

	int op = (com.parameters[3] >>  0) & 255;
	int fn = (com.parameters[3] >>  8) & 255;
	int flags = (com.parameters[3] >> 16) & 255;

	int hex_flag     = (flags >> 1) & 1;
	int extract_flag = (flags >> 2) & 1;
	int first_flag = (flags >> 3) & 1;

	int args[] = {
		com.parameters[4],
		com.parameters[5],
		com.parameters[6],
		0
	};
	if (com.parameters.size() > 7) {
		args[3] = com.parameters[7]; //The exMatch command is a edge case that uses 4 arguments
	}
	int modes[] = {
		(com.parameters[0] >>  4) & 15,
		(com.parameters[0] >>  8) & 15,
		(com.parameters[0] >> 12) & 15,

		(com.parameters[0] >> 16) & 15
	};

	std::string str_param = ToString(com.string);
	std::string result;
	Game_Strings::Str_Params str_params = {
		string_id_0,
		hex_flag,
		extract_flag,
	};

	switch (op)
	{
	case 0: //asg <fn(string text)>
	case 1: //cat <fn(string text)>
		switch (fn)
		{
		case 0: //String <fn(string text, int min_size)>
			result = ToString(Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables));
			args[1] = ValueOrVariable(modes[1], args[1]);

			// min_size
			result = Game_Strings::PrependMin(result, args[1], ' ');
			break;
		case 1: //Number <fn(int number, int min_size)>
			args[0] = ValueOrVariable(modes[0], args[0]);
			args[1] = ValueOrVariable(modes[1], args[1]);

			result = std::to_string(args[0]);
			result = Game_Strings::PrependMin(result, args[1], '0');
			break;
		case 2: //Switch <fn(int number, int min_size)>
		{
			if (modes[0] == 1) args[0] = Main_Data::game_variables->Get(args[0]);
			args[1] = ValueOrVariable(modes[1], args[1]);

			if (Main_Data::game_switches->Get(args[0])) {
				result = "ON";
			} else {
				result = "OFF";
			}
			result = Game_Strings::PrependMin(result, args[1], ' ');
			break;
		}
		case 3: //Database Names <fn(int database_id, int entity_id, bool dynamic)>
			args[1] = ValueOrVariable(modes[1], args[1]);
			result = ToString(ManiacPatch::GetLcfName(args[0], args[1], (bool)args[2]));
			break;
		case 4: //Database Descriptions <fn(int id, bool dynamic)>
			args[1] = ValueOrVariable(modes[1], args[1]);
			result = ToString(ManiacPatch::GetLcfDescription(args[0], args[1], (bool)args[2]));
			break;
		case 6: //Concatenate (cat) <fn(int id_or_length_a, int id_or_length_b, int id_or_length_c)>
		{
			int pos = 0;
			std::string op_string;
			for (int i = 0; i < 3; i++) {
				op_string += ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[i], args[i], &pos, *Main_Data::game_variables));
			}
			result = std::move(op_string);
			break;
		}
		case 7: //Insert (ins) <fn(string base, int index, string insert)>
		{
			int pos = 0;
			std::string base, insert;

			args[1] = ValueOrVariable(modes[1], args[1]);
			base = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[0], args[0], &pos, *Main_Data::game_variables));
			insert = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[2], args[2], &pos, *Main_Data::game_variables));

			result = base.insert(args[1], insert);
			break;
		}
		case 8: //Replace (rep) <fn(string base, string search, string replacement)>
		{
			int pos = 0;
			std::string base, search, replacement;

			base = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[0], args[0], &pos, *Main_Data::game_variables));
			search = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[1], args[1], &pos, *Main_Data::game_variables));
			replacement = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[2], args[2], &pos, *Main_Data::game_variables));

			std::size_t index = base.find(search);
			while (index != std::string::npos) {
				base.replace(index, search.length(), replacement);
				index = base.find(search, index + replacement.length());
			}

			result = std::move(base);
			break;
		}
		case 9: //Substring (subs) <fn(string base, int index, int size)>
			args[1] = ValueOrVariable(modes[1], args[1]);
			args[2] = ValueOrVariable(modes[2], args[2]);
			result = ToString(Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables).substr(args[1], args[2]));
			break;
		case 10: //Join (join) <fn(string delimiter, int id, int size)>
		{
			std::string op_string;
			std::string delimiter = ToString(Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables));

			// args[1] & mode[1] relates to starting ID for strings to join
			// mode 0 = id literal, 1 = direct var, 2 = var literal, 3 = direct var
			args[1] = ValueOrVariable(modes[1] % 2, args[1]);
			args[2] = ValueOrVariable(modes[2], args[2]);

			while (args[2] > 0) {
				if (modes[1] < 2) {
					op_string += ToString(Main_Data::game_strings->Get(args[1]++));
				} else {
					op_string += std::to_string(Main_Data::game_variables->Get(args[1]++));
				}

				if (--args[2] > 0) op_string += delimiter;
			}

			result = op_string;
			break;
		}
		case 12: //File (file) <fn(string filename, int encode)>
		{
			// maniacs does not like a file extension
			StringView filename = Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables);
			// args[1] is the encoding... 0 for ansi, 1 for utf8
			bool do_yield;
			result = Game_Strings::FromFile(filename, args[1], do_yield);

			if (do_yield) {
				// Wait for text file download and repeat
				_async_op = AsyncOp::MakeYieldRepeat();
				return true;
			}

			break;
		}
		case 13: //Remove (rem) <fn(string base, int index, int size)>
			args[1] = ValueOrVariable(modes[1], args[1]);
			args[2] = ValueOrVariable(modes[2], args[2]);
			result = ToString(Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables));
			result = result.erase(args[1], args[2]);
			break;
		case 14: //Replace Ex (exRep) <fn(string base, string search, string replacement, bool first)>, edge case: the arg "first" is at ((flags >> 19) & 1). Wtf BingShan
		{
			int pos = 0;
			std::string base, search, replacement;

			base = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[0], args[0], &pos, *Main_Data::game_variables));
			search = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[1], args[1], &pos, *Main_Data::game_variables));
			replacement = ToString(Main_Data::game_strings->GetWithModeAndPos(str_param, modes[2], args[2], &pos, *Main_Data::game_variables));

			std::regex rexp(search);

			if (first_flag) result = std::regex_replace(base, rexp, replacement, std::regex_constants::format_first_only);
			else result =            std::regex_replace(base, rexp, replacement);
			break;
		}
		default:
			Output::Warning("Unknown or unimplemented string sub-operation {}", op);
			break;
		}
		if (is_range) Main_Data::game_strings->RangeOp(str_params, string_id_1, result, op, nullptr, *Main_Data::game_variables);
		else {
			if (op == 0) Main_Data::game_strings->Asg(str_params, result);
			if (op == 1) Main_Data::game_strings->Cat(str_params, result);
		}
		break;
	case 2: //toNum <fn(int var_id)> takes hex
	case 3: //getLen <fn(int var_id)>
		if (is_range) Main_Data::game_strings->RangeOp(str_params, string_id_1, result, op, args, *Main_Data::game_variables);
		else {
			if (op == 2) {
				int num = Main_Data::game_strings->ToNum(str_params, args[0], *Main_Data::game_variables);
				Main_Data::game_variables->Set(args[0], num);
			}
			if (op == 3) Main_Data::game_strings->GetLen(str_params, args[0], *Main_Data::game_variables);
		}
		break;
	case 4: //inStr <fn(string text, int var_id, int begin)> FIXME: takes hex?
	{
		StringView search = Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables);
		args[1] = ValueOrVariable(modes[1], args[1]); // not sure this is necessary but better safe
		args[2] = ValueOrVariable(modes[2], args[2]);

		if (is_range) Main_Data::game_strings->RangeOp(str_params, string_id_1, ToString(search), op, args, *Main_Data::game_variables);
		else          Main_Data::game_strings->InStr(str_params, ToString(search), args[1], args[2], *Main_Data::game_variables);
		break;
	}
	case 5: //split <fn(string text, int str_id, int var_id)> takes hex
	{
		StringView delimiter = Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables);
		args[1] = ValueOrVariable(modes[1], args[1]);
		args[2] = ValueOrVariable(modes[2], args[2]);

		if (is_range) Main_Data::game_strings->RangeOp(str_params, string_id_1, ToString(delimiter), op, args, *Main_Data::game_variables);
		else          Main_Data::game_strings->Split(str_params, ToString(delimiter), args[1], args[2], *Main_Data::game_variables);
		break;
	}
	case 7: //toFile <fn(string filename, int encode)>  takes hex
	{
		StringView filename = Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables);
		args[1] = ValueOrVariable(modes[1], args[1]);

		Main_Data::game_strings->ToFile(str_params, ToString(filename), args[1]);
		break;
	}
	case 8: //popLine <fn(int output_str_id)> takes hex
		// a range parameter with popLine doesn't affect multiple strings;
		// it instead alters the behavior.
		// given a range t[a..b], it will pop the first (b-a)+1 lines,
		// and store the last popped line into the output string.
		args[1] = ValueOrVariable(modes[0], args[0]);

		if (is_range) Main_Data::game_strings->PopLine(str_params, string_id_1 - string_id_0, args[0]);
		else          Main_Data::game_strings->PopLine(str_params, 0, args[0]);
		break;
	case 9: //exInStr <fn(string text, int var_id, int begin)>
	case 10: //exMatch <fn(string text, int var_id, int begin, int str_id)>, edge case: the only command that generates 8 parameters instead of 7
	{
		// takes hex
		std::string expr = ToString(Main_Data::game_strings->GetWithMode(str_param, modes[0], args[0], *Main_Data::game_variables));
		args[1] = ValueOrVariable(modes[1], args[1]); // output var
		args[2] = ValueOrVariable(modes[2], args[2]); // beginning pos

		if (is_range) {
			Main_Data::game_strings->RangeOp(str_params, string_id_1, expr, op, args, *Main_Data::game_variables);
		}
		else {
			if (op == 9) Main_Data::game_strings->ExMatch(str_params, expr, args[1], args[2], -1, *Main_Data::game_variables);
			else         Main_Data::game_strings->ExMatch(str_params, expr, args[1], args[2], args[3], *Main_Data::game_variables);
		}
		break;
	}
	default:
		Output::Warning("Unknown or unimplemented string operation {}", op);
		break;
	}
	return true;
}

bool Game_Interpreter::CommandManiacCallCommand(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	lcf::rpg::EventCommand cmd;
	cmd.code = ValueOrVariableBitfield(com.parameters[0], 0, com.parameters[1]);

	cmd.string = lcf::DBString(CommandStringOrVariableBitfield(com, 0, 3, 4));

	int arr_begin = ValueOrVariableBitfield(com.parameters[0], 1, com.parameters[2]);
	int arr_length = ValueOrVariableBitfield(com.parameters[0], 2, com.parameters[3]);

	std::vector<int32_t> output_args;
	if (arr_length > 0) {
		output_args.reserve(arr_length);
		for (int i = 0; i < arr_length; ++i) {
			output_args.push_back(Main_Data::game_variables->Get(arr_begin + i));
		}
	}

	cmd.parameters = lcf::DBArray<int32_t>(output_args.begin(), output_args.end());

	// Our implementation pushes a new frame containing the command instead of invoking it directly.
	// This is incompatible to Maniacs but has a better compatibility with our code.
	Push({ cmd }, GetCurrentEventId(), false);

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

bool Game_Interpreter::CheckOperator(int val, int val2, int op) const {
	switch (op) {
		case 0:
			return val == val2;
		case 1:
			return val >= val2;
		case 2:
			return val <= val2;
		case 3:
			return val > val2;
		case 4:
			return val < val2;
		case 5:
			return val != val2;
		default:
			return false;
	}
}

bool Game_Interpreter::ManiacCheckContinueLoop(int val, int val2, int type, int op) const {
	switch (type) {
		case 0: // Infinite loop
			return true;
		case 1: // X times
		case 2: // Count up
			return val <= val2;
		case 3: // Count down
			return val >= val2;
		case 4: // While
		case 5: // Do While
			return CheckOperator(val, val2, op);
		default:
			return false;
	}
}

int Game_Interpreter::ManiacBitmask(int value, int mask) const {
	if (Player::IsPatchManiac()) {
		return value & mask;
	}

	return value;
}
