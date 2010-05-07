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
#include "event_reader.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read Event Page
////////////////////////////////////////////////////////////
RPG::EventPage LMU_Reader::ReadEventPage(FILE* stream) {
    RPG::EventPage eventpage;
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
        case ChunkEventPage::condition:
            eventpage.condition = ReadEventPageCondition(stream);
            break;
        case ChunkEventPage::character_name:
            eventpage.character_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkEventPage::tile_id:
            eventpage.tile_id = Reader::CInteger(stream);
            break;
        case ChunkEventPage::character_dir:
            eventpage.character_dir = Reader::CInteger(stream);
            break;
        case ChunkEventPage::character_pattern:
            eventpage.character_pattern = Reader::CInteger(stream);
            break;
        case ChunkEventPage::translucent:
            eventpage.translucent = Reader::Flag(stream);
            break;
        case ChunkEventPage::move_type:
            eventpage.move_type = Reader::CInteger(stream);
            break;
        case ChunkEventPage::move_frequency:
            eventpage.move_frequency = Reader::CInteger(stream);
            break;
        case ChunkEventPage::trigger:
            eventpage.trigger = Reader::CInteger(stream);
            break;
        case ChunkEventPage::priority_type:
            eventpage.priority_type = Reader::CInteger(stream);
            break;
        case ChunkEventPage::overlap:
            eventpage.overlap = Reader::Flag(stream);
            break;
        case ChunkEventPage::animation_type:
            eventpage.animation_type = Reader::CInteger(stream);
            break;
        case ChunkEventPage::move_speed:
            eventpage.move_speed = Reader::CInteger(stream);
            break;
        case ChunkEventPage::move_route:
            eventpage.move_route = ReadMoveRoute(stream);
            break;
        case ChunkEventPage::event_commands:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                eventpage.event_commands.push_back(Event_Reader::ReadEventCommand(stream));
            }
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return eventpage;
}

