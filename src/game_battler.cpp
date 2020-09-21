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
#define _USE_MATH_DEFINES
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include "player.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemyparty.h"
#include "game_battle.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_targets.h"
#include "game_screen.h"
#include "util_macro.h"
#include "main_data.h"
#include "utils.h"
#include "output.h"
#include <lcf/reader_util.h>
#include "game_battlealgorithm.h"
#include "state.h"
#include "shake.h"
#include "attribute.h"
#include "algo.h"
#include "rand.h"

Game_Battler::Game_Battler() {
	ResetBattle();
}

bool Game_Battler::HasState(int state_id) const {
	return State::Has(state_id, GetStates());
}

std::vector<int16_t> Game_Battler::GetInflictedStates() const {
	auto& states = GetStates();
	std::vector<int16_t> inf_states;
	for (size_t i = 0; i < states.size(); ++i) {
		if (states[i] > 0) {
			inf_states.push_back(i + 1);
		}
	}
	return inf_states;
}

PermanentStates Game_Battler::GetPermanentStates() const {
	return PermanentStates();
}

bool Game_Battler::EvadesAllPhysicalAttacks() const {
	for (auto state_id: GetInflictedStates()) {
		auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);
		if (state && state->avoid_attacks) {
			return true;
		}
	}
	return false;
}

lcf::rpg::State::Restriction Game_Battler::GetSignificantRestriction() const {
	return State::GetSignificantRestriction(GetStates());
}

bool Game_Battler::CanAct() const {
	const auto& states = GetStates();
	for (size_t i = 0; i < states.size(); ++i) {
		if (states[i] > 0) {
			const auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, i + 1);
			assert(state);
			if (state->restriction == lcf::rpg::State::Restriction_do_nothing) {
				return false;
			}
		}
	}
	return true;
}

bool Game_Battler::CanActOrRecoverable() const {
	const auto& states = GetStates();
	for (size_t i = 0; i < states.size(); ++i) {
		if (states[i] > 0) {
			const auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, i + 1);
			assert(state);
			if (state->restriction == lcf::rpg::State::Restriction_do_nothing && state->auto_release_prob == 0) {
				return false;
			}
		}
	}
	return true;
}

const lcf::rpg::State* Game_Battler::GetSignificantState() const {
	return State::GetSignificantState(GetStates());
}

int Game_Battler::GetStateRate(int state_id, int rate) const {
	return State::GetStateRate(state_id, rate);
}

bool Game_Battler::IsSkillUsable(int skill_id) const {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);

	if (!skill) {
		Output::Warning("IsSkillUsable: Invalid skill ID {}", skill_id);
		return false;
	}

	if (CalculateSkillCost(skill_id) > GetSp()) {
		return false;
	}

	if (skill->type == lcf::rpg::Skill::Type_escape) {
		return !Game_Battle::IsBattleRunning() && Main_Data::game_system->GetAllowEscape() && Main_Data::game_targets->HasEscapeTarget();
	}

	if (skill->type == lcf::rpg::Skill::Type_teleport) {
		return !Game_Battle::IsBattleRunning() && Main_Data::game_system->GetAllowTeleport() && Main_Data::game_targets->HasTeleportTargets();
	}

	if (skill->type == lcf::rpg::Skill::Type_switch) {
		if (Game_Battle::IsBattleRunning()) {
			return skill->occasion_battle;
		} else {
			return skill->occasion_field;
		}
	}

	// > 10 makes any skill usable
	int32_t smallest_physical_rate = 11;
	int32_t smallest_magical_rate = 11;

	const std::vector<int16_t> states = GetInflictedStates();
	for (std::vector<int16_t>::const_iterator it = states.begin();
		it != states.end(); ++it) {
		// States are guaranteed to be valid
		const lcf::rpg::State& state = *lcf::ReaderUtil::GetElement(lcf::Data::states, (*it));

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

bool Game_Battler::UseItem(int item_id, const Game_Battler* source) {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use item with invalid ID {}", item_id);
		return false;
	}

	if (item->type == lcf::rpg::Item::Type_medicine) {
		bool was_used = false;
		int revived = 0;
		int hp_change = item->recover_hp_rate * GetMaxHp() / 100 + item->recover_hp;
		int sp_change = item->recover_sp_rate * GetMaxSp() / 100 + item->recover_sp;

		if (IsDead()) {
			// Check if item can revive
			if (item->state_set.empty() || !item->state_set[0]) {
				return false;
			}
		} else if (item->ko_only) {
			// Must be dead
			return false;
		}

		for (int i = 0; i < (int)item->state_set.size(); i++) {
			if (item->state_set[i]) {
				was_used |= HasState(lcf::Data::states[i].ID);
				if (i == 0 && HasState(i + 1))
					revived = 1;
				RemoveState(lcf::Data::states[i].ID, false);
			}
		}

		if (hp_change > 0 && !HasFullHp()) {
			ChangeHp(hp_change - revived, false);
			was_used = true;
		}

		if (sp_change > 0 && !HasFullSp()) {
			ChangeSp(sp_change);
			was_used = true;
		}

		return was_used;
	}

	if (item->type == lcf::rpg::Item::Type_switch) {
		return true;
	}

	bool do_skill = (item->type == lcf::rpg::Item::Type_special)
		|| (item->use_skill && (
				item->type == lcf::rpg::Item::Type_weapon
				|| item->type == lcf::rpg::Item::Type_shield
				|| item->type == lcf::rpg::Item::Type_armor
				|| item->type == lcf::rpg::Item::Type_helmet
				|| item->type == lcf::rpg::Item::Type_accessory
				)
				);

	if (do_skill) {
		auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
		if (skill == nullptr) {
			Output::Warning("UseItem: Can't use item {} skill with invalid ID {}", item->ID, item->skill_id);
			return false;
		}
		return UseSkill(item->skill_id, source);
	}

	return false;
}

