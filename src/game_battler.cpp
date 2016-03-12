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
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "util_macro.h"
#include "main_data.h"

Game_Battler::Game_Battler() {
	ResetBattle();
}

bool Game_Battler::HasState(int state_id) const {
	const std::vector<int16_t> states = GetInflictedStates();

	return (std::find(states.begin(), states.end(), state_id) != states.end());
}

std::vector<int16_t> Game_Battler::GetInflictedStates() const {
	std::vector<int16_t> states;
	for (size_t i = 0; i < GetStates().size(); ++i) {
		if (GetStates()[i] > 0) {
			states.push_back(Data::states[i].ID);
		}
	}
	return states;
}

int Game_Battler::GetSignificantRestriction() {
	const std::vector<int16_t> states = GetInflictedStates();
	for (int i = 0; i < (int)states.size(); i++) {
		const RPG::State* state = &Data::states[states[i] - 1];
		if (state->restriction != RPG::State::Restriction_normal) {
			return state->restriction;
		}
	}
	return RPG::State::Restriction_normal;
}

bool Game_Battler::CanAct() {
	const std::vector<int16_t> states = GetInflictedStates();
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

	const std::vector<int16_t> states = GetInflictedStates();
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
	if (skill_id <= 0 || skill_id > (int)Data::skills.size()) {
		return false;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	if (CalculateSkillCost(skill_id) > GetSp()) {
		return false;
	}
	
	// > 10 makes any skill usable
	int smallest_physical_rate = 11;
	int smallest_magical_rate = 11;

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator it = states.begin();
		it != states.end(); ++it) {
		const RPG::State& state = Data::states[(*it) - 1];

		if (state.restrict_skill) {
			smallest_physical_rate = std::min(state.restrict_skill_level, smallest_physical_rate);
		}

		if (state.restrict_magic) {
			smallest_magical_rate = std::min(state.restrict_magic_level, smallest_magical_rate);
		}
	}

	if (skill.physical_rate >= smallest_physical_rate) {
		return false;
	}
	if (skill.magical_rate >= smallest_magical_rate) {
		return false;
	}

	return true;
}

bool Game_Battler::UseItem(int item_id) {
	const RPG::Item& item = Data::items[item_id - 1];

	if (item.type == RPG::Item::Type_medicine) {
		bool was_used = false;

		int hp_change = item.recover_hp_rate * GetMaxHp() / 100 + item.recover_hp;
		int sp_change = item.recover_sp_rate * GetMaxSp() / 100 + item.recover_sp;

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

		if (hp_change > 0 && !HasFullHp()) {
			ChangeHp(hp_change);
			was_used = true;
		}

		if (sp_change > 0 && !HasFullSp()) {
			ChangeSp(sp_change);
			was_used = true;
		}

		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				was_used |= HasState(Data::states[i].ID);
				RemoveState(Data::states[i].ID);
			}
		}

		return was_used;
	}
	
	if (item.type == RPG::Item::Type_switch) {
		return true;
	}
	
	switch (item.type) {
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			return item.use_skill && UseSkill(item.skill_id);
		case RPG::Item::Type_special:
			return UseSkill(item.skill_id);
	}

	return false;
}

bool Game_Battler::UseSkill(int skill_id) {
	const RPG::Skill& skill = Data::skills[skill_id - 1];

	bool was_used = false;

	switch (skill.type) {
		case RPG::Skill::Type_normal:
		case RPG::Skill::Type_subskill:
			// Only takes care of healing skills outside of battle,
			// the other skill logic is in Game_BattleAlgorithm

			if (!(skill.scope == RPG::Skill::Scope_ally ||
				skill.scope == RPG::Skill::Scope_party ||
				skill.scope == RPG::Skill::Scope_self)) {
				return false;
			}

			// Skills only increase hp and sp outside of battle
			if (skill.power > 0 && skill.affect_hp && !HasFullHp()) {
				was_used = true;
				ChangeHp(skill.power);
			}

			if (skill.power > 0 && skill.affect_sp && !HasFullSp()) {
				was_used = true;
				ChangeSp(skill.power);
			}

			for (int i = 0; i < (int)skill.state_effects.size(); i++) {
				if (skill.state_effects[i]) {
					if (skill.state_effect) {
						was_used |= !HasState(Data::states[i].ID);
						AddState(Data::states[i].ID);
					} else {
						was_used |= HasState(Data::states[i].ID);
						RemoveState(Data::states[i].ID);
					}
				}
			}
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
			// ToDo: Show Teleport/Escape target menu
			break;
		case RPG::Skill::Type_switch:
			Game_Switches[skill.switch_id] = true;
			return true;
	}

	return was_used;
}

int Game_Battler::CalculateSkillCost(int skill_id) const {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	return (Player::engine == Player::EngineRpg2k3 &&
			skill.sp_type == RPG::Skill::SpType_percent)
		? GetMaxSp() * skill.sp_percent / 100
		: skill.sp_cost;
}

void Game_Battler::SetAtkModifier(int modifier) {
	atk_modifier = modifier;
}

void Game_Battler::SetDefModifier(int modifier) {
	def_modifier = modifier;
}

void Game_Battler::SetSpiModifier(int modifier) {
	spi_modifier = modifier;
}

void Game_Battler::SetAgiModifier(int modifier) {
	agi_modifier = modifier;
}

