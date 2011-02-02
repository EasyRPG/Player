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
#include "game_battler.h"
#include <algorithm>
#include "game_actor.h"
#include "util_macro.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
Game_Battler::Game_Battler() :
	hp(0),
	sp(0),
	maxhp_plus(0),
	maxsp_plus(0),
	atk_plus(0),
	def_plus(0),
	spi_plus(0),
	agi_plus(0),
	hidden(false),
	immortal(false),
	damage_pop(false),
	damage(false),
	critical(false),
	animation_id(0),
	animation_hit(false),
	white_flash(false),
	blink(false) {
}

////////////////////////////////////////////////////////////
bool Game_Battler::HasState(int state_id) const {
	return (std::find(states.begin(), states.end(), state_id) != states.end());
}

////////////////////////////////////////////////////////////
std::vector<int> Game_Battler::GetStates() const {
	return states;
}

bool Game_Battler::IsDead() const {
	return !hidden && hp == 0 && !immortal;
}

bool Game_Battler::Exists() const {
	return !hidden && !IsDead();
}

////////////////////////////////////////////////////////////
int Game_Battler::GetHp() const {
	return hp;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetSp() const {
	return sp;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetMaxHp() {
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
int Game_Battler::GetMaxSp() {
	int base_maxsp = GetBaseMaxSp();
	int n = min(max(base_maxsp + maxsp_plus, 0), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO test needed
			n *= Data::states[(*i)].sp_change_max / 100;
	}

	n = min(max(n, 0), 999);

	return n;
}

////////////////////////////////////////////////////////////
void Game_Battler::SetMaxHp(int _maxhp) {
	maxhp_plus += _maxhp - GetMaxHp(); 
	maxhp_plus = min(max(maxhp_plus, -999), 999);
	hp = min(GetMaxHp(), hp);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetMaxSp(int _maxsp) {
	maxsp_plus += _maxsp - GetMaxSp(); 
	maxsp_plus = min(max(maxsp_plus, -999), 999);
	sp = min(GetMaxSp(), sp);
}

////////////////////////////////////////////////////////////
int Game_Battler::GetAtk() {
	int base_atk = GetBaseAtk();
	int n = min(max(base_atk + atk_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetDef() {
	int base_def = GetBaseDef();
	int n = min(max(base_def + def_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetSpi() {
	int base_spi = GetBaseSpi();
	int n = min(max(base_spi + spi_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetAgi() {
	int base_agi = GetBaseAgi();
	int n = min(max(base_agi + agi_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
void Game_Battler::SetHp(int _hp) {
	hp = min(_hp, GetMaxHp());
}

////////////////////////////////////////////////////////////
void Game_Battler::SetSp(int _sp) {
	sp = min(_sp, GetMaxSp());
}

////////////////////////////////////////////////////////////
void Game_Battler::SetAtk(int _atk) {
	atk_plus += _atk - GetAtk();
	atk_plus = min(max(atk_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetDef(int _def) {
	def_plus += _def - GetDef();
	def_plus = min(max(def_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetSpi(int _spi) {
	spi_plus += _spi - GetSpi();
	spi_plus = min(max(spi_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetAgi(int _agi) {
	agi_plus += _agi - GetAgi();
	agi_plus = min(max(agi_plus, 1), 999);
}

////////////////////////////////////////////////////////////
bool Game_Battler::IsSkillUsable(int skill_id) {
	if (CalculateSkillCost(skill_id) > sp) {
		return false;
	}
	// ToDo: Check for Movable(?) and Silence

	// ToDo: Escape and Teleport Spells need event SetTeleportPlace and
	// SetEscapePlace first. Not sure if any game uses this...
	//if (Data::skills[skill_id - 1].type == RPG::Skill::Type_teleport) {
	//	return is_there_a_teleport_set;
	//} else if (Data::skills[skill_id - 1].type == RPG::Skill::Type_escape) {
	//	return is_there_an_escape_set;
	//} else
	if (Data::skills[skill_id - 1].type == RPG::Skill::Type_normal) {
		int scope = Data::skills[skill_id - 1].scope;

		if (scope == RPG::Skill::Scope_self ||
			scope == RPG::Skill::Scope_ally ||
			scope == RPG::Skill::Scope_party) {
			// ToDo: A skill is also acceptable when it cures a status
			return (Data::skills[skill_id - 1].affect_hp ||
					Data::skills[skill_id - 1].affect_sp);
		}
	} else if (Data::skills[skill_id - 1].type == RPG::Skill::Type_switch) {
		// Todo:
		// if (Game_Temp::IsInBattle()) {
		// return Data::skills[skill_id - 1].occasion_battle;
		// else {
		return Data::skills[skill_id - 1].occasion_field;
		// }
	}

	return false;
}

////////////////////////////////////////////////////////////
int Game_Battler::CalculateSkillCost(int skill_id) {
	return Data::skills[skill_id - 1].sp_cost;
}

////////////////////////////////////////////////////////////
void Game_Battler::AddState(int state_id) {
	if (state_id > 0 && !HasState(state_id)) {
		states.push_back(state_id);
		std::sort(states.begin(), states.end());
	}
}

////////////////////////////////////////////////////////////
void Game_Battler::RemoveState(int state_id) {
	std::vector<int>::iterator it = std::find(states.begin(), states.end(), state_id);
	if (it != states.end())
		states.erase(it);
}

////////////////////////////////////////////////////////////
void Game_Battler::RemoveAllStates() {
	states.clear();
}

