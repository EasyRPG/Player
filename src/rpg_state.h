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

#ifndef _RPG_STATE_H_
#define _RPG_STATE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

////////////////////////////////////////////////////////////
/// RPG::State class
////////////////////////////////////////////////////////////
namespace RPG {
	class State {
	public:
		enum Type {
			Type_battle = 0,
			Type_map = 1
		};
		enum Restriction {
			Restriction_normal = 0,
			Restriction_do_nothing = 1,
			Restriction_attack_enemy = 2,
			Restriction_attack_ally = 3
		};
		enum AffectType {
			AffectType_nothing = 0,
			AffectType_doble = 1,
			AffectType_half = 2
		};
		enum ChangeType {
			ChangeType_nothing = 0,
			ChangeType_plus = 1,
			ChangeType_minus = 2
		};

		State();
		
		int ID;
		std::string name;
		int type;
		int color;
		int priority;
		int restriction;
		int a_rate;
		int b_rate;
		int c_rate;
		int d_rate;
		int e_rate;
		int hold_turn;
		int auto_release_prob;
		int release_by_damage;
		int affect_type;
		bool affect_attack;
		bool affect_defense;
		bool affect_spirit;
		bool affect_agility;
		int reduce_hit_ratio;
		bool avoid_attacks;
		bool reflect_magic;
		bool cursed;
		int battler_animation_id;
		bool restrict_skill;
		int restrict_skill_level;
		bool restrict_magic;
		int restrict_magic_level;
		int hp_change_type;
		int sp_change_type;
		std::string message_actor;
		std::string message_enemy;
		std::string message_already;
		std::string message_affected;
		std::string message_recovery;
		int hp_change_max;
		int hp_change_val;
		int hp_change_map_val;
		int hp_change_map_steps;
		int sp_change_max;
		int sp_change_val;
		int sp_change_map_val;
		int sp_change_map_steps;
	};
}

#endif
