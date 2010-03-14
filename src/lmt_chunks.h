//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

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
    };

    namespace ChunkTreeMap {
        enum ChunkTreeMap {
            start_map_id    = 0x01, // Integer
            start_x         = 0x02, // Integer
            start_y         = 0x03, // Integer
            boat_map_id     = 0x0B, // Integer
            boat_x          = 0x0C, // Integer
            boat_y          = 0x0D, // Integer
            ship_map_id     = 0x15, // Integer
            ship_x          = 0x16, // Integer
            ship_y          = 0x17, // Integer
            airship_map_id  = 0x1F, // Integer
            airship_x       = 0x20, // Integer
            airship_y       = 0x21  // Integer
        };
    };
    namespace ChunkMapInfo {
        enum ChunkMapInfo {
            name            = 0x01, // String
            parent_map      = 0x02, // Integer
            type            = 0x03, // Integer
            typeD1          = 0x04, // Integer
            //???           = 0x07, // ???
            music_type      = 0x0B, // Integer
            music_name      = 0x0C, // String
            background_type = 0x15, // Integer
            background_name = 0x16, // String
            teleport        = 0x1F, // Flag
            escape          = 0x20, // Flag
            save            = 0x21, // Flag
            encounters      = 0x29, // Array - RPG::Encounter
            encounter_steps = 0x2C, // Integer
            area_rect       = 0x33, // Uint32 x 4
        };
    };
    namespace ChunkEncounter {
        enum ChunkEncounter {
            ID = 0x01 // Integer
        };
    };
};

#endif
