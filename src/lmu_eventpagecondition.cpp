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
/// Read Event Page Condition
////////////////////////////////////////////////////////////
RPG::EventPageCondition LMU_Reader::ReadEventPageCondition(Reader& stream) {
    RPG::EventPageCondition eventpagecondition;
    unsigned char bitflag;

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
        case ChunkEventPageCondition::condition_flags:
            bitflag = stream.Read8();
            eventpagecondition.switch_a = (bitflag & 0x01) > 0;
            eventpagecondition.switch_b = (bitflag & 0x02) > 0;
            eventpagecondition.variable = (bitflag & 0x04) > 0;
            eventpagecondition.item = (bitflag & 0x08) > 0;
            eventpagecondition.actor = (bitflag & 0x10) > 0;
            eventpagecondition.timer = (bitflag & 0x20) > 0;
            break;
        case ChunkEventPageCondition::switch_a_id:
            eventpagecondition.switch_a_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::switch_b_id:
            eventpagecondition.switch_b_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::variable_id:
            eventpagecondition.variable_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::variable_value:
            eventpagecondition.variable_value = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::item_id:
            eventpagecondition.item_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::actor_id:
            eventpagecondition.actor_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkEventPageCondition::timer_sec:
            eventpagecondition.timer_sec = stream.Read32(Reader::CompressedInteger);
            break;
        default:
            stream.Seek(chunk_info.length, Reader::FromCurrent);
        }
    }
    return eventpagecondition;
}
