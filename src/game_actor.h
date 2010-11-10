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

#ifndef _GAME_ACTOR_H_
#define _GAME_ACTOR_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "bitmap.h"
#include "game_battler.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Game_Actor class
////////////////////////////////////////////////////////////
class Game_Actor : public Game_Battler {

public:
	Game_Actor(int actorId);
	~Game_Actor();

	void Setup(int actorId);

	std::string name;
	Bitmap* face;

	int weapon_id;
	int armor1_id;
	int armor2_id;
	int armor3_id;
	int armor4_id;

	int level;
	int exp;
};

#endif // _GAME_ACTOR_H_
