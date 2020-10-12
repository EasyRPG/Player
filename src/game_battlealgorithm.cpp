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

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include "game_actor.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "game_system.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <lcf/rpg/animation.h>
#include <lcf/rpg/state.h>
#include <lcf/rpg/skill.h>
#include <lcf/rpg/item.h>
#include "sprite_battler.h"
#include "utils.h"
#include "rand.h"
#include "state.h"
#include "algo.h"
#include "attribute.h"

static inline int MaxDamageValue() {
	return Player::IsRPG2k() ? 999 : 9999;
}

static void BattlePhysicalStateHeal(int physical_rate, std::vector<int16_t>& target_states, const PermanentStates& ps, std::vector<Game_BattleAlgorithm::StateEffect>& states) {
	if (physical_rate <= 0) {
		return;
	}

	for (int i = 0; i < (int)target_states.size(); ++i) {
		auto state_id = i + 1;
		if (!State::Has(state_id, target_states)) {
			continue;
		}

		auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);
		if (state == nullptr) {
			continue;
		}
		if (state->release_by_damage > 0) {
			int release_chance = state->release_by_damage * physical_rate / 100;

			if (!Rand::ChanceOf(release_chance, 100)) {
				continue;
			}

			if (State::Remove(state_id, target_states, ps)) {
				states.push_back(Game_BattleAlgorithm::StateEffect(state_id, Game_BattleAlgorithm::StateEffect::HealedByAttack));
			}
		}
	}
}



Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source) :
	type(ty), source(source), no_target(true), first_attack(true),
	source_restriction(lcf::rpg::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	source->SetIsDefending(false);
	physical_charged = source->IsCharged();
	source->SetCharged(false);

	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Battler* target) :
	type(ty), source(source), no_target(false), first_attack(true),
	source_restriction(lcf::rpg::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	source->SetIsDefending(false);
	physical_charged = source->IsCharged();
	source->SetCharged(false);

	SetTarget(target);
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Party_Base* target) :
	type(ty), source(source), no_target(false), first_attack(true),
	source_restriction(lcf::rpg::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	source->SetIsDefending(false);
	physical_charged = source->IsCharged();
	source->SetCharged(false);

	current_target = targets.end();
	party_target = target;
}

