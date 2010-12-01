/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#include "game_interpreter.h"
#include "output.h"
#include "input.h"
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
#include "graphics.h"
#include "util_macro.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Enumeration of codes
////////////////////////////////////////////////////////////
enum CommandCodes {
	SHOW_MESSAGE		= 10110,
	SHOW_MESSAGE_2		= 20110,
	MESSAGE_OPTIONS		= 10120,
	SHOW_FACE_GRAPHIC	= 10130,
	CHANGE_FACE_GRAPHIC = 10130,
	SHOW_CHOICE			= 10140,
	SHOW_CHOICE_OPTION	= 20140,
	SHOW_CHOICE_END		= 20141,
	CONTROL_VARS		= 10220,
	CONTROL_SWITCHES	= 10210,
	INPUT_NUMBER		= 10150,
	CHANGE_GOLD         = 10310,
	CHANGE_ITEMS        = 10320,
	CHANGE_EXP          = 10410,
	CHANGE_LEVEL        = 10420,
	CHANGE_PARAMETERS   = 10430,
	CONDITIONAL_BRANCH  = 12010,
	WAIT                = 11410
};

enum Sizes {
	MAXSIZE = 9999999,
	MINSIZE = -9999999
};

enum CharsID {
	PLAYER		= 10001,
	BOAT		= 10002,
	SHIP		= 10003,
	AIRSHIP		= 10004,
	THIS_EVENT	= 10005
};

////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////
Game_Interpreter::Game_Interpreter(int _depth, bool _main_flag)
{
	depth = _depth;
	main_flag = _main_flag;

	if (depth > 100) {
		Output::Warning("Too many event calls (over 9000)");
	}

	Clear();
}

Game_Interpreter::~Game_Interpreter()
{
}

////////////////////////////////////////////////////////////
/// Clear
////////////////////////////////////////////////////////////
void Game_Interpreter::Clear() {
	map_id = 0;						// map ID when starting up
	event_id = 0;					// event ID
	message_waiting = false;		// waiting for message to end
	move_route_waiting = false;		// waiting for move completion
	button_input_variable_id = 0;	// button input variable ID
	wait_count = 0;					// wait count
	child_interpreter = NULL;		// child interpreter for common events, etc
	branch.clear();
}

////////////////////////////////////////////////////////////
/// Is Interpreter Running
////////////////////////////////////////////////////////////
bool Game_Interpreter::IsRunning() {
	return !list.empty();
}

////////////////////////////////////////////////////////////
/// Setup
////////////////////////////////////////////////////////////
void Game_Interpreter::Setup(std::vector<RPG::EventCommand>& _list, int _event_id) {

	Clear();

	map_id = Game_Map::GetMapId();
	event_id = _event_id;
	list = _list;
	index = 0;

	branch.clear();

	CancelMenuCall();
}

