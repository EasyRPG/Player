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
/// Read Class
////////////////////////////////////////////////////////////
RPG::Class LDB_Reader::ReadClass(FILE* stream) {
    RPG::Class _class;
    _class.ID = Reader::CInteger(stream);

    Reader::Chunk chunk_info;
    do {
        chunk_info.ID = Reader::CInteger(stream);
        if (chunk_info.ID == ChunkData::END) {
            break;
        }
        else {
            chunk_info.length = Reader::CInteger(stream);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkData::END:
            break;
        case ChunkClass::name:
            _class.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkClass::two_swords_style:
            _class.name = Reader::Flag(stream);
            break;
        case ChunkClass::fix_equipment:
            _class.name = Reader::Flag(stream);
            break;
        case ChunkClass::auto_battle:
            _class.name = Reader::Flag(stream);
            break;
        case ChunkClass::super_guard:
            _class.name = Reader::Flag(stream);
            break;
        case ChunkClass::parameters:
            _class.parameter_maxhp = Reader::ArrayShort(stream, chunk_info.length / 6);
            _class.parameter_maxsp = Reader::ArrayShort(stream, chunk_info.length / 6);
            _class.parameter_attack = Reader::ArrayShort(stream, chunk_info.length / 6);
            _class.parameter_defense = Reader::ArrayShort(stream, chunk_info.length / 6);
            _class.parameter_spirit = Reader::ArrayShort(stream, chunk_info.length / 6);
            _class.parameter_agility = Reader::ArrayShort(stream, chunk_info.length / 6);
            break;
        case ChunkClass::exp_base:
            _class.name = Reader::CInteger(stream);
            break;
        case ChunkClass::exp_inflation:
            _class.name = Reader::CInteger(stream);
            break;
        case ChunkClass::exp_correction:
            _class.name = Reader::CInteger(stream);
            break;
        case ChunkClass::unarmed_animation:
            _class.name = Reader::CInteger(stream);
            break;
        case ChunkClass::skills:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                _class.skills.push_back(ReadLearning(stream));
            }
            break;
        case ChunkClass::state_ranks_size:
            Reader::CInteger(stream);
            break;
        case ChunkClass::state_ranks:
            _class.state_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkClass::attribute_ranks_size:
            Reader::CInteger(stream);
            break;
        case ChunkClass::attribute_ranks:
            _class.attribute_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkClass::battle_commands:
            _class.battle_commands = Reader::ArrayUint32(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return _class;
}
