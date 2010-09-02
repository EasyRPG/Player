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
/// Read Enemy
////////////////////////////////////////////////////////////
RPG::Enemy LDB_Reader::ReadEnemy(Reader& stream) {
	RPG::Enemy enemy;
	enemy.ID = stream.Read32(Reader::CompressedInteger);

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
		case ChunkEnemy::name:
			enemy.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkEnemy::battler_name:
			enemy.battler_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkEnemy::battler_hue:
			enemy.battler_hue = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::max_hp:
			enemy.max_hp = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::max_sp:
			enemy.max_sp = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::attack:
			enemy.attack = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::defense:
			enemy.defense = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::spirit:
			enemy.spirit = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::agility:
			enemy.agility = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::transparent:
			enemy.transparent = stream.ReadBool();
			break;
		case ChunkEnemy::exp:
			enemy.exp = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::gold:
			enemy.gold = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::drop_id:
			enemy.drop_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::drop_prob:
			enemy.drop_prob = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::critical_hit:
			enemy.critical_hit = stream.ReadBool();
			break;
		case ChunkEnemy::critical_hit_chance:
			enemy.critical_hit_chance = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::miss:
			enemy.miss = stream.ReadBool();
			break;
		case ChunkEnemy::levitate:
			enemy.levitate = stream.ReadBool();
			break;
		case ChunkEnemy::state_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::state_ranks:
			stream.Read8(enemy.state_ranks, chunk_info.length);
			break;
		case ChunkEnemy::attribute_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkEnemy::attribute_ranks:
			stream.Read8(enemy.attribute_ranks, chunk_info.length);
			break;
		case ChunkEnemy::actions:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				enemy.actions.push_back(ReadEnemyAction(stream));
			}
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return enemy;
}
