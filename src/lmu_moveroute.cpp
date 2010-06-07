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
/// Read Move Route
////////////////////////////////////////////////////////////
RPG::MoveRoute LMU_Reader::ReadMoveRoute(FILE* stream) {
    RPG::MoveRoute moveroute;
	Reader::CInteger(stream);

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
        case ChunkMoveRoute::move_commands:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                moveroute.move_commands.push_back(ReadMoveCommand(stream));
            }
            break;
        case ChunkMoveRoute::skippable:
            moveroute.skippable = Reader::Flag(stream);
            break;
        case ChunkMoveRoute::repeat:
            moveroute.repeat = Reader::Flag(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return moveroute;
}
