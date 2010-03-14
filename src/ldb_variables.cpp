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
/// Read Variables
////////////////////////////////////////////////////////////
std::vector<std::string> LDB_Reader::ReadVariables(FILE* stream) {
    std::vector<std::string> variables;
    variables.resize(Reader::CInteger(stream));

    int pos;
    Reader::Chunk chunk_info;
    for (int i = variables.size(); i > 0; i--) {
        pos = Reader::CInteger(stream);
        chunk_info.ID = Reader::CInteger(stream);
        if (chunk_info.ID != ChunkData::END) {
            chunk_info.length = Reader::CInteger(stream);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkData::END:
            break;
        case ChunkVariable::name:
            variables[pos] = Reader::String(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return variables;
}
