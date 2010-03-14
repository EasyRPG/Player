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
#include "lmt_reader.h"
#include "lmt_chunks.h"
#include "ldb_reader.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read MapInfo
////////////////////////////////////////////////////////////
RPG::MapInfo LMT_Reader::ReadMapInfo(FILE* stream) {
    RPG::MapInfo mapinfo;
    mapinfo.ID = Reader::CInteger(stream);

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
        case ChunkMapInfo::name:
            mapinfo.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkMapInfo::parent_map:
            mapinfo.parent_map = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::type:
            mapinfo.type = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::music_type:
            mapinfo.music_type = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::music_name:
            mapinfo.music = LDB_Reader::ReadMusic(stream);
            break;
        case ChunkMapInfo::background_type:
            mapinfo.background_type = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::background_name:
            mapinfo.background_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkMapInfo::teleport:
            mapinfo.teleport = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::escape:
            mapinfo.escape = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::save:
            mapinfo.save = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::encounters:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                mapinfo.encounters.push_back(ReadEncounter(stream));
            }
            break;
        case ChunkMapInfo::encounter_steps:
            mapinfo.encounter_steps = Reader::CInteger(stream);
            break;
        case ChunkMapInfo::area_rect:
            mapinfo.area_rect.x = Reader::Uint32(stream);
            mapinfo.area_rect.y = Reader::Uint32(stream);
            mapinfo.area_rect.width = Reader::Uint32(stream) - mapinfo.area_rect.x;
            mapinfo.area_rect.height = Reader::Uint32(stream) - mapinfo.area_rect.y;
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return mapinfo;
}
