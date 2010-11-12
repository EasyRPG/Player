#ifndef __game_temp__
#define __game_temp__

#include <string>
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

	std::string message_text;

	int common_event_id;
	
	/* Number of lines before the start
	of selection options.
	+-----------------------------------+
	|	Hi, hero, What's your name?		|
	|- Alex								|
	|- Brian							|
	|- Carol							|
	+-----------------------------------+
	In this case, choice_start would be 1.
	Same with num_input_start.
	*/
	int choice_start;
	int num_input_start;

	// Number of choices
	int choice_max;

	// Option to choose if cancel
	int choice_cancel_type;

	int num_input_variable_id;
	int num_input_digits_max;

};
#endif // __game_temp__
