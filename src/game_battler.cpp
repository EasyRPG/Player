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
#include <cstdlib>
#include "player.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_party_base.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "game_switches.h"
#include "game_temp.h"
#include "util_macro.h"
#include "main_data.h"

#define EASYRPG_GAUGE_MAX_VALUE 120000

Game_Battler::Game_Battler() {
	ResetBattle();
}

bool Game_Battler::HasState(int state_id) const {
	return (std::find(GetStates().begin(), GetStates().end(), state_id) != GetStates().end());
}

int Game_Battler::GetSignificantRestriction() {
	const std::vector<int16_t>& states = GetStates();
	for (int i = 0; i < (int)states.size(); i++) {
		const RPG::State* state = &Data::states[states[i] - 1];
		if (state->restriction != RPG::State::Restriction_normal) {
			return state->restriction;
		}
	}
	return RPG::State::Restriction_normal;
}

bool Game_Battler::CanAct() {
	const std::vector<int16_t>& states = GetStates();
	for (int i = 0; i < (int)states.size(); i++) {
		const RPG::State* state = &Data::states[states[i] - 1];
		if (state->restriction == RPG::State::Restriction_do_nothing) {
			return false;
		}
	}
	return true;
}

bool Game_Battler::IsDead() const {
	return HasState(1);
}

bool Game_Battler::Exists() const {
	return !IsHidden() && !IsDead();
}

const RPG::State* Game_Battler::GetSignificantState() {
	int priority = 0;
	const RPG::State* the_state = NULL;

	const std::vector<int16_t>& states = GetStates();
	for (int i = 0; i < (int) states.size(); i++) {
		const RPG::State* state = &Data::states[states[i] - 1];
		// Death has highest priority
		if (state->ID == 1)
			return state;

		if (state->priority >= priority) {
			the_state = state;
			priority = state->priority;
		}
	}

	return the_state;
}

int Game_Battler::GetStateRate(int state_id, int rate) {
	const RPG::State& state = Data::states[state_id - 1];

	switch (rate) {
	case 0:
		return state.a_rate;
	case 1:
		return state.b_rate;
	case 2:
		return state.c_rate;
	case 3:
		return state.d_rate;
	case 4:
		return state.e_rate;
	default:;
	}

	assert(false && "bad rate");
	return 0;
}

bool Game_Battler::IsSkillUsable(int skill_id) const {
	const RPG::Skill& skill = Data::skills[skill_id - 1];

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
	if (skill.type == RPG::Skill::Type_normal ||
		skill.type >= RPG::Skill::Type_subskill) {
		int scope = skill.scope;

		if (Game_Temp::battle_running) {
			return true;
		}
		else if (scope == RPG::Skill::Scope_self ||
			scope == RPG::Skill::Scope_ally ||
			scope == RPG::Skill::Scope_party) {

			return (skill.affect_hp ||
					skill.affect_sp ||
					skill.state_effect);
		}
	} else if (skill.type == RPG::Skill::Type_switch) {
		if (Game_Temp::battle_running) {
			return skill.occasion_battle;
		}
		else {
			return skill.occasion_field;
		}
	}

	return false;
}

bool Game_Battler::UseItem(int item_id) {
	const RPG::Item& item = Data::items[item_id - 1];

	if (item.type == RPG::Item::Type_medicine) {
		bool was_used;

		int hp_change = item.recover_hp_rate * GetMaxHp() / 100 + item.recover_hp;
		int sp_change = item.recover_sp_rate * GetMaxSp() / 100 + item.recover_sp;

		was_used = hp_change > 0 || sp_change > 0;

		if (IsDead()) {
			// Check if item can revive
			if (item.state_set.empty() || !item.state_set[0]) {
				return false;
			}

			// Revive gives at least 1 Hp
			if (hp_change == 0) {
				ChangeHp(1);
				was_used = true;
			}
		} else if (item.ko_only) {
			// Must be dead
			return false;
		}

		ChangeHp(hp_change);
		SetSp(GetSp() + sp_change);

		for (std::vector<bool>::const_iterator it = item.state_set.begin();
			it != item.state_set.end(); ++it) {
			if (*it) {
				was_used |= HasState(*it);
				RemoveState(*it);
			}
		}

		return was_used;
	} else if (item.type == RPG::Item::Type_switch) {
		return true;
	}

	return false;
}

