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
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "game_interpreter.h"
#include "audio.h"
#include "game_map.h"
#include "game_event.h"
#include "game_player.h"
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

// Forward declarations.

Game_Interpreter::Game_Interpreter(int _depth, bool _main_flag) {
	depth = _depth;
	main_flag = _main_flag;
	active = false;
	index = 0;

	if (depth > 100) {
		Output::Warning("Too many event calls (over 9000)");
	}

	Clear();
}

Game_Interpreter::~Game_Interpreter() {
}

// Clear.
void Game_Interpreter::Clear() {
	map_id = 0;						// map ID when starting up
	event_id = 0;					// event ID
	//Game_Message::message_waiting = false;	// waiting for message to end
	move_route_waiting = false;		// waiting for move completion
	button_input_variable_id = 0;	// button input variable ID
	wait_count = 0;					// wait count
	child_interpreter.reset();		// child interpreter for common events, etc
	continuation = NULL;			// function to execute to resume command
	button_timer = 0;
}

// Is interpreter running.
bool Game_Interpreter::IsRunning() const {
	return !list.empty();
}

// Setup.
void Game_Interpreter::Setup(const std::vector<RPG::EventCommand>& _list, int _event_id, int dbg_x, int dbg_y) {

	Clear();

	map_id = Game_Map::GetMapId();
	event_id = _event_id;
	list = _list;

	debug_x = dbg_x;
	debug_y = dbg_y;

	index = 0;

	CancelMenuCall();
}

void Game_Interpreter::CancelMenuCall() {
	// TODO
}

void Game_Interpreter::SetupWait(int duration) {
	CloseMessageWindow();
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

void Game_Interpreter::EndMoveRoute(Game_Character*) {
	// This will only ever be called on Game_Interpreter_Map instances
}

// Update
void Game_Interpreter::Update() {
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

			if (!child_interpreter->IsRunning()) {
				child_interpreter.reset();
			}

			// If child interpreter still exists
			if (child_interpreter) {
				return;
			}
		}

		if (Game_Message::message_waiting) {
			return;
		}

		// If waiting for a move to end
		if (move_route_waiting) {
			if (Main_Data::game_player->IsMoveRouteOverwritten()) {
				return;
			}

			Game_Event* g_event;
			for (size_t i = 0; i < Game_Map::GetEvents().size(); i++) {
				g_event = Game_Map::GetEvents().find(i)->second.get();

				if (g_event->IsMoveRouteOverwritten()) {
					return;
				}
			}
			move_route_waiting = false;
		}

		if (button_input_variable_id > 0) {
			InputButton();
			return;
		}

		if (wait_count > 0) {
			wait_count--;
			return;
		}

		if (Game_Temp::forcing_battler != NULL) {
			return;
		}

		if (Game_Temp::battle_calling ||
			Game_Temp::shop_calling ||
//			Game_Temp::inn_calling ||
			Game_Temp::name_calling ||
			Game_Temp::menu_calling ||
			Game_Temp::save_calling ||
			Game_Temp::to_title ||
			Game_Temp::gameover) {

			return;
		}

		if (continuation) {
			bool result = (this->*continuation)(list[index]);
			continuation = NULL;
			if (result)
				continue;
			else
				return;
		}

		if (!Main_Data::game_player->IsTeleporting()) {
			if (Game_Map::GetNeedRefresh()) {
				Game_Map::Refresh();
			}
		}

		if (list.empty()) {
			return;
		}

		if (!ExecuteCommand()) {
			CloseMessageWindow();
			active = true;
			return;
		}

		active = false;

		// FIXME?
		// After calling SkipTo this index++ will skip execution of e.g. END.
		// This causes a different timing because loop_count reaches 10000
		// faster then Player does.
		// No idea if any game depends on this special case.
		index++;
	} // for

	// Executed Events Count exceeded (10000)
	active = true;
	Output::Debug("Event %d exceeded execution limit", event_id);
	CloseMessageWindow();
}

// Setup Starting Event
void Game_Interpreter::SetupStartingEvent(Game_Event* ev) {
	ev->ClearStarting();
	Setup(ev->GetList(), ev->GetId(), ev->GetX(), ev->GetY());
}

void Game_Interpreter::SetupStartingEvent(Game_CommonEvent* ev) {
	Setup(ev->GetList(), 0, ev->GetIndex(), -2);
}

