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
#include "game_interpreter.h"
#include "audio.h"
#include "filefinder.h"
#include "game_map.h"
#include "game_event.h"
#include "game_enemyparty.h"
#include "game_player.h"
#include "game_targets.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_message.h"
#include "game_picture.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_map.h"
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include "reader_util.h"
#include "game_battle.h"
#include "utils.h"
#include "transition.h"

namespace {
	constexpr int max_var_size_2k3 = 9999999;
	constexpr int min_var_size_2k3 = -max_var_size_2k3;
	constexpr int max_var_size_2k = 999999;
	constexpr int min_var_size_2k = -max_var_size_2k;

	// Used to ensure that the interpreter that runs after a Erase/ShowScreen
	// is the invoker of the transition
	static Game_Interpreter* transition_owner = nullptr;
}

Game_Interpreter::Game_Interpreter(int _depth, bool _main_flag) {
	depth = _depth;
	main_flag = _main_flag;
	index = 0;
	updating = false;
	clear_child = false;

	if (depth > 100) {
		Output::Warning("Interpreter: Maximum callstack depth (100) exceeded");
	}

	Clear();
}

Game_Interpreter::~Game_Interpreter() {
}

// Clear.
void Game_Interpreter::Clear() {
	map_id = 0;						// map ID when starting up
	event_id = 0;					// event ID
	wait_count = 0;					// wait count
	waiting_battle_anim = false;
	triggered_by_decision_key = false;
	continuation = NULL;			// function to execute to resume command
	button_timer = 0;
	wait_messages = false;			// wait if message window is visible
	if (child_interpreter) {		// clear child interpreter for called events
		if (child_interpreter->updating)
			clear_child = true;
		else
			child_interpreter.reset();
	}
	list.clear();
}

// Is interpreter running.
bool Game_Interpreter::IsRunning() const {
	return !list.empty();
}

// Setup.
void Game_Interpreter::Setup(
	const std::vector<RPG::EventCommand>& _list,
	int _event_id,
	bool started_by_decision_key
) {
	Clear();

	map_id = Game_Map::GetMapId();
	event_id = _event_id;

	if (depth <= 100) {
		list = _list;
	}

	triggered_by_decision_key = started_by_decision_key;

	index = 0;

	CancelMenuCall();

	if (main_flag && depth == 0)
		Game_Message::SetFaceName("");
}

void Game_Interpreter::CancelMenuCall() {
	// TODO
}

void Game_Interpreter::SetupWait(int duration) {
	if (duration == 0) {
		// 0.0 waits 1 frame
		wait_count = 1;
	} else {
		wait_count = duration * DEFAULT_FPS / 10;
	}
}

void Game_Interpreter::SetContinuation(Game_Interpreter::ContinuationFunction func) {
	continuation = func;
}

// Update
void Game_Interpreter::Update() {
	updating = true;
	// 10000 based on: https://gist.github.com/4406621
	for (loop_count = 0; loop_count < 10000; ++loop_count) {
		/* If map is different than event startup time
		set event_id to 0 */
		if (Game_Map::GetMapId() != map_id) {
			event_id = 0;
		}

		/* If there's any active child interpreter, update it */
		if (child_interpreter) {

			child_interpreter->Update();

			if (!child_interpreter->IsRunning() || clear_child) {
				child_interpreter.reset();
				clear_child = false;
			}

			// If child interpreter still exists
			if (child_interpreter) {
				break;
			}
		}

		if (main_flag) {
			if (Main_Data::game_player->IsBoardingOrUnboarding())
				break;

			if (Game_Message::message_waiting)
				break;
		} else {
			if ((Game_Message::visible || Game_Message::message_waiting) && wait_messages)
				break;
		}

		if (Game_Temp::transition_processing) {
			break;
		}

		if (transition_owner && transition_owner != this) {
			break;
		} else {
			transition_owner = nullptr;
		}

		if (wait_count > 0) {
			wait_count--;
			break;
		}

		if ((Game_Temp::battle_calling && !Game_Temp::battle_running) ||
			Game_Temp::shop_calling ||
			Game_Temp::name_calling ||
			Game_Temp::menu_calling ||
			Game_Temp::save_calling ||
			Game_Temp::load_calling ||
			Game_Temp::to_title ||
			Game_Temp::gameover) {

			break;
		}

		if (continuation) {
			bool result;
			if (index >= list.size()) {
				result = (this->*continuation)(RPG::EventCommand());
			} else {
				result = (this->*continuation)(list[index]);
			}

			if (result)
				continue;
			else
				break;
		}

		if (!Main_Data::game_player->IsTeleporting() && Game_Map::GetNeedRefresh()) {
			Game_Map::Refresh();
		}

		if (list.empty()) {
			break;
		}

		if (!ExecuteCommand()) {
			break;
		}

		if (Game_Temp::battle_running && Player::IsRPG2k3() && Game_Battle::CheckWin()) {
			// Interpreter is cancelled when a win condition is fulfilled in RPG2k3 battle
			break;
		}

		// FIXME?
		// After calling SkipTo this index++ will skip execution of e.g. END.
		// This causes a different timing because loop_count reaches 10000
		// faster then Player does.
		// No idea if any game depends on this special case.
		index++;
	} // for

	if (loop_count > 9999) {
		// Executed Events Count exceeded (10000)
		Output::Debug("Event %d exceeded execution limit", event_id);
	}

	updating = false;
}

// Setup Starting Event
void Game_Interpreter::Setup(Game_Event* ev) {
	Setup(ev->GetList(), ev->GetId(), ev->WasStartedByDecisionKey());
	event_info.x = ev->GetX();
	event_info.y = ev->GetY();
	event_info.page = ev->GetActivePage();
	ev->ClearStarting();
}

void Game_Interpreter::Setup(Game_CommonEvent* ev, int caller_id) {
	Setup(ev->GetList(), caller_id, false);
	event_info.x = ev->GetIndex();
}

void Game_Interpreter::CheckGameOver() {
	if (!Game_Temp::battle_running && !Main_Data::game_party->IsAnyActive()) {
		// Empty party is allowed
		Game_Temp::gameover = Main_Data::game_party->GetBattlerCount() > 0;
	}
}

// Skip to command.
bool Game_Interpreter::SkipTo(int code, int code2, int min_indent, int max_indent, bool otherwise_end) {
	if (code2 < 0)
		code2 = code;
	if (min_indent < 0)
		min_indent = list[index].indent;
	if (max_indent < 0)
		max_indent = list[index].indent;

	int idx;
	for (idx = index; (size_t) idx < list.size(); idx++) {
		if (list[idx].indent < min_indent)
			return false;
		if (list[idx].indent > max_indent)
			continue;
		if (list[idx].code != code &&
			list[idx].code != code2)
			continue;
		index = idx;
		return true;
	}

	if (otherwise_end)
		index = idx;

	return true;
}

