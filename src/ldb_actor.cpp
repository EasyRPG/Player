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
/// Read Actor
////////////////////////////////////////////////////////////
RPG::Actor LDB_Reader::ReadActor(Reader& stream) {
	RPG::Actor actor;

	actor.ID = stream.Read32(Reader::CompressedInteger);

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
		case ChunkActor::name:
			actor.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkActor::title:
			actor.title = stream.ReadString(chunk_info.length);
			break;
		case ChunkActor::character_name:
			actor.character_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkActor::character_index:
			actor.character_index = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::transparent:
			actor.transparent = stream.ReadBool();
			break;
		case ChunkActor::initial_level:
			actor.initial_level = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::final_level:
			actor.final_level = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::critical_hit:
			actor.critical_hit = stream.ReadBool();
			break;
		case ChunkActor::critical_hit_chance:
			actor.critical_hit_chance = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::face_name:
			actor.face_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkActor::face_index:
			actor.face_index = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::two_swords_style:
			actor.two_swords_style = stream.ReadBool();
			break;
		case ChunkActor::fix_equipment:
			actor.fix_equipment = stream.ReadBool();
			break;
		case ChunkActor::auto_battle:
			actor.auto_battle = stream.ReadBool();
			break;
		case ChunkActor::super_guard:
			actor.super_guard = stream.ReadBool();
			break;
		case ChunkActor::parameters:
			stream.Read16(actor.parameter_maxhp, chunk_info.length / 6);
			stream.Read16(actor.parameter_maxsp, chunk_info.length / 6);
			stream.Read16(actor.parameter_attack, chunk_info.length / 6);
			stream.Read16(actor.parameter_defense, chunk_info.length / 6);
			stream.Read16(actor.parameter_spirit, chunk_info.length / 6);
			stream.Read16(actor.parameter_agility, chunk_info.length / 6);
			break;
		case ChunkActor::exp_base:
			actor.exp_base = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::exp_inflation:
			actor.exp_inflation = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::exp_correction:
			actor.exp_correction = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::initial_equipment:
			actor.weapon_id = stream.Read16();
			actor.shield_id = stream.Read16();
			actor.armor_id = stream.Read16();
			actor.helmet_id = stream.Read16();
			actor.accessory_id = stream.Read16();
			break;
		case ChunkActor::unarmed_animation:
			actor.unarmed_animation = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::class_id:
			actor.class_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::battler_animation:
			actor.battler_animation = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkActor::skills:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				actor.skills.push_back(ReadLearning(stream));
			}
			break;
		case ChunkActor::rename_skill:
			actor.rename_skill = stream.ReadBool();
			break;
		case ChunkActor::skill_name:
			actor.skill_name = stream.ReadString(chunk_info.length);
			break;
		/*case ChunkActor::state_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;*/
		case ChunkActor::state_ranks:
			stream.Read8(actor.state_ranks, chunk_info.length);
			break;
		/*case ChunkActor::attribute_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;*/
		case ChunkActor::attribute_ranks:
			stream.Read8(actor.attribute_ranks, chunk_info.length);
			break;
		case ChunkActor::battle_commands:
			stream.Read32(actor.battle_commands, chunk_info.length);
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return actor;
}