// Skip to command.
bool Game_Interpreter::SkipTo(int code, int code2, int min_indent, int max_indent) {
	if (code2 < 0)
		code2 = code;
	if (min_indent < 0)
		min_indent = list[index].indent;
	if (max_indent < 0)
		max_indent = list[index].indent;

	for (int idx = index; (size_t) idx < list.size(); idx++) {
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

	return true;
}

// Execute Command.
bool Game_Interpreter::ExecuteCommand() {
	RPG::EventCommand const& com = list[index];

	switch (com.code) {
		case Cmd::ShowMessage:
			return CommandShowMessage(com);
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
		case Cmd::ChangeGold:
			return CommandChangeGold(com);
		case Cmd::ChangeItems:
			return CommandChangeItems(com);
		case Cmd::ChangePartyMembers:
			return CommandChangePartyMember(com);
		case Cmd::ChangeLevel:
			return CommandChangeLevel(com);
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
		case Cmd::TintScreen:
			return CommandTintScreen(com);
		case Cmd::FlashScreen:
			return CommandFlashScreen(com);
		case Cmd::ShakeScreen:
			return CommandShakeScreen(com);
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
		default:
			return true;
	}
}

bool Game_Interpreter::CommandWait(RPG::EventCommand const& com) {
	if (com.parameters.size() <= 1 ||
		(com.parameters.size() > 1 && com.parameters[1] == 0)) {
		SetupWait(com.parameters[0]);
		return true;
	} else {
		return Input::IsAnyTriggered();
	}
}

void Game_Interpreter::InputButton() {
	Input::InputButton n = Input::BUTTON_COUNT;

	if (Input::IsTriggered(Input::UP)) {
		n = Input::UP;
	} else {
		if (Input::IsTriggered(Input::DOWN)) {
			n = Input::DOWN;
		} else {
			if (Input::IsTriggered(Input::LEFT)) {
				n = Input::LEFT;
			} else {
				if (Input::IsTriggered(Input::RIGHT)) {
					n = Input::RIGHT;
				} else {
					if (Input::IsTriggered(Input::DECISION)) {
						n = Input::DECISION;
					} else {
						if (Input::IsTriggered(Input::CANCEL)) {
							n = Input::CANCEL;
						} else {
							if (Input::IsTriggered(Input::SHIFT)) {
								n = Input::SHIFT;
							}
						}
					}
				}
			}
		}
	} // end first if

	// If a button was pressed
	if (n != Input::BUTTON_COUNT) {
		// Set variable
		Game_Variables[button_input_variable_id] = n;
		Game_Map::SetNeedRefresh(true);
		button_input_variable_id = 0;
		Input::ResetKeys();
	}
}

bool Game_Interpreter::CommandEnd() {
	CloseMessageWindow();
	list.clear();

	if ((main_flag) && (event_id > 0)) {
		Game_Map::GetEvents().find(event_id)->second->Unlock();
	}

	return true;
}

// Helper function
void Game_Interpreter::GetStrings(std::vector<std::string>& ret_val) {
	// Let's find the choices
	int current_indent = list[index + 1].indent;
	unsigned int index_temp = index + 1;
	std::vector<std::string> s_choices;
	while ( index_temp < list.size() ) {
		if ( (list[index_temp].code == Cmd::ShowChoiceOption) && (list[index_temp].indent == current_indent) ) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}
		// If found end of show choice command
		if ( ( (list[index_temp].code == Cmd::ShowChoiceEnd) && (list[index_temp].indent == current_indent) ) ||
			// Or found Cancel branch
			( (list[index_temp].code == Cmd::ShowChoiceOption) && (list[index_temp].indent == current_indent) &&
			(list[index_temp].string == "") ) ) {

			break;
		}
		// Move on to the next command
		index_temp++;
	}
	ret_val.swap(s_choices);
}

void Game_Interpreter::CloseMessageWindow() {
	if (Game_Message::visible) {
		Game_Message::visible = false;
		Game_Message::SemiClear();
	}
}

