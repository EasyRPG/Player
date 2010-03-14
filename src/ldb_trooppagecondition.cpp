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
/// Read TroopPageCondition
////////////////////////////////////////////////////////////
RPG::TroopPageCondition LDB_Reader::ReadTroopPageCondition(FILE* stream) {
    RPG::TroopPageCondition condition;
    unsigned char bitflag;

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
        case ChunkTroopPageCondition::condition_flags:
            bitflag = Reader::BitFlag(stream);
            condition.switch_a = (bitflag & 0x01) > 0;
            condition.switch_b = (bitflag & 0x02) > 0;
            condition.variable = (bitflag & 0x04) > 0;
            condition.turn = (bitflag & 0x08) > 0;
            condition.fatigue = (bitflag & 0x10) > 0;
            condition.enemy_hp = (bitflag & 0x20) > 0;
            condition.actor_hp = (bitflag & 0x40) > 0;
            if (chunk_info.length == 2) {
                bitflag = Reader::BitFlag(stream);
                condition.turn_enemy = (bitflag & 0x01) > 0;
                condition.turn_actor = (bitflag & 0x02) > 0;
                condition.command_actor = (bitflag & 0x03) > 0;
            }
            break;
        case ChunkTroopPageCondition::switch_a_id:
            condition.switch_a_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::switch_b_id:
            condition.switch_b_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::variable_id:
            condition.variable_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::variable_value:
            condition.variable_value = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_a:
            condition.turn_a = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_b:
            condition.turn_b = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::fatigue_min:
            condition.fatigue_min = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::fatigue_max:
            condition.fatigue_max = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::enemy_id:
            condition.enemy_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::enemy_hp_min:
            condition.enemy_hp_min = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::enemy_hp_max:
            condition.enemy_hp_max = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::actor_id:
            condition.actor_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::actor_hp_min:
            condition.actor_hp_min = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::actor_hp_max:
            condition.actor_hp_max = Reader::CInteger(stream);
            break;
        /*case ChunkTroopPageCondition::turn_enemy_id:
            condition.turn_enemy_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_enemy_a:
            condition.turn_enemy_a = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_enemy_b:
            condition.turn_enemy_b = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_actor_id:
            condition.turn_actor_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_actor_a:
            condition.turn_actor_a = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::turn_actor_b:
            condition.turn_actor_b = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::command_actor_id:
            condition.command_actor_id = Reader::CInteger(stream);
            break;
        case ChunkTroopPageCondition::command_id:
            condition.command_id = Reader::CInteger(stream);
            break;*/
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return condition;
}