bool Game_Battler::UseSkill(int skill_id, const Game_Battler* source) {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("UseSkill: Can't use skill with invalid ID {}", skill_id);
		return false;
	}

	bool cure_hp_percentage = false;
	bool was_used = false;
	int revived = 0;

	if (skill->type == lcf::rpg::Skill::Type_normal || skill->type >= lcf::rpg::Skill::Type_subskill) {
		// Only takes care of healing skills outside of battle,
		// the other skill logic is in Game_BattleAlgorithm

		if (!(skill->scope == lcf::rpg::Skill::Scope_ally ||
			  skill->scope == lcf::rpg::Skill::Scope_party ||
			  skill->scope == lcf::rpg::Skill::Scope_self)) {
			return false;
		}

		// Calculate effect:
		auto effect = Algo::CalcSkillEffect(*source, *this, *skill, true);

		// Negative attributes do damage but cannot kill
		bool negative_effect = false;
		if (effect < 0) {
			negative_effect = true;
			effect = -effect;
		}

		// Cure states
		for (int i = 0; i < (int)skill->state_effects.size(); i++) {
			if (skill->state_effects[i]) {
				if (skill->reverse_state_effect) {
					was_used |= !HasState(lcf::Data::states[i].ID);
					AddState(lcf::Data::states[i].ID, true);
				}
				else {
					if (i == 0 && IsDead()) {
						revived = 1;
					}

					was_used |= HasState(lcf::Data::states[i].ID);
					RemoveState(lcf::Data::states[i].ID, false);

					// If Death is cured and HP is not selected, we set a bool so it later heals HP percentage
					if (i == 0 && !skill->affect_hp && revived) {
						cure_hp_percentage = true;
					}
				}
			}
		}

		// Skills only increase hp and sp outside of battle
		if (!negative_effect) {
			if (effect > 0 && skill->affect_hp && !HasFullHp() && !IsDead()) {
				was_used = true;
				ChangeHp(effect - revived, false);
			}
			else if (effect > 0 && cure_hp_percentage) {
				was_used = true;
				ChangeHp(GetMaxHp() * effect / 100 - revived, false);
			}

			if (effect > 0 && skill->affect_sp && !HasFullSp() && !IsDead()) {
				was_used = true;
				ChangeSp(effect);
			}
		} else {
			if (effect > 0 && skill->affect_hp && !IsDead()) {
				was_used = true;
				ChangeHp(-effect, false);
			}

			if (effect > 0 && skill->affect_sp && !IsDead()) {
				was_used = true;
				ChangeSp(-effect);
			}
		}

	} else if (skill->type == lcf::rpg::Skill::Type_teleport || skill->type == lcf::rpg::Skill::Type_escape) {
		Main_Data::game_system->SePlay(skill->sound_effect);
		was_used = true;
	} else if (skill->type == lcf::rpg::Skill::Type_switch) {
		Main_Data::game_system->SePlay(skill->sound_effect);
		Main_Data::game_switches->Set(skill->switch_id, true);
		was_used = true;
	}

	return was_used;
}

