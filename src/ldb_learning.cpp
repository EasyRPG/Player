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
/// Read Learning
////////////////////////////////////////////////////////////
RPG::Learning LDB_Reader::ReadLearning(Reader& stream) {
    RPG::Learning learning;
    stream.Read32(Reader::CompressedInteger);

    Reader::Chunk chunk_info;
    while (!stream.Eof()) {
        chunk_info.ID = stream.Read32(Reader::CompressedInteger);
        if (chunk_info.ID == ChunkData::END) {
            break;
        }
        else {
            chunk_info.length = stream.Read32(Reader::CompressedInteger);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkLearning::level:
            learning.level = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkLearning::skill_id:
            learning.skill_id = stream.Read32(Reader::CompressedInteger);
            break;
        default:
            stream.Seek(chunk_info.length, Reader::FromCurrent);
        }
    }
    return learning;
}
