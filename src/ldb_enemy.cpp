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
/// Read Enemy
////////////////////////////////////////////////////////////
RPG::Enemy LDB_Reader::ReadEnemy(FILE* stream) {
    RPG::Enemy enemy;
    enemy.ID = Reader::CInteger(stream);

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
        case ChunkEnemy::name:
            enemy.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkEnemy::battler_name:
            enemy.battler_name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkEnemy::battler_hue:
            enemy.battler_hue = Reader::CInteger(stream);
            break;
        case ChunkEnemy::max_hp:
            enemy.max_hp = Reader::CInteger(stream);
            break;
        case ChunkEnemy::max_sp:
            enemy.max_sp = Reader::CInteger(stream);
            break;
        case ChunkEnemy::attack:
            enemy.attack = Reader::CInteger(stream);
            break;
        case ChunkEnemy::defense:
            enemy.defense = Reader::CInteger(stream);
            break;
        case ChunkEnemy::spirit:
            enemy.spirit = Reader::CInteger(stream);
            break;
        case ChunkEnemy::agility:
            enemy.agility = Reader::CInteger(stream);
            break;
        case ChunkEnemy::transparent:
            enemy.transparent = Reader::Flag(stream);
            break;
        case ChunkEnemy::exp:
            enemy.exp = Reader::CInteger(stream);
            break;
        case ChunkEnemy::gold:
            enemy.gold = Reader::CInteger(stream);
            break;
        case ChunkEnemy::drop_id:
            enemy.drop_id = Reader::CInteger(stream);
            break;
        case ChunkEnemy::drop_prob:
            enemy.drop_prob = Reader::CInteger(stream);
            break;
        case ChunkEnemy::critical_hit:
            enemy.critical_hit = Reader::Flag(stream);
            break;
        case ChunkEnemy::critical_hit_chance:
            enemy.critical_hit_chance = Reader::CInteger(stream);
            break;
        case ChunkEnemy::miss:
            enemy.miss = Reader::Flag(stream);
            break;
        case ChunkEnemy::levitate:
            enemy.levitate = Reader::Flag(stream);
            break;
        case ChunkEnemy::state_ranks_size:
            Reader::CInteger(stream);
            break;
        case ChunkEnemy::state_ranks:
            enemy.state_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkEnemy::attribute_ranks_size:
            Reader::CInteger(stream);
            break;
        case ChunkEnemy::attribute_ranks:
            enemy.attribute_ranks = Reader::ArrayUint8(stream, chunk_info.length);
            break;
        case ChunkEnemy::actions:
            for (int i = Reader::CInteger(stream); i > 0; i--) {
                enemy.actions.push_back(ReadEnemyAction(stream));
            }
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return enemy;
}
