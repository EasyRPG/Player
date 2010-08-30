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
#include "event_reader.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read CommonEvent
////////////////////////////////////////////////////////////
RPG::CommonEvent LDB_Reader::ReadCommonEvent(Reader& stream) {
    RPG::CommonEvent commonevent;
    commonevent.ID = stream.Read32(Reader::CompressedInteger);

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
        case ChunkCommonEvent::name:
            commonevent.name = stream.ReadString(chunk_info.length);
            break;
        case ChunkCommonEvent::trigger:
            commonevent.trigger = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkCommonEvent::switch_flag:
            commonevent.switch_flag = stream.ReadBool();
            break;
        case ChunkCommonEvent::switch_id:
            commonevent.switch_id = stream.Read32(Reader::CompressedInteger);
            break;
        /*case ChunkCommonEvent::event_commands_size:
            commonevent.event_commands.resize(stream.Read32(Reader::CompressedInteger));
            break;
        case ChunkCommonEvent::event_commands:
            for (unsigned int i = 0; i < stream.Read32(Reader::CompressedInteger); i++) {
                commonevent.event_commands[i] = Event_Reader::ReadEventCommand(stream);
            }
            break;*/
        default:
            stream.Seek(chunk_info.length, Reader::FromCurrent);
        }
    }
    return commonevent;
}
