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
#include <cassert>
#include <cmath>
#include "player.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "util_macro.h"
#include "main_data.h"
#include "utils.h"
#include "output.h"
#include "reader_util.h"

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
		// States are guaranteed to be valid
		const RPG::State* state = ReaderUtil::GetElement(Data::states, states[i]);
		if (state->restriction != RPG::State::Restriction_normal) {
			return state->restriction;
		}
	}
	return RPG::State::Restriction_normal;
}

bool Game_Battler::CanAct() const {
	const std::vector<int16_t> states = GetInflictedStates();
	for (int i = 0; i < (int)states.size(); i++) {
		// States are guaranteed to be valid
		const RPG::State* state = ReaderUtil::GetElement(Data::states, states[i]);
		if (state->restriction == RPG::State::Restriction_do_nothing) {
			return false;
		}
	}
	return true;
}

bool Game_Battler::IsDead() const {
	return HasState(1);
}

void Game_Battler::Kill() {
	ChangeHp(-GetHp());
}

bool Game_Battler::Exists() const {
	return !IsHidden() && !IsDead();
}

const RPG::State* Game_Battler::GetSignificantState() const {
	int priority = 0;
	const RPG::State* the_state = NULL;

	const std::vector<int16_t> states = GetInflictedStates();
	for (int i = 0; i < (int) states.size(); i++) {
		// States are guaranteed to be valid
		const RPG::State* state = ReaderUtil::GetElement(Data::states, states[i]);
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

int Game_Battler::GetStateRate(int state_id, int rate) const {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);

	if (!state) {
		Output::Warning("GetStateRate: Invalid state ID %d", state_id);
		return 0;
	}

	switch (rate) {
	case 0:
		return state->a_rate;
	case 1:
		return state->b_rate;
	case 2:
		return state->c_rate;
	case 3:
		return state->d_rate;
	case 4:
		return state->e_rate;
	default:;
	}

	assert(false && "bad rate");
	return 0;
}

int Game_Battler::GetAttributeRate(int attribute_id, int rate) const {
	const RPG::Attribute* attribute = ReaderUtil::GetElement(Data::attributes, attribute_id);

	if (!attribute) {
		Output::Warning("GetAttributeRate: Invalid attribute ID %d", attribute_id);
		return 0;
	}

	switch (rate) {
	case 0:
		return attribute->a_rate;
	case 1:
		return attribute->b_rate;
	case 2:
		return attribute->c_rate;
	case 3:
		return attribute->d_rate;
	case 4:
		return attribute->e_rate;
	default:;
	}

	assert(false && "bad rate");
	return 0;
}

bool Game_Battler::IsSkillUsable(int skill_id) const {
	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);

	if (!skill) {
		Output::Warning("IsSkillUsable: Invalid skill ID %d", skill_id);
		return false;
	}

	if (CalculateSkillCost(skill_id) > GetSp()) {
		return false;
	}

	// > 10 makes any skill usable
	int32_t smallest_physical_rate = 11;
	int32_t smallest_magical_rate = 11;

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator it = states.begin();
		it != states.end(); ++it) {
		// States are guaranteed to be valid
		const RPG::State& state = *ReaderUtil::GetElement(Data::states, (*it));

		if (state.restrict_skill) {
			smallest_physical_rate = std::min(state.restrict_skill_level, smallest_physical_rate);
		}

		if (state.restrict_magic) {
			smallest_magical_rate = std::min(state.restrict_magic_level, smallest_magical_rate);
		}
	}

	if (skill->physical_rate >= smallest_physical_rate) {
		return false;
	}
	if (skill->magical_rate >= smallest_magical_rate) {
		return false;
	}

	return true;
}

