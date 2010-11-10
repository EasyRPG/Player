#ifndef _H_GAMEEVENT
#define _H_GAMEEVENT

#include <vector>

// TODO Implement this class
#include "game_character.h"
#include "rpg_commonevent.h"

class Game_Event :
	public Game_Character
{
public:
	Game_Event();
	~Game_Event();

	void ClearStarting() {
		starting = false;
	}

	bool starting;
	int trigger;
	std::vector<RPG::EventCommand> list;
};

#endif