void Game_Interpreter::CancelMenuCall() {
	// TODO
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Game_Interpreter::Update() {

	loop_count = 0;

	for (;;) {
		loop_count++;

		if (loop_count > 100) {
			Graphics::Update(); // Freeze prevention
			loop_count = 0;
		}

		/* If map is different than event startup time
		set event_id to 0 */
		if (Game_Map::GetMapId() != map_id) {
			event_id = 0;
		}
		
		/* If there's any active child interpreter, update it */
		if (child_interpreter != NULL) {

			child_interpreter->Update();

			if (!child_interpreter->IsRunning()) {
				delete child_interpreter;
				child_interpreter = NULL;
			}

			// If child interpreter still exists
			if (child_interpreter != NULL) {
				return;
			}
		}

		if (message_waiting) {
			return;
		}

		// If waiting for a move to end
		if (move_route_waiting) {
			if (Main_Data::game_player->GetMoveRouteForcing()) {
				return;
			}
				
			Game_Event* g_event;
			for (size_t i = 0; i < Game_Map::GetEvents().size(); i++) {
				g_event = Game_Map::GetEvents()[i];

				if (g_event->GetMoveRouteForcing()) {
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
			Game_Temp::name_calling ||
			Game_Temp::menu_calling ||
			Game_Temp::save_calling ||
			Game_Temp::gameover) {
			
			return;
		}

		if (list.empty()) {
			if (main_flag) {
				SetupStartingEvent();
			}

			if (list.empty()) {
				return;
			}
		}

		if (!ExecuteCommand()) {
			return;
		}

		index++;
	} // for
}

////////////////////////////////////////////////////////////
/// Setup Starting Event
////////////////////////////////////////////////////////////
void Game_Interpreter::SetupStartingEvent() {

	if (Game_Map::GetNeedRefresh()) {
		Game_Map::Refresh();
	}

	if (Game_Temp::common_event_id > 0) {
		Setup(Data::commonevents[Game_Temp::common_event_id].event_commands, 0);
		Game_Temp::common_event_id = 0;
		return;
	}

	Game_Event* _event;
	for (tEventHash::iterator i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		_event = i->second;
		
		if (_event->GetStarting()) {
			_event->ClearStarting();
			Setup(_event->GetList(), _event->GetId()	);
			return;
		}
	}

	RPG::CommonEvent* common_event;
	for (size_t i = 0; i < Data::commonevents.size(); i++) {
		common_event = &Data::commonevents[i];

		// If trigger is auto run, and condition switch is ON
		if ( (common_event->trigger == 1) &&
			Game_Switches[common_event->switch_id]) {
			Setup(common_event->event_commands, 0);
			return;
		}
	}
}

////////////////////////////////////////////////////////////
/// Execute Command
////////////////////////////////////////////////////////////
bool Game_Interpreter::ExecuteCommand() {
	
	if (index >= list.size()) {
		CommandEnd();
		return true;
	}
	
	switch (list[index].code) {

		case SHOW_MESSAGE: 
			return CommandShowMessage();
		case SHOW_CHOICE: 
			return CommandShowChoices();
		case INPUT_NUMBER: 
			return CommandInputNumber();
		//case MESSAGE_OPTIONS: 
			//return CommandMessageOptions();
		case CHANGE_FACE_GRAPHIC: 
			return CommandChangeFaceGraphic();
		case CHANGE_EXP: 
			return CommandChangeExp();
		case CHANGE_GOLD: 
			return CommandChangeGold();
		case CHANGE_ITEMS: 
			return CommandChangeItems();
		case CHANGE_LEVEL: 
			return CommandChangeLevel();
		case CONDITIONAL_BRANCH: 
			return CommandConditionalBranch();
		case CONTROL_SWITCHES: 
			return CommandControlSwitches();
		case CONTROL_VARS: 
			return CommandControlVariables();
		case CHANGE_PARAMETERS:
			return CommandChangeParameters();
		case WAIT:
			return CommandWait();
		default:
			return true;

	}
}

bool Game_Interpreter::CommandWait() {
	wait_count = list[index].parameters[0] * DEFAULT_FPS / 10;
	return true;
}

////////////////////////////////////////////////////////////
/// Input Button
////////////////////////////////////////////////////////////
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
	}
}

void Game_Interpreter::CommandEnd() {
	list.clear();

	if ((main_flag) && (event_id > 0)) {
		Game_Map::GetEvents()[event_id]->Unlock();
	}

}

/////////////////////////////////////////////
/// Get Strings for choice selection
/// This is just a helper (private) method
/// to avoid repeating code
/////////////////////////////////////////////
std::vector<std::string> Game_Interpreter::GetStrings() {
	// Let's find the choices
	int current_indent = list[index+1].indent;
	unsigned int index_temp = index+2;
	std::vector<std::string> s_choices;
	while ( index_temp < list.size() ) {
		if ( (list[index_temp].code == SHOW_CHOICE_OPTION) && (list[index_temp].indent == current_indent) ) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}
		// If found end of show choice command
		if ( ( (list[index_temp].code == SHOW_CHOICE_END) && (list[index_temp].indent == current_indent) ) ||
			// Or found Cancel branch
			( (list[index_temp].code == SHOW_CHOICE_OPTION) && (list[index_temp].indent == current_indent) &&
			(list[index_temp].string == "") ) ) {
			
			break;
		}
		// Move on to the next command
		index_temp++;
	}
	return s_choices;
}