void Game_BattleAlgorithm::AlgorithmBase::Reset() {
	hp = -1;
	sp = -1;
	attack = -1;
	defense = -1;
	spirit = -1;
	agility = -1;
	switch_id = -1;
	healing = false;
	negative_effect = false;
	success = false;
	lethal = false;
	killed_by_dmg = false;
	critical_hit = false;
	absorb = false;
	revived = false;
	reflect = -1;
	states.clear();
	shift_attributes.clear();

	if (!IsFirstAttack()) {
		switch_on.clear();
		switch_off.clear();
	}
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedHp() const {
	return hp;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSp() const {
	return sp;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAttack() const {
	return attack;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedDefense() const {
	return defense;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSpirit() const {
	return spirit;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAgility() const {
	return agility;
}

const std::vector<int16_t>& Game_BattleAlgorithm::AlgorithmBase::GetShiftedAttributes() const {
	return shift_attributes;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSwitch() const {
	return switch_id;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsPositive() const {
	return healing;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorb() const {
	return absorb;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsRevived() const {
	return revived;
}

bool Game_BattleAlgorithm::AlgorithmBase::ActionIsPossible() const {
	return true;
}

const lcf::rpg::Animation* Game_BattleAlgorithm::AlgorithmBase::GetAnimation() const {
	return animation;
}

const lcf::rpg::Animation* Game_BattleAlgorithm::AlgorithmBase::GetSecondAnimation() const {
	return animation2;
}

void Game_BattleAlgorithm::AlgorithmBase::PlayAnimation(bool on_original_targets, bool invert) {
	if (current_target == targets.end() || !GetAnimation()) {
		return;
	}

	if (on_original_targets) {
		std::vector<Game_Battler*> anim_original_targets;
		for (Game_Battler* original_target : original_targets) {
			if (original_target->Exists()) anim_original_targets.push_back(original_target);
		}

		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, anim_original_targets, false, -1, invert);
		has_animation_played = true;
		return;
	}

	auto old_current_target = current_target;
	bool old_first_attack = first_attack;

	std::vector<Game_Battler*> anim_targets;

	do {
		anim_targets.push_back(*current_target);
	} while (TargetNextInternal());

	Game_Battle::ShowBattleAnimation(GetAnimation()->ID, anim_targets, false, -1, invert);
	has_animation_played = true;

	current_target = old_current_target;
	first_attack = old_first_attack;
}

void Game_BattleAlgorithm::AlgorithmBase::PlaySecondAnimation(bool on_original_targets, bool invert) {
	if (current_target == targets.end() || !GetSecondAnimation()) {
		return;
	}

	if (on_original_targets) {
		std::vector<Game_Battler*> anim_original_targets;
		for (Game_Battler* original_target : original_targets) {
			if (original_target->Exists()) anim_original_targets.push_back(original_target);
		}

		Game_Battle::ShowBattleAnimation(GetSecondAnimation()->ID, anim_original_targets, false, -1, invert);
		has_animation2_played = true;
		return;
	}

	auto old_current_target = current_target;
	bool old_first_attack = first_attack;

	std::vector<Game_Battler*> anim_targets;

	do {
		anim_targets.push_back(*current_target);
	} while (TargetNextInternal());

	Game_Battle::ShowBattleAnimation(GetSecondAnimation()->ID, anim_targets, false, -1, invert);
	has_animation2_played = true;

	current_target = old_current_target;
	first_attack = old_first_attack;
}

void Game_BattleAlgorithm::AlgorithmBase::PlaySoundAnimation(bool on_original_targets, int cutoff) {
	if (current_target == targets.end() || !GetAnimation()) {
		return;
	}

	if (on_original_targets) {
		std::vector<Game_Battler*> anim_original_targets;
		for (Game_Battler* original_target : original_targets) {
			if (original_target->Exists()) anim_original_targets.push_back(original_target);
		}

		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, anim_original_targets, true, cutoff);
		return;
	}

	auto old_current_target = current_target;
	bool old_first_attack = first_attack;

	std::vector<Game_Battler*> anim_targets;

	do {
		anim_targets.push_back(*current_target);
	} while (TargetNextInternal());

	Game_Battle::ShowBattleAnimation(
		GetAnimation()->ID,
		anim_targets, true, cutoff);

	current_target = old_current_target;
	first_attack = old_first_attack;
}

bool Game_BattleAlgorithm::AlgorithmBase::HasAnimationPlayed() const {
	return has_animation_played;
}

bool Game_BattleAlgorithm::AlgorithmBase::HasSecondAnimationPlayed() const {
	return has_animation2_played;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsSuccess() const {
	return success;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsLethal() const {
	return lethal;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsKilledByDamage() const {
	return killed_by_dmg;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsCriticalHit() const {
	return critical_hit;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsFirstAttack() const {
	return first_attack;
}

bool Game_BattleAlgorithm::AlgorithmBase::HasSecondStartMessage() const {
	return false;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetSecondStartMessage() const {
	return "";
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetDeathMessage() const {
	if (current_target == targets.end()) {
		return "";
	}

	bool is_ally = GetTarget()->GetType() == Game_Battler::Type_Ally;
	const lcf::rpg::State* state = lcf::ReaderUtil::GetElement(lcf::Data::states, 1);
	StringView message = is_ally ? StringView(state->message_actor) : StringView(state->message_enemy);

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(GetTarget()->GetName())
		);
	}
	else {
		return ToString(GetTarget()->GetName()) + ToString(message);
	}
}

lcf::rpg::State::Restriction Game_BattleAlgorithm::AlgorithmBase::GetSourceRestrictionWhenStarted() const {
	return source_restriction;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetAttackFailureMessage(StringView message) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(GetSource()->GetName(), GetTarget()->GetName())
		);
	}
	else {
		return ToString(GetTarget()->GetName()) + ToString(message);
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetHpSpRecoveredMessage(int value, StringView points) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.hp_recovery,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(GetTarget()->GetName(), std::to_string(value), points)
		);
	}
	else {
		std::stringstream ss;
		std::string particle, particle2, space = "";

		ss << GetTarget()->GetName();
		if (Player::IsCP932()) {
			particle = "の";
			particle2 = "が ";
			space += " ";
		}
		else {
			particle = particle2 = " ";
		}
		ss << particle << points << particle2;
		ss << value << space << lcf::Data::terms.hp_recovery;
		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetUndamagedMessage() const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_undamaged)
		: StringView(lcf::Data::terms.enemy_undamaged);

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(GetTarget()->GetName())
		);
	}
	else {
		return ToString(GetTarget()->GetName()) + ToString(message);
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetCriticalHitMessage() const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_critical)
		: StringView(lcf::Data::terms.enemy_critical);

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(GetSource()->GetName(), GetTarget()->GetName())
		);
	}
	else {
		return ToString(message);
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetHpSpAbsorbedMessage(int value, StringView points) const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_hp_absorbed)
		: StringView(lcf::Data::terms.enemy_hp_absorbed);

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O', 'V', 'U'),
			Utils::MakeSvArray(GetSource()->GetName(), GetTarget()->GetName(), std::to_string(value), points)
		);
	}
	else {
		std::stringstream ss;
		std::string particle, particle2, space = "";

		ss << GetTarget()->GetName();

		if (Player::IsCP932()) {
			particle = (target_is_ally ? "は" : "の");
			particle2 = "を ";
			space += " ";
		} else {
			particle = particle2 = " ";
		}
		ss << particle << points << particle2;
		ss << value << space << message;

		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetDamagedMessage(int value) const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_damaged)
		: StringView(lcf::Data::terms.enemy_damaged);

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(GetTarget()->GetName(), std::to_string(value), lcf::Data::terms.health_points)
		);
	}
	else {
		std::stringstream ss;
		std::string particle, space = "";
		ss << GetTarget()->GetName();

		if (Player::IsCP932()) {
			particle = (target_is_ally ? "は " : "に ");
			space += " ";
		} else {
			particle = " ";
		}
		ss << particle << value << space << message;
		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetParameterChangeMessage(int value, StringView points) const {
	const bool is_positive = (value >= 0);
	value = std::abs(value);
	if (value == 0) {
		return "";
	}

	StringView message = is_positive
		? StringView(lcf::Data::terms.parameter_increase)
	   	: StringView(lcf::Data::terms.parameter_decrease);


	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(GetTarget()->GetName(), std::to_string(value), points)
		);
	}
	else {
		std::stringstream ss;
		std::string particle, particle2, space = "";
		ss << GetTarget()->GetName();

		if (Player::IsCP932()) {
			particle = "の";
			particle2 = "が ";
			space += " ";
		}
		else {
			particle = particle2 = " ";
		}
		ss << particle << points << particle2 << value << space;
		ss << message;

		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetStateMessage(StringView message) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(GetTarget()->GetName())
		);
	}
	else {
		return ToString(GetTarget()->GetName()) + ToString(message);
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetAttributeShiftMessage(int value, StringView attribute) const {
	const bool is_positive = (value >= 0);
	value = std::abs(value);
	if (value == 0) {
		return "";
	}
	StringView message = is_positive
		? StringView(lcf::Data::terms.resistance_increase)
		: StringView(lcf::Data::terms.resistance_decrease);
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(GetTarget()->GetName(), attribute)
		);
	}
	else {
		std::string particle, space = "";
		ss << GetTarget()->GetName();

		if (Player::IsCP932()) {
			particle = "は";
			space += " ";
		}
		else {
			particle = " ";
		}
		ss << particle << attribute << space;
		ss << message;

		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetFailureMessage() const {
	return GetAttackFailureMessage(lcf::Data::terms.dodge);
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetSource() const {
	return source;
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetTarget() const {
	if (current_target == targets.end()) {
		return NULL;
	}

	return *current_target;
}

void Game_BattleAlgorithm::AlgorithmBase::SetTarget(Game_Battler* target) {
	targets.clear();

	if (target) {
		targets.push_back(target);
		current_target = targets.begin();
	}
	else {
		// Set target is invalid
		current_target = targets.end();
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyInitialEffect() {
	for (int s : switch_on) {
		Main_Data::game_switches->Set(s, true);
	}

	for (int s : switch_off) {
		Main_Data::game_switches->Set(s, false);
	}

	if (success && GetAffectedSwitch() != -1) {
		Main_Data::game_switches->Set(GetAffectedSwitch(), true);
	}
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyHpEffect() {
	auto* target = GetTarget();
	if (!target) {
		return 0;
	}

	bool was_dead = target->IsDead();

	if (GetAffectedHp() != -1 && !was_dead) {
		int hp = GetAffectedHp();
		hp = IsPositive() ? hp : -hp;
		hp = target->ChangeHp(hp, true);
		if (IsAbsorb() && !IsPositive()) {
			// Only absorb the hp that were left
			source->ChangeHp(-hp, false);
		}
		return hp;
	}
	return 0;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplySpEffect() {
	auto* target = GetTarget();
	if (target && GetAffectedSp() != -1) {
		int sp = GetAffectedSp();
		sp = IsPositive() ? sp : -sp;
		sp = target->ChangeSp(sp);
		if (IsAbsorb() && !IsPositive() && !IsKilledByDamage()) {
			// Only absorb the sp that were left
			source->ChangeSp(-sp);
		}
		return sp;
	}
	return 0;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAtkEffect() {
	auto* target = GetTarget();
	if (target && GetAffectedAttack() != -1) {
		int atk = GetAffectedAttack();
		target->ChangeAtkModifier(IsPositive() ? atk : -atk);
		return atk;
	}
	return 0;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyDefEffect() {
	auto* target = GetTarget();
	if (target && GetAffectedDefense() != -1) {
		int def = GetAffectedDefense();
		target->ChangeDefModifier(IsPositive() ? def : -def);
		return def;
	}
	return 0;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplySpiEffect() {
	auto* target = GetTarget();
	if (target && GetAffectedSpirit() != -1) {
		int spi = GetAffectedSpirit();
		target->ChangeSpiModifier(IsPositive() ? spi : -spi);
		return spi;
	}
	return 0;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAgiEffect() {
	auto* target = GetTarget();
	if (target && GetAffectedAgility() != -1) {
		int agi = GetAffectedAgility();
		target->ChangeAgiModifier(IsPositive() ? agi : -agi);
		return agi;
	}
	return 0;
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyStateEffect(StateEffect se) {
	auto* target = GetTarget();
	if (!target) {
		return;
	}

	bool was_dead = target->IsDead();

	// Apply states
	switch (se.effect) {
		case StateEffect::Inflicted:
			target->AddState(se.state_id, true);
			break;
		case StateEffect::Healed:
		case StateEffect::HealedByAttack:
			target->RemoveState(se.state_id, false);
			break;
		default:
			break;
	}

	// Apply revived hp healing
	if (IsPositive() && was_dead && !target->IsDead()) {
		if (GetAffectedHp()) {
			int hp = GetAffectedHp();
			target->ChangeHp(hp - 1, true);
		}
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyStateEffects() {
	auto* target = GetTarget();
	if (!target) {
		return;
	}

	bool was_dead = target->IsDead();

	// Apply states
	for (auto& se: states) {
		ApplyStateEffect(se);
	}

	// Apply revived hp healing
	if (IsPositive() && was_dead && !target->IsDead()) {
		if (GetAffectedHp()) {
			int hp = GetAffectedHp();
			target->ChangeHp(hp - 1, true);
		}
	}
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAttributeShiftEffect(int attr_id) {
	auto* target = GetTarget();
	if (target) {
		return target->ShiftAttributeRate(attr_id, IsPositiveSkill() ? 1 : -1);
	}
	return 0;
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyAttributeShiftEffects() {
	for (auto& sa: shift_attributes) {
		ApplyAttributeShiftEffect(sa);
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyAll() {
	ApplyInitialEffect();
	ApplyHpEffect();
	ApplySpEffect();
	ApplyAtkEffect();
	ApplyDefEffect();
	ApplySpiEffect();
	ApplyAgiEffect();
	ApplyStateEffects();
	ApplyAttributeShiftEffects();
}

bool Game_BattleAlgorithm::AlgorithmBase::IsTargetValid() const {
	if (no_target) {
		// Selected algorithm does not need a target because it targets
		// the source
		return true;
	}

	if (current_target == targets.end()) {
		// End of target list reached
		return false;
	}

	return GetTarget()->Exists();
}

int Game_BattleAlgorithm::AlgorithmBase::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Idle;
}

void Game_BattleAlgorithm::AlgorithmBase::TargetFirst() {
	if (party_target) {
		if (IsReflected()) {
			party_target->GetBattlers(original_targets);
			targets.clear();
			source->GetParty().GetActiveBattlers(targets);
		} else {
			party_target->GetBattlers(targets);
		}
		party_target = nullptr;
	} else {
		if (IsReflected()) {
			original_targets.push_back(GetTarget());
			targets.clear();
			targets.push_back(source);
		}
	}

	current_target = targets.begin();
	cur_repeat = 0;

	if (!IsTargetValid()) {
		TargetNext();
	}

	first_attack = true;
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	++cur_repeat;
	if (IsTargetValid() && cur_repeat < repeat) {
		first_attack = false;
		return true;
	}
	cur_repeat = 0;

	return TargetNextInternal();
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNextInternal() const {
	do {
		if (current_target == targets.end() ||
			current_target + 1 == targets.end()) {
			return false;
		}

		++current_target;
	} while (!IsTargetValid());

	first_attack = false;

	return true;
}

void Game_BattleAlgorithm::AlgorithmBase::SetRepeat(int repeat) {
	this->repeat = repeat;
}

bool Game_BattleAlgorithm::AlgorithmBase::OriginalTargetsSet() const {
	return (original_targets.size() > 0);
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetFirstOriginalTarget() const {
	if (original_targets.empty()) {
		return nullptr;
	} else {
		return *original_targets.begin();
	}
}

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchEnable(int switch_id) {
	switch_on.push_back(switch_id);
}

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchDisable(int switch_id) {
	switch_off.push_back(switch_id);
}

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	return NULL;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetFailureSe() const {
	return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Evasion);
}

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetResultSe() const {
	if (!success) {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Evasion);
	}
	if (healing || IsAbsorb()) {
		return NULL;
	}
	if (GetAffectedHp() > -1) {
		if (current_target != targets.end()) {
			return (GetTarget()->GetType() == Game_Battler::Type_Ally ?
				&Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_AllyDamage) :
				&Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyDamage));
		}
	}

	return NULL;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetDeathSe() const {
	return (GetTarget()->GetType() == Game_Battler::Type_Ally ?
		NULL : &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill));
}

int Game_BattleAlgorithm::AlgorithmBase::GetPhysicalDamageRate() const {
	return 0;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsReflected() const {
	return false;
}

Game_BattleAlgorithm::Null::Null(Game_Battler* source) :
AlgorithmBase(Type::Null, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Null::GetStartMessage() const {
	return "";
}

bool Game_BattleAlgorithm::Null::Execute() {
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(Type::Normal, source, target)
{
	Init();
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Party_Base* target) :
	AlgorithmBase(Type::Normal, source, target)
{
	Init();
}

void Game_BattleAlgorithm::Normal::Init() {
	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);

		if (ally->HasDualAttack()) {
			SetRepeat(2);
		}

		const auto* weapon1 = ally->GetWeapon();
		const auto* weapon2 = ally->Get2ndWeapon();
		if (weapon1 == nullptr) {
			weapon1 = weapon2;
		}

		if (weapon1) {
			animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, weapon1->animation_id);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID {}", weapon1->animation_id);
				return;
			}
			if (weapon2) {
				animation2 = lcf::ReaderUtil::GetElement(lcf::Data::animations, weapon2->animation_id);
				if (!animation2) {
					Output::Warning("Algorithm Normal: Invalid weapon animation ID {}", weapon2->animation_id);
				}
			}
		} else {
			const lcf::rpg::Actor& actor = *lcf::ReaderUtil::GetElement(lcf::Data::actors, ally->GetId());
			animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, actor.unarmed_animation);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid unarmed animation ID {}", actor.unarmed_animation);
			}
		}
	}
	if (source->GetType() == Game_Battler::Type_Enemy) {
		if (Player::IsRPG2k3() && !lcf::Data::animations.empty()) {
			animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, 1);
		}
	}
}

bool Game_BattleAlgorithm::Normal::Execute() {
	Reset();

	auto& source = *GetSource();
	auto& target = *GetTarget();

	auto to_hit = Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, Game_Battle::GetBattleCondition(), true);
	auto crit_chance = Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll);

	// Damage calculation
	if (!Rand::PercentChance(to_hit)) {
		this->success = false;
		return this->success;
	}

	if (Rand::PercentChance(crit_chance)) {
		critical_hit = true;
	}

	// FIXME: Differentiate the cases where 2k3 back attack bug applies
	auto effect = Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, critical_hit, true, Game_Battle::GetBattleCondition(), true);
	effect = Algo::AdjustDamageForDefend(effect, target);

	// Handle negative effect from attributes
	if (effect < 0) {
		this->negative_effect = true;
		this->healing = true;
		effect = -effect;
	}

	effect = Utils::Clamp(effect, 0, MaxDamageValue());

	this->hp = effect;

	// RPG_RT bug: Negative effects affect HP double
	if (this->negative_effect) {
		this->hp *= 2;
	}

	if (!this->healing && GetTarget()->GetHp() - this->hp <= 0) {
		// Death state
		lethal = true;
		killed_by_dmg = true;

		this->success = true;
		return this->success;
	}

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target.GetStates();
	auto target_perm_states = target.GetPermanentStates();

	// Conditions healed by physical attack:
	BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);

	// Conditions caused / healed by weapon.
	if (source.GetType() == Game_Battler::Type_Ally) {
		auto& ally = static_cast<Game_Actor&>(source);
		const bool is2k3 = Player::IsRPG2k3();
		auto* weapon1 = ally.GetWeapon();
		auto* weapon2 = ally.Get2ndWeapon();

		int state_limit = 0;
		if (weapon1) {
			state_limit = weapon1->state_set.size();
		}
		if (weapon2) {
			state_limit = std::max(state_limit, (int)weapon2->state_set.size());
		}

		auto addStates = [&](const lcf::rpg::Item* weapon, int state_id) {
			if (weapon == nullptr
					|| state_id > (int)weapon->state_set.size()
					|| !weapon->state_set[state_id - 1]
			   ) {
				return false;
			}
			bool weapon_heals_states = is2k3 && weapon->reverse_state_effect;
			auto pct = weapon->state_chance;
			if (!weapon_heals_states) {
				pct = pct * GetTarget()->GetStateProbability(state_id) / 100;
			}
			if (!Rand::PercentChance(pct)) {
				return false;
			}
			if (weapon_heals_states) {
				if (State::Remove(state_id, target_states, target_perm_states)) {
					states.push_back(StateEffect(state_id, StateEffect::Healed));
				}
				return false;
			}
			// Normal attacks don't produce AlreadyInflicted messages in 2k battle
			// so we filter on HasState.
			if (!State::Has(state_id, target_states) && State::Add(state_id, target_states, target_perm_states, true)) {
				states.push_back(StateEffect(state_id, StateEffect::Inflicted));
				return true;
			}
			return false;
		};

		if (addStates(weapon1, lcf::rpg::State::kDeathID)
				|| addStates(weapon2, lcf::rpg::State::kDeathID)) {
			lethal = true;
		}

		for (int state_id = lcf::rpg::State::kDeathID + 1; state_id <= state_limit; ++state_id) {
			addStates(weapon1, state_id);
			addStates(weapon2, state_id);
		}
	}

	this->success = true;
	return this->success;
}

void Game_BattleAlgorithm::Normal::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();

	if (source->GetType() == Game_Battler::Type_Ally && IsFirstAttack()) {
		source->ChangeSp(-static_cast<Game_Actor*>(source)->CalculateWeaponSpCost());
	}
}

std::string Game_BattleAlgorithm::Normal::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		if (Player::IsRPG2kE()) {
			return Utils::ReplacePlaceholders(
				lcf::Data::terms.attacking,
				Utils::MakeArray('S'),
				Utils::MakeSvArray(source->GetName())
			);
		}
		else {
			return ToString(source->GetName()) + ToString(lcf::Data::terms.attacking);
		}
	}
	else {
		if (repeat == 2) {
			return "Double Attack";
		} else {
			return "";
		}
	}
}

int Game_BattleAlgorithm::Normal::GetSourceAnimationState() const {
	// ToDo when it is Dual attack the 2nd call should return LeftHand

	return Sprite_Battler::AnimationState_RightHand;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Normal::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyAttacks);
	}
	else {
		return NULL;
	}
}

int Game_BattleAlgorithm::Normal::GetPhysicalDamageRate() const {
	return 100;
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item) :
	AlgorithmBase(Type::Skill, source, target), skill(skill), item(item)
{
	Init();
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item) :
	AlgorithmBase(Type::Skill, source, target), skill(skill), item(item)
{
	Init();
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, const lcf::rpg::Skill& skill, const lcf::rpg::Item* item) :
	AlgorithmBase(Type::Skill, source), skill(skill), item(item)
{
	Init();
}

void Game_BattleAlgorithm::Skill::Init() {
	animation = nullptr;
	if (skill.animation_id != 0) {
		animation = lcf::ReaderUtil::GetElement(lcf::Data::animations, skill.animation_id);
		if (!animation) {
			Output::Warning("Algorithm Skill: Invalid skill animation ID {}", skill.animation_id);
		}
	}
}

bool Game_BattleAlgorithm::Skill::IsTargetValid() const {
	if (no_target) {
		return true;
	}

	if (current_target == targets.end()) {
		return false;
	}

	if (skill.scope == lcf::rpg::Skill::Scope_ally ||
		skill.scope == lcf::rpg::Skill::Scope_party) {
		// Ignore hidden targets
		if (GetTarget()->IsHidden()) {
			return false;
		}

		if (GetTarget()->IsDead()) {
			// Cures death
			// NOTE: RPG_RT 2k3 also allows this targetting if reverse_state_effect.
			return !skill.state_effects.empty() && skill.state_effects[0];
		}

		return true;
	}

	return GetTarget()->Exists();
}


bool Game_BattleAlgorithm::Skill::Execute() {
	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	absorb = false;
	this->success = false;

	auto* target = GetTarget();

	this->healing = Algo::SkillTargetsAllies(skill);

	this->revived = this->healing
		&& !skill.state_effects.empty()
		&& skill.state_effects[lcf::rpg::State::kDeathID - 1]
		&& GetTarget()->IsDead();

	if (skill.type == lcf::rpg::Skill::Type_switch) {
		switch_id = skill.switch_id;
		this->success = true;
		return this->success;
	}

	if (!Algo::IsNormalOrSubskill(skill)) {
		this->success = false;
		return this->success;
	}

	auto to_hit = Algo::CalcSkillToHit(*GetSource(), *GetTarget(), skill);
	auto effect = Algo::CalcSkillEffect(*GetSource(), *GetTarget(), skill, true);

	// Handle negative effect from attributes
	if (effect < 0) {
		this->negative_effect = true;
		this->healing = !this->healing;
		effect = -effect;
	}

	effect = Utils::Clamp(effect, 0, MaxDamageValue());

	if (IsNegativeSkill()) absorb = skill.absorb_damage;

	if (skill.affect_hp && Rand::PercentChance(to_hit)) {
		if (IsNegativeSkill()) {
			this->hp = Algo::AdjustDamageForDefend(effect, *GetTarget());

			// RPG_RT bug: Negative effects affect HP double
			// HP absorbing is not affected by this bug
			if (this->negative_effect && !IsAbsorb()) {
				this->hp *= 2;
			}

			if (IsAbsorb() && !this->negative_effect)
				this->hp = std::min<int>(hp, GetTarget()->GetHp());

			if (!this->negative_effect && GetTarget()->GetHp() - this->hp <= 0) {
				// Death state
				lethal = true;
				killed_by_dmg = true;
			}
		} else {
			if (!this->negative_effect) {
				this->hp = effect;
			} else {
				// RPG_RT negative healing skills are non lethal
				this->hp = Utils::Clamp(effect, 0, GetTarget()->GetHp() - 1);
			}
		}
	}

	if (skill.affect_sp && Rand::PercentChance(to_hit)) {
		if (IsNegativeSkill()) {
			if (!this->negative_effect) {
				this->sp = std::min<int>(effect, GetTarget()->GetSp());
			} else {
				this->sp = effect;
			}
		} else {
			int sp_cost = GetSource() == GetTarget() ? source->CalculateSkillCost(skill.ID) : 0;
			if (!this->negative_effect) {
				this->sp = Utils::Clamp(effect, 0, GetTarget()->GetMaxSp() - GetTarget()->GetSp() + sp_cost);
			} else {
				this->sp = effect;
			}
		}
	}

	if (skill.affect_attack && Rand::PercentChance(to_hit)) {
		if (!this->healing) {
			this->attack = Utils::Clamp(effect, 0, GetTarget()->GetAtk() - (GetTarget()->GetBaseAtk() + 1) / 2);
		} else {
			this->attack = Utils::Clamp(effect, 0, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseAtk() * 2) - GetTarget()->GetAtk());
		}
	}
	if (skill.affect_defense && Rand::PercentChance(to_hit)) {
		if (!this->healing) {
			this->defense = Utils::Clamp(effect, 0, GetTarget()->GetDef() - (GetTarget()->GetBaseDef() + 1) / 2);
		} else {
			this->defense = Utils::Clamp(effect, 0, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseDef() * 2) - GetTarget()->GetDef());
		}
	}
	if (skill.affect_spirit && Rand::PercentChance(to_hit)) {
		if (!this->healing) {
			this->spirit = Utils::Clamp(effect, 0, GetTarget()->GetSpi() - (GetTarget()->GetBaseSpi() + 1) / 2);
		} else {
			this->spirit = Utils::Clamp(effect, 0, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseSpi() * 2) - GetTarget()->GetSpi());
		}
	}
	if (skill.affect_agility && Rand::PercentChance(to_hit)) {
		if (!this->healing) {
			this->agility = Utils::Clamp(effect, 0, GetTarget()->GetAgi() - (GetTarget()->GetBaseAgi() + 1) / 2);
		} else {
			this->agility = Utils::Clamp(effect, 0, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseAgi() * 2) - GetTarget()->GetAgi());
		}
	}

	if (IsNegativeSkill()) {
		if (skill.affect_hp) {
			if (skill.affect_sp) {
				if (GetAffectedHp() == -1 && GetAffectedSp() == -1) {
					this->success = false;
					return this->success;
				}
			} else {
				if (GetAffectedHp() == -1) {
					this->success = false;
					return this->success;
				}
			}
		} else {
			if (skill.affect_sp) {
				if (GetAffectedSp() == -1) {
					this->success = false;
					return this->success;
				}
			}
		}
	}

	this->success = (GetAffectedHp() != -1 && !IsAbsorb()) || (GetAffectedHp() > 0 && IsAbsorb()) || GetAffectedSp() > 0 || GetAffectedAttack() > 0
		|| GetAffectedDefense() > 0 || GetAffectedSpirit() > 0 || GetAffectedAgility() > 0;

	if (IsPositiveSkill()) {
		// If resurrected and no HP selected, the effect value is a percentage:
		if (IsRevived() && !skill.affect_hp) {
			this->hp = Utils::Clamp(GetTarget()->GetMaxHp() * effect / 100, 1, GetTarget()->GetMaxHp() - GetTarget()->GetHp());
			this->success = true;
		}
	}

	if (IsNegativeSkill()) {
		if (!success &&
				((IsAbsorb() && ((GetAffectedHp() == 0 && GetAffectedSp() <= 0) || (GetAffectedHp() <= 0 && GetAffectedSp() == 0))) ||
				 (!IsAbsorb() && GetAffectedSp() == 0 && GetAffectedHp() == -1)))
			return this->success;
	}

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target->GetStates();
	auto target_perm_states = target->GetPermanentStates();

	// Conditions healed by physical attack:
	if (!IsPositive() && skill.affect_hp && !IsAbsorb()) {
		BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);
	}

	// Conditions:
	// If the target gets killed by damage, do not add or remove states
	if (!lethal) {
		bool heals_states = IsPositiveSkill() ^ (Player::IsRPG2k3() && skill.reverse_state_effect);
		for (int i = 0; i < static_cast<int>(skill.state_effects.size()); i++) {
			if (!skill.state_effects[i])
				continue;
			auto state_id = i + 1;

			bool target_has_state = State::Has(state_id, target_states);

			if (!heals_states && target_has_state) {
				this->success = true;
				states.push_back({state_id, StateEffect::AlreadyInflicted});
				continue;
			}
			if (heals_states && !target_has_state) {
				continue;
			}
			if (!Rand::PercentChance(to_hit)) {
				continue;
			}

			if (heals_states) {
				// RPG_RT 2k3 skills which fail due to permanent states don't "miss"
				this->success = true;
				if (State::Remove(state_id, target_states, target_perm_states)) {
					states.push_back({state_id, StateEffect::Healed});
				}
			} else if (Rand::PercentChance(GetTarget()->GetStateProbability(state_id))) {
				if (State::Add(state_id, target_states, target_perm_states, true)) {
					this->success = true;
					states.push_back({state_id, StateEffect::Inflicted});
					if (state_id == lcf::rpg::State::kDeathID) {
						lethal = true;
					}
				}
			}
		}
	}

	// Attribute resistance / weakness + an attribute selected + can be modified
	if (skill.affect_attr_defence) {
		for (int i = 0; i < static_cast<int>(skill.attribute_effects.size()); i++) {
			if (skill.attribute_effects[i] && GetTarget()->CanShiftAttributeRate(i + 1, IsPositiveSkill() ? 1 : -1)) {
				if (!Rand::PercentChance(to_hit))
					continue;
				shift_attributes.push_back(i + 1);
				this->success = true;
			}
		}
	}

	return this->success;
}

