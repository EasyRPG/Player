/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */


// Headers
#include <algorithm>
#include "player.h"
#include "game_battler.h"
#include "game_actor.h"
#include "util_macro.h"
#include "main_data.h"

bool Game_Battler::HasState(int state_id) const {
	return (std::find(GetStates().begin(), GetStates().end(), state_id) != GetStates().end());
}

bool Game_Battler::IsDead() const {
	return !IsHidden() && GetHp() == 0 && !IsImmortal();
}

bool Game_Battler::Exists() const {
	return !IsHidden() && !IsDead();
}

const RPG::State* Game_Battler::GetState() {
	int priority = 0;
	const RPG::State* the_state = NULL;

	const std::vector<int16_t>& states = GetStates();
	for (int i = 0; i < (int) states.size(); i++) {
		const RPG::State* state = &Data::states[states[i]];
		// Death has highest priority
		if (state->ID == 1)
			return state;

		if (state->priority > priority) {
			the_state = state;
			priority = state->priority;
		}
	}

	return the_state;
}

bool Game_Battler::IsSkillUsable(int skill_id) {
	if (CalculateSkillCost(skill_id) > GetSp()) {
		return false;
	}
	// TODO: Check for Movable(?) and Silence

	// TODO: Escape and Teleport Spells need event SetTeleportPlace and
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
			// TODO: A skill is also acceptable when it cures a status
			return (Data::skills[skill_id - 1].affect_hp ||
					Data::skills[skill_id - 1].affect_sp);
		}
	} else if (Data::skills[skill_id - 1].type == RPG::Skill::Type_switch) {
		// TODO:
		// if (Game_Temp::IsInBattle()) {
		// return Data::skills[skill_id - 1].occasion_battle;
		// else {
		return Data::skills[skill_id - 1].occasion_field;
		// }
	}

	return false;
}

int Game_Battler::CalculateSkillCost(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return (Player::engine == Player::EngineRpg2k3 &&
			skill.sp_type == RPG::Skill::SpType_percent)
		? GetMaxSp() * skill.sp_percent / 100
		: skill.sp_cost;
}

void Game_Battler::AddState(int state_id) {
	std::vector<int16_t>& states = GetStates();
	if (state_id > 0 && !HasState(state_id)) {
		states.push_back(state_id);
		std::sort(states.begin(), states.end());
	}
}

void Game_Battler::RemoveState(int state_id) {
	std::vector<int16_t>& states = GetStates();
	std::vector<int16_t>::iterator it = std::find(states.begin(), states.end(), state_id);
	if (it != states.end())
		states.erase(it);
}

static bool NonPermanent(int state_id) {
	return Data::states[state_id - 1].type == 0;
}

void Game_Battler::RemoveStates() {
	std::vector<int16_t>& states = GetStates();
	std::vector<int16_t>::iterator end = std::remove_if(states.begin(), states.end(), NonPermanent);
	states.erase(end, states.end());
}

void Game_Battler::RemoveAllStates() {
	std::vector<int16_t>& states = GetStates();
	states.clear();
}

bool Game_Battler::IsHidden() const {
	return false;
}

bool Game_Battler::IsImmortal() const {
	return false;
}

int Game_Battler::GetMaxHp() const {
	return GetBaseMaxHp();
}

int Game_Battler::GetMaxSp() const {
	return GetBaseMaxSp();
}

static int AffectParameter(int const type, int const val) {
	return
		type == 0? val / 2 :
		type == 1? val * 2 :
		type == 2? val :
		val;
}

int Game_Battler::GetAtk() const {
	int base_atk = GetBaseAtk();
	int n = min(max(base_atk, 1), 999);

	const std::vector<int16_t>& states = GetStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); i++) {
		if(Data::states[(*i)].affect_attack) {
			n = AffectParameter(Data::states[(*i)].affect_type, base_atk);
			break;
		}
	}

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetDef() const {
	int base_def = GetBaseDef();
	int n = min(max(base_def, 1), 999);

	const std::vector<int16_t>& states = GetStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); i++) {
		if(Data::states[(*i)].affect_defense) {
			n = AffectParameter(Data::states[(*i)].affect_type, base_def);
			break;
		}
	}

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetSpi() const {
	int base_spi = GetBaseSpi();
	int n = min(max(base_spi, 1), 999);

	const std::vector<int16_t>& states = GetStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); i++) {
		if(Data::states[(*i)].affect_spirit) {
			n = AffectParameter(Data::states[(*i)].affect_type, base_spi);
			break;
		}
	}

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetAgi() const {
	int base_agi = GetBaseAgi();
	int n = min(max(base_agi, 1), 999);

	const std::vector<int16_t>& states = GetStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); i++) {
		if(Data::states[(*i)].affect_agility) {
			n = AffectParameter(Data::states[(*i)].affect_type, base_agi);
			break;
		}
	}

	n = min(max(n, 1), 999);

	return n;
}

