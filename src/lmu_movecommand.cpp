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

////////////////////////////////////////////////////////////
/// Read Move Command
////////////////////////////////////////////////////////////
RPG::MoveCommand LMU_Reader::ReadMoveCommand(FILE* stream) {
    RPG::MoveCommand movecommand;

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
        case ChunkMoveCommand::switch_on:
            movecommand.parameter_a = Reader::CInteger(stream);
        case ChunkMoveCommand::switch_off:
            movecommand.parameter_a = Reader::CInteger(stream);
        case ChunkMoveCommand::change_graphic:
            movecommand.parameter_string = Reader::String(stream, Reader::CInteger(stream));
            movecommand.parameter_a = Reader::CInteger(stream);
        case ChunkMoveCommand::play_sound:
            movecommand.parameter_string = Reader::String(stream, Reader::CInteger(stream));
            movecommand.parameter_a = Reader::CInteger(stream);
            movecommand.parameter_b = Reader::CInteger(stream);
            movecommand.parameter_c = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return movecommand;
}
