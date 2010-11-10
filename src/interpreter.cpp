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

#include "interpreter.h"
#include "output.h"
#include "input.h"
#include "game_map.h"
#include "game_player.h"
#include "game_event.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "graphics.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#define max(a, b)	(((a) >= (b)) ? (a) : (b))

Interpreter::Interpreter(int _depth, bool _main_flag)
{
	depth = _depth;
	main_flag = _main_flag;

	if (depth > 100) {
		Output::Warning("Too many event calls (over 9000)");
	}

	Clear();
}

Interpreter::~Interpreter()
{
}

////////////////////////////////////////////////////////////
/// Clear
////////////////////////////////////////////////////////////
void Interpreter::Clear() {
    map_id = 0;                   //    # map ID when starting up
    event_id = 0;                 //    # event ID
    message_waiting = false;      //    # waiting for message to end
    move_route_waiting = false;   //    # waiting for move completion
    button_input_variable_id = 0; //    # button input variable ID
    wait_count = 0;               //    # wait count
    child_interpreter = NULL;     //    # child interpreter for common events, etc
	branch.clear();
}

////////////////////////////////////////////////////////////
/// Is Interpreter Running
////////////////////////////////////////////////////////////
bool Interpreter::IsRunning() {
	return list.empty();
}

////////////////////////////////////////////////////////////
/// Setup
////////////////////////////////////////////////////////////
void Interpreter::Setup(std::vector<RPG::EventCommand> _list, int _event_id) {

	Clear();

	map_id = Main_Data::game_map->GetMapId();
	event_id = _event_id;
	list = _list;
	index = 0;

	branch.clear();
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Interpreter::Update() {

	loop_count = 0;

	for (;;) {
		loop_count++;

		if (loop_count > 100) {
			Graphics::Update(); // Freeze prevention
			loop_count = 0;
		}

		/* If map is different than event startup time
		set event_id to 0 */
		if (Main_Data::game_map->GetMapId() != map_id) {
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
				break;
			}
		}

		if (message_waiting) {
			break;
		}

		// If waiting for a move to end
		if (move_route_waiting) {
			if (Main_Data::game_player->move_route_forcing) {
				break;
			}
				
			unsigned int i;
			Game_Event* g_event;
			for (i = 0; i < Main_Data::game_map->events.size(); i++) {
				g_event = Main_Data::game_map->events[i];

				if (g_event->move_route_forcing) {
					return;
				}
			}
			move_route_waiting = false;
		}

		if (button_input_variable_id > 0) {
			InputButton();
			break;
		}

		if (wait_count > 0) {
			wait_count--;
			break;
		}

		if (Main_Data::game_temp->forcing_battler != NULL) {
			break;
		}

		if (Main_Data::game_temp->battle_calling ||
			Main_Data::game_temp->shop_calling ||
			Main_Data::game_temp->name_calling ||
			Main_Data::game_temp->menu_calling ||
			Main_Data::game_temp->save_calling ||
			Main_Data::game_temp->gameover) {
			
			break;
		}

		if (list.empty()) {
			if (main_flag) {
				SetupStartingEvent();
			}

			if (list.empty()) {
				break;
			}
		}

		if (!ExecuteCommand()) {
			break;
		}

		index++;
	} // for
}

////////////////////////////////////////////////////////////
/// Setup Starting Event
////////////////////////////////////////////////////////////
void Interpreter::SetupStartingEvent() {

	if (Main_Data::game_map->need_refresh) {
		Main_Data::game_map->Refresh();
	}

	if (Main_Data::game_temp->common_event_id > 0) {
		Setup(Main_Data::data_commonevents[Main_Data::game_temp->common_event_id].event_commands, 0);
		Main_Data::game_temp->common_event_id = 0;
		return;
	}
	
	unsigned int i;
	Game_Event* g_event;
	for (i = 0; i < Main_Data::game_map->events.size(); i++) {
		g_event = Main_Data::game_map->events[i];
		
		if (g_event->starting) {
			if (g_event->trigger < 3) {
				g_event->ClearStarting();
				g_event->Lock();
			}
			Setup(g_event->list, g_event->id);
			return;
		}
	}

	RPG::CommonEvent* common_event;
	for (i = 0; i < Main_Data::data_commonevents.size(); i++) {
		common_event = &Main_Data::data_commonevents[i];

		// If trigger is auto run, and condition switch is ON
		if ( (common_event->trigger == 1) &&
			(*Main_Data::game_switches)[common_event->switch_id]) {
			Setup(common_event->event_commands, 0);
			return;
		}
	}
}

////////////////////////////////////////////////////////////
/// Execute Command
////////////////////////////////////////////////////////////
bool Interpreter::ExecuteCommand() {
	/*
	if (index >= list.size() - 1) {
		//CommandEnd();
		return true;
	}
	
	switch (list[index].code) {

		case 10110: return CommandShowMessage();
		case 10120: return CommandMessageOptions();
		case 10130: return CommandShowFaceGraphic();
		case 10140: return CommandSelectOption();

	} */
	return true;
}