int Game_Battler::CalculateSkillCost(int skill_id) const {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("CalculateSkillCost: Invalid skill ID {}", skill_id);
		return 0;
	}

	return (Player::IsRPG2k3() && skill->sp_type == lcf::rpg::Skill::SpType_percent)
		? GetMaxSp() * skill->sp_percent / 100
		: skill->sp_cost;
}

bool Game_Battler::AddState(int state_id, bool allow_battle_states) {
	auto was_added = State::Add(state_id, GetStates(), GetPermanentStates(), allow_battle_states);

	if (!was_added) {
		return was_added;
	}

	if (state_id == lcf::rpg::State::kDeathID) {
		SetAtbGauge(0);
		SetHp(0);
		SetAtkModifier(0);
		SetDefModifier(0);
		SetSpiModifier(0);
		SetAgiModifier(0);
		SetIsDefending(false);
		SetCharged(false);
		attribute_shift.clear();
		attribute_shift.resize(lcf::Data::attributes.size());
	}

	if (GetSignificantRestriction() != lcf::rpg::State::Restriction_normal) {
		SetIsDefending(false);
		SetCharged(false);
	}

	return was_added;
}

bool Game_Battler::RemoveState(int state_id, bool always_remove_battle_states) {
	PermanentStates ps;

	auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);

	if (!(always_remove_battle_states && state && state->type == lcf::rpg::State::Persistence_ends)) {
		ps = GetPermanentStates();
	}

	auto was_removed = State::Remove(state_id, GetStates(), ps);

	if (was_removed && state_id == lcf::rpg::State::kDeathID) {
		SetHp(1);
	}

	return was_removed;
}

int Game_Battler::ApplyConditions() {
	int damageTaken = 0;
	for (int16_t inflicted : GetInflictedStates()) {
		// States are guaranteed to be valid
		lcf::rpg::State& state = *lcf::ReaderUtil::GetElement(lcf::Data::states, inflicted);
		int hp = state.hp_change_val + (GetMaxHp() * state.hp_change_max / 100);
		int sp = state.sp_change_val + (GetMaxSp() * state.sp_change_max / 100);
		int src_hp = 0;
		int src_sp = 0;
		if (state.hp_change_type == state.ChangeType_lose) {
			src_hp = -hp;
			if(src_hp > 0) {
				src_hp = 0;
			}
		}
		else if(state.hp_change_type == state.ChangeType_gain) {
			src_hp = hp;
			if(src_hp < 0) {
				src_hp = 0;
			}
		}
		else {
			src_hp = 0;
		}
		if (state.sp_change_type == state.ChangeType_lose) {
			src_sp = -sp;
			if(src_sp > 0) {
				src_sp = 0;
			}

		}
		else if (state.sp_change_type == state.ChangeType_gain) {
			src_sp = sp;
			if(src_sp < 0) {
				src_sp = 0;
			}
		}
		else {
			src_sp = 0;
		}
		this->ChangeHp(src_hp, false);
		this->ChangeSp(src_sp);
		damageTaken += src_hp;
	}

	return damageTaken;
}

void Game_Battler::RemoveBattleStates() {
	State::RemoveAllBattle(GetStates(), GetPermanentStates());
}

void Game_Battler::RemoveAllStates() {
	State::RemoveAll(GetStates(), GetPermanentStates());
}

int Game_Battler::ChangeHp(int hp, bool lethal) {
	if (IsDead()) {
		return 0;
	}
	const auto prev_hp = GetHp();
	auto req_new_hp = prev_hp + hp;
	if (!lethal) {
		req_new_hp = std::max(1, req_new_hp);
	}
	auto new_hp = SetHp(req_new_hp);

	// Death
	if (new_hp <= 0) {
		AddState(lcf::rpg::State::kDeathID, true);
	}
	return new_hp - prev_hp;
}