int Game_Interpreter::DecodeInt(std::vector<int32_t>::const_iterator& it) {
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

const std::string Game_Interpreter::DecodeString(std::vector<int32_t>::const_iterator& it) {
	std::ostringstream out;
	int len = DecodeInt(it);

	for (int i = 0; i < len; i++)
		out << (char)*it++;

	std::string result = ReaderUtil::Recode(out.str(), Player::encoding);

	return result;
}

RPG::MoveCommand Game_Interpreter::DecodeMove(std::vector<int32_t>::const_iterator& it) {
	RPG::MoveCommand cmd;
	cmd.command_id = *it++;

	switch (cmd.command_id) {
	case 32:	// Switch ON
	case 33:	// Switch OFF
		cmd.parameter_a = DecodeInt(it);
		break;
	case 34:	// Change Graphic
		cmd.parameter_string = DecodeString(it);
		cmd.parameter_a = DecodeInt(it);
		break;
	case 35:	// Play Sound Effect
		cmd.parameter_string = DecodeString(it);
		cmd.parameter_a = DecodeInt(it);
		cmd.parameter_b = DecodeInt(it);
		cmd.parameter_c = DecodeInt(it);
		break;
	}

	return cmd;
}

// Execute Command.
bool Game_Interpreter::ExecuteCommand() {
	RPG::EventCommand const& com = list[index];

	switch (com.code) {
		case Cmd::ShowMessage:
			return CommandShowMessage(com);
		case Cmd::MessageOptions:
			return CommandMessageOptions(com);
		case Cmd::ChangeFaceGraphic:
			return CommandChangeFaceGraphic(com);
		case Cmd::ShowChoice:
			return CommandShowChoices(com);
		case Cmd::ShowChoiceOption:
			return SkipTo(Cmd::ShowChoiceEnd);
		case Cmd::ShowChoiceEnd:
			return true;
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
			return true;
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
			return true;
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
			return SkipTo(Cmd::EndBranch);
		case Cmd::EndBranch:
			return true;
		case Cmd::ExitGame:
			return CommandExitGame(com);
		case Cmd::ToggleFullscreen:
			return CommandToggleFullscreen(com);
		default:
			return true;
	}
}

bool Game_Interpreter::CommandEnd() { // code 10
	if (main_flag && depth == 0) {
		Game_Message::SetFaceName("");
	}

	// FIXME: Hangs in some cases when Autostart events start
	//if (main_flag) {
	//	Game_Message::FullClear();
	//}

	list.clear();

	if (main_flag && depth == 0 && event_id > 0) {
		Game_Event* evnt = Game_Map::GetEvent(event_id);
		if (evnt)
			evnt->StopTalkToHero();
	}

	return true;
}

std::vector<std::string> Game_Interpreter::GetChoices() {
	// Let's find the choices
	int current_indent = list[index + 1].indent;
	std::vector<std::string> s_choices;
	for (unsigned index_temp = index + 1; index_temp < list.size(); ++index_temp) {
		if (list[index_temp].indent != current_indent) {
			continue;
		}

		if (list[index_temp].code == Cmd::ShowChoiceOption) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}

		if (list[index_temp].code == Cmd::ShowChoiceEnd) {
			// End of choices found
			if (s_choices.size() > 1 && s_choices.back().empty()) {
				// Remove cancel branch
				s_choices.pop_back();
			}
			break;
		}
	}
	return s_choices;
}

bool Game_Interpreter::CommandShowMessage(RPG::EventCommand const& com) { // code 10110
	// If there's a text already, return immediately
	if (Game_Message::message_waiting)
		return false;

	// Parallel interpreters must wait until the message window is closed
	if (!main_flag && Game_Message::visible)
		return false;

	wait_messages = true;
	unsigned int line_count = 0;

	Game_Message::message_waiting = true;
	Game_Message::owner_id = event_id;

	// Set first line
	Game_Message::texts.push_back(com.string);
	line_count++;

	for (; index + 1 < list.size(); index++) {
		// If next event command is the following parts of the message
		if (list[index+1].code == Cmd::ShowMessage_2) {
			// Add second (another) line
			line_count++;
			Game_Message::texts.push_back(list[index+1].string);
		} else {
			// If next event command is show choices
			if (list[index+1].code == Cmd::ShowChoice) {
				std::vector<std::string> s_choices = GetChoices();
				// If choices fit on screen
				if (s_choices.size() <= (4 - line_count)) {
					index++;
					Game_Message::choice_start = line_count;
					Game_Message::choice_cancel_type = list[index].parameters[0];
					SetupChoices(s_choices);
				}
			} else if (list[index+1].code == Cmd::InputNumber) {
				// If next event command is input number
				// If input number fits on screen
				if (line_count < 4) {
					index++;
					Game_Message::num_input_start = line_count;
					Game_Message::num_input_digits_max = list[index].parameters[0];
					Game_Message::num_input_variable_id = list[index].parameters[1];
				}
			}

			break;
		}
	} // End for

	return true;
}