////////////////////////////////////////////////////////////
/// Command Show Message
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowMessage() { // Code SHOW_MESSAGE
	// If there's a text already, return immediately
	if (!Main_Data::game_message->texts.empty()) {
		return false;
	}
	unsigned int line_count = 0;

	message_waiting = true;

	// Set first line
	Main_Data::game_message->texts[line_count] = list[index].string;
	line_count++;

	for (;;) {
		// If next event command is the following parts of the message
		if ( (index < list.size()) && (list[index+1].code == SHOW_MESSAGE_2) ) {
			// Add second (another) line
			line_count++;
			Main_Data::game_message->texts[line_count] += list[index+1].string;
		} else {
			// If next event command is show choices
			std::vector<std::string> s_choices;
			if ( (index < list.size()) && (list[index+1].code == SHOW_CHOICE) ) {
				s_choices = GetStrings();
				// If choices fit on screen
				if (s_choices.size() < (4 - line_count)) {
					index++;
					Main_Data::game_message->choice_start = line_count;
					Main_Data::game_message->choice_cancel_type = list[index].parameters[0];
					SetupChoices(s_choices);
				}
			} else {
				// If next event command is input number
				if ((index < list.size()) && (list[index+1].code == INPUT_NUMBER) ) {
					// If input number fits on screen
					if (line_count < 4) {
						index++;
						Main_Data::game_message->num_input_start = line_count;
						Main_Data::game_message->num_input_digits_max = list[index].parameters[0];
						Main_Data::game_message->num_input_variable_id = list[index].parameters[1];
					}
				}
			}
			return true;
		}
		index++;
	} // End for
}

////////////////////////////////////////////////////////////
/// Setup Choices
////////////////////////////////////////////////////////////
void Game_Interpreter::SetupChoices(const std::vector<std::string>& choices) {
	Main_Data::game_message->choice_max = choices.size();

	// Set choices to message text
	unsigned int i;
	for (i = 0; i < choices.size(); i++) {
		Main_Data::game_message->texts.push_back(choices[i]);
	}

	/* Set callback stuff */

	// TODO
}

////////////////////////////////////////////////////////////
/// Command Show choices
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowChoices() { // Code SHOW_CHOICE
	if (!Main_Data::game_message->texts.empty()) {
		return false;
	}

	message_waiting = true;

	// Choices setup
	Main_Data::game_message->texts.clear();
	Main_Data::game_message->choice_start = 0;
	Main_Data::game_message->choice_cancel_type = list[index].parameters[0];
	SetupChoices(GetStrings());

	return true;
}

////////////////////////////////////////////////////////////
/// Command control switches
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandControlSwitches() { // Code CONTROL_SWITCHES
	int i;
	switch (list[index].parameters[0]) {
		case 0:
		case 1:
			// Single and switch range
			for (i = list[index].parameters[1]; i <= list[index].parameters[2]; i++) {
				if (list[index].parameters[3] != 2) {
					Game_Switches[i] = list[index].parameters[3] == 0;
				} else {
					Game_Switches[i] = !Game_Switches[i];
				}
			}
			break;
		case 2:
			// Switch from variable
			if (list[index].parameters[3] != 2) {
				Game_Switches[list[index].parameters[2]] = list[index].parameters[3] == 0;
			} else {
				Game_Switches[list[index].parameters[2]] = !Game_Switches[list[index].parameters[2]];
			}
			break;
		default:
			return false;
	}
	Game_Map::SetNeedRefresh(true);
	return true;
}

