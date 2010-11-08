/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ldb_reader.h"
#include "ldb_chunks.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read State
////////////////////////////////////////////////////////////
RPG::State LDB_Reader::ReadState(Reader& stream) {
	RPG::State state;
	state.ID = stream.Read32(Reader::CompressedInteger);

	Reader::Chunk chunk_info;
	while (!stream.Eof()) {
		chunk_info.ID = stream.Read32(Reader::CompressedInteger);
		if (chunk_info.ID == ChunkData::END) {
			break;
		} else {
			chunk_info.length = stream.Read32(Reader::CompressedInteger);
			if (chunk_info.length == 0) continue;
		}
		switch (chunk_info.ID) {
		case ChunkState::name:
			state.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::type:
			state.type = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::color:
			state.color = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::priority:
			state.priority = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::restriction:
			state.restriction = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::a_rate:
			state.a_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::b_rate:
			state.b_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::c_rate:
			state.c_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::d_rate:
			state.d_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::e_rate:
			state.e_rate = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::hold_turn:
			state.hold_turn = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::auto_release_prob:
			state.auto_release_prob = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::release_by_damage:
			state.release_by_damage = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::affect_type:
			state.affect_type = stream.ReadBool();
			break;
		case ChunkState::affect_attack:
			state.affect_attack = stream.ReadBool();
			break;
		case ChunkState::affect_defense:
			state.affect_defense = stream.ReadBool();
			break;
		case ChunkState::affect_spirit:
			state.affect_spirit = stream.ReadBool();
			break;
		case ChunkState::affect_agility:
			state.affect_agility = stream.ReadBool();
			break;
		case ChunkState::reduce_hit_ratio:
			state.reduce_hit_ratio = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::avoid_attacks:
			state.avoid_attacks = stream.ReadBool();
			break;
		case ChunkState::reflect_magic:
			state.reflect_magic = stream.ReadBool();
			break;
		case ChunkState::cursed:
			state.cursed = stream.ReadBool();
			break;
		case ChunkState::battler_animation_id:
			state.battler_animation_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::restrict_skill:
			state.restrict_skill = stream.ReadBool();
			break;
		case ChunkState::restrict_skill_level:
			state.restrict_skill_level = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::restrict_magic:
			state.restrict_magic = stream.ReadBool();
			break;
		case ChunkState::restrict_magic_level:
			state.restrict_magic_level = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::hp_change_type:
			state.hp_change_type = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::sp_change_type:
			state.sp_change_type = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::message_actor:
			state.message_actor = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::message_enemy:
			state.message_enemy = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::message_already:
			state.message_already = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::message_affected:
			state.message_affected = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::message_recovery:
			state.message_recovery = stream.ReadString(chunk_info.length);
			break;
		case ChunkState::hp_change_max:
			state.hp_change_max = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::hp_change_val:
			state.hp_change_val = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::hp_change_map_val:
			state.hp_change_map_val = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::hp_change_map_steps:
			state.hp_change_map_steps = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::sp_change_max:
			state.sp_change_max = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::sp_change_val:
			state.sp_change_val = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::sp_change_map_val:
			state.sp_change_map_val = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkState::sp_change_map_steps:
			state.sp_change_map_steps = stream.Read32(Reader::CompressedInteger);
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return state;
}
