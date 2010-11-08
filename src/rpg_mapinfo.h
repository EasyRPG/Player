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

#ifndef _RPG_MAPINFO_H_
#define _RPG_MAPINFO_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rect.h"
#include "rpg_encounter.h"
#include "rpg_music.h"

////////////////////////////////////////////////////////////
/// RPG::MapInfo class
////////////////////////////////////////////////////////////
namespace RPG {
	class MapInfo {
	public:
		MapInfo();
		
		int ID;
		std::string name;
		int parent_map;
		int type;
		int music_type;
		Music music;
		int background_type;
		std::string background_name;
		int teleport;
		int escape;
		int save;
		std::vector<Encounter> encounters;
		int encounter_steps;
		Rect area_rect;
	};
}

#endif