bool Game_Interpreter::CommandMessageOptions(RPG::EventCommand const& com) { //code 10120
	Game_Message::SetTransparent(com.parameters[0] != 0);
	Game_Message::SetPosition(com.parameters[1]);
	Game_Message::SetPositionFixed(com.parameters[2] == 0);
	Game_Message::SetContinueEvents(com.parameters[3] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeFaceGraphic(RPG::EventCommand const& com) { // Code 10130
	if (Game_Message::message_waiting && Game_Message::owner_id != event_id)
		return false;

	Game_Message::SetFaceName(com.string);
	Game_Message::SetFaceIndex(com.parameters[0]);
	Game_Message::SetFaceRightPosition(com.parameters[1] != 0);
	Game_Message::SetFaceFlipped(com.parameters[2] != 0);
	return true;
}

void Game_Interpreter::SetupChoices(const std::vector<std::string>& choices) {
	Game_Message::choice_start = Game_Message::texts.size();
	Game_Message::choice_max = choices.size();
	Game_Message::choice_disabled.reset();

	// Set choices to message text
	unsigned int i;
	for (i = 0; i < 4 && i < choices.size(); i++) {
		Game_Message::texts.push_back(choices[i]);
	}

	SetContinuation(&Game_Interpreter::ContinuationChoices);
}

bool Game_Interpreter::ContinuationChoices(RPG::EventCommand const& com) {
	continuation = NULL;
	int indent = com.indent;
	for (;;) {
		if (!SkipTo(Cmd::ShowChoiceOption, Cmd::ShowChoiceEnd, indent, indent))
			return false;
		int which = list[index].parameters[0];
		index++;
		if (which > Game_Message::choice_result)
			return false;
		if (which < Game_Message::choice_result)
			continue;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandShowChoices(RPG::EventCommand const& com) { // code 10140
	if (!Game_Message::texts.empty()) {
		return false;
	}

	Game_Message::message_waiting = true;
	Game_Message::owner_id = event_id;
	wait_messages = true;
	// Choices setup
	std::vector<std::string> choices = GetChoices();
	Game_Message::choice_cancel_type = com.parameters[0];
	SetupChoices(choices);

	return true;
}

bool Game_Interpreter::CommandInputNumber(RPG::EventCommand const& com) { // code 10150
	if (Game_Message::message_waiting) {
		return false;
	}

	Game_Message::message_waiting = true;
	Game_Message::owner_id = event_id;

	Game_Message::num_input_start = 0;
	Game_Message::num_input_variable_id = com.parameters[1];
	Game_Message::num_input_digits_max = com.parameters[0];

	// Continue
	return true;
}

bool Game_Interpreter::CommandControlSwitches(RPG::EventCommand const& com) { // code 10210
	if (com.parameters[0] >= 0 && com.parameters[0] <= 2) {
		// Param0: 0: Single, 1: Range, 2: Indirect
		// For Range set end to param 2, otherwise to start, this way the loop runs exactly once

		int start = com.parameters[0] == 2 ? Game_Variables[com.parameters[1]] : com.parameters[1];
		int end = com.parameters[0] == 1 ? com.parameters[2] : start;

		for (int i = start; i <= end; ++i) {
			if (com.parameters[3] != 2) {
				Game_Switches[i] = com.parameters[3] == 0;
			} else {
				Game_Switches[i] = !Game_Switches[i];
			}
		}

		Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
	}

	return true;
}

bool Game_Interpreter::CommandControlVariables(RPG::EventCommand const& com) { // code 10220
	int value = 0;
	int i = 0;
	Game_Actor* actor;
	Game_Character* character;

	switch (com.parameters[4]) {
		case 0:
			// Constant
			value = com.parameters[5];
			break;
		case 1:
			// Var A ops B
			value = Game_Variables[com.parameters[5]];
			break;
		case 2:
			// Number of var A ops B
			value = Game_Variables[Game_Variables[com.parameters[5]]];
			break;
		case 3:
			// Random between range
			int a, b;
			a = max(com.parameters[5], com.parameters[6]);
			b = min(com.parameters[5], com.parameters[6]);
			value = Utils::GetRandomNumber(b, a);
			break;
		case 4:
			// Items
			switch (com.parameters[6]) {
				case 0:
					// Number of items posessed
					value = Main_Data::game_party->GetItemCount(com.parameters[5]);
					break;
				case 1:
					// How often the item is equipped
					value = Main_Data::game_party->GetItemCount(com.parameters[5], true);
					break;
			}
			break;
		case 5:
			// Hero
			actor = Game_Actors::GetActor(com.parameters[5]);

			if (!actor) {
				Output::Warning("ControlVariables: Invalid actor ID %d", com.parameters[5]);
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
				switch (com.parameters[6]) {
					case 0:
						// Map ID
						value = character->GetMapId();
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
						dir = character->GetSpriteDirection();
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
					value = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer1);
					break;
				case 2:
					// Number of heroes in party
					value = Main_Data::game_party->GetActors().size();
					break;
				case 3:
					// Number of saves
					value = Game_System::GetSaveCount();
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
					value = Audio().BGM_GetTicks();
					break;
				case 9:
					value = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer2);
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

		int start = com.parameters[0] == 2 ? Game_Variables[com.parameters[1]] : com.parameters[1];
		int end = com.parameters[0] == 1 ? com.parameters[2] : start;

		for (i = start; i <= end; ++i) {
			switch (com.parameters[3]) {
				case 0:
					// Assignement
					Game_Variables[i] = value;
					break;
				case 1:
					// Addition
					Game_Variables[i] += value;
					break;
				case 2:
					// Subtraction
					Game_Variables[i] -= value;
					break;
				case 3:
					// Multiplication
					Game_Variables[i] *= value;
					break;
				case 4:
					// Division
					if (value != 0) {
						Game_Variables[i] /= value;
					}
					break;
				case 5:
					// Module
					if (value != 0) {
						Game_Variables[i] %= value;
					} else {
						Game_Variables[i] = 0;
					}
			}

			// Clamp to variable range
			if (Player::IsRPG2k3()) {
				Game_Variables[i] = std::max(
						std::min(Game_Variables[i], max_var_size_2k3), min_var_size_2k3);
			} else {
				Game_Variables[i] = std::max(
						std::min(Game_Variables[i], max_var_size_2k), min_var_size_2k);
			}
		}

		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	}

	return true;
}

int Game_Interpreter::OperateValue(int operation, int operand_type, int operand) {
	int value = 0;

	if (operand_type == 0) {
		value = operand;
	} else {
		value = Game_Variables[operand];
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
		actor = Game_Actors::GetActor(id);

		if (!actor) {
			Output::Warning("Invalid actor ID %d", id);
			return actors;
		}

		actors.push_back(actor);
		break;
	case 2:
		// Var hero
		actor = Game_Actors::GetActor(Game_Variables[id]);
		if (!actor) {
			Output::Warning("Invalid actor ID %d", Game_Variables[id]);
			return actors;
		}

		actors.push_back(actor);
		break;
	}

	return actors;
}

Game_Character* Game_Interpreter::GetCharacter(int character_id) const {
	if (!event_info.page && character_id == Game_Character::CharThisEvent) {
		// Is a common event
		if (event_id == 0) {
			// With no map parent
			Output::Warning("Can't use ThisEvent in common event %d: Not called from a map event", event_info.x);
			return nullptr;
		}
	}

	Game_Character* ch = Game_Character::GetCharacter(character_id, event_id);
	if (!ch) {
		Output::Warning("Unknown event with id %d", character_id);
	}
	return ch;
}

bool Game_Interpreter::CommandTimerOperation(RPG::EventCommand const& com) { // code 10230
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

bool Game_Interpreter::CommandChangeGold(RPG::EventCommand const& com) { // Code 10310
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

bool Game_Interpreter::CommandChangeItems(RPG::EventCommand const& com) { // Code 10320
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
			Game_Variables[com.parameters[2]],
			value
		);
	}
	Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	// Continue
	return true;
}

bool Game_Interpreter::CommandChangePartyMember(RPG::EventCommand const& com) { // Code 10330
	Game_Actor* actor;
	int id;

	if (com.parameters[1] == 0) {
		id = com.parameters[2];
	} else {
		id = Game_Variables[com.parameters[2]];
	}

	actor = Game_Actors::GetActor(id);

	if (!actor) {
		Output::Warning("ChangePartyMember: Invalid actor ID %d", id);
		return true;
	}

	if (com.parameters[0] == 0) {
		// Add members
		Main_Data::game_party->AddActor(id);

	} else {
		// Remove members
		Main_Data::game_party->RemoveActor(id);

		CheckGameOver();
	}

	Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);

	// Continue
	return true;
}

bool Game_Interpreter::CommandChangeExp(RPG::EventCommand const& com) { // Code 10410
	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeExp(actor->GetExp() + value, com.parameters[5] != 0);
	}

	return true;
}

bool Game_Interpreter::CommandChangeLevel(RPG::EventCommand const& com) { // Code 10420
	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeLevel(actor->GetLevel() + value, com.parameters[5] != 0);
	}

	return true;
}

int Game_Interpreter::ValueOrVariable(int mode, int val) {
	switch (mode) {
		case 0:
			return val;
		case 1:
			return Game_Variables[val];
		default:
			return -1;
	}
}

bool Game_Interpreter::CommandChangeParameters(RPG::EventCommand const& com) { // Code 10430
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
	return true;
}

bool Game_Interpreter::CommandChangeSkills(RPG::EventCommand const& com) { // Code 10440
	bool remove = com.parameters[2] != 0;
	int skill_id = ValueOrVariable(com.parameters[3], com.parameters[4]);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		if (remove)
			actor->UnlearnSkill(skill_id);
		else
			actor->LearnSkill(skill_id);
	}

	return true;
}

bool Game_Interpreter::CommandChangeEquipment(RPG::EventCommand const& com) { // Code 10450
	int item_id;
	int slot;
	const RPG::Item* item;

	switch (com.parameters[2]) {
		case 0:
			item_id = ValueOrVariable(com.parameters[3],
									  com.parameters[4]);
			item = ReaderUtil::GetElement(Data::items, item_id);
			if (!item) {
				Output::Warning("ChangeEquipment: Invalid item ID %d", item_id);
				return true;
			}

			switch (item->type) {
				case RPG::Item::Type_weapon:
				case RPG::Item::Type_shield:
				case RPG::Item::Type_armor:
				case RPG::Item::Type_helmet:
				case RPG::Item::Type_accessory:
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
			if (actor->HasTwoWeapons() && slot == RPG::Item::Type_shield && item_id != 0) {
				continue;
			}

			if (Main_Data::game_party->GetItemCount(item_id, false) == 0 && !actor->IsEquipped(item_id)) {
				Main_Data::game_party->AddItem(item_id, 1);
			}

			if (actor->HasTwoWeapons() && slot == RPG::Item::Type_weapon) {
				RPG::Item* new_equipment = ReaderUtil::GetElement(Data::items, item_id);
				RPG::Item* equipment1 = ReaderUtil::GetElement(Data::items, actor->GetWeaponId());
				RPG::Item* equipment2 = ReaderUtil::GetElement(Data::items, actor->GetShieldId());

				if (equipment1 && !equipment2 && !equipment1->two_handed && !new_equipment->two_handed) {
					actor->ChangeEquipment(slot + 1, item_id);
					continue;
				}
			}

			actor->ChangeEquipment(slot, item_id);
		}
	}

	return true;
}

bool Game_Interpreter::CommandChangeHP(RPG::EventCommand const& com) { // Code 10460
	bool remove = com.parameters[2] != 0;
	int amount = ValueOrVariable(com.parameters[3],
								 com.parameters[4]);
	bool lethal = com.parameters[5] != 0;

	if (remove)
		amount = -amount;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		int hp = actor->GetHp() + amount;
		if (!lethal && hp <= 0) {
			amount += hp * (-1) + 1;
		}
		actor->ChangeHp(amount);

		if (actor->IsDead()) {
			Game_Battle::SetNeedRefresh(true);
		}
	}

	if (lethal) {
		CheckGameOver();
	}

	return true;
}

bool Game_Interpreter::CommandChangeSP(RPG::EventCommand const& com) { // Code 10470
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

	return true;
}

bool Game_Interpreter::CommandChangeCondition(RPG::EventCommand const& com) { // Code 10480
	bool remove = com.parameters[2] != 0;
	int state_id = com.parameters[3];

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		if (remove) {
			actor->RemoveState(state_id);
			Game_Battle::SetNeedRefresh(true);
		} else {
			if(state_id == 1) {
				actor->ChangeHp(-actor->GetHp());
			}
			actor->AddState(state_id);
			Game_Battle::SetNeedRefresh(true);
			CheckGameOver();
		}
	}

	return true;
}

