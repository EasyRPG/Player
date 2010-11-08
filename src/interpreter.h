#ifndef _H_INTERPRETER
#define _H_INTERPRETER

#include <map>

class Interpreter
{
public:
	Interpreter(int _depth, bool _main);
	~Interpreter(void);

	void Clear();
private:
	int depth;
	bool main;

	int map_id;
	int event_id;
	bool message_waiting;
	bool move_route_waiting;
	int button_input_variable_id;
	int wait_count;

	Interpreter* child_interpreter;

	std::map<int, bool> branch;


};

#endif
