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
#include "rpg_enemy.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::Enemy::Enemy() {
	ID = 0;
	name = "";
	battler_name = "";
	battler_hue = 0;
	max_hp = 10;
	max_sp = 10;
	attack = 10;
	defense = 10;
	spirit = 10;
	agility = 10;
	transparent = false;
	exp = 0;
	gold = 0;
	drop_id = 0;
	drop_prob = 100;
	critical_hit = false;
	critical_hit_chance = 30;
	miss = false;
	levitate = false;
}