bool Game_Interpreter::CommandFullHeal(RPG::EventCommand const& com) { // Code 10490
	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
		actor->RemoveAllStates();
	}

	Game_Battle::SetNeedRefresh(true);

	return true;
}

bool Game_Interpreter::CommandSimulatedAttack(RPG::EventCommand const& com) { // code 10500
	int atk = com.parameters[2];
	int def = com.parameters[3];
	int spi = com.parameters[4];
	int var = com.parameters[5];

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ResetBattle();
		int result = atk;
		result -= (actor->GetDef() * def) / 400;
		result -= (actor->GetSpi() * spi) / 800;
		if (var != 0) {
			int rperc = var * 5;
			int rval = Utils::GetRandomNumber(-rperc, rperc - 1);
			result += result * rval / 100;
		}

		result = std::max(0, result);
		actor->ChangeHp(-result);

		CheckGameOver();

		if (com.parameters[6] != 0) {
			Game_Variables[com.parameters[7]] = result;
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
		}
	}

	return true;
}

bool Game_Interpreter::CommandWait(RPG::EventCommand const& com) { // code 11410
	// Wait a given time
	if (com.parameters.size() <= 1 ||
		(com.parameters.size() > 1 && com.parameters[1] == 0)) {
		SetupWait(com.parameters[0]);
		return true;
	}

	// Wait until decision key pressed, but skip the first frame so that
	// it ignores keys that were pressed before this command started.
	if (!Game_Message::visible && button_timer > 0 && Input::IsTriggered(Input::DECISION)) {
		button_timer = 0;
		return true;
	}

	button_timer++;
	return false;
}

bool Game_Interpreter::CommandPlayBGM(RPG::EventCommand const& com) { // code 11510
	RPG::Music music;
	music.name = com.string;
	music.fadein = com.parameters[0];
	music.volume = com.parameters[1];
	music.tempo = com.parameters[2];
	music.balance = com.parameters[3];
	Game_System::BgmPlay(music);
	return true;
}

bool Game_Interpreter::CommandFadeOutBGM(RPG::EventCommand const& com) { // code 11520
	int fadeout = com.parameters[0];
	Game_System::BgmFade(fadeout);
	return true;
}

bool Game_Interpreter::CommandPlaySound(RPG::EventCommand const& com) { // code 11550
	RPG::Sound sound;
	sound.name = com.string;
	sound.volume = com.parameters[0];
	sound.tempo = com.parameters[1];
	sound.balance = com.parameters[2];
	Game_System::SePlay(sound, true);
	return true;
}

bool Game_Interpreter::CommandEndEventProcessing(RPG::EventCommand const& /* com */) { // code 12310
	index = list.size();
	return true;
}

bool Game_Interpreter::CommandGameOver(RPG::EventCommand const& /* com */) { // code 12420
	Game_Temp::gameover = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandChangeHeroName(RPG::EventCommand const& com) { // code 10610
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeHeroName: Invalid actor ID %d", com.parameters[0]);
		return true;
	}

	actor->SetName(com.string);
	return true;
}

bool Game_Interpreter::CommandChangeHeroTitle(RPG::EventCommand const& com) { // code 10620
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeHeroTitle: Invalid actor ID %d", com.parameters[0]);
		return true;
	}

	actor->SetTitle(com.string);
	return true;
}

bool Game_Interpreter::CommandChangeSpriteAssociation(RPG::EventCommand const& com) { // code 10630
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("ChangeSpriteAssociation: Invalid actor ID %d", com.parameters[0]);
		return true;
	}

	const std::string &file = com.string;
	int idx = com.parameters[1];
	bool transparent = com.parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	Main_Data::game_player->Refresh();
	return true;
}

bool Game_Interpreter::CommandChangeActorFace(RPG::EventCommand const& com) { // code 10640
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);

	if (!actor) {
		Output::Warning("CommandChangeActorFace: Invalid actor ID %d", com.parameters[0]);
		return true;
	}

	actor->SetFace(com.string, com.parameters[1]);
	return true;
}

bool Game_Interpreter::CommandChangeVehicleGraphic(RPG::EventCommand const& com) { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0] + 1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

	if (!vehicle) {
		Output::Warning("ChangeVehicleGraphic: Invalid vehicle ID %d", vehicle_id);
		return true;
	}

	const std::string& name = com.string;
	int vehicle_index = com.parameters[1];

	vehicle->SetGraphic(name, vehicle_index);

	return true;
}

bool Game_Interpreter::CommandChangeSystemBGM(RPG::EventCommand const& com) { //code 10660
	RPG::Music music;
	int context = com.parameters[0];
	music.name = com.string;
	music.fadein = com.parameters[1];
	music.volume = com.parameters[2];
	music.tempo = com.parameters[3];
	music.balance = com.parameters[4];
	Game_System::SetSystemBGM(context, music);
	return true;
}

bool Game_Interpreter::CommandChangeSystemSFX(RPG::EventCommand const& com) { //code 10670
	RPG::Sound sound;
	int context = com.parameters[0];
	sound.name = com.string;
	sound.volume = com.parameters[1];
	sound.tempo = com.parameters[2];
	sound.balance = com.parameters[3];
	Game_System::SetSystemSE(context, sound);
	return true;
}

void Game_Interpreter::OnChangeSystemGraphicReady(FileRequestResult* result) {
	Game_System::SetSystemName(result->file);

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();

	if (!scene)
		return;

	scene->spriteset->SystemGraphicUpdated();
}

bool Game_Interpreter::CommandChangeSystemGraphics(RPG::EventCommand const& com) { // code 10680
	FileRequestAsync* request = AsyncHandler::RequestFile("System", com.string);
	request_id = request->Bind(&Game_Interpreter::OnChangeSystemGraphicReady, this);
	request->SetImportantFile(true);
	request->Start();

	Game_System::SetMessageStretch((RPG::System::Stretch)com.parameters[0]);
	Game_System::SetFontId(com.parameters[1]);

	return true;
}

bool Game_Interpreter::CommandChangeScreenTransitions(RPG::EventCommand const& com) { // code 10690
	Game_System::SetTransition(com.parameters[0], com.parameters[1]);
	return true;
}

bool Game_Interpreter::CommandMemorizeLocation(RPG::EventCommand const& com) { // code 10820
	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	Game_Variables[var_map_id] = Game_Map::GetMapId();
	Game_Variables[var_x] = player->GetX();
	Game_Variables[var_y] = player->GetY();
	Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	return true;
}

bool Game_Interpreter::CommandSetVehicleLocation(RPG::EventCommand const& com) { // code 10850
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
			Output::Warning("SetVehicleLocation: Invalid vehicle ID %d", vehicle_id);
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
				vehicle->SetPosition(map_id, x, y);
			}
			Main_Data::game_player->MoveTo(x, y);
			return true;
		};

		// This implements a bug in RPG_RT which allows moving the party to a new map while boarded (or when using -1)
		// without doing a teleport + transition.
		// The implementation of this bug does a normal teleport with transition because other solution would be too
		// invasive for little gain.

		if (Main_Data::game_player->IsTeleporting() ||
			Game_Message::visible) {
			return false;
		}

		if (vehicle) {
			vehicle->SetPosition(map_id, x, y);
		}

		Main_Data::game_player->ReserveTeleport(map_id, x, y);
		Main_Data::game_player->StartTeleport();

		// Parallel events should keep on running in 2k and 2k3, unlike in later versions
		if (!main_flag)
			return true;

		index++;
		return false;
	} else if (vehicle) {
		vehicle->SetPosition(map_id, x, y);
	}

	return true;
}

