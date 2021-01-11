/*
 * This file is part of liblcf. Copyright (c) 2020 liblcf authors.
 * https://github.com/EasyRPG/liblcf - https://easyrpg.org
 *
 * liblcf is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#include "lcf/rpg/database.h"
#include "lcf/data.h"

namespace lcf {

namespace Data {
	rpg::Database data;

	std::vector<rpg::Actor>& actors = data.actors;
	std::vector<rpg::Skill>& skills = data.skills;
	std::vector<rpg::Item>& items = data.items;
	std::vector<rpg::Enemy>& enemies = data.enemies;
	std::vector<rpg::Troop>& troops = data.troops;
	std::vector<rpg::Terrain>& terrains = data.terrains;
	std::vector<rpg::Attribute>& attributes = data.attributes;
	std::vector<rpg::State>& states = data.states;
	std::vector<rpg::Animation>& animations = data.animations;
	std::vector<rpg::Chipset>& chipsets = data.chipsets;
	std::vector<rpg::CommonEvent>& commonevents = data.commonevents;
	rpg::BattleCommands& battlecommands = data.battlecommands;
	std::vector<rpg::Class>& classes = data.classes;
	std::vector<rpg::BattlerAnimation>& battleranimations = data.battleranimations;
	rpg::Terms& terms = data.terms;
	rpg::System& system = data.system;
	std::vector<rpg::Switch>& switches = data.switches;
	std::vector<rpg::Variable>& variables = data.variables;

	rpg::TreeMap treemap;
}

void Data::Clear() {
	actors.clear();
	skills.clear();
	items.clear();
	enemies.clear();
	troops.clear();
	terrains.clear();
	attributes.clear();
	states.clear();
	animations.clear();
	chipsets.clear();
	commonevents.clear();
	battlecommands = rpg::BattleCommands();
	classes.clear();
	battleranimations.clear();
	terms = rpg::Terms();
	system = rpg::System();
	switches.clear();
	variables.clear();
	treemap.active_node = 0;
	treemap.maps.clear();
	treemap.tree_order.clear();
}

} //namespace lcf
