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
/// Game System namespace
////////////////////////////////////////////////////////////
namespace Game_System {
	////////////////////////////////////////////////////////
	/// Initialize Game System.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Play a Music.
	/// @param bgm : music data.
	////////////////////////////////////////////////////////
	void BgmPlay(RPG::Music bmg);

	////////////////////////////////////////////////////////
	/// Play a Sound.
	/// @param se : sound data.
	////////////////////////////////////////////////////////
	void SePlay(RPG::Sound se);

	////////////////////////////////////////////////////////
	/// Get system graphic name.
	/// @return system graphic filename
	////////////////////////////////////////////////////////
	std::string GetSystemName();

	////////////////////////////////////////////////////////
	/// Set the system graphic.
	/// @param system_name : new system name
	////////////////////////////////////////////////////////
	void SetSystemName(std::string system_name);

	/// Menu saving option disabled flag.
	extern bool save_disabled;
	extern bool teleport_disabled;
	extern bool escape_disabled;
	extern bool main_menu_disabled;
	extern RPG::Music current_bgm;
	extern RPG::Music memorized_bgm;

	/// Numbers of saves.
	extern unsigned int save_count;
}

#endif
