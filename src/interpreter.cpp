#include "interpreter.h"
#include "output.h"
#include "game_map.h"
#include "game_player.h"
#include "game_event.h"
#include "game_temp.h"
#include "graphics.h"
#include "main_data.h"

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

bool Interpreter::IsRunning() {
	return list.empty();
}

void Interpreter::Setup(std::vector<RPG::EventCommand> _list, int _event_id) {

	Clear();

	map_id = Main_Data::game_map->GetMapId();
	event_id = _event_id;
	list = _list;
	index = 0;

	branch.clear();
}

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
				
			int i;
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

void Interpreter::SetupStartingEvent() {
	//TODO
}

bool Interpreter::ExecuteCommand() {
	// TODO
	return true;
}

void Interpreter::InputButton() {
	// TODO
}