void Game_BattleAlgorithm::Skill::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();

	if (IsFirstAttack()) {
		if (item) {
			Main_Data::game_party->ConsumeItemUse(item->ID);
		}
		else {
			source->ChangeSp(-source->CalculateSkillCost(skill.ID));
		}
	}
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		if (item && item->using_message == 0) {
			// Use item message
			return Item(source, *item).GetStartMessage();
		}
		if (Player::IsRPG2kE()) {
			auto* target = GetTarget();
			return Utils::ReplacePlaceholders(
				skill.using_message1,
				Utils::MakeArray('S', 'O', 'U'),
				Utils::MakeSvArray(GetSource()->GetName(), (target ? target->GetName() : "???"), skill.name)
			);
		}
		else {
			return ToString(source->GetName()) + ToString(skill.using_message1);
		}
	}
	else {
		return ToString(skill.name);
	}
}

bool Game_BattleAlgorithm::Skill::HasSecondStartMessage() const {
	return Player::IsRPG2k() && (!item || item->using_message != 0) && !skill.using_message2.empty();
}

std::string Game_BattleAlgorithm::Skill::GetSecondStartMessage() const {
	if (Player::IsRPG2k()) {
		if (item && item->using_message == 0) {
			return "";
		}
		if (Player::IsRPG2kE()) {
			auto* target = GetTarget();
			return Utils::ReplacePlaceholders(
				skill.using_message2,
				Utils::MakeArray('S', 'O', 'U'),
				Utils::MakeSvArray(GetSource()->GetName(), (target ? target->GetName() : "???"), skill.name)
			);
		}
		else {
			return ToString(skill.using_message2);
		}
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::Skill::GetSourceAnimationState() const {
	if (source->GetType() == Game_Battler::Type_Ally && skill.animation_id > 0) {
		if (static_cast<int>(skill.battler_animation_data.size()) > source->GetId() - 1) {
			int pose = skill.battler_animation_data[source->GetId() - 1].pose;

			if (pose > 0) {
				return pose + 1;
			}
		}
	}

	return Sprite_Battler::AnimationState_SkillUse;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Skill::GetStartSe() const {
	if (skill.type == lcf::rpg::Skill::Type_switch) {
		return &skill.sound_effect;
	}
	else {
		return NULL;
	}
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Skill::GetFailureSe() const {
	return skill.failure_message != 3
		? nullptr
		: AlgorithmBase::GetResultSe();
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Skill::GetResultSe() const {
	return !success && skill.failure_message != 3 ? NULL : AlgorithmBase::GetResultSe();
}

std::string Game_BattleAlgorithm::Skill::GetFailureMessage() const {
	switch (skill.failure_message) {
		case 0:
			return AlgorithmBase::GetAttackFailureMessage(lcf::Data::terms.skill_failure_a);
		case 1:
			return AlgorithmBase::GetAttackFailureMessage(lcf::Data::terms.skill_failure_b);
		case 2:
			return AlgorithmBase::GetAttackFailureMessage(lcf::Data::terms.skill_failure_c);
		case 3:
			return AlgorithmBase::GetAttackFailureMessage(lcf::Data::terms.dodge);
		default:
			break;
	}
	return "BUG: INVALID SKILL FAIL MSG";
}

int Game_BattleAlgorithm::Skill::GetPhysicalDamageRate() const {
	return skill.physical_rate * 10;
}

bool Game_BattleAlgorithm::Skill::IsReflected() const {
	// Reflect already checked?
	if (reflect == -1) {
		reflect = 0;
		// Skills invoked by items ignore reflect
		if (!item) {
			// One or more targets?
			if (party_target) {
				std::vector<Game_Battler*> targetlist;
				party_target->GetActiveBattlers(targetlist);
				for (Game_Battler* t : targetlist) {
					// Targets on enemy side?
					if (GetSource()->GetType() != t->GetType()) {
						// If at least one enemy has the reflect state, return true
						if (t->HasReflectState()) {
							reflect = 1;
							return true;
						}
					}
				}
			} else {
				// Check if target exists to prevent crashes
				if (GetTarget() != nullptr) {
					// Target on enemy side?
					if (GetSource()->GetType() != GetTarget()->GetType()) {
						// If the target has the reflect state, return true
						if (GetTarget()->HasReflectState()) {
							reflect = 1;
							return true;
						}
					}
				}
			}
		}
	}
	return !!reflect;
}

bool Game_BattleAlgorithm::Skill::ActionIsPossible() const {
	if (item) {
		int count = Main_Data::game_party->GetItemCount(item->ID);
		if (count == 0) {
			auto* src = GetSource();
			if (src && src->GetType() == Game_Battler::Type_Ally) {
				count += static_cast<Game_Actor*>(src)->IsEquipped(item->ID);
			}
		}
		return count > 0;
	}
	return source->GetSp() >= source->CalculateSkillCost(skill.ID);
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Battler* target, const lcf::rpg::Item& item) :
	AlgorithmBase(Type::Item, source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Party_Base* target, const lcf::rpg::Item& item) :
	AlgorithmBase(Type::Item, source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, const lcf::rpg::Item& item) :
AlgorithmBase(Type::Item, source), item(item) {
	// no-op
}

bool Game_BattleAlgorithm::Item::IsTargetValid() const {
	if (no_target) {
		return true;
	}

	if (current_target == targets.end()) {
		return false;
	}
	return item.type == lcf::rpg::Item::Type_medicine;
}

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	auto* target = GetTarget();

	this->success = false;

	// All other items are handled as skills because they invoke skills
	switch (item.type) {
		case lcf::rpg::Item::Type_medicine:
		case lcf::rpg::Item::Type_switch:
			break;
		default:
			assert("Unsupported battle item type");
			return false;
	}

	if (item.type == lcf::rpg::Item::Type_medicine) {
		this->healing = true;

		this->revived = !item.state_set.empty()
			&& item.state_set[lcf::rpg::State::kDeathID - 1]
			&& GetTarget()->IsDead();

		// RM2k3 BUG: In rm2k3 battle system, this IsItemUsable() check is only applied when equipment_setting == actor, not for class.
		if (GetTarget()->GetType() == Game_Battler::Type_Ally && !static_cast<Game_Actor*>(GetTarget())->IsItemUsable(item.ID)) {
			// No effect, but doesn't behave like a dodge or damage to set healing and success to true.
			this->success = true;
			return this->success;
		}
		if (item.ko_only && !GetTarget()->IsDead()) {
			return this->success;
		}

		// HP recovery
		if (item.recover_hp != 0 || item.recover_hp_rate != 0) {
			if (Player::IsRPG2k3()) {
				this->hp = item.recover_hp_rate * GetTarget()->GetMaxHp() / 100 + item.recover_hp;
			} else {
				this->hp = Utils::Clamp<int>(GetTarget()->GetMaxHp() - GetTarget()->GetHp(), 0, item.recover_hp_rate * GetTarget()->GetMaxHp() / 100 + item.recover_hp);
			}
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->sp = Utils::Clamp<int>(GetTarget()->GetMaxSp() - GetTarget()->GetSp(), 0, item.recover_sp_rate * GetTarget()->GetMaxSp() / 100 + item.recover_sp);
		}

		// Make a copy of the target's state set and see what we can apply.
		auto target_states = target->GetStates();
		auto target_perm_states = target->GetPermanentStates();

		bool is_dead_cured = false;
		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				if (i == 0)
					is_dead_cured = true;
				if (State::Remove(i + 1, target_states, target_perm_states)) {
					states.push_back({i+1, StateEffect::Healed});
				}
			}
		}

		if (GetTarget()->IsDead() && !is_dead_cured)
			this->hp = -1;

		this->success = this->hp > -1 || this->sp > -1 || !states.empty();
	}
	else if (item.type == lcf::rpg::Item::Type_switch) {
		switch_id = item.switch_id;
		this->success = true;
	}

	return this->success;
}

void Game_BattleAlgorithm::Item::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();

	if (first_attack) {
		Main_Data::game_party->ConsumeItemUse(item.ID);
	}
}

std::string Game_BattleAlgorithm::Item::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.use_item,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source->GetName(), item.name)
		);
	}
	else if (Player::IsRPG2k()) {
		std::string particle;
		if (Player::IsCP932())
			particle = "は";
		else
			particle = " ";
		return ToString(source->GetName()) + particle + ToString(item.name) + ToString(lcf::Data::terms.use_item);
	}
	else {
		return ToString(item.name);
	}
}