int Game_Battler::GetMaxHp() const {
	return GetBaseMaxHp();
}

bool Game_Battler::HasFullHp() const {
	return GetMaxHp() == GetHp();
}

int Game_Battler::ChangeSp(int sp) {
	const auto prev_sp = GetSp();
	const auto new_sp = SetSp(prev_sp + sp);
	return new_sp - prev_sp;
}

int Game_Battler::GetMaxSp() const {
	return GetBaseMaxSp();
}

bool Game_Battler::HasFullSp() const {
	return GetMaxSp() == GetSp();
}

static int AdjustParam(int base, int mod, int maxval, Span<const int16_t> states, bool lcf::rpg::State::*adj) {
	auto value = Utils::Clamp(base + mod, 1, maxval);
	bool half = false;
	bool dbl = false;
	for (auto i: states) {
		const auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, i);
		assert(state);
		if (state->*adj) {
			half |= (state->affect_type == lcf::rpg::State::AffectType_half);
			dbl |= (state->affect_type == lcf::rpg::State::AffectType_double);
		}
	}
	if (dbl != half) {
		if (dbl) {
			value *= 2;
		} else {
			value = std::max(1, value / 2);
		}
	}
	// NOTE: RPG_RT does not clamp these values to the upper range!
	// Exceptions:
	// * 2k3 special function which computes atk for individual weapons / dual wield dmg does clamp at the end
	// * 2k3 special function which computes agi for individual weapons / dual wield hit ratio does clamp, but also has a bug where it ignores states which modify agi!
	return value;
}

int Game_Battler::GetAtk(Weapon weapon) const {
	return AdjustParam(GetBaseAtk(weapon), atk_modifier, MaxStatBattleValue(), GetInflictedStates(), &lcf::rpg::State::affect_attack);
}

int Game_Battler::GetDef(Weapon weapon) const {
	return AdjustParam(GetBaseDef(weapon), def_modifier, MaxStatBattleValue(), GetInflictedStates(), &lcf::rpg::State::affect_defense);
}

int Game_Battler::GetSpi(Weapon weapon) const {
	return AdjustParam(GetBaseSpi(weapon), spi_modifier, MaxStatBattleValue(), GetInflictedStates(), &lcf::rpg::State::affect_spirit);
}

int Game_Battler::GetAgi(Weapon weapon) const {
	return AdjustParam(GetBaseAgi(weapon), agi_modifier, MaxStatBattleValue(), GetInflictedStates(), &lcf::rpg::State::affect_agility);
}

int Game_Battler::GetDisplayX() const {
	int shake_pos = Main_Data::game_screen->GetShakeOffsetX() + shake.position;
	return (GetBattlePosition().x + shake_pos) * SCREEN_TARGET_WIDTH / 320;
}

int Game_Battler::GetDisplayY() const {
	int shake_pos = Main_Data::game_screen->GetShakeOffsetY();
	return (GetBattlePosition().y + GetFlyingOffset() + shake_pos) * SCREEN_TARGET_HEIGHT / 240;
}

Game_Party_Base& Game_Battler::GetParty() const {
	if (GetType() == Type_Ally) {
		return *Main_Data::game_party;
	} else {
		return *Main_Data::game_enemyparty;
	}
}

void Game_Battler::UpdateBattle() {
	Shake::Update(shake.position, shake.time_left, shake.strength, shake.speed, false);
	Flash::Update(flash.current_level, flash.time_left);
}

std::vector<int16_t> Game_Battler::BattleStateHeal() {
	std::vector<int16_t> healed_states;
	std::vector<int16_t>& states = GetStates();

	for (size_t i = 0; i < states.size(); ++i) {
		if (HasState(i + 1)) {
			if (states[i] > lcf::Data::states[i].hold_turn
					&& Rand::ChanceOf(lcf::Data::states[i].auto_release_prob, 100)
					&& RemoveState(i + 1, false)
					) {
				healed_states.push_back(i + 1);
			} else {
				++states[i];
			}
		}
	}

	return healed_states;
}