bool Game_Battler::UseItem(int item_id) {
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use item with invalid ID %d", item_id);
		return false;
	}

	if (item->type == RPG::Item::Type_medicine) {
		bool was_used = false;

		int hp_change = item->recover_hp_rate * GetMaxHp() / 100 + item->recover_hp;
		int sp_change = item->recover_sp_rate * GetMaxSp() / 100 + item->recover_sp;

		if (IsDead()) {
			// Check if item can revive
			if (item->state_set.empty() || !item->state_set[0]) {
				return false;
			}

			// Revive gives at least 1 Hp
			if (hp_change == 0) {
				ChangeHp(1);
				was_used = true;
			}
		} else if (item->ko_only) {
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

		for (int i = 0; i < (int)item->state_set.size(); i++) {
			if (item->state_set[i]) {
				was_used |= HasState(Data::states[i].ID);
				RemoveState(Data::states[i].ID);
			}
		}

		return was_used;
	}

	if (item->type == RPG::Item::Type_switch) {
		return true;
	}

	switch (item->type) {
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			return item->use_skill && UseSkill(item->skill_id);
		case RPG::Item::Type_special:
			return UseSkill(item->skill_id);
	}

	return false;
}

bool Game_Battler::UseSkill(int skill_id) {
	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
	if (!skill) {
		Output::Warning("UseSkill: Can't use skill with invalid ID %d", skill_id);
		return false;
	}

	bool was_used = false;

	if (skill->type == RPG::Skill::Type_normal || skill->type >= RPG::Skill::Type_subskill) {
		// Only takes care of healing skills outside of battle,
		// the other skill logic is in Game_BattleAlgorithm

		if (!(skill->scope == RPG::Skill::Scope_ally ||
			  skill->scope == RPG::Skill::Scope_party ||
			  skill->scope == RPG::Skill::Scope_self)) {
			return false;
		}

		// Skills only increase hp and sp outside of battle
		if (skill->power > 0 && skill->affect_hp && !HasFullHp()) {
			was_used = true;
			ChangeHp(skill->power);
		}

		if (skill->power > 0 && skill->affect_sp && !HasFullSp()) {
			was_used = true;
			ChangeSp(skill->power);
		}

		for (int i = 0; i < (int) skill->state_effects.size(); i++) {
			if (skill->state_effects[i]) {
				if (skill->state_effect) {
					was_used |= !HasState(Data::states[i].ID);
					AddState(Data::states[i].ID);
				} else {
					was_used |= HasState(Data::states[i].ID);
					RemoveState(Data::states[i].ID);
				}
			}
		}
	} else if (skill->type == RPG::Skill::Type_teleport || skill->type == RPG::Skill::Type_escape) {
		was_used = true;
	} else if (skill->type == RPG::Skill::Type_switch) {
		Game_Switches[skill->switch_id] = true;
		was_used = true;
	}

	return was_used;
}