int Game_BattleAlgorithm::Item::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Item;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == lcf::rpg::Item::Type_medicine || item.type == lcf::rpg::Item::Type_switch) {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_UseItem);
	}
	else {
		return NULL;
	}
}

bool Game_BattleAlgorithm::Item::ActionIsPossible() const {
	return Main_Data::game_party->GetItemCount(item.ID) > 0;
}

Game_BattleAlgorithm::Defend::Defend(Game_Battler* source) :
	AlgorithmBase(Type::Defend, source) {
		source->SetIsDefending(true);
}

std::string Game_BattleAlgorithm::Defend::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.defending,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source->GetName())
		);
	}
	else if (Player::IsRPG2k()) {
		return ToString(source->GetName()) + ToString(lcf::Data::terms.defending);
	}
	else {
		return "Defend";
	}
}

int Game_BattleAlgorithm::Defend::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Defending;
}

bool Game_BattleAlgorithm::Defend::Execute() {
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(Type::Observe, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Observe::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.observing,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source->GetName())
		);
	}
	else if (Player::IsRPG2k()) {
		return ToString(source->GetName()) + ToString(lcf::Data::terms.observing);
	}
	else {
		return "Observe";
	}
}

bool Game_BattleAlgorithm::Observe::Execute() {
	// Observe only prints the start message
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Charge::Charge(Game_Battler* source) :
AlgorithmBase(Type::Charge, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Charge::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.focus,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source->GetName())
		);
	}
	else if (Player::IsRPG2k()) {
		return ToString(source->GetName()) + ToString(lcf::Data::terms.focus);
	}
	else {
		return "Charge Up";
	}
}

