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
/// Read Item
////////////////////////////////////////////////////////////
RPG::Item LDB_Reader::ReadItem(FILE* stream) {
    RPG::Item item;
    item.ID = Reader::CInteger(stream);

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
        case ChunkItem::name:
            item.name = Reader::String(stream, chunk_info.length);
            break;
        case ChunkItem::description:
            item.description = Reader::String(stream, chunk_info.length);
            break;
        case ChunkItem::type:
            item.type = Reader::CInteger(stream);
            break;
        case ChunkItem::price:
            item.price = Reader::CInteger(stream);
            break;
        case ChunkItem::uses:
            item.uses = Reader::CInteger(stream);
            break;
        case ChunkItem::atk_points1:
            item.atk_points = Reader::CInteger(stream);
            break;
        case ChunkItem::def_points1:
            item.def_points = Reader::CInteger(stream);
            break;
        case ChunkItem::spi_points1:
            item.spi_points = Reader::CInteger(stream);
            break;
        case ChunkItem::agi_points1:
            item.agi_points = Reader::CInteger(stream);
            break;
        case ChunkItem::two_handed:
            item.two_handed = Reader::Flag(stream);
            break;
        case ChunkItem::sp_cost:
            item.sp_cost = Reader::CInteger(stream);
            break;
        case ChunkItem::hit:
            item.hit = Reader::CInteger(stream);
            break;
        case ChunkItem::critical_hit:
            item.critical_hit = Reader::CInteger(stream);
            break;
        case ChunkItem::animation_id:
            item.animation_id = Reader::CInteger(stream);
            break;
        case ChunkItem::preemptive:
            item.preemptive = Reader::Flag(stream);
            break;
        case ChunkItem::dual_attack:
            item.dual_attack = Reader::Flag(stream);
            break;
        case ChunkItem::attack_all:
            item.attack_all = Reader::Flag(stream);
            break;
        case ChunkItem::ignore_evasion:
            item.ignore_evasion = Reader::Flag(stream);
            break;
        case ChunkItem::prevent_critical:
            item.prevent_critical = Reader::Flag(stream);
            break;
        case ChunkItem::raise_evasion:
            item.raise_evasion = Reader::Flag(stream);
            break;
        case ChunkItem::half_sp_cost:
            item.half_sp_cost = Reader::Flag(stream);
            break;
        case ChunkItem::no_terrain_damage:
            item.no_terrain_damage = Reader::Flag(stream);
            break;
        case ChunkItem::cursed:
            item.cursed = Reader::Flag(stream);
            break;
        case ChunkItem::entire_party:
            item.entire_party = Reader::Flag(stream);
            break;
        case ChunkItem::recover_hp:
            item.recover_hp = Reader::CInteger(stream);
            break;
        case ChunkItem::recover_hp_rate:
            item.recover_hp_rate = Reader::CInteger(stream);
            break;
        case ChunkItem::recover_sp:
            item.recover_sp = Reader::CInteger(stream);
            break;
        case ChunkItem::recover_sp_rate:
            item.recover_sp_rate = Reader::CInteger(stream);
            break;
        case ChunkItem::ocassion_field1:
            item.ocassion_field = Reader::Flag(stream);
            break;
        case ChunkItem::ko_only:
            item.ko_only = Reader::Flag(stream);
            break;
        case ChunkItem::max_hp_points:
            item.max_hp_points = Reader::CInteger(stream);
            break;
        case ChunkItem::max_sp_points:
            item.max_sp_points = Reader::CInteger(stream);
            break;
        case ChunkItem::atk_points2:
            item.atk_points = Reader::CInteger(stream);
            break;
        case ChunkItem::def_points2:
            item.def_points = Reader::CInteger(stream);
            break;
        case ChunkItem::spi_points2:
            item.spi_points = Reader::CInteger(stream);
            break;
        case ChunkItem::agi_points2:
            item.agi_points = Reader::CInteger(stream);
            break;
        case ChunkItem::using_messsage:
            item.using_messsage = Reader::CInteger(stream);
            break;
        case ChunkItem::skill_id:
            item.skill_id = Reader::CInteger(stream);
            break;
        case ChunkItem::switch_id:
            item.switch_id = Reader::CInteger(stream);
            break;
        case ChunkItem::ocassion_field2:
            item.ocassion_field = Reader::Flag(stream);
            break;
        case ChunkItem::ocassion_battle:
            item.ocassion_battle = Reader::Flag(stream);
            break;
        case ChunkItem::actor_set_size:
            Reader::CInteger(stream);
            break;
        case ChunkItem::actor_set:
            item.actor_set = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        case ChunkItem::state_set_size:
            Reader::CInteger(stream);
            break;
        case ChunkItem::state_set:
            item.state_set = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        case ChunkItem::attribute_set_size:
            Reader::CInteger(stream);
            break;
        case ChunkItem::attribute_set:
            item.attribute_set = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        case ChunkItem::state_chance:
            item.state_chance = Reader::Flag(stream);
            break;
        case ChunkItem::weapon_animation:
            item.weapon_animation = Reader::CInteger(stream);
            break;
        case ChunkItem::use_skill:
            item.use_skill = Reader::Flag(stream);
            break;
        case ChunkItem::class_set_size:
            Reader::CInteger(stream);
            break;
        case ChunkItem::class_set:
            item.class_set = Reader::ArrayFlag(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    }
    return item;
}