int Game_Battler::CalculateSkillCost(int skill_id) const {
	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
	if (!skill) {
		Output::Warning("CalculateSkillCost: Invalid skill ID %d", skill_id);
		return 0;
	}

	return (Player::engine == Player::EngineRpg2k3 &&
			skill->sp_type == RPG::Skill::SpType_percent)
		? GetMaxSp() * skill->sp_percent / 100
		: skill->sp_cost;
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

void Game_Battler::ChangeAtkModifier(int modifier) {
	SetAtkModifier(atk_modifier + modifier);
}

void Game_Battler::ChangeDefModifier(int modifier) {
	SetDefModifier(def_modifier + modifier);
}

void Game_Battler::ChangeSpiModifier(int modifier) {
	SetSpiModifier(spi_modifier + modifier);
}

void Game_Battler::ChangeAgiModifier(int modifier) {
	SetAgiModifier(agi_modifier + modifier);
}

void Game_Battler::AddState(int state_id) {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
	if (!state) {
		Output::Warning("AddState: Can't add state with invalid ID %d", state_id);
		return;
	}

	std::vector<int16_t>& states = GetStates();
	if (state_id - 1 >= static_cast<int>(states.size())) {
		states.resize(state_id);
	}

	states[state_id - 1] = std::max<int> (1, states[state_id - 1]);
	FilterStatesByPriority();
}

void Game_Battler::FilterStatesByPriority() {
	if (!IsDead()) {
		int new_priority = GetSignificantState() != nullptr? GetSignificantState()->priority : -1;
		for (auto state_id : GetInflictedStates()) {
			if (state_id <= 0 || state_id > Data::states.size()) {
				continue;
			}
			if (ReaderUtil::GetElement(Data::states, state_id)->priority < new_priority - 9) {
				RemoveState(state_id);
			}
		}
	}
}

void Game_Battler::RemoveState(int state_id) {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
	if (!state) {
		Output::Warning("RemoveState: Can't delete state with invalid ID %d", state_id);
		return;
	}

	std::vector<int16_t>& states = GetStates();
	if (state_id - 1 >= static_cast<int>(states.size())) {
		return;
	}

	states[state_id - 1] = 0;
}

int Game_Battler::ApplyConditions() {
	int damageTaken = 0;
	for (int16_t inflicted : GetInflictedStates()) {
		// States are guaranteed to be valid
		RPG::State& state = *ReaderUtil::GetElement(Data::states, inflicted);
		int hp = state.hp_change_val + (int)(std::ceil(GetMaxHp() * state.hp_change_max / 100.0));
		int sp = state.sp_change_val + (int)(std::ceil(GetMaxHp() * state.sp_change_max / 100.0));
		int source_hp = this->GetHp();
		int source_sp = this->GetSp();
		int src_hp = 0;
		int src_sp = 0;
		if (state.hp_change_type == state.ChangeType_lose) {
			src_hp = -std::min(source_hp - 1, hp);
			if(src_hp > 0) {
				src_hp = 0;
			}
		}
		else if(state.hp_change_type == state.ChangeType_gain) {
			src_hp = std::min(source_hp, hp);
			if(src_hp < 0) {
				src_hp = 0;
			}
		}
		else {
			src_hp = 0;
		}
		if (state.sp_change_type == state.ChangeType_lose) {
			src_sp = -std::min(source_sp, sp);
			if(src_sp > 0) {
				src_sp = 0;
			}

		}
		else if (state.sp_change_type == state.ChangeType_gain) {
			src_sp = std::min(source_sp, sp);
			if(src_sp < 0 ) {
				src_sp = 0;
			}
		}
		else {
			src_sp = 0;
		}
		this->ChangeHp(src_hp);
		this->ChangeSp(src_sp);
		damageTaken += src_hp;
	}

	return damageTaken;
}

static bool non_permanent(int state_id) {
	return ReaderUtil::GetElement(Data::states, state_id)->type == RPG::State::Persistence_ends;
}

void Game_Battler::RemoveBattleStates() {
	std::vector<int16_t>& states = GetStates();

	// If death is non-permanent change HP to 1
	if (IsDead() &&
		non_permanent(1)) {
		ChangeHp(1);
	}

	for (size_t i = 0; i < states.size(); ++i) {
		if (i != 0 && (non_permanent(i + 1) || ReaderUtil::GetElement(Data::states, i + 1)->auto_release_prob == 0)) {
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

bool Game_Battler::HasStrongDefense() const {
	return false;
}

bool Game_Battler::HasPreemptiveAttack() const {
	return false;
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

static int AffectParameter(const int type, const int val) {
	return
		type == 0? val / 2 :
		type == 1? val * 2 :
		val;
}

int Game_Battler::GetAtk() const {
	int base_atk = GetBaseAtk();
	int n = min(max(base_atk, 1), MaxStatBaseValue());

	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		const RPG::State& state = *ReaderUtil::GetElement(Data::states, i);
		if (state.affect_attack) {
			n = AffectParameter(state.affect_type, base_atk);
			break;
		}
	}

	n += atk_modifier;

	n = min(max(n, 1), MaxStatBattleValue());

	return n;
}

int Game_Battler::GetDef() const {
	int base_def = GetBaseDef();
	int n = min(max(base_def, 1), MaxStatBaseValue());

	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		const RPG::State& state = *ReaderUtil::GetElement(Data::states, i);
		if (state.affect_defense) {
			n = AffectParameter(state.affect_type, base_def);
			break;
		}
	}

	n += def_modifier;

	n = min(max(n, 1), MaxStatBattleValue());

	return n;
}

int Game_Battler::GetSpi() const {
	int base_spi = GetBaseSpi();
	int n = min(max(base_spi, 1), MaxStatBaseValue());

	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		const RPG::State& state = *ReaderUtil::GetElement(Data::states, i);
		if (state.affect_spirit) {
			n = AffectParameter(state.affect_type, base_spi);
			break;
		}
	}

	n += spi_modifier;

	n = min(max(n, 1), MaxStatBattleValue());

	return n;
}

int Game_Battler::GetAgi() const {
	int base_agi = GetBaseAgi();
	int n = min(max(base_agi, 1), MaxStatBaseValue());

	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		const RPG::State& state = *ReaderUtil::GetElement(Data::states, i);
		if (state.affect_agility) {
			n = AffectParameter(state.affect_type, base_agi);
			break;
		}
	}

	n += agi_modifier;

	n = min(max(n, 1), MaxStatBattleValue());

	return n;
}

int Game_Battler::GetDisplayX() const {
	int shake_pos = Main_Data::game_data.screen.shake_position;
	return GetBattleX() + shake_pos;
}

int Game_Battler::GetDisplayY() const {
	return GetBattleY() + GetFlyingOffset();
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
		if (IsDead()) {
			SetGauge(0);
		}
		return;
	}

	if (gauge > GetMaxGauge()) {
		return;
	}
	gauge += GetAgi() * multiplier;

	//printf("%s: %.2f\n", GetName().c_str(), ((float)gauge / EASYRPG_GAUGE_MAX_VALUE) * 100);
}