bool Game_Battler::UseSkill(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];

	switch (skill.type) {
		case RPG::Skill::Type_normal: {
			int effect = skill.power;

			if (skill.variance > 0) {
				int var_perc = skill.variance * 5;
				int act_perc = rand() % (var_perc * 2) - var_perc;
				int change = effect * act_perc / 100;
				effect += change;
			}

			if (skill.affect_hp) {
				ChangeHp(effect);
			}
			if (skill.affect_sp) {
				SetSp(GetSp() + effect);
			}

			// ToDo
			return true;
		}
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
			// ToDo: Show Teleport/Escape target menu
			break;
		case RPG::Skill::Type_switch:
			SetSp(GetSp() - skill.sp_cost);
			Game_Switches[skill.switch_id] = true;
			break;
	}

	return false;
}

int Game_Battler::CalculateSkillCost(int skill_id) const {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return (Player::engine == Player::EngineRpg2k3 &&
			skill.sp_type == RPG::Skill::SpType_percent)
		? GetMaxSp() * skill.sp_percent / 100
		: skill.sp_cost;
}

void Game_Battler::AddState(int state_id) {
	std::vector<int16_t>& states = GetStates();
	if (state_id > 0 && !HasState(state_id)) {
		states.push_back((int16_t)state_id);
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
	return Data::states[state_id - 1].type == RPG::State::Persistence_ends;
}

void Game_Battler::RemoveBattleStates() {
	std::vector<int16_t>& states = GetStates();

	// If death is non-permanent change HP to 1
	if (GetSignificantState() != NULL &&
		GetSignificantState()->ID == 1 &&
		NonPermanent(1)) {
		ChangeHp(1);
	}

	std::vector<int16_t>::iterator end = std::remove_if(states.begin(), states.end(), NonPermanent);
	states.erase(end, states.end());
}

void Game_Battler::RemoveAllStates() {
	std::vector<int16_t>& states = GetStates();
	states.clear();
}

bool Game_Battler::IsCharged() const {
	return charged;
}

void Game_Battler::SetCharged(bool charge) {
	charged = charge;
}

bool Game_Battler::IsDefending() const {
	return defending;
}

void Game_Battler::SetDefending(bool defend) {
	defending = defend;
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
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_attack) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_atk);
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
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_defense) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_def);
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
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_spirit) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_spi);
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
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_agility) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_agi);
			break;
		}
	}

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetHue() const {
	return 0;
}

Game_Party_Base& Game_Battler::GetParty() const {
	if (GetType() == Type_Ally) {
		return *Main_Data::game_party;
	} else {
		return *Main_Data::game_enemyparty;
	}
}

int Game_Battler::GetGauge() const {
	return gauge / (EASYRPG_GAUGE_MAX_VALUE / 100);
}

void Game_Battler::SetGauge(int new_gauge) {
	new_gauge = min(max(new_gauge, 0), 100);

	gauge = new_gauge * (EASYRPG_GAUGE_MAX_VALUE / 100);
}

bool Game_Battler::IsGaugeFull() const {
	return gauge >= EASYRPG_GAUGE_MAX_VALUE;
}

void Game_Battler::UpdateGauge(int multiplier) {
	if (IsDead()) {
		return;
	}

	if (gauge > EASYRPG_GAUGE_MAX_VALUE) {
		return;
	}
	gauge += GetAgi() * multiplier;

	//printf("%s: %.2f\n", GetName().c_str(), ((float)gauge / EASYRPG_GAUGE_MAX_VALUE) * 100);
}

const BattleAlgorithmRef Game_Battler::GetBattleAlgorithm() const {
	return battle_algorithm;
}

void Game_Battler::SetBattleAlgorithm(BattleAlgorithmRef battle_algorithm) {
	this->battle_algorithm = battle_algorithm;
}

void Game_Battler::ResetBattle() {
	gauge = EASYRPG_GAUGE_MAX_VALUE / 2;
	charged = false;
	defending = false;
}
