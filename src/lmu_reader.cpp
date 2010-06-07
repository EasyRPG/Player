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
#include "lmu_reader.h"
#include "lmu_chunks.h"
#include "reader.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Load Map
////////////////////////////////////////////////////////////
RPG::Map LMU_Reader::LoadMap(const std::string& filename) {
    FILE* stream;
    stream = fopen(filename.c_str(), "rb");
    if (!stream) {
        Output::Error("Couldn't find %s map tree file.\n", filename.c_str());
    }
    std::string header = Reader::String(stream, Reader::CInteger(stream));
    if (header != "LcfMapUnit") {
        Output::Error("%s is not a valid RPG2000 map.\n", filename.c_str());
    }
    RPG::Map map = ReadMapData(stream);
    fclose(stream);
    return map;
}

////////////////////////////////////////////////////////////
/// Read Map
////////////////////////////////////////////////////////////
RPG::Map LMU_Reader::ReadMapData(FILE* stream) {
    RPG::Map map;
    //Reader::CInteger(stream);

    Reader::Chunk chunk_info;
    while (!feof(stream)) {
        chunk_info.ID = Reader::CInteger(stream);
        if (chunk_info.ID == ChunkData::END) {
            break;
        }
        else {
            chunk_info.length = Reader::CInteger(stream);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkMap::chipset_id:
            map.chipset_id = Reader::CInteger(stream);
            break;
        case ChunkMap::width:
            map.width = Reader::CInteger(stream);
            break;
        case ChunkMap::height:
            map.height = Reader::CInteger(stream);
            break;
        case ChunkMap::scroll_type:
            map.scroll_type = Reader::CInteger(stream);
            break;
        case ChunkMap::parallax_flag:
            map.parallax_flag = Reader::Flag(stream);
            break;
        case ChunkMap::parallax_name:
            map.parallax_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkMap::parallax_loop_x:
            map.parallax_loop_x = Reader::Flag(stream);
            break;
        case ChunkMap::parallax_loop_y:
            map.parallax_loop_y = Reader::Flag(stream);
            break;
        case ChunkMap::parallax_auto_loop_x:
            map.parallax_auto_loop_x = Reader::Flag(stream);
            break;
        case ChunkMap::parallax_sx:
            map.parallax_sx = Reader::CInteger(stream);
            break;
        case ChunkMap::parallax_auto_loop_y:
            map.parallax_auto_loop_y = Reader::Flag(stream);
            break;
        case ChunkMap::parallax_sy:
            map.parallax_sy = Reader::CInteger(stream);
            break;
        case ChunkMap::lower_layer:
            map.lower_layer = Reader::ArrayShort(stream, chunk_info.length);
            break;
        case ChunkMap::upper_layer:
            map.upper_layer = Reader::ArrayShort(stream, chunk_info.length);
            break;
        /*case ChunkMap::events: // It's reading bad, FIXME
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                map.events.push_back(ReadEvent(stream));
            }
            break;*/
        case ChunkMap::save_times:
            map.save_times = Reader::CInteger(stream);
            break;
        case ChunkMap::generator_flag:
            map.generator_flag = Reader::Flag(stream);
            break;
        case ChunkMap::generator_mode:
            map.generator_mode = Reader::CInteger(stream);
            break;
        case ChunkMap::generator_tiles:
            map.generator_tiles = Reader::CInteger(stream);
            break;
        case ChunkMap::generator_width:
            map.generator_width = Reader::CInteger(stream);
            break;
        case ChunkMap::generator_height:
            map.generator_height = Reader::CInteger(stream);
            break;
        case ChunkMap::generator_surround:
            map.generator_surround = Reader::Flag(stream);
            break;
        case ChunkMap::generator_upper_wall:
            map.generator_upper_wall = Reader::Flag(stream);
            break;
        case ChunkMap::generator_floor_b:
            map.generator_floor_b = Reader::Flag(stream);
            break;
        case ChunkMap::generator_floor_c:
            map.generator_floor_c = Reader::Flag(stream);
            break;
        case ChunkMap::generator_extra_b:
            map.generator_extra_b = Reader::Flag(stream);
            break;
        case ChunkMap::generator_extra_c:
            map.generator_extra_c = Reader::Flag(stream);
            break;
        case ChunkMap::generator_x:
            map.generator_x = Reader::ArrayUint32(stream, chunk_info.length);
            break;
        case ChunkMap::generator_y:
            map.generator_y = Reader::ArrayUint32(stream, chunk_info.length);
            break;
        case ChunkMap::generator_tile_ids:
            map.generator_tile_ids = Reader::ArrayShort(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return map;
}