// Command Show Message
bool Game_Interpreter::CommandShowMessage(RPG::EventCommand const& com) { // Code ShowMessage
	// If there's a text already, return immediately
	if (!Game_Message::texts.empty()) {
		return false;
	}
	unsigned int line_count = 0;

	Game_Message::message_waiting = true;

	// Set first line
	Game_Message::texts.push_back(com.string);
	line_count++;

	for (;;) {
		// If next event command is the following parts of the message
		if ( index < list.size() - 1 && list[index+1].code == Cmd::ShowMessage_2 ) {
			// Add second (another) line
			line_count++;
			Game_Message::texts.push_back(list[index+1].string);
		} else {
			// If next event command is show choices
			std::vector<std::string> s_choices;
			if ( (index < list.size() - 1) && (list[index+1].code == Cmd::ShowChoice) ) {
				GetStrings(s_choices);
				// If choices fit on screen
				if (s_choices.size() <= (4 - line_count)) {
					index++;
					Game_Message::choice_start = line_count;
					Game_Message::choice_cancel_type = list[index].parameters[0];
					SetupChoices(s_choices);
				}
			} else if ((index < list.size() - 1) && (list[index+1].code == Cmd::InputNumber) ) {
				// If next event command is input number
				// If input number fits on screen
				if (line_count < 4) {
					index++;
					Game_Message::num_input_start = line_count;
					Game_Message::num_input_digits_max = list[index].parameters[0];
					Game_Message::num_input_variable_id = list[index].parameters[1];
				}
			}

			return true;
		}
		index++;
	} // End for
}

// Setup Choices
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

// Command Show choices
bool Game_Interpreter::CommandShowChoices(RPG::EventCommand const& com) { // Code ShowChoice
	if (!Game_Message::texts.empty()) {
		return false;
	}

	Game_Message::message_waiting = true;

	// Choices setup
	std::vector<std::string> choices;
	Game_Message::choice_cancel_type = com.parameters[0];
	GetStrings(choices);
	SetupChoices(choices);

	return true;
}

// Command control switches
bool Game_Interpreter::CommandControlSwitches(RPG::EventCommand const& com) { // Code ControlSwitches
	int i;
	switch (com.parameters[0]) {
		case 0:
		case 1:
			// Single and switch range
			for (i = com.parameters[1]; i <= com.parameters[2]; i++) {
				if (com.parameters[3] != 2) {
					Game_Switches[i] = com.parameters[3] == 0;
				} else {
					Game_Switches[i] = !Game_Switches[i];
				}
			}
			break;
		case 2:
			// Switch from variable
			if (com.parameters[3] != 2) {
				Game_Switches[Game_Variables[com.parameters[1]]] = com.parameters[3] == 0;
			} else {
				Game_Switches[Game_Variables[com.parameters[1]]] = !Game_Switches[Game_Variables[com.parameters[1]]];
			}
			break;
		default:
			return false;
	}
	Game_Map::SetNeedRefresh(true);
	return true;
}

// Command control vars
bool Game_Interpreter::CommandControlVariables(RPG::EventCommand const& com) { // Code ControlVars
	int i, value = 0;
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
			value = rand() % (a-b+1)+b;
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
			if (actor != NULL) {
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
			}
			break;
		case 6:
			// Characters
			if (com.parameters[6] != 0){
				character = GetCharacter(com.parameters[5]);
			} else {
				// Special case for Player Map ID
				character = NULL;
				value = Game_Map::GetMapId();
			}
			// Other cases
			if (character != NULL) {
				switch (com.parameters[6]) {
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
						value = character->GetDirection();
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
					value = Main_Data::game_party->ReadTimer(Main_Data::game_party->Timer1);
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
					// TODO: MIDI play position
					break;
				case 9:
					value = Main_Data::game_party->ReadTimer(Main_Data::game_party->Timer2);
					break;
			}
			break;
		default:
			;
	}

	switch (com.parameters[0]) {
		case 0:
		case 1:
			// Single and Var range
			for (i = com.parameters[1]; i <= com.parameters[2]; i++) {
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
				if (Game_Variables[i] > MaxSize) {
					Game_Variables[i] = MaxSize;
				}
				if (Game_Variables[i] < MinSize) {
					Game_Variables[i] = MinSize;
				}
			}
			break;

		case 2:
			switch (com.parameters[3]) {
				case 0:
					// Assignement
					Game_Variables[com.parameters[1]] = value;
					break;
				case 1:
					// Addition
					Game_Variables[com.parameters[1]] += value;
					break;
				case 2:
					// Subtraction
					Game_Variables[com.parameters[1]] -= value;
					break;
				case 3:
					// Multiplication
					Game_Variables[com.parameters[1]] *= value;
					break;
				case 4:
					// Division
					if (value != 0) {
						Game_Variables[com.parameters[1]] /= value;
					}
					break;
				case 5:
					// Module
					if (value != 0) {
						Game_Variables[com.parameters[1]] %= value;
					}
			}
			if (Game_Variables[com.parameters[1]] > MaxSize) {
				Game_Variables[com.parameters[1]] = MaxSize;
			}
			if (Game_Variables[com.parameters[1]] < MinSize) {
				Game_Variables[com.parameters[1]] = MinSize;
			}
	}

	Game_Map::SetNeedRefresh(true);
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

	switch (mode) {
	case 0:
		// Party
		actors = Main_Data::game_party->GetActors();
		break;
	case 1:
		// Hero
		actors.push_back(Game_Actors::GetActor(id));
		break;
	case 2:
		// Var hero
		actors.push_back(Game_Actors::GetActor(Game_Variables[id]));
		break;
	}

	return actors;
}