bool Game_Interpreter::CommandChangeEventLocation(RPG::EventCommand const& com) { // Code 10860
	int event_id = com.parameters[0];
	Game_Character *event = GetCharacter(event_id);
	if (event != NULL) {
		int x = ValueOrVariable(com.parameters[1], com.parameters[2]);
		int y = ValueOrVariable(com.parameters[1], com.parameters[3]);
		event->MoveTo(x, y);

		// RPG2k3 feature
		int direction = com.parameters.size() > 4 ? com.parameters[4] - 1 : -1;
		// Only for the constant case, not for variables
		if (com.parameters[1] == 0 && direction != -1) {
			event->SetDirection(direction);
			if (!(event->IsDirectionFixed() || event->IsFacingLocked()))
				event->SetSpriteDirection(direction);
		}
	}
	return true;
}

bool Game_Interpreter::CommandTradeEventLocations(RPG::EventCommand const& com) { // Code 10870
	int event1_id = com.parameters[0];
	int event2_id = com.parameters[1];

	Game_Character *event1 = GetCharacter(event1_id);
	Game_Character *event2 = GetCharacter(event2_id);

	if (event1 != NULL && event2 != NULL) {
		int x1 = event1->GetX();
		int y1 = event1->GetY();

		int x2 = event2->GetX();
		int y2 = event2->GetY();

		event1->MoveTo(x2, y2);
		event2->MoveTo(x1, y1);
	}

	return true;
}

bool Game_Interpreter::CommandStoreTerrainID(RPG::EventCommand const& com) { // code 10820
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	Game_Variables[var_id] = Game_Map::GetTerrainTag(x, y);
	Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	return true;
}

bool Game_Interpreter::CommandStoreEventID(RPG::EventCommand const& com) { // code 10920
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);
	Game_Variables[var_id] = events.size() > 0 ? events.back()->GetId() : 0;
	Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	return true;
}

bool Game_Interpreter::CommandEraseScreen(RPG::EventCommand const& com) { // code 11010
	if (Game_Temp::transition_processing || Game_Message::visible)
		return false;

	if (!main_flag) {
		Game_Map::SetTeleportDelayed(true);
	}

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = true;
	transition_owner = this;

	switch (com.parameters[0]) {
	case -1:
		Game_Temp::transition_type = (Transition::TransitionType)Game_System::GetTransition(
			Game_System::Transition_TeleportErase);
		return true;
	case 0:
		Game_Temp::transition_type = Transition::TransitionFadeOut;
		return true;
	case 1:
		Game_Temp::transition_type = Transition::TransitionRandomBlocks;
		return true;
	case 2:
		Game_Temp::transition_type = Transition::TransitionRandomBlocksUp;
		return true;
	case 3:
		Game_Temp::transition_type = Transition::TransitionRandomBlocksDown;
		return true;
	case 4:
		Game_Temp::transition_type = Transition::TransitionBlindClose;
		return true;
	case 5:
		Game_Temp::transition_type = Transition::TransitionVerticalStripesOut;
		return true;
	case 6:
		Game_Temp::transition_type = Transition::TransitionHorizontalStripesOut;
		return true;
	case 7:
		Game_Temp::transition_type = Transition::TransitionBorderToCenterOut;
		return true;
	case 8:
		Game_Temp::transition_type = Transition::TransitionCenterToBorderOut;
		return true;
	case 9:
		Game_Temp::transition_type = Transition::TransitionScrollUpOut;
		return true;
	case 10:
		Game_Temp::transition_type = Transition::TransitionScrollDownOut;
		return true;
	case 11:
		Game_Temp::transition_type = Transition::TransitionScrollLeftOut;
		return true;
	case 12:
		Game_Temp::transition_type = Transition::TransitionScrollRightOut;
		return true;
	case 13:
		Game_Temp::transition_type = Transition::TransitionVerticalDivision;
		return true;
	case 14:
		Game_Temp::transition_type = Transition::TransitionHorizontalDivision;
		return true;
	case 15:
		Game_Temp::transition_type = Transition::TransitionCrossDivision;
		return true;
	case 16:
		Game_Temp::transition_type = Transition::TransitionZoomIn;
		return true;
	case 17:
		Game_Temp::transition_type = Transition::TransitionMosaicOut;
		return true;
	case 18:
		Game_Temp::transition_type = Transition::TransitionWaveOut;
		return true;
	case 19:
		Game_Temp::transition_type = Transition::TransitionErase;
		return true;
	default:
		Game_Temp::transition_type = Transition::TransitionNone;
		return true;
	}
}

bool Game_Interpreter::CommandShowScreen(RPG::EventCommand const& com) { // code 11020
	if (Game_Temp::transition_processing || Game_Message::visible)
		return false;

	if (!main_flag) {
		Game_Map::SetTeleportDelayed(true);
	}

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = false;
	transition_owner = this;

	switch (com.parameters[0]) {
	case -1:
		Game_Temp::transition_type = (Transition::TransitionType)Game_System::GetTransition(
			Game_System::Transition_TeleportShow);
		return true;
	case 0:
		Game_Temp::transition_type = Transition::TransitionFadeIn;
		return true;
	case 1:
		Game_Temp::transition_type = Transition::TransitionRandomBlocks;
		return true;
	case 2:
		Game_Temp::transition_type = Transition::TransitionRandomBlocksUp;
		return true;
	case 3:
		Game_Temp::transition_type = Transition::TransitionRandomBlocksDown;
		return true;
	case 4:
		Game_Temp::transition_type = Transition::TransitionBlindOpen;
		return true;
	case 5:
		Game_Temp::transition_type = Transition::TransitionVerticalStripesIn;
		return true;
	case 6:
		Game_Temp::transition_type = Transition::TransitionHorizontalStripesIn;
		return true;
	case 7:
		Game_Temp::transition_type = Transition::TransitionBorderToCenterIn;
		return true;
	case 8:
		Game_Temp::transition_type = Transition::TransitionCenterToBorderIn;
		return true;
	case 9:
		Game_Temp::transition_type = Transition::TransitionScrollUpIn;
		return true;
	case 10:
		Game_Temp::transition_type = Transition::TransitionScrollDownIn;
		return true;
	case 11:
		Game_Temp::transition_type = Transition::TransitionScrollLeftIn;
		return true;
	case 12:
		Game_Temp::transition_type = Transition::TransitionScrollRightIn;
		return true;
	case 13:
		Game_Temp::transition_type = Transition::TransitionVerticalCombine;
		return true;
	case 14:
		Game_Temp::transition_type = Transition::TransitionHorizontalCombine;
		return true;
	case 15:
		Game_Temp::transition_type = Transition::TransitionCrossCombine;
		return true;
	case 16:
		Game_Temp::transition_type = Transition::TransitionZoomOut;
		return true;
	case 17:
		Game_Temp::transition_type = Transition::TransitionMosaicIn;
		return true;
	case 18:
		Game_Temp::transition_type = Transition::TransitionWaveIn;
		return true;
	case 19:
		Game_Temp::transition_type = Transition::TransitionErase;
		return true;
	default:
		Game_Temp::transition_type = Transition::TransitionNone;
		return true;
	}
}

