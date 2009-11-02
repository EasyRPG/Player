#include "rpg_skill.h"

RPG::Skill::Skill() {
    id = 0;
	name = "";
	type = 0;
	sp_cost = 0;
	description = "";
	
	// type == Normal
	scope = 0;
	animation_id = 0;
	using_message1 = "";
	using_message2 = "";
	failure_message = 0;
	pdef_f = 0;
	mdef_f = 0;
	variance = 0;
	power = 0;
	hit = 0;
	affect_hp = false;
	affect_sp = false;
	affect_attack = false;
	affect_defense = false;
	affect_spirit = false;
	affect_agility = false;
	absorb_damage = false;
	ignore_defense = false;
	condition_size = 0;
	attribute_size = 0;
	affect_attr_defence = false;

	// type == Teleport/Escape
	sound_effect = new RPG::Sound();
	
	// type == Switch
	switch_id = 1;
	occasion_field = false;
	occasion_battle = false;
}
