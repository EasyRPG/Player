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
/// Read Chipset
////////////////////////////////////////////////////////////
RPG::Chipset LDB_Reader::ReadChipset(FILE* stream) {
    RPG::Chipset chipset;
    chipset.ID = Reader::CInteger(stream);

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
        case ChunkChipset::name:
            chipset.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkChipset::chipset_name:
            chipset.chipset_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkChipset::terrain_data:
            chipset.terrain_data = Reader::ArrayShort(stream, chunk_info.length);
            break;
        case ChunkChipset::passable_data_lower:
            chipset.passable_data_lower = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkChipset::passable_data_upper:
            chipset.passable_data_upper = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkChipset::animation_type:
            chipset.animation_type = Reader::CInteger(stream);
            break;
        case ChunkChipset::animation_speed:
            chipset.animation_speed = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return chipset;
}
