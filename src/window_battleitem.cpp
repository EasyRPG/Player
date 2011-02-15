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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "rpg_item.h"
#include "rpg_skill.h"
#include "player.h"
#include "game_battle.h"
#include "window_battleitem.h"

////////////////////////////////////////////////////////////
Window_BattleItem::Window_BattleItem(int ix, int iy, int iwidth, int iheight) :
	Window_Item(ix, iy, iwidth, iheight), actor_id(0) {}

////////////////////////////////////////////////////////////
Window_BattleItem::~Window_BattleItem() {
}

////////////////////////////////////////////////////////////
void Window_BattleItem::SetActor(int _actor_id) {
	actor_id = _actor_id;
}

////////////////////////////////////////////////////////////
bool Window_BattleItem::CheckEnable(int item_id) {
	const RPG::Item& item = Data::items[item_id - 1];

	switch (item.type) {
		case RPG::Item::Type_normal:
			return false;
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			return item.use_skill && CanUseItem(item) && CanUseSkill(item.skill_id);
		case RPG::Item::Type_medicine:
			return CanUseItem(item) && !item.ocassion_field && (!item.ko_only || Game_Battle::HaveCorpse());
		case RPG::Item::Type_book:
		case RPG::Item::Type_material:
			return false;
		case RPG::Item::Type_special:
			return CanUseItem(item) && CanUseSkill(item.skill_id);
		case RPG::Item::Type_switch:
			return item.ocassion_battle;
		default:
			return false;
	}
}

////////////////////////////////////////////////////////////
bool Window_BattleItem::CanUseItem(const RPG::Item& item) {
	if (actor_id <= 0)
		return false;
	if (actor_id >= (int) item.actor_set.size() ||
		item.actor_set[actor_id])
		return true;

	if (Player::engine != Player::EngineRpg2k3)
		return false;

	const RPG::Actor& actor = Data::actors[actor_id - 1];
	if (actor.class_id < 0)
		return false;
	if (actor.class_id >= (int) item.class_set.size() ||
		item.class_set[actor.class_id])
		return true;

	return false;
}

////////////////////////////////////////////////////////////
bool Window_BattleItem::CanUseSkill(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return skill.type != RPG::Skill::Type_switch || skill.occasion_battle;
}

