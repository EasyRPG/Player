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

#ifndef _LMT_READER_CHUNKS_H_
#define _LMT_READER_CHUNKS_H_

////////////////////////////////////////////////////////////
/// LMT Reader namespace
////////////////////////////////////////////////////////////
namespace LMT_Reader {
	namespace ChunkData {
		enum ChunkData {
			END = 0x00 // End of chunk
		};
	}

	namespace ChunkTreeMap {
		enum ChunkTreeMap {
			start_map_id	= 0x01, // Integer
			start_x			= 0x02, // Integer
			start_y			= 0x03, // Integer
			boat_map_id		= 0x0B, // Integer
			boat_x			= 0x0C, // Integer
			boat_y			= 0x0D, // Integer
			ship_map_id		= 0x15, // Integer
			ship_x			= 0x16, // Integer
			ship_y			= 0x17, // Integer
			airship_map_id	= 0x1F, // Integer
			airship_x		= 0x20, // Integer
			airship_y		= 0x21	// Integer
		};
	}
	namespace ChunkMapInfo { //TODO FIXME: this enum can be used by areas, too
		enum ChunkMapInfo { // Please don't remove editor specific chunks, readers are used by EasyRPG editor for project import
			name			= 0x01, // String	// Note: Map ID 0 used to be game title but it should be ignored (TreeCtrl dummy editor dumped data), always use RPG_RT.ini GameTitle instead
			parent_map		= 0x02, // Integer	// Used to inherit parent map properties
			//indentation	= 0x03, // Integer	// Dummy editor dumped data, branch indentation level in TreeCtrl
			type			= 0x04, // Integer	// 0=lmt root, can be ignored; 1=map; 2=area
			scrollbar_x		= 0x05, // Integer	// Editor only
			scrollbar_y		= 0x06, // Integer	// Editor only
			expanded_node	= 0x07, // Flag		// Editor only
			music_type		= 0x0B, // Integer	// 0=inherit; 1=from event; 2=specified in 0x0C
			music_name		= 0x0C, // Array - RPG::Music
			background_type = 0x15, // Integer // 0=inherit; 1=from terrain ldb data; 2=specified in 0x16
			background_name = 0x16, // String
			teleport		= 0x1F, // Flag // 0=inherit; 1=allow; 2=disallow
			escape			= 0x20, // Flag // 0=inherit; 1=allow; 2=disallow
			save			= 0x21, // Flag // 0=inherit; 1=allow; 2=disallow
			encounters		= 0x29, // Array - RPG::Encounter
			encounter_steps = 0x2C, // Integer // 0 inherits from parent (?) FIXME
			area_rect		= 0x33	// Uint32 x 4 (X,Y,W,H) // normal map (non-area) is 0,0,0,0
		};
	}
	namespace ChunkEncounter {
		enum ChunkEncounter {
			ID = 0x01 // Integer
		};
	}
}

#endif