bool Game_BattleAlgorithm::Charge::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Charge::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();
	source->SetCharged(true);
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source, Game_Party_Base* target) :
AlgorithmBase(Type::SelfDestruct, source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::SelfDestruct::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.autodestruction,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source->GetName())
		);
	}
	else if (Player::IsRPG2k()) {
		return ToString(source->GetName()) + ToString(lcf::Data::terms.autodestruction);
	}
	else {
		return "Self-Destruct";
	}
}

int Game_BattleAlgorithm::SelfDestruct::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_SelfDestruct;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::SelfDestruct::GetStartSe() const {
	return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill);
}

int Game_BattleAlgorithm::SelfDestruct::GetPhysicalDamageRate() const {
	return 100;
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Reset();

	auto& source = *GetSource();
	auto& target = *GetTarget();

	auto effect = Algo::CalcSelfDestructEffect(source, target, true);
	effect = Algo::AdjustDamageForDefend(effect, target);
	effect = Utils::Clamp(effect, 0, MaxDamageValue());

	this->hp = effect;

	if (GetTarget()->GetHp() - this->hp <= 0) {
		// Death state
		lethal = true;
		killed_by_dmg = true;
	}

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target.GetStates();
	auto target_perm_states = target.GetPermanentStates();

	// Conditions healed by physical attack:
	BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);

	success = true;

	return true;
}