////////////////////////////////////////////////////////////
/// Input Button
////////////////////////////////////////////////////////////
void Interpreter::InputButton() {
	Input::InputButton n = Input::NOBUTTON;
	
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
	if (n != Input::NOBUTTON) {
		// Set variable
		(*Main_Data::game_variables)[button_input_variable_id] = n;
		Main_Data::game_map->need_refresh = true;
		button_input_variable_id = 0;
	}
}

/////////////////////////////////////////////
/// Get Strings for choice selection
/// This is just a helper (private) method
/// to avoid repeating code
/////////////////////////////////////////////
std::vector<std::string> Interpreter::GetStrings() {
	/* Let's find the choices */
	int current_indent = list[index+1].indent;
	unsigned int index_temp = index+2;
	std::vector<std::string> s_choices;
	while ( index_temp < list.size() ) {
		if ( (list[index_temp].code == 20140) && (list[index_temp].indent == current_indent) ) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}
		// If found end of show choice command
		if ( ( (list[index_temp].code == 20141) && (list[index_temp].indent == current_indent) ) ||
			// Or found Cancel branch
			( (list[index_temp].code == 20140) && (list[index_temp].indent == current_indent) &&
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
bool Interpreter::CommandShowMessage() { // Code 10110
	// If there's a text already, return immediately
	if (!Main_Data::game_temp->message_text.empty()) {
		return false;
	}
	unsigned int line_count = 0;

	message_waiting = true;

	// Set first line
	Main_Data::game_temp->message_text = list[index].string + "\n";
	line_count++;

	for (;;) {
		// If next event command is the following parts of the message
		if ( (index < list.size()) && (list[index+1].code == 20110) )  {
			// Add second (another) line
			Main_Data::game_temp->message_text += list[index+1].string + "\n";
			line_count++;
		} else {
			// If next event command is show choices
			std::vector<std::string> s_choices;
			if ( (index < list.size()) && (list[index+1].code == 10140) ) {
				s_choices = GetStrings();
				// If choices fit on screen
				if (s_choices.size() < (4 - line_count)) {
					index++;
					Main_Data::game_temp->choice_start = line_count;
					Main_Data::game_temp->choice_cancel_type = list[index].parameters[0];
					SetupChoices(s_choices);
				}
			} else {
				// If next event command is input number
				if ((index < list.size()) && (list[index+1].code == 10150) ) {
					// If input number fits on screen 
					if (line_count < 4) {
						index++;
						Main_Data::game_temp->num_input_start = line_count;
						Main_Data::game_temp->num_input_digits_max = list[index].parameters[0];
						Main_Data::game_temp->num_input_variable_id = list[index].parameters[1];
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
void Interpreter::SetupChoices(const std::vector<std::string>& choices) {
	Main_Data::game_temp->choice_max = choices.size();

	// Set choices to message text
	unsigned int i;
	for (i = 0; i < choices.size(); i++) {
		Main_Data::game_temp->message_text += choices[i] + "\n";
	}

	/* Set callback stuff */

	// TODO
}

////////////////////////////////////////////////////////////
/// Command Show choices
////////////////////////////////////////////////////////////
bool Interpreter::CommandShowChoices() { // Code 10140
	if (!Main_Data::game_temp->message_text.empty()) {
		return false;
	}

	message_waiting = true;

	// Choices setup
	Main_Data::game_temp->message_text.clear();
	Main_Data::game_temp->choice_start = 0;
	Main_Data::game_temp->choice_cancel_type = list[index].parameters[0];
	SetupChoices(GetStrings());

	return true;
}

////////////////////////////////////////////////////////////
/// Command control switches
////////////////////////////////////////////////////////////
bool Interpreter::CommandControlSwitches() { // Code 10210
	int i;
	switch (list[index].parameters[0]) {
		case 0:
		case 1:
			// Single and switch range
			for (i = list[index].parameters[1]; i <= list[index].parameters[2]; i++) {
				if (list[index].parameters[3] != 2) {
					Main_Data::game_switches->SetAt(i, list[index].parameters[3] == 0);
				} else {
					Main_Data::game_switches->ToggleAt(i);
				}
			}
			break;
		case 2:
			// Switch from variable
			if (list[index].parameters[3] != 2) {
				Main_Data::game_switches->SetAt(list[index].parameters[2], list[index].parameters[3] == 0);
			} else {
				Main_Data::game_switches->ToggleAt(list[index].parameters[2]);
			}
			break;
		default: 
			return false;
	}
	return true;
}

////////////////////////////////////////////////////////////
/// Command control vars
////////////////////////////////////////////////////////////
bool Interpreter::CommandControlVariables() { // Code 10220
	int i, value = 0;
	Game_Actor* actor;

	switch (list[index].parameters[0]) {
		case 0:
			// Constant
			value = list[index].parameters[5];
			break;
		case 1:
			// Var A ops B
			value = (*Main_Data::game_variables)[list[index].parameters[5]];
			break;
		case 2:
			// Number of var A ops B
			value = (*Main_Data::game_variables)[(*Main_Data::game_variables)[list[index].parameters[5]]];
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
					value = Main_Data::game_party->ItemNumber(list[index].parameters[6]);
					break;
				case 1:
					// Number of heroes that have the item equipped
					std::vector<Game_Actor*>::iterator j;
					for (j = Main_Data::game_party->actors.begin(); j != Main_Data::game_party->actors.end(); j++) {
						if ( ((*j)->weapon_id == list[index].parameters[6]) || 
							((*j)->armor1_id == list[index].parameters[6]) ||
							((*j)->armor2_id == list[index].parameters[6]) ||
							((*j)->armor3_id == list[index].parameters[6]) ||
							((*j)->armor4_id == list[index].parameters[6]) ) {
								value++;
						}
					}
					break;
			}
			break;
		case 5:
			// Hero
			actor = Main_Data::game_actors->GetActor(list[index].parameters[6]);
			if (actor != NULL) {
				switch (list[index].parameters[7]) {
					case 0:
						// Level
						value = actor->level;
						break;
					case 1:
						// Experience
						value = actor->exp;
						break;
					case 2: 
						// Current HP
						value = actor->hp;
						break;
					case 3: 
						// Current MP
						value = actor->mp;
						break;
					case 4:
						// Max HP
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_maxhp[actor->level-1];
						break;
					case 5:
						// Max MP
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_maxsp[actor->level-1];
						break;
					case 6:
						// Attack 
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_attack[actor->level-1];
						break;
					case 7:
						// Defense
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_defense[actor->level-1];
						break;
					case 8:
						// Intelligence
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_spirit[actor->level-1];
						break;
					case 9:
						// Agility
						value = Main_Data::data_actors[list[index].parameters[6]].parameter_agility[actor->level-1];
						break;
					case 10:
						// Weapon ID
						value = actor->weapon_id;
						break;
					case 11:
						// Shield ID
						value = actor->armor1_id;
						break;
					case 12:
						// Armor ID
						value = actor->armor2_id;
						break;
					case 13:
						// Helmet ID
						value = actor->armor3_id;
						break;
					case 14:
						// Accesory ID
						value = actor->armor4_id;
						break;
				}
			}
			break;
		case 6:
			// Characters
			break;
		case 7:
			// More
			switch (list[index].parameters[6]) {
				case 0:
					// Gold
					value = Main_Data::game_party->gold;
					break;
				case 1:
					// TODO Seconds remaining in the timer
					break;
				case 2:
					// Number of heroes in party
					value = Main_Data::game_party->actors.size();
					break;
				case 3:
					// Number of saves
					value = Main_Data::game_system->save_count;
					break;
				case 4:
					// Number of battles
					value = Main_Data::game_party->battle_count;
					break;
				case 5:
					// Number of wins
					value = Main_Data::game_party->win_count;
					break;
				case 6:
					// Number of defeats
					value = Main_Data::game_party->defeat_count;
					break;
				case 7:
					// Number of flights (aka run away)
					value = Main_Data::game_party->run_count;
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
						(*Main_Data::game_variables)[i] = value;
						break;
					case 1:
						// Addition
						(*Main_Data::game_variables)[i] += value;
						break;
					case 2:
						// Subtraction
						(*Main_Data::game_variables)[i] -= value;
						break;
					case 3:
						// Multiplication
						(*Main_Data::game_variables)[i] *= value;
						break;
					case 4:
						// Division
						if (value != 0) {
							(*Main_Data::game_variables)[i] /= value;
						}
						break;
					case 5:
						// Module
						if (value != 0) {
							(*Main_Data::game_variables)[i] %= value;
						}
				}
				if ((*Main_Data::game_variables)[i] > 9999999) {
					(*Main_Data::game_variables)[i] = 9999999;
				}
				if ((*Main_Data::game_variables)[i] < -99999999) {
					(*Main_Data::game_variables)[i] = -99999999;
				}
			}
			break;

		case 2:
			switch (list[index].parameters[3]) {
				case 0:
					// Assignement
					(*Main_Data::game_variables)[list[index].parameters[1]] = value;
					break;
				case 1:
					// Addition
					(*Main_Data::game_variables)[list[index].parameters[1]] += value;
					break;
				case 2:
					// Subtraction
					(*Main_Data::game_variables)[list[index].parameters[1]] -= value;
					break;
				case 3:
					// Multiplication
					(*Main_Data::game_variables)[list[index].parameters[1]] *= value;
					break;
				case 4:
					// Division
					if (value != 0) {
						(*Main_Data::game_variables)[list[index].parameters[1]] /= value;
					}
					break;
				case 5:
					// Module
					if (value != 0) {
						(*Main_Data::game_variables)[list[index].parameters[1]] %= value;
					}
			}
			if ((*Main_Data::game_variables)[list[index].parameters[1]] > 9999999) {
				(*Main_Data::game_variables)[list[index].parameters[1]] = 9999999;
			}
			if ((*Main_Data::game_variables)[list[index].parameters[1]] < -99999999) {
				(*Main_Data::game_variables)[list[index].parameters[1]] = -99999999;
			}
	}
	Main_Data::game_map->need_refresh = true;
	return true;
}
