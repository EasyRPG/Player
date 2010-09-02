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

#ifndef _RPG_TERMS_H_
#define _RPG_TERMS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

////////////////////////////////////////////////////////////
/// RPG::Terms class
////////////////////////////////////////////////////////////
namespace RPG {
	class Terms {
	public:
		Terms();
		
		std::string encounter;
		std::string special_combat;
		std::string escape_success;
		std::string escape_failure;
		std::string victory;
		std::string defeat;
		std::string exp_received;
		std::string gold_recieved_a;
		std::string gold_recieved_b;
		std::string item_recieved;
		std::string attacking;
		std::string actor_critical;
		std::string enemy_critical;
		std::string defending;
		std::string observing;
		std::string focus;
		std::string autodestruction;
		std::string enemy_escape;
		std::string enemy_transform;
		std::string enemy_damaged;
		std::string enemy_undamaged;
		std::string actor_damaged;
		std::string actor_undamaged;
		std::string skill_failure_a;
		std::string skill_failure_b;
		std::string skill_failure_c;
		std::string dodge;
		std::string use_item;
		std::string hp_recovery;
		std::string parameter_increase;
		std::string parameter_decrease;
		std::string actor_hp_absorbed;
		std::string enemy_hp_absorbed;
		std::string resistance_increase;
		std::string resistance_decrease;
		std::string level_up;
		std::string skill_learned;
		std::string shop_greeting1;
		std::string shop_regreeting1;
		std::string shop_buy1;
		std::string shop_sell1;
		std::string shop_leave1;
		std::string shop_buy_select1;
		std::string shop_buy_number1;
		std::string shop_purchased1;
		std::string shop_sell_select1;
		std::string shop_sell_number1;
		std::string shop_sold1;
		std::string shop_greeting2;
		std::string shop_regreeting2;
		std::string shop_buy2;
		std::string shop_sell2;
		std::string shop_leave2;
		std::string shop_buy_select2;
		std::string shop_buy_number2;
		std::string shop_purchased2;
		std::string shop_sell_select2;
		std::string shop_sell_number2;
		std::string shop_sold2;
		std::string shop_greeting3;
		std::string shop_regreeting3;
		std::string shop_buy3;
		std::string shop_sell3;
		std::string shop_leave3;
		std::string shop_buy_select3;
		std::string shop_buy_number3;
		std::string shop_purchased3;
		std::string shop_sell_select3;
		std::string shop_sell_number3;
		std::string shop_sold3;
		std::string inn_a_greeting_1;
		std::string inn_a_greeting_2;
		std::string inn_a_greeting_3;
		std::string inn_a_accept;
		std::string inn_a_cancel;
		std::string inn_b_greeting_1;
		std::string inn_b_greeting_2;
		std::string inn_b_greeting_3;
		std::string inn_b_accept;
		std::string inn_b_cancel;
		std::string possessed_items;
		std::string equipped_items;
		std::string gold;
		std::string battle_fight;
		std::string battle_auto;
		std::string battle_escape;
		std::string command_attack;
		std::string command_defend;
		std::string command_item;
		std::string command_skill;
		std::string menu_equipment;
		std::string menu_save;
		std::string menu_quit;
		std::string new_game;
		std::string load_game;
		std::string exit_game;
		std::string level;
		std::string health_points;
		std::string spirit_points;
		std::string normal_status;
		std::string exp_short;
		std::string lvl_short;
		std::string hp_short;
		std::string sp_short;
		std::string sp_cost;
		std::string attack;
		std::string defense;
		std::string spirit;
		std::string agility;
		std::string weapon;
		std::string shield;
		std::string armor;
		std::string helmet;
		std::string accessory;
		std::string save_game_message;
		std::string load_game_message;
		std::string exit_game_message;
		std::string file;
		std::string yes;
		std::string no;
	};
}

#endif
