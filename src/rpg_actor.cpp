#include "rpg_actor.h"

RPG::Actor::Actor() {
	id = 0;
	name = "";
	title = "";
	character_name = "";
	character_index = 0;
	transparent = false;
	initial_level = 1;
	final_level = 50;
	critical_hit = false;
	critical_hit_chance = 0;
	face_name = "";
	face_index = 0;
	two_swords_style = false;
	fix_equipment = false;
	auto_battle = false;
	super_guard = false;
	/*for (int i = 1; i <= final_level; ++i) {
		parameter_maxhp[i] = 500 + i * 50;
		parameter_maxsp[i] = 500 + i * 50;
		parameter_attack[i] = 50 + i * 5;
		parameter_defense[i] = 50 + i * 5;
		parameter_spirit[i] = 50 + i * 5;
		parameter_agility[i] = 50 + i * 5;
	}*/
	exp_base = 10;
	exp_inflation = 50;
	exp_correction = 0;
	unarmed_animation = 0;
	weapon_id = 0;
	shield_id = 0;
	armor_id = 0;
	head_id = 0;
	accessory_id = 0;
	condition_size = 0;
	attribute_size = 0;
	
	rename_skill = false; // 2000
	skill_name = ""; // 2000
	
	class_id = 0; // 2003
	battler_animation = 1; // 2003
}