// Get Character.
Game_Character* Game_Interpreter::GetCharacter(int character_id) {
	Game_Character* ch = Game_Character::GetCharacter(character_id, event_id);
	if (ch == NULL) {
		Output::Warning("Unknown event with id %d", event_id);
	}
	return Game_Character::GetCharacter(character_id, event_id);
}

// Change Gold.
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

// Change Items.
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
	Game_Map::SetNeedRefresh(true);
	// Continue
	return true;
}

// Input Number.
bool Game_Interpreter::CommandInputNumber(RPG::EventCommand const& com) {
	if (!Game_Message::texts.empty()) {
		return false;
	}

	Game_Message::message_waiting = true;

	Game_Message::texts.clear();
	Game_Message::num_input_start = 0;
	Game_Message::num_input_variable_id = com.parameters[1];
	Game_Message::num_input_digits_max = com.parameters[0];

	// Continue
	return true;
}

// Change Face Graphic.
bool Game_Interpreter::CommandChangeFaceGraphic(RPG::EventCommand const& com) { // Code 10130
	Game_Message::SetFaceName(com.string);
	Game_Message::SetFaceIndex(com.parameters[0]);
	Game_Message::SetFaceRightPosition(com.parameters[1] != 0);
	Game_Message::SetFaceFlipped(com.parameters[2] != 0);
	return true;
}

// Change Party Member.
bool Game_Interpreter::CommandChangePartyMember(RPG::EventCommand const& com) { // Code 10330
	Game_Actor* actor;
	int id;

	if (com.parameters[1] == 0) {
		id = com.parameters[2];
	} else {
		id = Game_Variables[com.parameters[2]];
	}

	actor = Game_Actors::GetActor(id);

	if (actor != NULL) {

		if (com.parameters[0] == 0) {
			// Add members
			Main_Data::game_party->AddActor(id);

		} else {
			// Remove members
			Main_Data::game_party->RemoveActor(id);
		}
	}

	Game_Map::SetNeedRefresh(true);

	// Continue
	return true;
}

// Change Experience.
bool Game_Interpreter::CommandChangeLevel(RPG::EventCommand const& com) { // Code 10420
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
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

bool Game_Interpreter::CommandChangeSkills(RPG::EventCommand const& com) { // Code 10440
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	bool remove = com.parameters[2] != 0;
	int skill_id = ValueOrVariable(com.parameters[3],
								   com.parameters[4]);

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		if (remove)
			actor->UnlearnSkill(skill_id);
		else
			actor->LearnSkill(skill_id);
	}

	return true;
}

bool Game_Interpreter::CommandChangeEquipment(RPG::EventCommand const& com) { // Code 10450
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	int item_id;
	int type;
	int slot;

	switch (com.parameters[2]) {
		case 0:
			item_id = ValueOrVariable(com.parameters[3],
									  com.parameters[4]);
			type = Data::items[item_id - 1].type;
			switch (type) {
				case RPG::Item::Type_weapon:
				case RPG::Item::Type_shield:
				case RPG::Item::Type_armor:
				case RPG::Item::Type_helmet:
				case RPG::Item::Type_accessory:
					slot = type - 1;
				default:
					return true;
			}
			break;
		case 1:
			item_id = 0;
			slot = com.parameters[3];
			break;
		default:
			return false;
	}

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		actor->ChangeEquipment(slot, item_id);
	}

	return true;
}

