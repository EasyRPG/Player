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

#ifndef _RPG_MAP_H_
#define _RPG_MAP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_event.h"

////////////////////////////////////////////////////////////
/// RPG::Map class
////////////////////////////////////////////////////////////
namespace RPG {
	class Map {
	public:
		Map();
		
		int ID;
		int chipset_id;
		int width;
		int height;
		int scroll_type;
		bool parallax_flag;
		std::string parallax_name;
		bool parallax_loop_x;
		bool parallax_loop_y;
		bool parallax_auto_loop_x;
		int parallax_sx;
		bool parallax_auto_loop_y;
		int parallax_sy;
		std::vector<short> lower_layer;
		std::vector<short> upper_layer;
		std::vector<RPG::Event> events;
		int save_times;

		bool generator_flag;
		int generator_mode;
		int generator_tiles;
		int generator_width;
		int generator_height;
		bool generator_surround;
		bool generator_upper_wall;
		bool generator_floor_b;
		bool generator_floor_c;
		bool generator_extra_b;
		bool generator_extra_c;
		std::vector<unsigned int> generator_x;
		std::vector<unsigned int> generator_y;
		std::vector<short> generator_tile_ids;
	};
}

#endif
