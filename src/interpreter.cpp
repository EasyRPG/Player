#include "interpreter.h"
#include "output.h"
#include "game_map.h"
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

}

Interpreter::~Interpreter(void)
{
}
