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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ldb_reader.h"
#include "ldb_chunks.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read Terrain
////////////////////////////////////////////////////////////
RPG::Terrain LDB_Reader::ReadTerrain(Reader& stream) {
	RPG::Terrain terrain;
	terrain.ID = stream.Read32(Reader::CompressedInteger);

	unsigned char bitflag;

	Reader::Chunk chunk_info;
	while (!stream.Eof()) {
		chunk_info.ID = stream.Read32(Reader::CompressedInteger);
		if (chunk_info.ID == ChunkData::END) {
			break;
		} else {
			chunk_info.length = stream.Read32(Reader::CompressedInteger);
			if (chunk_info.length == 0) continue;
		}
		switch (chunk_info.ID) {
		case ChunkTerrain::name:
			terrain.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkTerrain::damage:
			terrain.damage = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::encounter_rate:
			terrain.encounter_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::background_name:
			terrain.background_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkTerrain::boat_pass:
			terrain.boat_pass = stream.ReadBool();
			break;
		case ChunkTerrain::ship_pass:
			terrain.ship_pass = stream.ReadBool();
			break;
		case ChunkTerrain::airship_pass:
			terrain.airship_pass = stream.ReadBool();
			break;
		case ChunkTerrain::airship_land:
			terrain.airship_land = stream.ReadBool();
			break;
		case ChunkTerrain::bush_depth:
			terrain.bush_depth = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::footstep:
			terrain.footstep = ReadSound(stream);
			break;
		case ChunkTerrain::on_damage_se:
			terrain.on_damage_se = stream.ReadBool();
			break;
		case ChunkTerrain::background_type:
			terrain.background_type = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::background_a_name:
			terrain.background_a_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkTerrain::background_a_scrollh:
			terrain.background_a_scrollh = stream.ReadBool();
			break;
		case ChunkTerrain::background_a_scrollv:
			terrain.background_a_scrollv = stream.ReadBool();
			break;
		case ChunkTerrain::background_a_scrollh_speed:
			terrain.background_a_scrollh_speed = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::background_a_scrollv_speed:
			terrain.background_a_scrollv_speed = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::background_b:
			terrain.background_b = stream.ReadBool();
			break;
		case ChunkTerrain::background_b_name:
			terrain.background_b_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkTerrain::background_b_scrollh:
			terrain.background_b_scrollh = stream.ReadBool();
			break;
		case ChunkTerrain::background_b_scrollv:
			terrain.background_b_scrollv = stream.ReadBool();
			break;
		case ChunkTerrain::background_b_scrollh_speed:
			terrain.background_b_scrollh_speed = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::background_b_scrollv_speed:
			terrain.background_b_scrollv_speed = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::special_flags:
			bitflag = stream.Read8();
			terrain.special_back_party_flag = (bitflag & 0x01) > 0;
			terrain.special_back_enemies_flag = (bitflag & 0x02) > 0;
			terrain.special_lateral_party_flag = (bitflag & 0x04) > 0;
			terrain.special_lateral_enemies_flag = (bitflag & 0x08) > 0;
			break;
		case ChunkTerrain::special_back_party:
			terrain.special_back_party = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::special_back_enemies:
			terrain.special_back_enemies = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::special_lateral_party:
			terrain.special_lateral_party = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::special_lateral_enemies:
			terrain.special_lateral_enemies = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::grid_location:
			terrain.grid_location = stream.Read32(Reader::CompressedInteger);
			break;
		/*case ChunkTerrain::grid_a:
			terrain.grid_a = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::grid_b:
			terrain.grid_b = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkTerrain::grid_c:
			terrain.grid_c = stream.Read32(Reader::CompressedInteger);
			break;*/
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return terrain;
}
