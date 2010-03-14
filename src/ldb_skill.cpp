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
/// Read Skill
////////////////////////////////////////////////////////////
RPG::Skill LDB_Reader::ReadSkill(FILE* stream) {
    RPG::Skill skill;
    skill.ID = Reader::CInteger(stream);

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
        case ChunkSkill::name:
            skill.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkSkill::description:
            skill.description = Reader::String(stream, chunk_info.length);
            break;
        case ChunkSkill::using_message1:
            skill.using_message1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkSkill::using_message2:
            skill.using_message2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkSkill::failure_message:
            skill.failure_message = Reader::CInteger(stream);
            break;
        case ChunkSkill::type:
            skill.type = Reader::CInteger(stream);
            break;
        case ChunkSkill::sp_type:
            skill.sp_type = Reader::CInteger(stream);
            break;
        case ChunkSkill::sp_percent:
            skill.sp_percent = Reader::CInteger(stream);
            break;
        case ChunkSkill::sp_cost:
            skill.sp_cost = Reader::CInteger(stream);
            break;
        case ChunkSkill::scope:
            skill.scope = Reader::CInteger(stream);
            break;
        case ChunkSkill::switch_id:
            skill.switch_id = Reader::CInteger(stream);
            break;
        case ChunkSkill::animation_id:
            skill.animation_id = Reader::CInteger(stream);
            break;
        case ChunkSkill::sound_effect:
            skill.sound_effect = ReadSound(stream);
            break;
        case ChunkSkill::occasion_field:
            skill.occasion_field = Reader::Flag(stream);
            break;
        case ChunkSkill::occasion_battle:
            skill.occasion_battle = Reader::Flag(stream);
            break;
        case ChunkSkill::state_effect:
            skill.state_effect = Reader::Flag(stream);
            break;
        case ChunkSkill::pdef_f:
            skill.pdef_f = Reader::CInteger(stream);
            break;
        case ChunkSkill::mdef_f:
            skill.mdef_f = Reader::CInteger(stream);
            break;
        case ChunkSkill::variance:
            skill.variance = Reader::CInteger(stream);
            break;
        case ChunkSkill::power:
            skill.power = Reader::CInteger(stream);
            break;
        case ChunkSkill::hit:
            skill.hit = Reader::CInteger(stream);
            break;
        case ChunkSkill::affect_hp:
            skill.affect_hp = Reader::Flag(stream);
            break;
        case ChunkSkill::affect_sp:
            skill.affect_sp = Reader::Flag(stream);
            break;
        case ChunkSkill::affect_attack:
            skill.affect_attack = Reader::Flag(stream);
            break;
        case ChunkSkill::affect_defense:
            skill.affect_defense = Reader::Flag(stream);
            break;
        case ChunkSkill::affect_spirit:
            skill.affect_spirit = Reader::Flag(stream);
            break;
        case ChunkSkill::affect_agility:
            skill.affect_agility = Reader::Flag(stream);
            break;
        case ChunkSkill::absorb_damage:
            skill.absorb_damage = Reader::Flag(stream);
            break;
        case ChunkSkill::ignore_defense:
            skill.ignore_defense = Reader::Flag(stream);
            break;
        case ChunkSkill::state_size:
            Reader::CInteger(stream);
            break;
        case ChunkSkill::state_effects:
            skill.state_effects = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        case ChunkSkill::attribute_size:
            Reader::CInteger(stream);
            break;
        case ChunkSkill::attribute_effects:
            skill.attribute_effects = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        case ChunkSkill::affect_attr_defence:
            skill.affect_attr_defence = Reader::Flag(stream);
            break;
        case ChunkSkill::battler_animation:
            skill.battler_animation = Reader::CInteger(stream);
            break;
        case ChunkSkill::cba_data:
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return skill;
}