void Game_BattleAlgorithm::SelfDestruct::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();

	// Only monster can self destruct
	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source) :
	AlgorithmBase(Type::Escape, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Escape::GetStartMessage() const {
	// Only monsters can escape during a battle phase
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.enemy_escape,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source->GetName())
		);
	}
	else if (Player::IsRPG2k()) {
		if (source->GetType() == Game_Battler::Type_Enemy) {
			return ToString(source->GetName()) + ToString(lcf::Data::terms.enemy_escape);
		}
	}

	return "Escape";
}

int Game_BattleAlgorithm::Escape::GetSourceAnimationState() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetSourceAnimationState();
	}
	else {
		return Sprite_Battler::AnimationState_Dead;
	}
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Escape::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetStartSe();
	}
	else {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Escape);
	}
}

bool Game_BattleAlgorithm::Escape::Execute() {
	Reset();

	this->success = true;

	return this->success;
}

void Game_BattleAlgorithm::Escape::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();
	static_cast<Game_Enemy*>(source)->SetHidden(true);
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(Type::Transform, source), new_monster_id(new_monster_id) {
	// no-op
}

std::string Game_BattleAlgorithm::Transform::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.enemy_transform,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source->GetName(), lcf::ReaderUtil::GetElement(lcf::Data::enemies, new_monster_id)->name) // Sanity check in Game_Enemy
		);
	}
	else if (Player::IsRPG2k()) {
		return ToString(source->GetName()) + ToString(lcf::Data::terms.enemy_transform);
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Transform::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Transform::ApplyInitialEffect() {
	AlgorithmBase::ApplyInitialEffect();
	static_cast<Game_Enemy*>(source)->Transform(new_monster_id);
}

Game_BattleAlgorithm::NoMove::NoMove(Game_Battler* source) :
AlgorithmBase(Type::NoMove, source) {
	// no-op
}

std::string Game_BattleAlgorithm::NoMove::GetStartMessage() const {
	return "";
}

bool Game_BattleAlgorithm::NoMove::Execute() {
	this->success = true;
	return true;
}

