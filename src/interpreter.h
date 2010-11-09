#ifndef _H_INTERPRETER
#define _H_INTERPRETER

#include <map>
#include <vector>
#include "rpg_eventcommand.h"

class Interpreter
{
public:
	Interpreter(int _depth = 0, bool _main_flag = false);
	~Interpreter();

	void Clear();
	void Setup(std::vector<RPG::EventCommand> _list, int _event_id);
	bool IsRunning();
	void Update();

	void SetupStartingEvent();
	bool ExecuteCommand();
	void InputButton();

private:
	int depth;
	bool main_flag;

	int loop_count;

	bool message_waiting;
	bool move_route_waiting;

	int button_input_variable_id;
	int index;
	int map_id;
	int event_id;
	int wait_count;

	Interpreter* child_interpreter;

	std::vector<RPG::EventCommand> list;
	std::map<int, bool> branch;


};

#endif