////////////////////////////////////////////////////////////
/// Command control vars
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandControlVariables() { // Code CONTROL_VARS
	int i, value = 0;
	Game_Actor* actor;
	Game_Character* character;

	switch (list[index].parameters[0]) {
		case 0:
			// Constant
			value = list[index].parameters[5];
			break;
		case 1:
			// Var A ops B
			value = Game_Variables[list[index].parameters[5]];
			break;
		case 2:
			// Number of var A ops B
			value = Game_Variables[Game_Variables[list[index].parameters[5]]];
			break;
		case 3:
			// Random between range
			int a, b;
			a = max(list[index].parameters[5], list[index].parameters[6]);
			b = min(list[index].parameters[5], list[index].parameters[6]);
			value = rand() % (a-b)+b;
			break;
		case 4:
			// Items
			switch (list[index].parameters[7]) {
				case 0:
					// Number of items posessed
					value = Game_Party::ItemNumber(list[index].parameters[6]);
					break;
				case 1:
					// Number of heroes that have the item equipped
					std::vector<Game_Actor*>::iterator j;
					for (j = Game_Party::GetActors().begin(); j != Game_Party::GetActors().end(); j++) {
						if ( ((*j)->GetWeaponId() == list[index].parameters[6]) ||
							((*j)->GetShieldId() == list[index].parameters[6]) ||
							((*j)->GetArmorId() == list[index].parameters[6]) ||
							((*j)->GetHelmetId() == list[index].parameters[6]) ||
							((*j)->GetAccessoryId() == list[index].parameters[6]) ) {
								value++;
						}
					}
					break;
			}
			break;
		case 5:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[6]);
			if (actor != NULL) {
				switch (list[index].parameters[7]) {
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
			if (list[index].parameters[7] != 0){
				character = GetCharacter(list[index].parameters[6]);
			} else {
				// Special case for Player Map ID
				character = NULL;
				value = Game_Map::GetMapId();
			}
			// Other cases
			if (character != NULL) {
				switch (list[index].parameters[7]) {
					case 1:
						// X Coordinate
						value = character->GetX();
						break;
					case 2:
						// Y Coordinate
						value = character->GetY();
						break;
					case 3:
						// TODO Orientation
						// Needs testing
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
			switch (list[index].parameters[6]) {
				case 0:
					// Gold
					value = Game_Party::GetGold();
					break;
				case 1:
					// TODO Seconds remaining in the timer
					break;
				case 2:
					// Number of heroes in party
					value = Game_Party::GetActors().size();
					break;
				case 3:
					// Number of saves
					value = Game_System::save_count;
					break;
				case 4:
					// Number of battles
					value = Game_Party::GetBattleCount();
					break;
				case 5:
					// Number of wins
					value = Game_Party::GetWinCount();
					break;
				case 6:
					// Number of defeats
					value = Game_Party::GetDefeatCount();
					break;
				case 7:
					// Number of flights (aka run away)
					value = Game_Party::GetRunCount();
					break;
				case 8:
					// TODO MIDI performance position (wtf is this?)
					break;
				case 9:
					// TODO Something for RM2003
					break;
			}
			break;
		default:
			;
	}

	switch (list[index].parameters[0]) {
		case 0:
		case 1:
			// Single and Var range
			for (i = list[index].parameters[1]; i <= list[index].parameters[2]; i++) {
				switch (list[index].parameters[3]) {
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
						}
				}
				if (Game_Variables[i] > MAXSIZE) {
					Game_Variables[i] = MAXSIZE;
				}
				if (Game_Variables[i] < MINSIZE) {
					Game_Variables[i] = MINSIZE;
				}
			}
			break;

		case 2:
			switch (list[index].parameters[3]) {
				case 0:
					// Assignement
					Game_Variables[list[index].parameters[1]] = value;
					break;
				case 1:
					// Addition
					Game_Variables[list[index].parameters[1]] += value;
					break;
				case 2:
					// Subtraction
					Game_Variables[list[index].parameters[1]] -= value;
					break;
				case 3:
					// Multiplication
					Game_Variables[list[index].parameters[1]] *= value;
					break;
				case 4:
					// Division
					if (value != 0) {
						Game_Variables[list[index].parameters[1]] /= value;
					}
					break;
				case 5:
					// Module
					if (value != 0) {
						Game_Variables[list[index].parameters[1]] %= value;
					}
			}
			if (Game_Variables[list[index].parameters[1]] > MAXSIZE) {
				Game_Variables[list[index].parameters[1]] = MAXSIZE;
			}
			if (Game_Variables[list[index].parameters[1]] < MINSIZE) {
				Game_Variables[list[index].parameters[1]] = MINSIZE;
			}
	}
	Game_Map::SetNeedRefresh(true);
	return true;
}

////////////////////////////////////////////////////////////
/// * Calculate Operated Value
///     operation    : operation (increase: 0, decrease: 1)
///     operand_type : operand type (0: set, 1: variable)
///     operand      : operand (number or var ID)
////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
/// Get Character
////////////////////////////////////////////////////////////
Game_Character* Game_Interpreter::GetCharacter(int character_id) {

	switch (character_id) {
		case PLAYER:
			// Player/Hero
			return Main_Data::game_player;
		case BOAT:
			// TODO Boat
			break;
		case SHIP:
			// TODO Ship
			break;
		case AIRSHIP:
			// TODO Airship
			break;
		case THIS_EVENT:
			// This event
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents()[event_id];
			break;
		default:
			// Other events
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents()[character_id];
			break;
	}
	return NULL;
}

////////////////////////////////////////////////////////////
/// Change Gold
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeGold() { // Code 10310
	int value;
	value = OperateValue(
		list[index].parameters[0], 
		list[index].parameters[1],
		list[index].parameters[2]
	);

	Game_Party::GainGold(value);

	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Items
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeItems() { // Code 10320
	int value;
	value = OperateValue(
		list[index].parameters[0],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	if (list[index].parameters[1] == 0) {
		// Item by const number
		Game_Party::GainItem(list[index].parameters[2], value);
	} else {
		// Item by variable
		Game_Party::GainItem(
			Game_Variables[list[index].parameters[2]],
			value
		);
	}
	Game_Map::SetNeedRefresh(true);
	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Input Number
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandInputNumber() {
	if (!Main_Data::game_message->texts.empty()) {
		return false;
	}

	message_waiting = true;

	Main_Data::game_message->texts.clear();
	Main_Data::game_message->num_input_start = 0;
	Main_Data::game_message->num_input_variable_id = list[index].parameters[1];
	Main_Data::game_message->num_input_digits_max = list[index].parameters[0];
	
	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Face Graphic
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeFaceGraphic() { // Code 10130
	Main_Data::game_message->face_name = list[index].string;
	Main_Data::game_message->face_index = list[index].parameters[0];
	Main_Data::game_message->face_left_position = (list[index].parameters[1] == 0);
	Main_Data::game_message->face_flipped = (list[index].parameters[2] == 0);
	return true;
}

////////////////////////////////////////////////////////////
/// Change Party Member
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangePartyMember() { // Code 10330
	Game_Actor* actor;
	int id;

	if (list[index].parameters[2] == 0) {
		id = list[index].parameters[2];
	} else {
		id = Game_Variables[list[index].parameters[2]];
	}

	actor = Game_Actors::GetActor(id);

	if (actor != NULL) {

		if (list[index].parameters[0] == 0) {
			// Add members
			Game_Party::AddActor(id);

		} else {
			// Remove members
			Game_Party::RemoveActor(id);
		}
	}

	Game_Map::SetNeedRefresh(true);

	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Experience
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeExp() { // Code 10410
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	Game_Actor* actor;

	switch (list[index].parameters[0]) {
		case 0:
			// All party
			for (std::vector<Game_Actor*>::iterator i = Game_Party::GetActors().begin(); 
				i != Game_Party::GetActors().end(); 
				i++) {
				(*i)->SetExp((*i)->GetExp() + value);
			}
			break;
		case 1:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[1]);
			actor->SetExp(actor->GetExp() + value);
			break;
		case 2:
			// Var hero
			actor = Game_Actors::GetActor(Game_Variables[list[index].parameters[1]]);
			actor->SetExp(actor->GetExp() + value);
			break;
		default:
			;
	}

	if (list[index].parameters[5] != 0) {
		// TODO
		// Show message increase level
	} else {
		// Don't show message increase level
	}

	// Continue
	return true;
}


bool Game_Interpreter::CommandChangeLevel() { // Code 10420
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	Game_Actor* actor;

	switch (list[index].parameters[0]) {
		case 0:
			// All party
			for (std::vector<Game_Actor*>::iterator i = Game_Party::GetActors().begin(); 
				i != Game_Party::GetActors().end(); 
				i++) {
				(*i)->ChangeLevel((*i)->GetLevel() + value);
			}
			break;
		case 1:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[1]);
			actor->ChangeLevel(actor->GetLevel() + value);
			break;
		case 2:
			// Var hero
			actor = Game_Actors::GetActor(Game_Variables[list[index].parameters[1]]);
			actor->ChangeLevel(actor->GetLevel() + value);
			break;
		default:
			;
	}

	if (list[index].parameters[5] != 0) {
		// TODO
		// Show message increase level
	} else {
		// Don't show message increase level
	}

	// Continue
	return true;
}

bool Game_Interpreter::CommandChangeParameters() { // Code 10430
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[4],
		list[index].parameters[5]
	);

	Game_Actor* actor;
	switch (list[index].parameters[0]) {
		case 0:
			// Party
			for (std::vector<Game_Actor*>::iterator i = Game_Party::GetActors().begin(); 
				i != Game_Party::GetActors().end(); 
				i++) {
					actor = Game_Actors::GetActor((*i)->GetId());
				switch (list[index].parameters[3]) {
					case 0:
						// Max HP
						actor->SetMaxHp(actor->GetMaxHp() + value);
						break;
					case 1:
						// Max MP
						actor->SetMaxSp(actor->GetMaxSp() + value);
						break;
					case 2:
						// Attack
						actor->SetAtk(actor->GetAtk() + value);
						break;
					case 3:
						// Defense
						actor->SetDef(actor->GetDef() + value);
						break;
					case 4:
						// Spirit
						actor->SetSpi(actor->GetSpi() + value);
						break;
					case 5:
						// Agility
						actor->SetAgi(actor->GetAgi() + value);
						break;
				}				
			}
			// Continue
			return true;
		case 1:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[1]);
			break;
		case 2:
			// Var hero
			actor = Game_Actors::GetActor(Game_Variables[list[index].parameters[1]]);
			break;
		default:
			actor = NULL;
	}
	switch (list[index].parameters[3]) {
		case 0:
			// Max HP
			actor->SetMaxHp(actor->GetMaxHp() + value);
			break;
		case 1:
			// Max MP
			actor->SetMaxSp(actor->GetMaxSp() + value);
			break;
		case 2:
			// Attack
			actor->SetAtk(actor->GetAtk() + value);
			break;
		case 3:
			// Defense
			actor->SetDef(actor->GetDef() + value);
			break;
		case 4:
			// Spirit
			actor->SetSpi(actor->GetSpi() + value);
			break;
		case 5:
			// Agility
			actor->SetAgi(actor->GetAgi() + value);
			break;
	}	
	return true;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandConditionalBranch() { // Code 12010
	bool result = false;
	int value1, value2;
	Game_Actor* actor;
	Game_Character* character;

	switch (list[index].parameters[0]) {
		case 0:
			// Switch
			result = Game_Switches[list[index].parameters[1]] == (list[index].parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Game_Variables[list[index].parameters[1]];
			if (list[index].parameters[2] == 0) {
				value2 = list[index].parameters[3];
			} else {
				value2 = Game_Variables[list[index].parameters[3]];
			}
			switch (list[index].parameters[4]) {
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
			// TODO Timer
			break;
		case 3:
			// Gold
			if (list[index].parameters[2] == 0) {
				// Greater than or equal
				result = (Game_Party::GetGold() >= list[index].parameters[1]);
			} else {
				// Less than or equal
				result = (Game_Party::GetGold() >= list[index].parameters[1]);
			}
			break;
		case 4:
			// Item
			result = (Game_Party::ItemNumber(list[index].parameters[1]) > 0);
			break;
		case 5:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[1]);
			switch (list[index].parameters[2]) {
				case 0:
					// Is actor in party
					result = Game_Party::IsActorInParty(actor);
					break;
				case 1:
					// Name
					result = (actor->GetName() ==  list[index].string);
					break;
				case 2:
					// Higher or equal level
					result = (actor->GetLevel() >= list[index].parameters[3]);
					break;
				case 3:
					// Higher or equal HP
					//result = (actor->hp >= list[index].parameters[3]);
					break;
				case 4:
					// Can learn skill
					result = (actor->HasSkill(list[index].parameters[3]));
					break;
				case 5:
					// Equipped object
					result = ( 
						(actor->GetShieldId() == list[index].parameters[3]) ||
						(actor->GetArmorId() == list[index].parameters[3]) ||
						(actor->GetHelmetId() == list[index].parameters[3]) ||
						(actor->GetAccessoryId() == list[index].parameters[3]) ||
						(actor->GetWeaponId() == list[index].parameters[3])
					);
					break;
				case 6:
					// Has state
					result = (actor->HasState(list[index].parameters[3]));
					break;
				default:
					;
			}
			break;
		case 6:
			// Orientation of char
			character = GetCharacter(list[index].parameters[3]);
			if (character != NULL) {
				switch (list[index].parameters[2]) {
					case 0:
						// Up 8
						result = (character->GetDirection() == 8);
						break;
					case 1:
						// Right 6
						result = (character->GetDirection() == 6);
						break;
					case 2:
						// Down 2
						result = (character->GetDirection() == 2);
						break;
					case 3:
						// Left 4
						result = (character->GetDirection() == 4);
						break;
				}
			}
			break;
		case 7:
			// TODO On vehicle
			break;
		case 8:
			// TODO Key decision initiated this event
			break;
		case 9:
			// TODO BGM Playing
			break;
		case 10:
			// TODO Something with timer RM2003 specific
			break;
	}

	// Store result in branch
	branch[list[index].indent] = result;

	if (result) {
		branch.erase(list[index].indent);
		return true;
	}

	return CommandSkip();
}

////////////////////////////////////////////////////////////
/// Skip Command
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandSkip() {
	int indent = list[index].indent;

	for (;;) {
		// If next event command is at the same level as indent
		if ( (index < list.size()) && (list[index+1].indent == indent) ) {
			// Continue
			return true;
		}
		// Advance index
		index++;
	}

}
