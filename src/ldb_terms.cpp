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
/// Read Terms
////////////////////////////////////////////////////////////
RPG::Terms LDB_Reader::ReadTerms(Reader& stream) {
    RPG::Terms terms;

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
        case ChunkTerms::encounter:
            terms.encounter = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::special_combat:
            terms.special_combat = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::escape_success:
            terms.escape_success = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::escape_failure:
            terms.escape_failure = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::victory:
            terms.victory = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::defeat:
            terms.defeat = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::exp_received:
            terms.exp_received = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::gold_recieved_a:
            terms.gold_recieved_a = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::gold_recieved_b:
            terms.gold_recieved_b = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::item_recieved:
            terms.item_recieved = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::attacking:
            terms.attacking = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::actor_critical:
            terms.actor_critical = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_critical:
            terms.enemy_critical = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::defending:
            terms.defending = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::observing:
            terms.observing = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::focus:
            terms.focus = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::autodestruction:
            terms.autodestruction = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_escape:
            terms.enemy_escape = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_transform:
            terms.enemy_transform = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_damaged:
            terms.enemy_damaged = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_undamaged:
            terms.enemy_undamaged = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::actor_damaged:
            terms.actor_damaged = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::actor_undamaged:
            terms.actor_undamaged = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::skill_failure_a:
            terms.skill_failure_a = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::skill_failure_b:
            terms.skill_failure_b = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::skill_failure_c:
            terms.skill_failure_c = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::dodge:
            terms.dodge = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::use_item:
            terms.use_item = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::hp_recovery:
            terms.hp_recovery = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::parameter_increase:
            terms.parameter_increase = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::parameter_decrease:
            terms.parameter_decrease = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::actor_hp_absorbed:
            terms.actor_hp_absorbed = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::enemy_hp_absorbed:
            terms.enemy_hp_absorbed = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::resistance_increase:
            terms.resistance_increase = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::resistance_decrease:
            terms.resistance_decrease = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::level_up:
            terms.level_up = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::skill_learned:
            terms.skill_learned = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_greeting1:
            terms.shop_greeting1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting1:
            terms.shop_regreeting1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy1:
            terms.shop_buy1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell1:
            terms.shop_sell1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_leave1:
            terms.shop_leave1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select1:
            terms.shop_buy_select1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number1:
            terms.shop_buy_number1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_purchased1:
            terms.shop_purchased1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select1:
            terms.shop_sell_select1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number1:
            terms.shop_sell_number1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sold1:
            terms.shop_sold1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_greeting2:
            terms.shop_greeting2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting2:
            terms.shop_regreeting2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy2:
            terms.shop_buy2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell2:
            terms.shop_sell2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_leave2:
            terms.shop_leave2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select2:
            terms.shop_buy_select2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number2:
            terms.shop_buy_number2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_purchased2:
            terms.shop_purchased2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select2:
            terms.shop_sell_select2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number2:
            terms.shop_sell_number2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sold2:
            terms.shop_sold2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_greeting3:
            terms.shop_greeting3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting3:
            terms.shop_regreeting3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy3:
            terms.shop_buy3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell3:
            terms.shop_sell3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_leave3:
            terms.shop_leave3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select3:
            terms.shop_buy_select3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number3:
            terms.shop_buy_number3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_purchased3:
            terms.shop_purchased3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select3:
            terms.shop_sell_select3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number3:
            terms.shop_sell_number3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shop_sold3:
            terms.shop_sold3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_1:
            terms.inn_a_greeting_1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_2:
            terms.inn_a_greeting_2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_3:
            terms.inn_a_greeting_3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_a_accept:
            terms.inn_a_accept = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_a_cancel:
            terms.inn_a_cancel = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_1:
            terms.inn_b_greeting_1 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_2:
            terms.inn_b_greeting_2 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_3:
            terms.inn_b_greeting_3 = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_b_accept:
            terms.inn_b_accept = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::inn_b_cancel:
            terms.inn_b_cancel = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::possessed_items:
            terms.possessed_items = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::equipped_items:
            terms.equipped_items = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::gold:
            terms.gold = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::battle_fight:
            terms.battle_fight = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::battle_auto:
            terms.battle_auto = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::battle_escape:
            terms.battle_escape = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::command_attack:
            terms.command_attack = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::command_defend:
            terms.command_defend = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::command_item:
            terms.command_item = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::command_skill:
            terms.command_skill = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::menu_equipment:
            terms.menu_equipment = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::menu_save:
            terms.menu_save = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::menu_quit:
            terms.menu_quit = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::new_game:
            terms.new_game = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::load_game:
            terms.load_game = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::exit_game:
            terms.exit_game = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::level:
            terms.level = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::health_points:
            terms.health_points = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::spirit_points:
            terms.spirit_points = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::normal_status:
            terms.normal_status = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::exp_short:
            terms.exp_short = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::lvl_short:
            terms.lvl_short = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::hp_short:
            terms.hp_short = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::sp_short:
            terms.sp_short = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::sp_cost:
            terms.sp_cost = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::attack:
            terms.attack = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::defense:
            terms.defense = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::spirit:
            terms.spirit = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::agility:
            terms.agility = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::weapon:
            terms.weapon = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::shield:
            terms.shield = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::armor:
            terms.armor = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::helmet:
            terms.helmet = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::accessory:
            terms.accessory = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::save_game_message:
            terms.save_game_message = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::load_game_message:
            terms.load_game_message = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::exit_game_message:
            terms.exit_game_message = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::file:
            terms.file = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::yes:
            terms.yes = stream.ReadString(chunk_info.length);
            break;
        case ChunkTerms::no:
            terms.no = stream.ReadString(chunk_info.length);
            break;
        default:
            stream.Seek(chunk_info.length, Reader::FromCurrent);
        }
    }
    return terms;
}
