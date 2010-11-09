#ifndef _H_GAMECOMMONEVENT
#define _H_GAMECOMMONEVENT

#include <string>
#include <vector>
#include "rpg_commonevent.h"
#include "interpreter.h"

class Game_CommonEvent
{
public:
	Game_CommonEvent(int _common_event_id);
	~Game_CommonEvent();

	std::string Name();
	int Trigger();
	int SwitchId();
	std::vector<RPG::EventCommand> List();

	void Refresh();
	void Update();

private:

	int common_event_id;

	Interpreter* interpreter; // Interpreter for parallel procceses
};

#endif