bool Game_Interpreter::CommandChangeHP(RPG::EventCommand const& com) { // Code 10460
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	bool remove = com.parameters[2] != 0;
	int amount = ValueOrVariable(com.parameters[3],
								 com.parameters[4]);
	bool lethal = com.parameters[5] != 0;

	if (remove)
		amount = -amount;

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		int hp = actor->GetHp() + amount;
		if (!lethal && hp <= 0) {
			amount += hp * (-1) + 1;
		}
		actor->ChangeHp(amount);
	}

	return true;
}

bool Game_Interpreter::CommandChangeSP(RPG::EventCommand const& com) { // Code 10470
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	bool remove = com.parameters[2] != 0;
	int amount = ValueOrVariable(com.parameters[3],
								 com.parameters[4]);

	if (remove)
		amount = -amount;

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		int sp = actor->GetSp() + amount;
		if (sp < 0)
			sp = 0;
		actor->SetSp(sp);
	}

	return true;
}

bool Game_Interpreter::CommandChangeCondition(RPG::EventCommand const& com) { // Code 10480
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);
	bool remove = com.parameters[2] != 0;
	int state_id = com.parameters[3];

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		if (remove)
			actor->RemoveState(state_id);
		else
			actor->AddState(state_id);
	}

	return true;
}

bool Game_Interpreter::CommandFullHeal(RPG::EventCommand const& com) { // Code 10490
	std::vector<Game_Actor*> actors = GetActors(com.parameters[0],
												com.parameters[1]);

	for (std::vector<Game_Actor*>::iterator i = actors.begin();
		 i != actors.end();
		 i++) {
		Game_Actor* actor = *i;
		actor->ChangeHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
		actor->RemoveAllStates();
	}

	return true;
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
	Audio().BGM_Fade(fadeout);
	return true;
}

bool Game_Interpreter::CommandPlaySound(RPG::EventCommand const& com) { // code 11550
	RPG::Sound sound;
	sound.name = com.string;
	sound.volume = com.parameters[0];
	sound.tempo = com.parameters[1];
	sound.balance = com.parameters[2];
	Game_System::SePlay(sound);
	return true;
}

bool Game_Interpreter::CommandTintScreen(RPG::EventCommand const& com) { // code 11030
	Game_Screen* screen = Main_Data::game_screen.get();
	int r = com.parameters[0];
	int g = com.parameters[1];
	int b = com.parameters[2];
	int s = com.parameters[3];
	int tenths = com.parameters[4];
	bool wait = com.parameters[5] != 0;

	screen->TintScreen(r, g, b, s, tenths);

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

	if (Player::engine == Player::EngineRpg2k3) {
		switch (com.parameters[6]) {
			case 0:
				screen->FlashOnce(r, g, b, s, tenths);
				if (wait)
					SetupWait(tenths);
				break;
			case 1:
				screen->FlashBegin(r, g, b, s, tenths);
				break;
			case 2:
				screen->FlashEnd();
				break;
		}
	} else {
		screen->FlashOnce(r, g, b, s, tenths);
		if (wait)
			SetupWait(tenths);
	}

	return true;
}

bool Game_Interpreter::CommandShakeScreen(RPG::EventCommand const& com) { // code 11050
	Game_Screen* screen = Main_Data::game_screen.get();
	int strength = com.parameters[0];
	int speed = com.parameters[1];
	int tenths = com.parameters[2];
	bool wait = com.parameters[3] != 0;

	if (Player::engine == Player::EngineRpg2k) {
		screen->ShakeOnce(strength, speed, tenths);
		if (wait) {
			SetupWait(tenths);
		}
	} else {
		switch (com.parameters[4]) {
			case 0:
				screen->ShakeOnce(strength, speed, tenths);
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

bool Game_Interpreter::CommandEndEventProcessing(RPG::EventCommand const& /* com */) { // code 12310
	index = list.size();
	return true;
}

bool Game_Interpreter::DefaultContinuation(RPG::EventCommand const& /* com */) {
	index++;
	return true;
}

bool Game_Interpreter::CommandGameOver(RPG::EventCommand const& /* com */) { // code 12420
	CloseMessageWindow();
	Game_Temp::gameover = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

// Dummy Continuations

bool Game_Interpreter::ContinuationOpenShop(RPG::EventCommand const& /* com */) { return true; }
bool Game_Interpreter::ContinuationShowInn(RPG::EventCommand const& /* com */) { return true; }
bool Game_Interpreter::ContinuationEnemyEncounter(RPG::EventCommand const& /* com */) { return true; }