int Game_Battler::GetFlyingOffset() const {
	return 0;
}

void Game_Battler::UpdateBattle() {
	// no-op
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
			if (states[i] > Data::states[i].hold_turn + 1) {
				if (Utils::ChanceOf(Data::states[i].auto_release_prob, 100)) {
					healed_states.push_back(i + 1);
					RemoveState(i + 1);
				}
				else {
					states[i] -= Data::states[i].hold_turn;
				}
			}
		}
	}

	return healed_states;
}

std::vector<int16_t> Game_Battler::BattlePhysicalStateHeal(int physical_rate) {
	std::vector<int16_t> healed_states;
	std::vector<int16_t>& states = GetStates();

	if (physical_rate <= 0) {
		return healed_states;
	}

	for (size_t i = 0; i < states.size(); ++i) {
		if (HasState(i + 1) && Data::states[i].release_by_damage > 0) {
			int release_chance = (int)(Data::states[i].release_by_damage * physical_rate / 100.0);

			if (Utils::ChanceOf(release_chance, 100)) {
				healed_states.push_back(i + 1);
			}
		}
	}

	return healed_states;
}

bool Game_Battler::HasReflectState() const {
	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		if (ReaderUtil::GetElement(Data::states, i)->reflect_magic) {
			return true;
		}
	}

	return false;
}

void Game_Battler::ResetBattle() {
	gauge = GetMaxGauge();
	if (!HasPreemptiveAttack()) {
		gauge /= 2;
	}
	charged = false;
	defending = false;
	battle_turn = 0;
	last_battle_action = -1;
	atk_modifier = 0;
	def_modifier = 0;
	spi_modifier = 0;
	agi_modifier = 0;
	battle_combo_command_id = -1;
	battle_combo_times = -1;
	attribute_shift.clear();
	attribute_shift.resize(Data::attributes.size());
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

void Game_Battler::SetBattleCombo(int command_id, int times) {
	battle_combo_command_id = command_id;
	battle_combo_times = times;
}

void Game_Battler::GetBattleCombo(int &command_id, int &times) const {
	command_id = battle_combo_command_id;
	times = battle_combo_times;
}

void Game_Battler::ShiftAttributeRate(int attribute_id, int shift) {
	if (attribute_id < 1 || attribute_id > (int)Data::attributes.size()) {
		assert(false && "invalid attribute_id");
	}

	if (shift < -1 || shift > 1) {
		assert(false && "Invalid shift");
	}

	if (shift == 0) {
		return;
	}

	int& old_shift = attribute_shift[attribute_id - 1];
	if ((old_shift == -1 || old_shift == 0) && shift == 1) {
		++old_shift;
	} else if ((old_shift == 1 || old_shift == 0) && shift == -1) {
		--old_shift;
	}
}

int Game_Battler::GetShiftAttributeRate(int attribute_id) {
	if (attribute_id < 1 || attribute_id >(int)Data::attributes.size()) {
		assert(false && "invalid attribute_id");
	}
	return attribute_shift[attribute_id - 1];
}

void Game_Battler::SetRandomOrderAgi() {
	agi_order = GetAgi() + Utils::GetRandomNumber(std::ceil(GetAgi() * -10 / 100.0) - 1, std::ceil(GetAgi() * 10 / 100.0) + 1);
}

int Game_Battler::GetRandomOrderAgi() {
	return agi_order;
}