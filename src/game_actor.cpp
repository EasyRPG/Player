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
#include <sstream>
#include "game_actor.h"
#include "main_data.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
Game_Actor::Game_Actor(int actor_id) {
	Setup(actor_id);
	SetGameActor(this);
}

////////////////////////////////////////////////////////////
void Game_Actor::Setup(int actor_id) {
	this->actor_id = actor_id;
	name = Data::actors[actor_id - 1].name;
	character_name = Data::actors[actor_id - 1].character_name;
	character_index = Data::actors[actor_id - 1].character_index;
	face_name = Data::actors[actor_id - 1].face_name;
	face_index = Data::actors[actor_id - 1].face_index;
	title = Data::actors[actor_id - 1].title;
	weapon_id = Data::actors[actor_id - 1].weapon_id;
	shield_id = Data::actors[actor_id - 1].shield_id;
	armor_id = Data::actors[actor_id - 1].armor_id;
	helmet_id = Data::actors[actor_id - 1].helmet_id;
	accessory_id = Data::actors[actor_id - 1].accessory_id;
	level = Data::actors[actor_id - 1].initial_level;
	exp_list.resize(Data::actors[actor_id - 1].final_level, 0);
	MakeExpList();
	exp = exp_list[level - 1];
	hp = GetMaxHp();
	sp = GetMaxSp();
}

////////////////////////////////////////////////////////////
int Game_Actor::GetId() const {
	return actor_id;
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
int Game_Actor::GetMaxHp() {
	int base_maxhp = GetBaseMaxHp();
	int n = min(max(base_maxhp + maxhp_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO test needed
			n *= Data::states[(*i)].hp_change_max / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetMaxSp() {
	int base_maxsp = GetBaseMaxSp();
	int n = min(max(base_maxsp + maxsp_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO test needed
			n *= Data::states[(*i)].sp_change_max / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseMaxHp() const {
	return Data::actors[actor_id - 1].parameter_maxhp[level - 1];
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseMaxSp() const {
	return Data::actors[actor_id - 1].parameter_maxsp[level - 1];
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseAtk() const {
	int n = Data::actors[actor_id - 1].parameter_attack[level - 1];

	n += Data::items[weapon_id - 1].atk_points;
	n += Data::items[shield_id - 1].atk_points;
	n += Data::items[armor_id - 1].atk_points;
	n += Data::items[helmet_id - 1].atk_points;
	n += Data::items[accessory_id - 1].atk_points;

	return min(max(n, 1), 999);
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseDef() const {
	int n = Data::actors[actor_id - 1].parameter_defense[level - 1];

	n += Data::items[weapon_id - 1].def_points;
	n += Data::items[shield_id - 1].def_points;
	n += Data::items[armor_id - 1].def_points;
	n += Data::items[helmet_id - 1].def_points;
	n += Data::items[accessory_id - 1].def_points;

	return min(max(n, 1), 999);
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseSpi() const {
	int n = Data::actors[actor_id - 1].parameter_spirit[level - 1];

	n += Data::items[weapon_id - 1].spi_points;
	n += Data::items[shield_id - 1].spi_points;
	n += Data::items[armor_id - 1].spi_points;
	n += Data::items[helmet_id - 1].spi_points;
	n += Data::items[accessory_id - 1].spi_points;

	return min(max(n, 1), 999);
}

////////////////////////////////////////////////////////////
int Game_Actor::GetBaseAgi() const {
	int n = Data::actors[actor_id - 1].parameter_agility[level - 1];

	n += Data::items[weapon_id - 1].agi_points;
	n += Data::items[shield_id - 1].agi_points;
	n += Data::items[armor_id - 1].agi_points;
	n += Data::items[helmet_id - 1].agi_points;
	n += Data::items[accessory_id - 1].agi_points;

	return min(max(n, 1), 999);
}

////////////////////////////////////////////////////////////
int Game_Actor::CalculateExp(int level)
{
        double base = Data::actors[actor_id - 1].exp_base;
        double inflation = Data::actors[actor_id - 1].exp_inflation;
		double correction = Data::actors[actor_id - 1].exp_correction;

        int result = 0;

        inflation = 1.5 + (inflation * 0.01);

        for (int i = level; i >= 1; i--)
        {
                result = result + (int)(correction + base);
                base = base * inflation;
                inflation = ((level+1) * 0.002 + 0.8) * (inflation - 1) + 1;
        }
        return min(result, 1000000);
}

////////////////////////////////////////////////////////////
void Game_Actor::MakeExpList() {
	for (int i = 1; i < Data::actors[actor_id - 1].final_level; ++i) {
		exp_list[i] = CalculateExp(i);
	}
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetExpString() {
	if ((unsigned)level == exp_list.size()) {
		return "------";
	} else {
		std::stringstream ss;
		ss << exp;
		return ss.str();
	}
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetNextExpString() {
	if ((unsigned)level == exp_list.size()) {
		return "------";
	} else {
		std::stringstream ss;
		ss << exp_list[level];
		return ss.str();
	}
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetName() const {
	return name;
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetCharacterName() const {
	return character_name;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetCharacterIndex() const {
	return character_index;
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetFaceName() const {
	return face_name;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetFaceIndex() const {
	return face_index;
}

////////////////////////////////////////////////////////////
std::string Game_Actor::GetTitle() const {
	return title;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetWeaponId() const {
	return weapon_id;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetShieldId() const {
	return shield_id;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetArmorId() const {
	return armor_id;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetHelmetId() const {
	return helmet_id;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetAccessoryId() const {
	return accessory_id;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetLevel() const {
	return level;
}

////////////////////////////////////////////////////////////
int Game_Actor::GetExp() const {
	return exp;
}

////////////////////////////////////////////////////////////
void Game_Actor::SetExp(int _exp) {
	exp = _exp;
}

////////////////////////////////////////////////////////////
void Game_Actor::SetLevel(int _level) {
	level = _level;
}

void Game_Actor::ChangeLevel(int level) {
	this->level = max(min(level, 50), 1);
	//ChangeExp()
}

void Game_Actor::ChangeExp(int exp) {
	// TODO
	/*int last_level = level;

	this->exp = max(min(exp, 0), 999999);
	while (this->exp*/
}

////////////////////////////////////////////////////////////
std::vector<int> Game_Actor::GetSkills() const {
	return skills;
}
