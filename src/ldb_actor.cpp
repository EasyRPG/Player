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
/// Read Actor
////////////////////////////////////////////////////////////
RPG::Actor LDB_Reader::ReadActor(FILE* stream) {
    RPG::Actor actor;
    actor.ID = Reader::CInteger(stream);

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
        case ChunkActor::name:
            actor.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkActor::title:
            actor.title = Reader::String(stream, chunk_info.length);
            break;
        case ChunkActor::character_name:
            actor.character_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkActor::character_index:
            actor.character_index = Reader::CInteger(stream);
            break;
        case ChunkActor::transparent:
            actor.transparent = Reader::Flag(stream);
            break;
        case ChunkActor::initial_level:
            actor.initial_level = Reader::CInteger(stream);
            break;
        case ChunkActor::final_level:
            actor.final_level = Reader::CInteger(stream);
            break;
        case ChunkActor::critical_hit:
            actor.critical_hit = Reader::Flag(stream);
            break;
        case ChunkActor::critical_hit_chance:
            actor.critical_hit_chance = Reader::CInteger(stream);
            break;
        case ChunkActor::face_name:
            actor.face_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkActor::face_index:
            actor.face_index = Reader::CInteger(stream);
            break;
        case ChunkActor::two_swords_style:
            actor.two_swords_style = Reader::Flag(stream);
            break;
        case ChunkActor::fix_equipment:
            actor.fix_equipment = Reader::Flag(stream);
            break;
        case ChunkActor::auto_battle:
            actor.auto_battle = Reader::Flag(stream);
            break;
        case ChunkActor::super_guard:
            actor.super_guard = Reader::Flag(stream);
            break;
        case ChunkActor::parameters:
            actor.parameter_maxhp = Reader::ArrayShort(stream, chunk_info.length / 6);
            actor.parameter_maxsp = Reader::ArrayShort(stream, chunk_info.length / 6);
            actor.parameter_attack = Reader::ArrayShort(stream, chunk_info.length / 6);
            actor.parameter_defense = Reader::ArrayShort(stream, chunk_info.length / 6);
            actor.parameter_spirit = Reader::ArrayShort(stream, chunk_info.length / 6);
            actor.parameter_agility = Reader::ArrayShort(stream, chunk_info.length / 6);
            break;
        case ChunkActor::exp_base:
            actor.exp_base = Reader::CInteger(stream);
            break;
        case ChunkActor::exp_inflation:
            actor.exp_inflation = Reader::CInteger(stream);
            break;
        case ChunkActor::exp_correction:
            actor.exp_correction = Reader::CInteger(stream);
            break;
        case ChunkActor::initial_equipment:
            actor.weapon_id = Reader::Short(stream);
            actor.shield_id = Reader::Short(stream);
            actor.armor_id = Reader::Short(stream);
            actor.helmet_id = Reader::Short(stream);
            actor.accessory_id = Reader::Short(stream);
            break;
        case ChunkActor::unarmed_animation:
            actor.unarmed_animation = Reader::CInteger(stream);
            break;
        case ChunkActor::class_id:
            actor.class_id = Reader::CInteger(stream);
            break;
        case ChunkActor::battler_animation:
            actor.battler_animation = Reader::CInteger(stream);
            break;
        case ChunkActor::skills:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                actor.skills.push_back(ReadLearning(stream));
            }
            break;
        case ChunkActor::rename_skill:
            actor.rename_skill = Reader::Flag(stream);
            break;
        case ChunkActor::skill_name:
            actor.skill_name = Reader::String(stream, chunk_info.length);
            break;
        /*case ChunkActor::state_ranks_size:
            Reader::CInteger(stream);
            break;*/
        case ChunkActor::state_ranks:
            actor.state_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        /*case ChunkActor::attribute_ranks_size:
            Reader::CInteger(stream);
            break;*/
        case ChunkActor::attribute_ranks:
            actor.attribute_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkActor::battle_commands:
            actor.battle_commands = Reader::ArrayUint32(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return actor;
}
