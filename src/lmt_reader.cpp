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
#include "reader.h"
#include "output.h"

////////////////////////////////////////////////////////////
/// Load Map Tree
////////////////////////////////////////////////////////////
void LMT_Reader::Load(const std::string& filename) {
    FILE* stream;
    stream = fopen(filename.c_str(), "rb");
    if (!stream) {
        Output::Error("Couldn't find %s map tree file.\n", filename.c_str());
    }
    std::string header = Reader::String(stream, Reader::CInteger(stream));
    if (header != "LcfMapTree") {
        Output::Error("%s is not a valid RPG2000 map tree.\n", filename.c_str());
    }
    ReadTreeMap(stream);
    fclose(stream);
}

////////////////////////////////////////////////////////////
/// Read Tree Map
////////////////////////////////////////////////////////////
void LMT_Reader::ReadTreeMap(FILE* stream) {
    // Array - RPG::MapInfo
    Main_Data::data_treemap.maps.resize(Reader::CInteger(stream) + 1);
    for (unsigned int i = 1; i < Main_Data::data_treemap.maps.size(); i++) {
        Main_Data::data_treemap.maps[i] = ReadMapInfo(stream);
    }

    // Array - Integer
    for (int i = Reader::CInteger(stream); i > 0; i--) {
        Main_Data::data_treemap.tree_order.push_back(Reader::CInteger(stream));
    }

    // Integer
    Main_Data::data_treemap.active_node = Reader::CInteger(stream);

    // RPG::TreeMap
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
        case ChunkTreeMap::start_map_id:
            Main_Data::data_treemap.start_map_id = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::start_x:
            Main_Data::data_treemap.start_x = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::start_y:
            Main_Data::data_treemap.start_y = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::boat_map_id:
            Main_Data::data_treemap.boat_map_id = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::boat_x:
            Main_Data::data_treemap.boat_x = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::boat_y:
            Main_Data::data_treemap.boat_y = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::ship_map_id:
            Main_Data::data_treemap.ship_map_id = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::ship_x:
            Main_Data::data_treemap.ship_x = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::ship_y:
            Main_Data::data_treemap.ship_y = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::airship_map_id:
            Main_Data::data_treemap.airship_map_id = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::airship_x:
            Main_Data::data_treemap.airship_x = Reader::CInteger(stream);
            break;
        case ChunkTreeMap::airship_y:
            Main_Data::data_treemap.airship_y = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
}
