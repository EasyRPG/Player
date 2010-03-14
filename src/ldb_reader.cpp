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
#include "output.h"

////////////////////////////////////////////////////////////
/// Load Database
////////////////////////////////////////////////////////////
void LDB_Reader::Load(const std::string& filename) {
    FILE* stream;
    stream = fopen(filename.c_str(), "rb");
    if (!stream) {
        Output::Error("Couldn't find %s database file.\n", filename);
    }
    std::string header = Reader::String(stream, Reader::CInteger(stream));
    if (header != "LcfDataBase") {
        Output::Error("%s is not a valid RPG2000 database.\n", filename);
    }
    LoadChunks(stream);
    fclose(stream);
}

////////////////////////////////////////////////////////////
/// Load data chunks
////////////////////////////////////////////////////////////
void LDB_Reader::LoadChunks(FILE* stream) {
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
        case ChunkData::Actor:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_actors.push_back(ReadActor(stream));
            }
            break;
        case ChunkData::Skill:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_skills.push_back(ReadSkill(stream));
            }
            break;
        case ChunkData::Item:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_items.push_back(ReadItem(stream));
            }
            break;
        case ChunkData::Enemy:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_enemies.push_back(ReadEnemy(stream));
            }
            break;
        case ChunkData::Troop:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_troops.push_back(ReadTroop(stream));
            }
            break;
        case ChunkData::Terrain:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_terrains.push_back(ReadTerrain(stream));
            }
            break;
        case ChunkData::Attribute:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_attributes.push_back(ReadAttribute(stream));
            }
            break;
        case ChunkData::State:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_states.push_back(ReadState(stream));
            }
            break;
        case ChunkData::Animation:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_animations.push_back(ReadAnimation(stream));
            }
            break;
        case ChunkData::Chipset:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_chipsets.push_back(ReadChipset(stream));
            }
            break;
        case ChunkData::Terms:
            Main_Data::data_terms = ReadTerms(stream);
            break;
        case ChunkData::System:
            Main_Data::data_system = ReadSystem(stream);
            break;
        case ChunkData::Switches:
            Main_Data::data_switches = ReadSwitches(stream);
            break;
        case ChunkData::Variables:
            Main_Data::data_variables = ReadVariables(stream);
            break;
        case ChunkData::CommonEvent:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_commonevents.push_back(ReadCommonEvent(stream));
            }
            break;
        case ChunkData::BattleCommand:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_battlecommands.push_back(ReadBattleCommand(stream));
            }
            break;
        case ChunkData::Class:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_classes.push_back(ReadClass(stream));
            }
            break;
        case ChunkData::BattlerAnimation:
           for (int i = Reader::CInteger(stream); i > 0; i--) {
                Main_Data::data_battleranimations.push_back(ReadBattlerAnimation(stream));
            }
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
}
