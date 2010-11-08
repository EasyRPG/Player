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
#include "lmu_reader.h"
#include "lmu_chunks.h"
#include "reader.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Load Map
////////////////////////////////////////////////////////////
RPG::Map LMU_Reader::LoadMap(const std::string& filename) {
	Reader reader(filename);
	if (!reader.IsOk()) {
		Output::Error("Couldn't find %s map tree file.\n", filename.c_str());
	}
	std::string header = reader.ReadString(reader.Read32(Reader::CompressedInteger));
	if (header != "LcfMapUnit") {
		Output::Error("%s is not a valid RPG2000 map.\n", filename.c_str());
	}
	RPG::Map map = ReadMapData(reader);
	return map;
}

////////////////////////////////////////////////////////////
/// Read Map
////////////////////////////////////////////////////////////
RPG::Map LMU_Reader::ReadMapData(Reader& stream) {
	RPG::Map map;
	//stream.Read32(Reader::CompressedInteger);

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
		case ChunkMap::chipset_id:
			map.chipset_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::width:
			map.width = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::height:
			map.height = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::scroll_type:
			map.scroll_type = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::parallax_flag:
			map.parallax_flag = stream.ReadBool();
			break;
		case ChunkMap::parallax_name:
			map.parallax_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkMap::parallax_loop_x:
			map.parallax_loop_x = stream.ReadBool();
			break;
		case ChunkMap::parallax_loop_y:
			map.parallax_loop_y = stream.ReadBool();
			break;
		case ChunkMap::parallax_auto_loop_x:
			map.parallax_auto_loop_x = stream.ReadBool();
			break;
		case ChunkMap::parallax_sx:
			map.parallax_sx = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::parallax_auto_loop_y:
			map.parallax_auto_loop_y = stream.ReadBool();
			break;
		case ChunkMap::parallax_sy:
			map.parallax_sy = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::lower_layer:
			stream.Read16(map.lower_layer, chunk_info.length);
			break;
		case ChunkMap::upper_layer:
			stream.Read16(map.upper_layer, chunk_info.length);
			break;
		case ChunkMap::events:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				map.events.push_back(ReadEvent(stream));
			}
			break;
		case ChunkMap::save_times:
			map.save_times = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::generator_flag:
			map.generator_flag = stream.ReadBool();
			break;
		case ChunkMap::generator_mode:
			map.generator_mode = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::generator_tiles:
			map.generator_tiles = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::generator_width:
			map.generator_width = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::generator_height:
			map.generator_height = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkMap::generator_surround:
			map.generator_surround = stream.ReadBool();
			break;
		case ChunkMap::generator_upper_wall:
			map.generator_upper_wall = stream.ReadBool();
			break;
		case ChunkMap::generator_floor_b:
			map.generator_floor_b = stream.ReadBool();
			break;
		case ChunkMap::generator_floor_c:
			map.generator_floor_c = stream.ReadBool();
			break;
		case ChunkMap::generator_extra_b:
			map.generator_extra_b = stream.ReadBool();
			break;
		case ChunkMap::generator_extra_c:
			map.generator_extra_c = stream.ReadBool();
			break;
		case ChunkMap::generator_x:
			stream.Read32(map.generator_x, chunk_info.length);
			break;
		case ChunkMap::generator_y:
			stream.Read32(map.generator_y, chunk_info.length);
			break;
		case ChunkMap::generator_tile_ids:
			stream.Read16(map.generator_tile_ids, chunk_info.length);
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return map;
}

