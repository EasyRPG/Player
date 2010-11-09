#ifndef __game_temp__
#define __game_temp__

#include "game_battler.h"

class Game_Temp {

public:
	Game_Temp();
	~Game_Temp();

	bool menu_calling;
	bool menu_beep;

	Game_Battler* forcing_battler;

	bool battle_calling;
	bool shop_calling;
	bool name_calling;
	bool save_calling;
	bool gameover;

};
#endif // __game_temp__
