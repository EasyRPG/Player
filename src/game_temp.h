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

#ifndef _GAME_TEMP_H_
#define _GAME_TEMP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "game_battler.h"
#include "graphics.h"

////////////////////////////////////////////////////////////
/// Game Temp static class
////////////////////////////////////////////////////////////
class Game_Temp {
public:
	////////////////////////////////////////////////////////
	/// Initialize Game Temp.
	////////////////////////////////////////////////////////
	static void Init();

	static bool menu_calling;
	static bool menu_beep;

	static Game_Battler* forcing_battler;

	static bool battle_calling;
	static bool shop_calling;
	static bool name_calling;
	static bool save_calling;
	static bool gameover;

	static int common_event_id;

	static bool transition_processing;
	static Graphics::TransitionType transition_type;
	static bool transition_erase;

private:
	Game_Temp();
};

#endif
