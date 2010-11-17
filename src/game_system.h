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
/// Game System static class
////////////////////////////////////////////////////////////
class Game_System {
public:
	////////////////////////////////////////////////////////
	/// Initialize Game System.
	////////////////////////////////////////////////////////
	static void Init();

	////////////////////////////////////////////////////////
	/// Play a Music.
	/// @param bgm : music data.
	////////////////////////////////////////////////////////
	static void BgmPlay(RPG::Music bmg);

	////////////////////////////////////////////////////////
	/// Play a Sound.
	/// @param se : sound data.
	////////////////////////////////////////////////////////
	static void SePlay(RPG::Sound se);

	////////////////////////////////////////////////////////
	/// Get system graphic name.
	/// @return system graphic filename
	////////////////////////////////////////////////////////
	static std::string GetSystemName();

	////////////////////////////////////////////////////////
	/// Set the system graphic.
	/// @param system_name : new system name
	////////////////////////////////////////////////////////
	static void SetSystemName(std::string system_name);

	/// Menu saving option disabled flag.
	static bool save_disabled;

	/// Numbers of saves.
	static unsigned int save_count;

private:
	Game_System();

	static std::string system_name;
};

#endif