bool Game_Interpreter::CommandTintScreen(RPG::EventCommand const& com) { // code 11030
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

bool Game_Interpreter::CommandFlashScreen(RPG::EventCommand const& com) { // code 11040
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

bool Game_Interpreter::CommandShakeScreen(RPG::EventCommand const& com) { // code 11050
	Game_Screen* screen = Main_Data::game_screen.get();
	int strength = com.parameters[0];
	int speed = com.parameters[1];
	int tenths = com.parameters[2];
	bool wait = com.parameters[3] != 0;

	if (Player::IsRPG2k()) {
		screen->ShakeOnce(strength, speed, tenths * DEFAULT_FPS / 10);
		if (wait) {
			SetupWait(tenths);
		}
	} else {
		switch (com.parameters[4]) {
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
	}

	return true;
}

bool Game_Interpreter::CommandWeatherEffects(RPG::EventCommand const& com) { // code 11070
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
				new_id = Game_Variables[pic_id - 50000];
			} else {
				new_id = Game_Variables[pic_id - 10000];
			}

			if (new_id > 0) {
				Output::Debug("PicPointer: ID %d replaced with ID %d", pic_id, new_id);
				pic_id = new_id;
			}
		}
	}

	static void AdjustParams(Game_Picture::Params& params) {
		if (params.magnify > 10000) {
			int new_magnify = Game_Variables[params.magnify - 10000];
			Output::Debug("PicPointer: Zoom %d replaced with %d", params.magnify, new_magnify);
			params.magnify = new_magnify;
		}

		if (params.top_trans > 10000) {
			int new_top_trans = Game_Variables[params.top_trans - 10000];
			Output::Debug("PicPointer: Top transparency %d replaced with %d", params.top_trans, new_top_trans);
			params.top_trans = new_top_trans;
		}

		if (params.bottom_trans > 10000) {
			int new_bottom_trans = Game_Variables[params.bottom_trans - 10000];
			Output::Debug("PicPointer: Bottom transparency %d replaced with %d", params.bottom_trans, new_bottom_trans);
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
			Output::Debug("PicPointer: File %s replaced with %s", str.c_str(), new_pic_name.c_str());
		}
		return new_pic_name;
	}

	static void AdjustShowParams(int& pic_id, Game_Picture::ShowParams& params) {
		// Adjust name
		if (pic_id >= 50000) {
			// Name substitution is pic_id + 1
			int pic_num = Game_Variables[pic_id - 50000 + 1];

			if (pic_num >= 0) {
				params.name = ReplaceName(params.name, pic_num, 4);
			}
		}

		AdjustId(pic_id);
		AdjustParams(params);
	}

	static void AdjustMoveParams(int& pic_id, Game_Picture::MoveParams& params) {
		AdjustId(pic_id);
		AdjustParams(params);

		if (params.duration > 10000) {
			int new_duration = Game_Variables[params.duration - 10000];
			Output::Debug("PicPointer: Move duration %d replaced with %d", params.duration, new_duration);
			params.duration = new_duration;
		}
	}

}

bool Game_Interpreter::CommandShowPicture(RPG::EventCommand const& com) { // code 11110
	if (Game_Temp::battle_running) {
		Output::Warning("ShowPicture: Not supported in battle");
		return true;
	}

	int pic_id = com.parameters[0];

	Game_Picture::ShowParams params = {};
	params.name = com.string;
	params.position_x = ValueOrVariable(com.parameters[1], com.parameters[2]);
	params.position_y = ValueOrVariable(com.parameters[1], com.parameters[3]);
	params.fixed_to_map = com.parameters[4] > 0;
	params.magnify = com.parameters[5];
	params.transparency = com.parameters[7] > 0;
	params.top_trans = com.parameters[6];
	params.red = com.parameters[8];
	params.green = com.parameters[9];
	params.blue = com.parameters[10];
	params.saturation = com.parameters[11];
	params.effect_mode = com.parameters[12];
	params.effect_power = com.parameters[13];

	size_t param_size = com.parameters.size();

	if (Player::IsRPG2k() || Player::IsRPG2k3E()) {
		if (param_size > 16) {
			// Handling of RPG2k3 1.12 chunks
			pic_id = ValueOrVariable(com.parameters[17], pic_id);
			if (com.parameters[19] != 0) {
				int var = 0;
				if (Game_Variables.IsValid(com.parameters[19])) {
					var = Game_Variables[com.parameters[19]];
				}
				params.name = PicPointerPatch::ReplaceName(params.name, var, com.parameters[18]);
			}
			params.magnify = ValueOrVariable(com.parameters[20], params.magnify);
			params.top_trans = ValueOrVariable(com.parameters[21], params.top_trans);
			params.spritesheet_cols = com.parameters[22];
			params.spritesheet_rows = com.parameters[23];

			// Animate and index selection are exclusive
			if (com.parameters[24] == 2) {
				params.spritesheet_speed = com.parameters[25];
			} else {
				params.spritesheet_frame = ValueOrVariable(com.parameters[24], com.parameters[25]);
			}

			params.spritesheet_loop = !com.parameters[26];
			params.map_layer = com.parameters[27];
			params.battle_layer = com.parameters[28];
			params.flags = com.parameters[29];
		}

		// RPG2k and RPG2k3 1.10 do not support this option
		params.bottom_trans = params.top_trans;
	} else {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		params.bottom_trans = param_size > 14 ? com.parameters[14] : params.top_trans;
	}

	PicPointerPatch::AdjustShowParams(pic_id, params);

	// Sanitize input
	params.magnify = std::max(0, std::min(params.magnify, 2000));
	params.top_trans = std::max(0, std::min(params.top_trans, 100));
	params.bottom_trans = std::max(0, std::min(params.bottom_trans, 100));

	Game_Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	picture->Show(params);

	return true;
}

bool Game_Interpreter::CommandMovePicture(RPG::EventCommand const& com) { // code 11120
	if (Game_Temp::battle_running) {
		Output::Warning("MovePicture: Not supported in battle");
		return true;
	}

	int pic_id = com.parameters[0];

	Game_Picture::MoveParams params;
	params.position_x = ValueOrVariable(com.parameters[1], com.parameters[2]);
	params.position_y = ValueOrVariable(com.parameters[1], com.parameters[3]);
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

	Game_Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	picture->Move(params);

	if (wait)
		SetupWait(params.duration);

	return true;
}

bool Game_Interpreter::CommandErasePicture(RPG::EventCommand const& com) { // code 11130
	if (Game_Temp::battle_running) {
		Output::Warning("ErasePicture: Not supported in battle");
		return true;
	}

	int pic_id = com.parameters[0];

	if (com.parameters.size() > 1) {
		// Handling of RPG2k3 1.12 chunks
		int id_type = com.parameters[1];

		int max;
		if (id_type < 2) {
			pic_id = ValueOrVariable(id_type, pic_id);
			max = pic_id;
		} else {
			max = com.parameters[2];
		}

		for (int i = pic_id; i <= max; ++i) {
			Game_Picture *picture = Main_Data::game_screen->GetPicture(i);
			picture->Erase(true);
		}
	} else {
		PicPointerPatch::AdjustId(pic_id);

		Game_Picture *picture = Main_Data::game_screen->GetPicture(pic_id);
		picture->Erase(true);
	}

	return true;
}

bool Game_Interpreter::CommandSpriteTransparency(RPG::EventCommand const& com) { // code 11310
	bool visible = com.parameters[0] != 0;
	Game_Character* player = Main_Data::game_player.get();
	player->SetVisible(visible);

	return true;
}

bool Game_Interpreter::CommandMoveEvent(RPG::EventCommand const& com) { // code 11330
	int event_id = com.parameters[0];
	Game_Character* event = GetCharacter(event_id);
	if (event != NULL) {
		// If the event is a vehicle in use, push the commands to the player instead
		if (event_id >= Game_Character::CharBoat && event_id <= Game_Character::CharAirship)
			if (static_cast<Game_Vehicle*>(event)->IsInUse())
				event = Main_Data::game_player.get();

		RPG::MoveRoute route;
		int move_freq = com.parameters[1];

		if (move_freq <= 0 || move_freq > 8) {
			// Invalid values
			move_freq = 6;
		}

		route.repeat = com.parameters[2] != 0;
		route.skippable = com.parameters[3] != 0;

		std::vector<int32_t>::const_iterator it;
		for (it = com.parameters.begin() + 4; it < com.parameters.end(); )
			route.move_commands.push_back(DecodeMove(it));

		event->ForceMoveRoute(route, move_freq);
	}
	return true;
}

bool Game_Interpreter::CommandMemorizeBGM(RPG::EventCommand const& /* com */) { // code 11530
	Game_System::MemorizeBGM();
	return true;
}

bool Game_Interpreter::CommandPlayMemorizedBGM(RPG::EventCommand const& /* com */) { // code 11540
	Game_System::PlayMemorizedBGM();
	return true;
}