void Game_Battler::AddState(int state_id) {
	if (state_id <= 0) {
		return;
	}

	std::vector<int16_t>& states = GetStates();
	if (state_id - 1 >= states.size()) {
		states.resize(state_id);
	}

	states[state_id - 1] = 1;
}

void Game_Battler::RemoveState(int state_id) {
	if (state_id <= 0) {
		return;
	}

	std::vector<int16_t>& states = GetStates();
	if (state_id - 1 >= states.size()) {
		return;
	}

	states[state_id - 1] = 0;
}

static bool non_permanent(int state_id) {
	return Data::states[state_id - 1].type == RPG::State::Persistence_ends;
}

int Game_Battler::ApplyConditions()
{
	int damageTaken = 0;
	std::vector<int16_t> inflictedStates = this->GetInflictedStates();
	if (inflictedStates.size() != 0) {
		for (int i = 0; i<inflictedStates.size(); ++i) {
			RPG::State state = Data::states[inflictedStates[i]];
			int hp = state.hp_change_val;
			int sp = state.sp_change_val;
			int source_hp = this->GetHp();
			int source_sp = this->GetSp();
			int src_hp = 0;
			int src_sp = 0;
			if (state.hp_change_type == state.ChangeType_lose) {
				src_hp = -std::min(source_hp + 1, hp);
			}
			else {
				src_hp = std::min(source_hp, hp);
			}
			if (state.sp_change_type == state.ChangeType_lose) {
				 source_sp = -std::min(source_sp, sp);
			}
			else {
				source_sp = std::min(source_sp, sp);
			}
			this->ChangeHp(src_hp);
			this->ChangeSp(src_sp);
			damageTaken += src_hp;
		}
	}
	return damageTaken;
}


void Game_Battler::RemoveBattleStates() {
	std::vector<int16_t>& states = GetStates();

	// If death is non-permanent change HP to 1
	if (IsDead() &&
		non_permanent(1)) {
		ChangeHp(1);
	}

	for (size_t i = 0; i < states.size(); ++i) {
		if (non_permanent(i + 1)) {
			states[i] = 0;
		}
	}
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

void Game_Battler::ChangeHp(int hp) {
	SetHp(GetHp() + hp);
}

int Game_Battler::GetMaxHp() const {
	return GetBaseMaxHp();
}

bool Game_Battler::HasFullHp() const {
	return GetMaxHp() == GetHp();
}

void Game_Battler::ChangeSp(int sp) {
	SetSp(GetSp() + sp);
}

int Game_Battler::GetMaxSp() const {
	return GetBaseMaxSp();
}

bool Game_Battler::HasFullSp() const {
	return GetMaxSp() == GetSp();
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

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_attack) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_atk);
			break;
		}
	}

	n += atk_modifier;

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetDef() const {
	int base_def = GetBaseDef();
	int n = min(max(base_def, 1), 999);

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_defense) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_def);
			break;
		}
	}

	n += def_modifier;

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetSpi() const {
	int base_spi = GetBaseSpi();
	int n = min(max(base_spi, 1), 999);

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_spirit) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_spi);
			break;
		}
	}

	n += spi_modifier;

	n = min(max(n, 1), 999);

	return n;
}

int Game_Battler::GetAgi() const {
	int base_agi = GetBaseAgi();
	int n = min(max(base_agi, 1), 999);

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator i = states.begin(); i != states.end(); ++i) {
		if(Data::states[(*i) - 1].affect_agility) {
			n = AffectParameter(Data::states[(*i) - 1].affect_type, base_agi);
			break;
		}
	}

	n += agi_modifier;

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

int Game_Battler::GetMaxGauge() const {
	return 120000;
}

int Game_Battler::GetGauge() const {
	return gauge / (GetMaxGauge() / 100);
}

void Game_Battler::SetGauge(int new_gauge) {
	new_gauge = min(max(new_gauge, 0), 100);

	gauge = new_gauge * (GetMaxGauge() / 100);
}

bool Game_Battler::IsGaugeFull() const {
	return gauge >= GetMaxGauge();
}

void Game_Battler::UpdateGauge(int multiplier) {
	if (!Exists()) {
		return;
	}

	if (gauge > GetMaxGauge()) {
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

std::vector<int16_t> Game_Battler::NextBattleTurn() {
	++battle_turn;

	std::vector<int16_t> healed_states;
	std::vector<int16_t>& states = GetStates();

	for (size_t i = 0; i < states.size(); ++i) {
		if (HasState(i + 1)) {
			states[i] += 1;

			if (states[i] >= Data::states[i].hold_turn) {
				if (rand() % 100 < Data::states[i].auto_release_prob) {
					healed_states.push_back(i + 1);
					RemoveState(i + 1);
				}
			}
		}
	}

	return healed_states;
}

void Game_Battler::ResetBattle() {
	gauge = GetMaxGauge() / 2;
	charged = false;
	defending = false;
	battle_turn = 0;
	last_battle_action = -1;
	atk_modifier = 0;
	def_modifier = 0;
	spi_modifier = 0;
	agi_modifier = 0;
}

int Game_Battler::GetBattleTurn() const {
	return battle_turn;
}

void Game_Battler::SetLastBattleAction(int battle_action) {
	last_battle_action = battle_action;
}

int Game_Battler::GetLastBattleAction() const {
	return last_battle_action;
}
