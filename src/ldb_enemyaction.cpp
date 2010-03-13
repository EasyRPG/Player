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
/// Read EnemyAction
////////////////////////////////////////////////////////////
RPG::EnemyAction LDB_Reader::ReadEnemyAction(FILE* stream) {
    RPG::EnemyAction enemyaction;
    Reader::CInteger(stream);

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
        case ChunkEnemyAction::kind:
            enemyaction.kind = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::basic:
            enemyaction.basic = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::skill_id:
            enemyaction.skill_id = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::enemy_id:
            enemyaction.enemy_id = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::condition_type:
            enemyaction.condition_type = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::condition_param1:
            enemyaction.condition_param1 = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::condition_param2:
            enemyaction.condition_param2 = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::switch_id:
            enemyaction.switch_id = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::switch_on:
            enemyaction.switch_on = Reader::Flag(stream);
            break;
        case ChunkEnemyAction::switch_on_id:
            enemyaction.switch_on_id = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::switch_off:
            enemyaction.switch_off = Reader::Flag(stream);
            break;
        case ChunkEnemyAction::switch_off_id:
            enemyaction.switch_off_id = Reader::CInteger(stream);
            break;
        case ChunkEnemyAction::rating:
            enemyaction.rating = Reader::CInteger(stream);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return enemyaction;
}
