#include "interpreter.h"
#include "output.h"

Interpreter::Interpreter(int _depth, bool _main)
{
	depth = _depth;
	main = _main;

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
}

Interpreter::~Interpreter(void)
{
}
