#include "rpg_item.h"

RPG::Item::Item() {
	id = 0;
	name = "";
	description = "";
	type = COMMON;
	price = 0;
	uses = 1;
	atk_points = 0;
	pdef_points = 0;
	int_points = 0;
	agi_points = 0;
	state_chance = 0;
	two_handed = false;
	sp_cost = 0;
	hit = 90;
	critical_hit = 0;
	animation_id = 0;
	preemptive = false;
	dual_attack = false;
	attack_all = false;
	ignore_evasion = false;
	prevent_critical = false;
	raise_evasion = false;
	half_sp_cost = false;
	no_terrain_damage = false;
	entire_party = false;
	recover_hp = 0;
	recover_hp_rate = 0;
	recover_sp = 0;
	recover_sp_rate = 0;
	ocassion_field = false;
	ko_only = false;
	skill_id = false;
	using_messsage = false;
	max_hp_points = 0;
	max_sp_points = 0;
	switch_id = 0;
	ocassion_battle = false;
}
