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
#include <algorithm>
#include "game_actor.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
Game_Actor::Game_Actor(int actor_id) {
	Setup(actor_id);
}

////////////////////////////////////////////////////////////
void Game_Actor::Setup(int actor_id) {
	name = Data::actors[actor_id - 1].name;
	character_name = Data::actors[actor_id - 1].character_name;
	character_index = Data::actors[actor_id - 1].character_index;
	face_name = Data::actors[actor_id - 1].face_name;
	face_index = Data::actors[actor_id - 1].face_index;
	weapon_id = Data::actors[actor_id - 1].weapon_id;
	shield_id = Data::actors[actor_id - 1].shield_id;
	armor_id = Data::actors[actor_id - 1].armor_id;
	helmet_id = Data::actors[actor_id - 1].helmet_id;
	accessory_id = Data::actors[actor_id - 1].accessory_id;
	level = Data::actors[actor_id - 1].initial_level;
}

////////////////////////////////////////////////////////////
bool Game_Actor::HasSkill(int skill_id) const {
	return std::find(skills.begin(), skills.end(), skill_id) != skills.end();
}

////////////////////////////////////////////////////////////
void Game_Actor::LearnSkill(int skill_id) {
	if (skill_id > 0 && !HasSkill(skill_id)) {
		skills.push_back(skill_id);
		std::sort(skills.begin(), skills.end());
	}
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetName() const {
	return name;
}

std::string Game_Actor::GetCharacterName() const {
	return character_name;
}

int Game_Actor::GetCharacterIndex() const {
	return character_index;
}

std::string Game_Actor::GetFaceName() const {
	return face_name;
}

int Game_Actor::GetFaceIndex() const {
	return face_index;
}

int Game_Actor::GetWeaponId() const {
	return weapon_id;
}

int Game_Actor::GetShieldId() const {
	return shield_id;
}

int Game_Actor::GetArmorId() const {
	return armor_id;
}

int Game_Actor::GetHelmetId() const {
	return helmet_id;
}

int Game_Actor::GetAccessoryId() const {
	return accessory_id;
}

int Game_Actor::GetLevel() const {
	return level;
}

int Game_Actor::GetExp() const {
	return exp;
}

std::vector<int> Game_Actor::GetSkills() const {
	return skills;
}
