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
/// Read State
////////////////////////////////////////////////////////////
RPG::State LDB_Reader::ReadState(FILE* stream) {
    RPG::State state;
    state.ID = Reader::CInteger(stream);

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
        case ChunkState::name:
            state.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::type:
            state.type = Reader::CInteger(stream);
            break;
        case ChunkState::color:
            state.color = Reader::CInteger(stream);
            break;
        case ChunkState::priority:
            state.priority = Reader::CInteger(stream);
            break;
        case ChunkState::restriction:
            state.restriction = Reader::CInteger(stream);
            break;
        case ChunkState::a_rate:
            state.a_rate = Reader::CInteger(stream);
            break;
        case ChunkState::b_rate:
            state.b_rate = Reader::CInteger(stream);
            break;
        case ChunkState::c_rate:
            state.c_rate = Reader::CInteger(stream);
            break;
        case ChunkState::d_rate:
            state.d_rate = Reader::CInteger(stream);
            break;
        case ChunkState::e_rate:
            state.e_rate = Reader::CInteger(stream);
            break;
        case ChunkState::hold_turn:
            state.hold_turn = Reader::CInteger(stream);
            break;
        case ChunkState::auto_release_prob:
            state.auto_release_prob = Reader::CInteger(stream);
            break;
        case ChunkState::release_by_damage:
            state.release_by_damage = Reader::CInteger(stream);
            break;
        case ChunkState::affect_type:
            state.affect_type = Reader::Flag(stream);
            break;
        case ChunkState::affect_attack:
            state.affect_attack = Reader::Flag(stream);
            break;
        case ChunkState::affect_defense:
            state.affect_defense = Reader::Flag(stream);
            break;
        case ChunkState::affect_spirit:
            state.affect_spirit = Reader::Flag(stream);
            break;
        case ChunkState::affect_agility:
            state.affect_agility = Reader::Flag(stream);
            break;
        case ChunkState::reduce_hit_ratio:
            state.reduce_hit_ratio = Reader::CInteger(stream);
            break;
        case ChunkState::avoid_attacks:
            state.avoid_attacks = Reader::Flag(stream);
            break;
        case ChunkState::reflect_magic:
            state.reflect_magic = Reader::Flag(stream);
            break;
        case ChunkState::cursed:
            state.cursed = Reader::Flag(stream);
            break;
        case ChunkState::battler_animation_id:
            state.battler_animation_id = Reader::CInteger(stream);
            break;
        case ChunkState::restrict_skill:
            state.restrict_skill = Reader::Flag(stream);
            break;
        case ChunkState::restrict_skill_level:
            state.restrict_skill_level = Reader::CInteger(stream);
            break;
        case ChunkState::restrict_magic:
            state.restrict_magic = Reader::Flag(stream);
            break;
        case ChunkState::restrict_magic_level:
            state.restrict_magic_level = Reader::CInteger(stream);
            break;
        case ChunkState::hp_change_type:
            state.hp_change_type = Reader::CInteger(stream);
            break;
        case ChunkState::sp_change_type:
            state.sp_change_type = Reader::CInteger(stream);
            break;
        case ChunkState::message_actor:
            state.message_actor = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::message_enemy:
            state.message_enemy = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::message_already:
            state.message_already = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::message_affected:
            state.message_affected = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::message_recovery:
            state.message_recovery = Reader::String(stream, chunk_info.length);
            break;
        case ChunkState::hp_change_max:
            state.hp_change_max = Reader::CInteger(stream);
            break;
        case ChunkState::hp_change_val:
            state.hp_change_val = Reader::CInteger(stream);
            break;
        case ChunkState::hp_change_map_val:
            state.hp_change_map_val = Reader::CInteger(stream);
            break;
        case ChunkState::hp_change_map_steps:
            state.hp_change_map_steps = Reader::CInteger(stream);
            break;
        case ChunkState::sp_change_max:
            state.sp_change_max = Reader::CInteger(stream);
            break;
        case ChunkState::sp_change_val:
            state.sp_change_val = Reader::CInteger(stream);
            break;
        case ChunkState::sp_change_map_val:
            state.sp_change_map_val = Reader::CInteger(stream);
            break;
        case ChunkState::sp_change_map_steps:
            state.sp_change_map_steps = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return state;
}
