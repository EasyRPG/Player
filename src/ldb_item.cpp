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
RPG::Item LDB_Reader::ReadItem(Reader& stream) {
    RPG::Item item;
    item.ID = stream.Read32(Reader::CompressedInteger);

    Reader::Chunk chunk_info;
    while (!stream.Eof()) {
        chunk_info.ID = stream.Read32(Reader::CompressedInteger);
        if (chunk_info.ID == ChunkData::END) {
            break;
        }
        else {
            chunk_info.length = stream.Read32(Reader::CompressedInteger);
            if (chunk_info.length == 0) continue;
        }
        switch (chunk_info.ID) {
        case ChunkItem::name:
            item.name = stream.ReadString(chunk_info.length);
            break;
        case ChunkItem::description:
            item.description = stream.ReadString(chunk_info.length);
            break;
        case ChunkItem::type:
            item.type = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::price:
            item.price = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::uses:
            item.uses = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::atk_points1:
            item.atk_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::def_points1:
            item.def_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::spi_points1:
            item.spi_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::agi_points1:
            item.agi_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::two_handed:
            item.two_handed = stream.ReadBool();
            break;
        case ChunkItem::sp_cost:
            item.sp_cost = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::hit:
            item.hit = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::critical_hit:
            item.critical_hit = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::animation_id:
            item.animation_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::preemptive:
            item.preemptive = stream.ReadBool();
            break;
        case ChunkItem::dual_attack:
            item.dual_attack = stream.ReadBool();
            break;
        case ChunkItem::attack_all:
            item.attack_all = stream.ReadBool();
            break;
        case ChunkItem::ignore_evasion:
            item.ignore_evasion = stream.ReadBool();
            break;
        case ChunkItem::prevent_critical:
            item.prevent_critical = stream.ReadBool();
            break;
        case ChunkItem::raise_evasion:
            item.raise_evasion = stream.ReadBool();
            break;
        case ChunkItem::half_sp_cost:
            item.half_sp_cost = stream.ReadBool();
            break;
        case ChunkItem::no_terrain_damage:
            item.no_terrain_damage = stream.ReadBool();
            break;
        case ChunkItem::cursed:
            item.cursed = stream.ReadBool();
            break;
        case ChunkItem::entire_party:
            item.entire_party = stream.ReadBool();
            break;
        case ChunkItem::recover_hp:
            item.recover_hp = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::recover_hp_rate:
            item.recover_hp_rate = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::recover_sp:
            item.recover_sp = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::recover_sp_rate:
            item.recover_sp_rate = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::ocassion_field1:
            item.ocassion_field = stream.ReadBool();
            break;
        case ChunkItem::ko_only:
            item.ko_only = stream.ReadBool();
            break;
        case ChunkItem::max_hp_points:
            item.max_hp_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::max_sp_points:
            item.max_sp_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::atk_points2:
            item.atk_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::def_points2:
            item.def_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::spi_points2:
            item.spi_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::agi_points2:
            item.agi_points = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::using_messsage:
            item.using_messsage = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::skill_id:
            item.skill_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::switch_id:
            item.switch_id = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::ocassion_field2:
            item.ocassion_field = stream.ReadBool();
            break;
        case ChunkItem::ocassion_battle:
            item.ocassion_battle = stream.ReadBool();
            break;
        case ChunkItem::actor_set_size:
            stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::actor_set:
            stream.ReadBool(item.actor_set, chunk_info.length);
            break;
        case ChunkItem::state_set_size:
            stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::state_set:
            stream.ReadBool(item.state_set, chunk_info.length);
            break;
        case ChunkItem::attribute_set_size:
            stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::attribute_set:
            stream.ReadBool(item.attribute_set, chunk_info.length);
            break;
        case ChunkItem::state_chance:
            item.state_chance = stream.ReadBool();
            break;
        case ChunkItem::weapon_animation:
            item.weapon_animation = stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::use_skill:
            item.use_skill = stream.ReadBool();
            break;
        case ChunkItem::class_set_size:
            stream.Read32(Reader::CompressedInteger);
            break;
        case ChunkItem::class_set:
            stream.ReadBool(item.class_set, chunk_info.length);
            break;
        default:
            stream.Seek(chunk_info.length, Reader::FromCurrent);
        }
    }
    return item;
}
