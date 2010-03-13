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
RPG::Terms LDB_Reader::ReadTerms(FILE* stream) {
    RPG::Terms terms;

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
        case ChunkTerms::encounter:
            terms.encounter = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::special_combat:
            terms.special_combat = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::escape_success:
            terms.escape_success = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::escape_failure:
            terms.escape_failure = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::victory:
            terms.victory = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::defeat:
            terms.defeat = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::exp_received:
            terms.exp_received = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::gold_recieved_a:
            terms.gold_recieved_a = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::gold_recieved_b:
            terms.gold_recieved_b = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::item_recieved:
            terms.item_recieved = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::attacking:
            terms.attacking = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::actor_critical:
            terms.actor_critical = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_critical:
            terms.enemy_critical = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::defending:
            terms.defending = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::observing:
            terms.observing = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::focus:
            terms.focus = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::autodestruction:
            terms.autodestruction = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_escape:
            terms.enemy_escape = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_transform:
            terms.enemy_transform = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_damaged:
            terms.enemy_damaged = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_undamaged:
            terms.enemy_undamaged = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::actor_damaged:
            terms.actor_damaged = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::actor_undamaged:
            terms.actor_undamaged = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::skill_failure_a:
            terms.skill_failure_a = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::skill_failure_b:
            terms.skill_failure_b = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::skill_failure_c:
            terms.skill_failure_c = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::dodge:
            terms.dodge = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::use_item:
            terms.use_item = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::hp_recovery:
            terms.hp_recovery = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::parameter_increase:
            terms.parameter_increase = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::parameter_decrease:
            terms.parameter_decrease = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::actor_hp_absorbed:
            terms.actor_hp_absorbed = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::enemy_hp_absorbed:
            terms.enemy_hp_absorbed = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::resistance_increase:
            terms.resistance_increase = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::resistance_decrease:
            terms.resistance_decrease = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::level_up:
            terms.level_up = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::skill_learned:
            terms.skill_learned = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_greeting1:
            terms.shop_greeting1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting1:
            terms.shop_regreeting1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy1:
            terms.shop_buy1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell1:
            terms.shop_sell1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_leave1:
            terms.shop_leave1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select1:
            terms.shop_buy_select1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number1:
            terms.shop_buy_number1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_purchased1:
            terms.shop_purchased1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select1:
            terms.shop_sell_select1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number1:
            terms.shop_sell_number1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sold1:
            terms.shop_sold1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_greeting2:
            terms.shop_greeting2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting2:
            terms.shop_regreeting2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy2:
            terms.shop_buy2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell2:
            terms.shop_sell2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_leave2:
            terms.shop_leave2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select2:
            terms.shop_buy_select2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number2:
            terms.shop_buy_number2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_purchased2:
            terms.shop_purchased2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select2:
            terms.shop_sell_select2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number2:
            terms.shop_sell_number2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sold2:
            terms.shop_sold2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_greeting3:
            terms.shop_greeting3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_regreeting3:
            terms.shop_regreeting3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy3:
            terms.shop_buy3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell3:
            terms.shop_sell3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_leave3:
            terms.shop_leave3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_select3:
            terms.shop_buy_select3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_buy_number3:
            terms.shop_buy_number3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_purchased3:
            terms.shop_purchased3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_select3:
            terms.shop_sell_select3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sell_number3:
            terms.shop_sell_number3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shop_sold3:
            terms.shop_sold3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_1:
            terms.inn_a_greeting_1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_2:
            terms.inn_a_greeting_2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_a_greeting_3:
            terms.inn_a_greeting_3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_a_accept:
            terms.inn_a_accept = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_a_cancel:
            terms.inn_a_cancel = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_1:
            terms.inn_b_greeting_1 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_2:
            terms.inn_b_greeting_2 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_b_greeting_3:
            terms.inn_b_greeting_3 = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_b_accept:
            terms.inn_b_accept = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::inn_b_cancel:
            terms.inn_b_cancel = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::possessed_items:
            terms.possessed_items = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::equipped_items:
            terms.equipped_items = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::gold:
            terms.gold = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::battle_fight:
            terms.battle_fight = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::battle_auto:
            terms.battle_auto = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::battle_escape:
            terms.battle_escape = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::command_attack:
            terms.command_attack = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::command_defend:
            terms.command_defend = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::command_item:
            terms.command_item = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::command_skill:
            terms.command_skill = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::menu_equipment:
            terms.menu_equipment = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::menu_save:
            terms.menu_save = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::menu_quit:
            terms.menu_quit = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::new_game:
            terms.new_game = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::load_game:
            terms.load_game = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::exit_game:
            terms.exit_game = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::level:
            terms.level = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::health_points:
            terms.health_points = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::spirit_points:
            terms.spirit_points = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::normal_status:
            terms.normal_status = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::exp_short:
            terms.exp_short = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::lvl_short:
            terms.lvl_short = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::hp_short:
            terms.hp_short = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::sp_short:
            terms.sp_short = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::sp_cost:
            terms.sp_cost = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::attack:
            terms.attack = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::defense:
            terms.defense = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::spirit:
            terms.spirit = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::agility:
            terms.agility = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::weapon:
            terms.weapon = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::shield:
            terms.shield = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::armor:
            terms.armor = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::helmet:
            terms.helmet = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::accessory:
            terms.accessory = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::save_game_message:
            terms.save_game_message = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::load_game_message:
            terms.load_game_message = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::exit_game_message:
            terms.exit_game_message = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::file:
            terms.file = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::yes:
            terms.yes = Reader::String(stream, chunk_info.length);
            break;
        case ChunkTerms::no:
            terms.no = Reader::String(stream, chunk_info.length);
            break;
        default:
            fseek(stream, chunk_info.length, SEEK_CUR);
        }
    } while(chunk_info.ID != ChunkData::END);
    return terms;
}