bool Game_Interpreter::CommandKeyInputProc(RPG::EventCommand const& com) { // code 11610
	int var_id = com.parameters[0];
	bool wait = com.parameters[1] != 0;

	if (wait) {
		// While waiting the variable is reset to 0 every frame
		Game_Variables[var_id] = 0;
		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	}

	if (wait && Game_Message::visible)
		return false;

	// Wait the first frame so that it ignores keys that were pressed before this command started.
	if (wait && button_timer == 0) {
		button_timer++;
		return false;
	}

	bool time = false;
	int time_id = 0;

	bool check_decision = com.parameters[3] != 0;
	bool check_cancel = com.parameters[4] != 0;
	bool check_numbers = false;
	bool check_arith = false;
	bool check_shift = false;
	bool check_down = false;
	bool check_left = false;
	bool check_right = false;
	bool check_up = false;
	int result = 0;
	size_t param_size = com.parameters.size();

	// Use a function pointer to check triggered keys if it waits for input and pressed keys otherwise
	bool(*check)(Input::InputButton) = wait ? Input::IsTriggered : Input::IsPressed;

	if (param_size < 6) {
		// For Rpg2k <1.50
		bool check_dir = com.parameters[2] != 0;
		check_down = check_dir;
		check_left = check_dir;
		check_right = check_dir;
		check_up = check_dir;
	} else if (param_size < 11) {
		// For Rpg2k >=1.50
		check_shift = com.parameters[5] != 0;
		check_down = param_size > 6 ? com.parameters[6] != 0 : false;
		check_left = param_size > 7 ? com.parameters[7] != 0 : false;
		check_right = param_size > 8 ? com.parameters[8] != 0 : false;
		check_up = param_size > 9 ? com.parameters[9] != 0 : false;
	} else {
		// For Rpg2k3
		check_numbers = com.parameters[5] != 0;
		check_arith = com.parameters[6] != 0;
		time_id = com.parameters[7];
		time = com.parameters[8] != 0;
		check_shift = com.parameters[9] != 0;
		check_down = com.parameters[10] != 0;
		check_left = param_size > 11 ? com.parameters[11] != 0 : false;
		check_right = param_size > 12 ? com.parameters[12] != 0 : false;
		check_up = param_size > 13 ? com.parameters[13] != 0 : false;
	}

	if (check_down && check(Input::DOWN)) {
		result = 1;
	}
	if (check_left && check(Input::LEFT)) {
		result = 2;
	}
	if (check_right && check(Input::RIGHT)) {
		result = 3;
	}
	if (check_up && check(Input::UP)) {
		result = 4;
	}
	if (check_decision && check(Input::DECISION)) {
		result = 5;
	}
	if (check_cancel && check(Input::CANCEL)) {
		result = 6;
	}
	if (check_shift && check(Input::SHIFT)) {
		result = 7;
	}
	if (check_numbers) {
		for (int i = 0; i < 10; ++i) {
			if (check((Input::InputButton)(Input::N0 + i))) {
				result = 10 + i;
			}
		}
	}
	if (check_arith) {
		for (int i = 0; i < 5; ++i) {
			if (check((Input::InputButton)(Input::PLUS + i))) {
				result = 20 + i;
			}
		}
	}

	if (var_id > 0) {
		Game_Variables[var_id] = result;
		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	}

	if (!wait)
		return true;

	button_timer++;

	if (result == 0)
		return false;

	if (time) {
		// 10 per second
		Game_Variables[time_id] = (int)((float)button_timer / Graphics::GetDefaultFps() * 10);
	}

	button_timer = 0;

	return true;
}

