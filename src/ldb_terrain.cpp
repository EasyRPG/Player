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

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "ldb_reader.h"
#include "ldb_chunks.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read Terrain
////////////////////////////////////////////////////////////
RPG::Terrain LDB_Reader::ReadTerrain(FILE* stream) {
    RPG::Terrain terrain;
    terrain.ID = Reader::CInteger(stream);

    unsigned char bitflag;

    Reader::Chunk chunk_info;
    do {
        chunk_info.ID = Reader::CInteger(stream);
        if (chunk_info.ID == ChunkData::END) {
            break;
        }
        else {
            chunk_info.length = Reader::CInteger(stream);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkData::END:
            break;
        case ChunkTerrain::name:
            terrain.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerrain::damage:
            terrain.damage = Reader::CInteger(stream);
            break;
        case ChunkTerrain::encounter_rate:
            terrain.encounter_rate = Reader::CInteger(stream);
            break;
        case ChunkTerrain::background_name:
            terrain.background_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerrain::boat_pass:
            terrain.boat_pass = Reader::Flag(stream);
            break;
        case ChunkTerrain::ship_pass:
            terrain.ship_pass = Reader::Flag(stream);
            break;
        case ChunkTerrain::airship_pass:
            terrain.airship_pass = Reader::Flag(stream);
            break;
        case ChunkTerrain::airship_land:
            terrain.airship_land = Reader::Flag(stream);
            break;
        case ChunkTerrain::bush_depth:
            terrain.bush_depth = Reader::CInteger(stream);
            break;
        case ChunkTerrain::footstep:
            terrain.footstep = ReadSound(stream);
            break;
        case ChunkTerrain::on_damage_se:
            terrain.on_damage_se = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_type:
            terrain.background_type = Reader::CInteger(stream);
            break;
        case ChunkTerrain::background_a_name:
            terrain.background_a_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerrain::background_a_scrollh:
            terrain.background_a_scrollh = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_a_scrollv:
            terrain.background_a_scrollv = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_a_scrollh_speed:
            terrain.background_a_scrollh_speed = Reader::CInteger(stream);
            break;
        case ChunkTerrain::background_a_scrollv_speed:
            terrain.background_a_scrollv_speed = Reader::CInteger(stream);
            break;
        case ChunkTerrain::background_b:
            terrain.background_b = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_b_name:
            terrain.background_b_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerrain::background_b_scrollh:
            terrain.background_b_scrollh = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_b_scrollv:
            terrain.background_b_scrollv = Reader::Flag(stream);
            break;
        case ChunkTerrain::background_b_scrollh_speed:
            terrain.background_b_scrollh_speed = Reader::CInteger(stream);
            break;
        case ChunkTerrain::background_b_scrollv_speed:
            terrain.background_b_scrollv_speed = Reader::CInteger(stream);
            break;
        case ChunkTerrain::special_flags:
            bitflag = Reader::BitFlag(stream);
            terrain.special_back_party_flag = (bitflag & 0x01) > 0;
            terrain.special_back_enemies_flag = (bitflag & 0x02) > 0;
            terrain.special_lateral_party_flag = (bitflag & 0x04) > 0;
            terrain.special_lateral_enemies_flag = (bitflag & 0x08) > 0;
            break;
        case ChunkTerrain::special_back_party:
            terrain.special_back_party = Reader::CInteger(stream);
            break;
        case ChunkTerrain::special_back_enemies:
            terrain.special_back_enemies = Reader::CInteger(stream);
            break;
        case ChunkTerrain::special_lateral_party:
            terrain.special_lateral_party = Reader::CInteger(stream);
            break;
        case ChunkTerrain::special_lateral_enemies:
            terrain.special_lateral_enemies = Reader::CInteger(stream);
            break;
        case ChunkTerrain::grid_location:
            terrain.grid_location = Reader::CInteger(stream);
            break;
        /*case ChunkTerrain::grid_a:
            terrain.grid_a = Reader::CInteger(stream);
            break;
        case ChunkTerrain::grid_b:
            terrain.grid_b = Reader::CInteger(stream);
            break;
        case ChunkTerrain::grid_c:
            terrain.grid_c = Reader::CInteger(stream);
            break;*/
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return terrain;
}
  