bool Game_Battler::HasReflectState() const {
	for (int16_t i : GetInflictedStates()) {
		// States are guaranteed to be valid
		if (lcf::ReaderUtil::GetElement(lcf::Data::states, i)->reflect_magic) {
			return true;
		}
	}

	return false;
}

void Game_Battler::ResetBattle() {
	// Note: ATB gauge is not reset here. This is on purpose because RPG_RT will freeze
	// the gauge and carry it between battles if !CanActOrRecoverable().
	SetCharged(false);
	SetIsDefending(false);
	SetHidden(false);
	SetDirectionFlipped(false);
	battle_turn = 0;
	last_battle_action = -1;
	atk_modifier = 0;
	def_modifier = 0;
	spi_modifier = 0;
	agi_modifier = 0;
	battle_combo_command_id = -1;
	battle_combo_times = -1;
	attribute_shift.clear();
	attribute_shift.resize(lcf::Data::attributes.size());
	SetBattleAlgorithm(nullptr);
}

int Game_Battler::GetAttributeRate(int attribute_id) const {
	auto rate = GetBaseAttributeRate(attribute_id);
	rate += GetAttributeRateShift(attribute_id);
	return Utils::Clamp(rate, 0, 4);
}

void Game_Battler::ShiftAttributeRate(int attribute_id, int shift) {
	if (attribute_id < 1 || attribute_id > static_cast<int>(lcf::Data::attributes.size())) {
		return;
	}

	auto& a = attribute_shift[attribute_id -1];
	a = Utils::Clamp(a + shift, -1, 1);
}

int Game_Battler::GetAttributeRateShift(int attribute_id) const {
	if (attribute_id < 1 || attribute_id > static_cast<int>(lcf::Data::attributes.size())) {
		return 0;
	}
	return attribute_shift[attribute_id - 1];
}

bool Game_Battler::CanShiftAttributeRate(int attribute_id, int shift) const {
	if (attribute_id < 1 || attribute_id > static_cast<int>(lcf::Data::attributes.size())) {
		return false;
	}
	auto new_shift = attribute_shift[attribute_id - 1] + shift;
	return new_shift >= -1 && new_shift <= 1;
}

int Game_Battler::GetHitChanceModifierFromStates() const {
	int modifier = 100;
	// Modify hit chance for each state the source has
	for (const auto id : GetInflictedStates()) {
		auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, id);
		if (state) {
			modifier = std::min<int>(modifier, state->reduce_hit_ratio);
		}
	}
	return modifier;
}

void Game_Battler::ShakeOnce(int strength, int speed, int frames) {
	shake.strength = strength;
	shake.speed = speed;
	shake.time_left = frames;
	// FIXME: RPG_RT doesn't reset position for screen shake. So we guess? it doesn't do so here either.
}

void Game_Battler::Flash(int r, int g, int b, int power, int frames) {
	flash.red = r;
	flash.green = g;
	flash.blue = b;
	flash.current_level = power;
	flash.time_left = frames;
}

int Game_Battler::ChangeAtkModifier(int modifier) {
	const auto prev = atk_modifier;
	const auto base = GetBaseAtk();
	SetAtkModifier(Utils::Clamp(atk_modifier + modifier, -base / 2, base));
	return atk_modifier - prev;
}

int Game_Battler::ChangeDefModifier(int modifier) {
	const auto prev = def_modifier;
	const auto base = GetBaseDef();
	SetDefModifier(Utils::Clamp(def_modifier + modifier, -base / 2, base));
	return def_modifier - prev;
}

int Game_Battler::ChangeSpiModifier(int modifier) {
	const auto prev = spi_modifier;
	const auto base = GetBaseSpi();
	SetSpiModifier(Utils::Clamp(spi_modifier + modifier, -base / 2, base));
	return spi_modifier - prev;
}

int Game_Battler::ChangeAgiModifier(int modifier) {
	const auto prev = agi_modifier;
	const auto base = GetBaseAgi();
	SetAgiModifier(Utils::Clamp(agi_modifier + modifier, -base / 2, base));
	return agi_modifier - prev;
}