bool Game_Interpreter::CommandChangeMapTileset(RPG::EventCommand const& com) { // code 11710
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

bool Game_Interpreter::CommandChangePBG(RPG::EventCommand const& com) { // code 11720
	Game_Map::Parallax::Params params;
	params.name = com.string;
	params.scroll_horz = com.parameters[0] != 0;
	params.scroll_vert = com.parameters[1] != 0;
	params.scroll_horz_auto = com.parameters[2] != 0;
	params.scroll_horz_speed = com.parameters[3];
	params.scroll_vert_auto = com.parameters[4] != 0;
	params.scroll_vert_speed = com.parameters[5];

	Game_Map::Parallax::ChangeBG(params);

	return true;
}

bool Game_Interpreter::CommandChangeEncounterRate(RPG::EventCommand const& com) { // code 11740
	int steps = com.parameters[0];

	Game_Map::SetEncounterRate(steps);

	return true;
}

bool Game_Interpreter::CommandTileSubstitution(RPG::EventCommand const& com) { // code 11750
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

bool Game_Interpreter::CommandTeleportTargets(RPG::EventCommand const& com) { // code 11810
	int map_id = com.parameters[1];

	if (com.parameters[0] != 0) {
		Game_Targets::RemoveTeleportTarget(map_id);
		return true;
	}

	int x = com.parameters[2];
	int y = com.parameters[3];
	int switch_id = (com.parameters[4] != 0)
		? com.parameters[5]
		: -1;
	Game_Targets::AddTeleportTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter::CommandChangeTeleportAccess(RPG::EventCommand const& com) { // code 11820
	Game_System::SetAllowTeleport(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandEscapeTarget(RPG::EventCommand const& com) { // code 11830
	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];
	int switch_id = (com.parameters[3] != 0)
		? com.parameters[4]
		: -1;
	Game_Targets::SetEscapeTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter::CommandChangeEscapeAccess(RPG::EventCommand const& com) { // code 11840
	Game_System::SetAllowEscape(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeSaveAccess(RPG::EventCommand const& com) { // code 11930
	Game_System::SetAllowSave(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandChangeMainMenuAccess(RPG::EventCommand const& com) { // code 11960
	Game_System::SetAllowMenu(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter::CommandConditionalBranch(RPG::EventCommand const& com) { // Code 12010
	bool result = false;
	int value1, value2;
	int actor_id;
	Game_Actor* actor;
	Game_Character* character;

	switch (com.parameters[0]) {
	case 0:
		// Switch
		result = Game_Switches[com.parameters[1]] == (com.parameters[2] == 0);
		break;
	case 1:
		// Variable
		value1 = Game_Variables[com.parameters[1]];
		if (com.parameters[2] == 0) {
			value2 = com.parameters[3];
		} else {
			value2 = Game_Variables[com.parameters[3]];
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
		value1 = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer1);
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
				+ Main_Data::game_party->GetItemCount(com.parameters[1], true) > 0;
		} else {
			// Not having
			result = Main_Data::game_party->GetItemCount(com.parameters[1])
				+ Main_Data::game_party->GetItemCount(com.parameters[1], true) == 0;
		}
		break;
	case 5:
		// Hero
		actor_id = com.parameters[1];
		actor = Game_Actors::GetActor(actor_id);

		if (!actor) {
			Output::Warning("ConditionalBranch: Invalid actor ID %d", actor_id);
			// Use Else branch
			return SkipTo(Cmd::ElseBranch, Cmd::EndBranch);
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
			result = character->GetSpriteDirection() == com.parameters[2];
		}
		break;
	case 7: {
		// Vehicle in use
		Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[1] + 1);
		Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);

		if (!vehicle) {
			Output::Warning("ConditionalBranch: Invalid vehicle ID %d", vehicle_id);
			return true;
		}

		result = vehicle->IsInUse();
		break;
	}
	case 8:
		// Key decision initiated this event
		result = triggered_by_decision_key;
		break;
	case 9:
		// BGM looped at least once
		result = Audio().BGM_PlayedOnce();
		break;
	case 10:
		value1 = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer2);
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
			result = Main_Data::game_data.system.atb_mode == RPG::SaveSystem::AtbMode_atb_wait;
			break;
		case 3:
			// Is Fullscreen active?
			result = DisplayUi->IsFullscreen();
			break;
		}
		break;
	default:
		Output::Warning("ConditionalBranch: Branch %d unsupported", com.parameters[0]);
	}

	if (result)
		return true;

	return SkipTo(Cmd::ElseBranch, Cmd::EndBranch);
}

bool Game_Interpreter::CommandJumpToLabel(RPG::EventCommand const& com) { // code 12120
	int label_id = com.parameters[0];

	for (int idx = 0; (size_t)idx < list.size(); idx++) {
		if (list[idx].code != Cmd::Label)
			continue;
		if (list[idx].parameters[0] != label_id)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandBreakLoop(RPG::EventCommand const& com) { // code 12220
	return SkipTo(Cmd::EndLoop, Cmd::EndLoop, 0, com.indent - 1, true);
}

bool Game_Interpreter::CommandEndLoop(RPG::EventCommand const& com) { // code 22210
	int indent = com.indent;

	for (int idx = index; idx >= 0; idx--) {
		if (list[idx].indent > indent)
			continue;
		if (list[idx].indent < indent)
			return false;
		if (list[idx].code != Cmd::Loop)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandEraseEvent(RPG::EventCommand const& /* com */) { // code 12320
	if (event_id == 0)
		return true;

	if (!event_info.page && !Player::IsRPG2k3E()) {
		// When a common event and not RPG2k3E engine ignore the call, otherwise
		// this breaks games because older version did a no-op in this case
		Output::Debug("Common Event %d: Erasing of the calling map event only supported in RPG2k3E", event_id, event_info.x, event_info.y);
		return true;
	}

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

bool Game_Interpreter::CommandCallEvent(RPG::EventCommand const& com) { // code 12330
	int evt_id;
	int event_page;

	if (child_interpreter)
		return false;

	clear_child = false;

	child_interpreter.reset(new Game_Interpreter_Map(depth + 1, main_flag));

	switch (com.parameters[0]) {
	case 0: { // Common Event
		evt_id = com.parameters[1];
		Game_CommonEvent* common_event = ReaderUtil::GetElement(Game_Map::GetCommonEvents(), evt_id);
		if (!common_event) {
			Output::Warning("CallEvent: Can't call invalid common event %d", evt_id);
			return true;
		}

		// Forwarding the event_id is safe because all RPG Maker engines prior 2k3 1.12
		// threw an error when ThisEvent was used in CommonEvents.
		// The exception is EraseEvent which is handled special (see the code)
		child_interpreter->Setup(common_event, event_id);
		return true;
	}
	case 1: // Map Event
		evt_id = com.parameters[1];
		event_page = com.parameters[2];
		break;
	case 2: // Indirect
		evt_id = Game_Variables[com.parameters[1]];
		event_page = Game_Variables[com.parameters[2]];
		break;
	default:
		return false;
	}

	Game_Event* event = static_cast<Game_Event*>(GetCharacter(evt_id));
	if (event) {
		const RPG::EventPage* page = event->GetPage(event_page);
		if (page) {
			child_interpreter->Setup(page->event_commands, event->GetId(), false);
			child_interpreter->event_info.x = event->GetX();
			child_interpreter->event_info.y = event->GetY();
			child_interpreter->event_info.page = page;
		} else {
			Output::Warning("CallEvent: Can't call non-existant page %d of event %d", event_page, evt_id);
		}
	}

	return true;
}

bool Game_Interpreter::CommandReturnToTitleScreen(RPG::EventCommand const& /* com */) { // code 12510
	Game_Temp::to_title = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandChangeClass(RPG::EventCommand const& com) { // code 1008
	int class_id = com.parameters[2]; // 0: No class, 1+: Specific class
	bool level1 = com.parameters[3] > 0;
	int skill_mode = com.parameters[4]; // no change, replace, add
	int stats_mode = com.parameters[5]; // no change, halve, level 1, current level
	bool show = com.parameters[6] > 0;

	const RPG::Class* cls = ReaderUtil::GetElement(Data::classes, class_id);
	if (!cls) {
		Output::Warning("ChangeClass: Can't change class. Class %d is invalid", class_id);
		return true;
	}

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		int actor_id = actor->GetId();

		int cur_lvl = actor->GetLevel();
		int cur_exp = actor->GetExp();
		int cur_cid = actor->GetClass() ? actor->GetClass()->ID : -1;

		switch (stats_mode) {
		case 2:
			actor->SetClass(class_id);
			actor->SetLevel(1);
			actor->SetExp(0);
			break;
		case 3:
			actor->SetClass(class_id);
			break;
		}

		int cur_hp = actor->GetBaseMaxHp();
		int cur_sp = actor->GetBaseMaxSp();
		int cur_atk = actor->GetBaseAtk();
		int cur_def = actor->GetBaseDef();
		int cur_spi = actor->GetBaseSpi();
		int cur_agi = actor->GetBaseAgi();

		switch (stats_mode) {
		case 1:
			cur_hp /= 2;
			cur_sp /= 2;
			cur_atk /= 2;
			cur_def /= 2;
			cur_spi /= 2;
			cur_agi /= 2;
			break;
		}

		actor->SetClass(class_id);
		if (level1) {
			actor->SetLevel(1);
			actor->SetExp(0);
		} else {
			actor->SetExp(cur_exp);
			actor->SetLevel(cur_lvl);
		}

		actor->SetBaseMaxHp(cur_hp);
		actor->SetBaseMaxSp(cur_sp);
		actor->SetBaseAtk(cur_atk);
		actor->SetBaseDef(cur_def);
		actor->SetBaseSpi(cur_spi);
		actor->SetBaseAgi(cur_agi);

		int level = actor->GetLevel();

		// same class, not doing skill processing
		if (class_id == cur_cid)
			return true;

		bool level_up = false;

		if (show && !level1) {
			std::stringstream ss;
			ss << actor->GetName();
			if (Player::IsRPG2k3E()) {
				ss << " " << Data::terms.level_up << " ";
				ss << " " << Data::terms.level << " " << level;
			} else {
				std::string particle, space = "";
				if (Player::IsCP932()) {
					particle = "は";
					space += " ";
				}
				else {
					particle = " ";
				}
				ss << particle << Data::terms.level << " ";
				ss << level << space << Data::terms.level_up;
			}
			Game_Message::texts.push_back(ss.str());
			level_up = true;
		}

		if (skill_mode == 1) {
			// Learn based on level (replace)
			actor->UnlearnAllSkills();
		}
		if (skill_mode > 0) {
			// Learn additionally
			for (const RPG::Learning& learn : cls->skills) {
				if (level >= learn.level) {
					actor->LearnSkill(learn.skill_id);
					if (show) {
						std::stringstream ss;
						ss << Data::skills[learn.skill_id - 1].name;
						ss << (Player::IsRPG2k3E() ? " " : "") << Data::terms.skill_learned;
						Game_Message::texts.push_back(ss.str());
						level_up = true;
					}
				}
			}
		}
		else {
			for (const RPG::Learning& learn : Data::actors[actor_id - 1].skills) {
				if (level >= learn.level) {
					actor->LearnSkill(learn.skill_id);
					if (show) {
						std::stringstream ss;
						ss << Data::skills[learn.skill_id - 1].name;
						ss << (Player::IsRPG2k3E() ? " " : "") << Data::terms.skill_learned;
						Game_Message::texts.push_back(ss.str());
						level_up = true;
					}
				}
			}
		}

		if (level_up) {
			Game_Message::texts.back().append("\f");
			Game_Message::message_waiting = true;
		}
	}

	return true;
}

bool Game_Interpreter::CommandChangeBattleCommands(RPG::EventCommand const& com) { // code 1009
	int cmd_id = com.parameters[2];
	bool add = com.parameters[3] != 0;

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeBattleCommands(add, cmd_id);
	}

	return true;
}

bool Game_Interpreter::CommandExitGame(RPG::EventCommand const& /* com */) {
	if (Scene::Find(Scene::GameBrowser)) {
		Scene::PopUntil(Scene::GameBrowser);
	} else {
		Player::exit_flag = true;
	}
	return true;
}

bool Game_Interpreter::CommandToggleFullscreen(RPG::EventCommand const& /* com */) {
	DisplayUi->BeginDisplayModeChange();
	DisplayUi->ToggleFullscreen();
	DisplayUi->EndDisplayModeChange();
	return true;
}

bool Game_Interpreter::DefaultContinuation(RPG::EventCommand const& /* com */) {
	continuation = NULL;
	index++;
	return true;
}

// Dummy Continuations

bool Game_Interpreter::ContinuationOpenShop(RPG::EventCommand const& /* com */) { return true; }
bool Game_Interpreter::ContinuationShowInnStart(RPG::EventCommand const& /* com */) { return true; }
bool Game_Interpreter::ContinuationShowInnFinish(RPG::EventCommand const& /* com */) { return true; }
bool Game_Interpreter::ContinuationEnemyEncounter(RPG::EventCommand const& /* com */) { return true; }
