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
/// Read AnimationCellData
////////////////////////////////////////////////////////////
RPG::AnimationCellData LDB_Reader::ReadAnimationCellData(FILE* stream) {
    RPG::AnimationCellData celldata;
    Reader::CInteger(stream);

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
        /*case ChunkAnimationCellData::priority:
            celldata.priority = Reader::CInteger(stream);
            break;*/
        case ChunkAnimationCellData::ID:
            celldata.ID = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::x:
            celldata.x = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::y:
            celldata.y = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::zoom:
            celldata.zoom = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::tone_red:
            celldata.tone_red = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::tone_green:
            celldata.tone_green = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::tone_blue:
            celldata.tone_blue = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::tone_gray:
            celldata.tone_gray = Reader::CInteger(stream);
            break;
        case ChunkAnimationCellData::opacity:
            celldata.opacity = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return celldata;
}
