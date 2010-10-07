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

#ifndef _GAME_SYSTEM_H_
#define _GAME_SYSTEM_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "rpg_music.h"
#include "rpg_sound.h"

////////////////////////////////////////////////////////////
/// Game System class
////////////////////////////////////////////////////////////
class Game_System {
public:
	Game_System();
	~Game_System();

	void BgmPlay(RPG::Music bmg);
	void SePlay(RPG::Sound se);

	std::string GetSystemName() const;
	void SetSystemName(std::string nsystem_name);

private:
	std::string system_name;
};

#endif
