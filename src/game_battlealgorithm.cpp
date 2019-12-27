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
#include "reader_util.h"
#include "rpg_animation.h"
#include "rpg_state.h"
#include "rpg_skill.h"
#include "rpg_item.h"
#include "sprite_battler.h"
#include "utils.h"
#include "state.h"

static inline int MaxDamageValue() {
	return Player::IsRPG2k() ? 999 : 9999;
}

static inline int ToHitPhysical(Game_Battler *source, Game_Battler *target, int to_hit) {
	// If target has rm2k3 state which grants 100% dodge.
	if (target->EvadesAllPhysicalAttacks()) {
		return 0;
	}

	// If target has Restriction "do_nothing", the attack always hits
	if (target->GetSignificantRestriction() == RPG::State::Restriction_do_nothing) {
		return 100;
	}

	// Modify hit chance for each state the source has
	to_hit = (to_hit * source->GetHitChanceModifierFromStates()) / 100;

	// Stop here if attacker ignores evasion.
	if (source->GetType() == Game_Battler::Type_Ally
		&& static_cast<Game_Actor*>(source)->AttackIgnoresEvasion()) {
		return to_hit;
	}

	// AGI adjustment.
	to_hit = 100 - (100 - to_hit) * (1.5f * (float(target->GetAgi()) / float(source->GetAgi()) - 1.0f));

	// If target has physical dodge evasion:
	if (target->GetType() == Game_Battler::Type_Ally
			&& static_cast<Game_Actor*>(target)->HasPhysicalEvasionUp()) {
		to_hit -= 25;
	}

	return to_hit;
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

		auto* state = ReaderUtil::GetElement(Data::states, state_id);
		if (state == nullptr) {
			continue;
		}
		if (state->release_by_damage > 0) {
			int release_chance = state->release_by_damage * physical_rate / 100;

			if (!Utils::ChanceOf(release_chance, 100)) {
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
	source_restriction(RPG::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Battler* target) :
	type(ty), source(source), no_target(false), first_attack(true),
	source_restriction(RPG::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	SetTarget(target);
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Party_Base* target) :
	type(ty), source(source), no_target(false), first_attack(true),
	source_restriction(RPG::State::Restriction(source->GetSignificantRestriction()))
{
	Reset();

	target->GetBattlers(targets);
	current_target = targets.begin();
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
	success = false;
	lethal = false;
	killed_by_dmg = false;
	critical_hit = false;
	absorb = false;
	revived = false;
	reflect = -1;
	states.clear();

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

const RPG::Animation* Game_BattleAlgorithm::AlgorithmBase::GetAnimation() const {
	return animation;
}

const RPG::Animation* Game_BattleAlgorithm::AlgorithmBase::GetSecondAnimation() const {
	return animation2;
}

void Game_BattleAlgorithm::AlgorithmBase::PlayAnimation(bool on_source) {
	if (current_target == targets.end() || !GetAnimation()) {
		return;
	}

	if (on_source) {
		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, { GetSource() });
		has_animation_played = true;
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
		anim_targets);
	has_animation_played = true;

	current_target = old_current_target;
	first_attack = old_first_attack;
}

void Game_BattleAlgorithm::AlgorithmBase::PlaySecondAnimation(bool on_source) {
	if (current_target == targets.end() || !GetSecondAnimation()) {
		return;
	}

	if (on_source) {
		Game_Battle::ShowBattleAnimation(GetSecondAnimation()->ID, { GetSource() });
		has_animation2_played = true;
		return;
	}

	auto old_current_target = current_target;
	bool old_first_attack = first_attack;

	std::vector<Game_Battler*> anim_targets;

	do {
		anim_targets.push_back(*current_target);
	} while (TargetNextInternal());

	Game_Battle::ShowBattleAnimation(
		GetSecondAnimation()->ID,
		anim_targets);
	has_animation2_played = true;

	current_target = old_current_target;
	first_attack = old_first_attack;
}

void Game_BattleAlgorithm::AlgorithmBase::PlaySoundAnimation(bool on_source, int cutoff) {
	if (current_target == targets.end() || !GetAnimation()) {
		return;
	}

	if (on_source) {
		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, { GetSource() }, true, cutoff);
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
	const RPG::State* state = ReaderUtil::GetElement(Data::states, 1);
	const std::string& message = is_ally ? state->message_actor
										: state->message_enemy;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S'},
			{GetTarget()->GetName()}
		);
	}
	else {
		return GetTarget()->GetName() + message;
	}
}

RPG::State::Restriction Game_BattleAlgorithm::AlgorithmBase::GetSourceRestrictionWhenStarted() const {
	return source_restriction;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetAttackFailureMessage(const std::string& message) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'O'},
			{GetSource()->GetName(), GetTarget()->GetName()}
		);
	}
	else {
		return GetTarget()->GetName() + message;
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetHpSpRecoveredMessage(int value, const std::string& points) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.hp_recovery,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), std::to_string(value), points}
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
		ss << value << space << Data::terms.hp_recovery;
		return ss.str();
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetUndamagedMessage() const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	const std::string& message = target_is_ally ?
		Data::terms.actor_undamaged :
		Data::terms.enemy_undamaged;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S'},
			{GetTarget()->GetName()}
		);
	}
	else {
		return GetTarget()->GetName() + message;
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetCriticalHitMessage() const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	const std::string& message = target_is_ally ?
		Data::terms.actor_critical :
		Data::terms.enemy_critical;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'O'},
			{GetSource()->GetName(), GetTarget()->GetName()}
		);
	}
	else {
		return message;
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetHpSpAbsorbedMessage(int value, const std::string& points) const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	const std::string& message = target_is_ally ?
		Data::terms.actor_hp_absorbed :
		Data::terms.enemy_hp_absorbed;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'O', 'V', 'U'},
			{GetSource()->GetName(), GetTarget()->GetName(), std::to_string(value), points}
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

std::string Game_BattleAlgorithm::AlgorithmBase::GetDamagedMessage() const {
	bool target_is_ally = (GetTarget()->GetType() ==
			Game_Battler::Type_Ally);
	const std::string& message = target_is_ally ?
		Data::terms.actor_damaged :
		Data::terms.enemy_damaged;
	int value = GetAffectedHp();

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), std::to_string(value), Data::terms.health_points}
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

std::string Game_BattleAlgorithm::AlgorithmBase::GetParameterChangeMessage(bool is_positive, int value, const std::string& points) const {
	const std::string& message = is_positive ?
		Data::terms.parameter_increase :
		Data::terms.parameter_decrease;

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), std::to_string(value), points}
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

std::string Game_BattleAlgorithm::AlgorithmBase::GetStateMessage(const std::string& message) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			message,
			{'S'},
			{GetTarget()->GetName()}
		);
	}
	else {
		return GetTarget()->GetName() + message;
	}
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetAttributeShiftMessage( const std::string& attribute) const {
	const std::string& message = IsPositive() ?
		Data::terms.resistance_increase :
		Data::terms.resistance_decrease;
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << attribute;
		return Utils::ReplacePlaceholders(
			message,
			{ 'S', 'O' },
			{ GetTarget()->GetName(), ss.str() }
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
	return GetAttackFailureMessage(Data::terms.dodge);
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetSource() const {
	return source;
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetTarget() const {
	if (IsReflected()) {
		return source;
	}

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

void Game_BattleAlgorithm::AlgorithmBase::Apply() {
	ApplyActionSwitches();

	if (!success)
		return;

	if (GetAffectedSwitch() != -1) {
		Main_Data::game_switches->Set(GetAffectedSwitch(), true);
	}

	auto* target = GetTarget();

	if (!target) {
		return;
	}

	bool was_dead = target->IsDead();

	if (GetAffectedHp() != -1 && !was_dead) {
		int hp = GetAffectedHp();
		int target_hp = target->GetHp();
		target->ChangeHp(IsPositive() ? hp : -hp);
		if (IsAbsorb()) {
			// Only absorb the hp that were left
			int src_hp = std::min(target_hp, hp);
			source->ChangeHp(src_hp);
		}
	}

	if (GetAffectedSp() != -1) {
		int sp = GetAffectedSp();
		int target_sp = target->GetSp();
		target->SetSp(target->GetSp() + (IsPositive() ? sp : -sp));
		if (IsAbsorb()) {
			int src_sp = std::min(target_sp, sp);
			source->ChangeSp(src_sp);
		}
	}

	if (GetAffectedAttack() != -1) {
		int atk = GetAffectedAttack();
		target->ChangeAtkModifier(IsPositive() ? atk : -atk);
		if (IsAbsorb()) {
			atk = std::max<int>(0, std::min<int>(atk, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeAtkModifier(atk);
		}
	}

	if (GetAffectedDefense() != -1) {
		int def = GetAffectedDefense();
		target->ChangeDefModifier(IsPositive() ? def : -def);
		if (IsAbsorb()) {
			def = std::max<int>(0, std::min<int>(def, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeDefModifier(def);
		}
	}

	if (GetAffectedSpirit() != -1) {
		int spi = GetAffectedSpirit();
		target->ChangeSpiModifier(IsPositive() ? spi : -spi);
		if (IsAbsorb()) {
			spi = std::max<int>(0, std::min<int>(spi, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeSpiModifier(spi);
		}
	}

	if (GetAffectedAgility() != -1) {
		int agi = GetAffectedAgility();
		target->ChangeAgiModifier(IsPositive() ? agi : -agi);
		if (IsAbsorb()) {
			agi = std::max<int>(0, std::min<int>(agi, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeAgiModifier(agi);
		}
	}

	// Apply states
	for (auto& se: states) {
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
	}

	// Apply revived hp healing
	if (IsPositive() && was_dead && !target->IsDead()) {
		if (GetAffectedHp()) {
			int hp = GetAffectedHp();
			target->ChangeHp(hp - 1);
		}
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyActionSwitches() {
	for (int s : switch_on) {
		Main_Data::game_switches->Set(s, true);
	}

	for (int s : switch_off) {
		Main_Data::game_switches->Set(s, false);
	}
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
	current_target = targets.begin();
	cur_repeat = 0;

	if (!IsTargetValid()) {
		TargetNext();
	}

	first_attack = true;
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	if (IsReflected()) {
		// Only source available, can't target again
		return false;
	}
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

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchEnable(int switch_id) {
	switch_on.push_back(switch_id);
}

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchDisable(int switch_id) {
	switch_off.push_back(switch_id);
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	return NULL;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetFailureSe() const {
	return &Game_System::GetSystemSE(Game_System::SFX_Evasion);
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetResultSe() const {
	if (!success) {
		return &Game_System::GetSystemSE(Game_System::SFX_Evasion);
	}
	if (healing || IsAbsorb()) {
		return NULL;
	}
	if (GetAffectedHp() > -1) {
		if (current_target != targets.end()) {
			return (GetTarget()->GetType() == Game_Battler::Type_Ally ?
				&Game_System::GetSystemSE(Game_System::SFX_AllyDamage) :
				&Game_System::GetSystemSE(Game_System::SFX_EnemyDamage));
		}
	}

	return NULL;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetDeathSe() const {
	return (GetTarget()->GetType() == Game_Battler::Type_Ally ?
		NULL : &Game_System::GetSystemSE(Game_System::SFX_EnemyKill));
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

void Game_BattleAlgorithm::Null::Apply() {
	ApplyActionSwitches();
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
			animation = ReaderUtil::GetElement(Data::animations, weapon1->animation_id);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon1->animation_id);
				return;
			}
			if (weapon2) {
				animation2 = ReaderUtil::GetElement(Data::animations, weapon2->animation_id);
				if (!animation2) {
					Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon2->animation_id);
				}
			}
		} else {
			const RPG::Actor& actor = *ReaderUtil::GetElement(Data::actors, ally->GetId());
			animation = ReaderUtil::GetElement(Data::animations, actor.unarmed_animation);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid unarmed animation ID %d", actor.unarmed_animation);
			}
		}
	}
	if (source->GetType() == Game_Battler::Type_Enemy) {
		if (Player::IsRPG2k3() && !Data::animations.empty()) {
			animation = ReaderUtil::GetElement(Data::animations, 1);
		}
	}
}

bool Game_BattleAlgorithm::Normal::Execute() {
	Reset();

	float multiplier = 1;

	auto* target = GetTarget();

	// Criticals cannot occur when ally attacks ally or enemy attacks enemy (e.g. confusion)
	float crit_chance = 0.0f;
	if (source->GetType() != GetTarget()->GetType()) {
		if (GetTarget()->GetType() != Game_Battler::Type_Ally || !static_cast<Game_Actor*>(GetTarget())->PreventsCritical()) {
			crit_chance = source->GetCriticalHitChance();
		}
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		const auto* weapon1 = ally->GetWeapon();
		const auto* weapon2 = ally->Get2ndWeapon();
		if (weapon1 == nullptr) {
			weapon1 = weapon2;
		}

		if (weapon1) {
			if (!weapon2) {
				multiplier = GetTarget()->GetAttributeMultiplier(weapon1->attribute_set);
			} else {
				auto& a1 = weapon1->attribute_set;
				auto& a2 = weapon2->attribute_set;
				std::vector<bool> attribute_set(std::max(a1.size(), a2.size()), false);
				for (size_t i = 0; i < attribute_set.size(); ++i) {
					if (i < a1.size())
						attribute_set[i] = attribute_set[i] | a1[i];
					if (i < a2.size())
						attribute_set[i] = attribute_set[i] | a2[i];
				}

				multiplier = GetTarget()->GetAttributeMultiplier(attribute_set);
			}
		}
	}

	auto to_hit = source->GetHitChance();
	to_hit = ToHitPhysical(GetSource(), GetTarget(), to_hit);

	// Damage calculation
	if (Utils::PercentChance(to_hit)) {
		if (Utils::PercentChance(crit_chance)) {
			critical_hit = true;
		}

		int effect = (source->GetAtk() / 2 - GetTarget()->GetDef() / 4);

		if (effect < 0)
			effect = 0;

		effect *= multiplier;
		if (critical_hit) {
			effect *= 3;
		} else if(source->IsCharged()) {
			effect *= 2;
		}
		if (GetTarget()->IsDefending()) {
			if (GetTarget()->HasStrongDefense()) {
				effect /= 4;
			} else {
				effect /= 2;
			}
		}

		auto var = Utils::GetRandomNumber(-20, 20);
		effect += (effect * var) / 100;

		effect = Utils::Clamp(effect, 0, MaxDamageValue());

		this->hp = effect;

		if (GetTarget()->GetHp() - this->hp <= 0) {
			// Death state
			lethal = true;
			killed_by_dmg = true;
		}
		else {
			// Make a copy of the target's state set and see what we can apply.
			auto target_states = target->GetStates();
			auto target_perm_states = target->GetPermanentStates();

			// Conditions healed by physical attack:
			BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);

			// Conditions caused / healed by weapon.
			if (source->GetType() == Game_Battler::Type_Ally) {
				auto* ally = static_cast<Game_Actor*>(source);
				const bool is2k3 = Player::IsRPG2k();
				auto* weapon1 = ally->GetWeapon();
				auto* weapon2 = ally->Get2ndWeapon();

				int state_limit = 0;
				if (weapon1) {
					state_limit = weapon1->state_set.size();
				}
				if (weapon2) {
					state_limit = std::max(state_limit, (int)weapon2->state_set.size());
				}

				auto addStates = [&](const RPG::Item* weapon, int state_id) {
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
					if (!Utils::PercentChance(pct)) {
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

				if (addStates(weapon1, RPG::State::kDeathID)
						|| addStates(weapon2, RPG::State::kDeathID)) {
					// If death is inflicted, we're done adding states.
					lethal = true;
				} else {
					for (int state_id = RPG::State::kDeathID + 1; state_id <= state_limit; ++state_id) {
						addStates(weapon1, state_id);
						addStates(weapon2, state_id);
					}
				}
			}
		}
	}
	else {
		this->success = false;
		return this->success;
	}

	this->success = true;
	return this->success;
}

void Game_BattleAlgorithm::Normal::Apply() {
	AlgorithmBase::Apply();

	source->SetCharged(false);
	if (source->GetType() == Game_Battler::Type_Ally && IsFirstAttack()) {
		source->ChangeSp(-static_cast<Game_Actor*>(source)->CalculateWeaponSpCost());
	}
}

std::string Game_BattleAlgorithm::Normal::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		if (Player::IsRPG2kE()) {
			return Utils::ReplacePlaceholders(
				Data::terms.attacking,
				{'S'},
				{source->GetName()}
			);
		}
		else {
			return source->GetName() + Data::terms.attacking;
		}
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::Normal::GetSourceAnimationState() const {
	// ToDo when it is Dual attack the 2nd call should return LeftHand

	return Sprite_Battler::AnimationState_RightHand;
}

const RPG::Sound* Game_BattleAlgorithm::Normal::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		return &Game_System::GetSystemSE(Game_System::SFX_EnemyAttacks);
	}
	else {
		return NULL;
	}
}

int Game_BattleAlgorithm::Normal::GetPhysicalDamageRate() const {
	return 100;
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(Type::Skill, source, target), skill(skill), item(item)
{
	Init();
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Party_Base* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(Type::Skill, source, target), skill(skill), item(item)
{
	Init();
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(Type::Skill, source), skill(skill), item(item)
{
	Init();
}

void Game_BattleAlgorithm::Skill::Init() {
	animation = nullptr;
	if (skill.animation_id != 0) {
		animation = ReaderUtil::GetElement(Data::animations, skill.animation_id);
		if (!animation) {
			Output::Warning("Algorithm Skill: Invalid skill animation ID %d", skill.animation_id);
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

	if (skill.scope == RPG::Skill::Scope_ally ||
		skill.scope == RPG::Skill::Scope_party) {
		if (GetTarget()->IsDead()) {
			// Cures death
			return !skill.state_effects.empty() && skill.state_effects[0];
		}

		return true;
	}

	return (!GetTarget()->IsDead());
}


bool Game_BattleAlgorithm::Skill::Execute() {
	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	absorb = false;
	this->success = false;

	auto* target = GetTarget();

	this->healing =
		skill.scope == RPG::Skill::Scope_ally ||
		skill.scope == RPG::Skill::Scope_party ||
		skill.scope == RPG::Skill::Scope_self;

	this->revived = this->healing
		&& !skill.state_effects.empty()
		&& skill.state_effects[RPG::State::kDeathID - 1]
		&& GetTarget()->IsDead();

	if (skill.type == RPG::Skill::Type_normal ||
		skill.type >= RPG::Skill::Type_subskill) {

		int to_hit = skill.hit;

		// If Physical technique, apply physical restrictions
		if (skill.failure_message == 3) {
			to_hit = ToHitPhysical(GetSource(), GetTarget(), to_hit);
		}

		if (this->healing) {
			int effect = skill.power +
				source->GetAtk() * skill.physical_rate / 20 +
				source->GetSpi() * skill.magical_rate / 40;

			effect *= GetTarget()->GetAttributeMultiplier(skill.attribute_effects);

			effect += (effect * Utils::GetRandomNumber(-skill.variance, skill.variance) / 10);

			effect = Utils::Clamp(effect, 0, MaxDamageValue());

			if (skill.affect_hp)
				this->hp = std::max<int>(0, std::min<int>(effect, GetTarget()->GetMaxHp() - GetTarget()->GetHp()));
			if (skill.affect_sp) {
				int sp_cost = GetSource() == GetTarget() ? source->CalculateSkillCost(skill.ID) : 0;
				this->sp = std::max<int>(0, std::min<int>(effect, GetTarget()->GetMaxSp() - GetTarget()->GetSp() + sp_cost));
			}
			if (skill.affect_attack)
				this->attack = std::max<int>(0, std::min<int>(effect, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseAtk() * 2) - GetTarget()->GetAtk()));
			if (skill.affect_defense)
				this->defense = std::max<int>(0, std::min<int>(effect, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseDef() * 2) - GetTarget()->GetDef()));
			if (skill.affect_spirit)
				this->spirit = std::max<int>(0, std::min<int>(effect, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseSpi() * 2) - GetTarget()->GetSpi()));
			if (skill.affect_agility)
				this->agility = std::max<int>(0, std::min<int>(effect, std::min<int>(GetTarget()->MaxStatBattleValue(), GetTarget()->GetBaseAgi() * 2) - GetTarget()->GetAgi()));

			this->success = GetAffectedHp() != -1 || GetAffectedSp() != -1 || GetAffectedAttack() > 0
				|| GetAffectedDefense() > 0 || GetAffectedSpirit() > 0 || GetAffectedAgility() > 0;

			// If resurrected and no HP selected, the effect value is a percentage:
			if (IsRevived() && !skill.affect_hp) {
				this->hp = Utils::Clamp(GetTarget()->GetMaxHp() * effect / 100, 1, GetTarget()->GetMaxHp() - GetTarget()->GetHp());
				this->success = true;
			}
		}
		if (!healing && Utils::PercentChance(to_hit)) {
			absorb = skill.absorb_damage;

			int effect = skill.power +
				source->GetAtk() * skill.physical_rate / 20 +
				source->GetSpi() * skill.magical_rate / 40;

			if (!skill.ignore_defense) {
				effect -= GetTarget()->GetDef() * skill.physical_rate / 40;
				effect -= GetTarget()->GetSpi() * skill.magical_rate / 80;
			}
			effect *= GetTarget()->GetAttributeMultiplier(skill.attribute_effects);

			effect += (effect * Utils::GetRandomNumber(-skill.variance, skill.variance) / 10);

			effect = Utils::Clamp(effect, 0, MaxDamageValue());

			if (skill.affect_hp) {
				this->hp = effect /
					(GetTarget()->IsDefending() ? GetTarget()->HasStrongDefense() ? 4 : 2 : 1);

				if (IsAbsorb())
					this->hp = std::min<int>(hp, GetTarget()->GetHp());

				if (GetTarget()->GetHp() - this->hp <= 0) {
					// Death state
					lethal = true;
					killed_by_dmg = true;
				}
			}

			if (skill.affect_sp) {
				this->sp = std::min<int>(effect, GetTarget()->GetSp());
			}

			if (skill.affect_attack)
				this->attack = std::max<int>(0, std::min<int>(effect, GetTarget()->GetAtk() - (GetTarget()->GetBaseAtk() + 1) / 2));
			if (skill.affect_defense)
				this->defense = std::max<int>(0, std::min<int>(effect, GetTarget()->GetDef() - (GetTarget()->GetBaseDef() + 1) / 2));
			if (skill.affect_spirit)
				this->spirit = std::max<int>(0, std::min<int>(effect, GetTarget()->GetSpi() - (GetTarget()->GetBaseSpi() + 1) / 2));
			if (skill.affect_agility)
				this->agility = std::max<int>(0, std::min<int>(effect, GetTarget()->GetAgi() - (GetTarget()->GetBaseAgi() + 1) / 2));

			this->success = (GetAffectedHp() != -1 && !IsAbsorb()) || (GetAffectedHp() > 0 && IsAbsorb()) || GetAffectedSp() > 0 || GetAffectedAttack() > 0
				|| GetAffectedDefense() > 0 || GetAffectedSpirit() > 0 || GetAffectedAgility() > 0;

			if (!success &&
					((IsAbsorb() && ((GetAffectedHp() == 0 && GetAffectedSp() <= 0) || (GetAffectedHp() <= 0 && GetAffectedSp() == 0))) ||
					(!IsAbsorb() && GetAffectedSp() == 0 && GetAffectedHp() == -1)))
				return this->success;
		}

		// Make a copy of the target's state set and see what we can apply.
		auto target_states = target->GetStates();
		auto target_perm_states = target->GetPermanentStates();

		// Conditions healed by physical attack:
		if (!IsPositive() && skill.affect_hp) {
			BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);
		}

		// Conditions:
		bool heals_states = IsPositive() ^ (Player::IsRPG2k3() && skill.reverse_state_effect);
		for (int i = 0; i < (int) skill.state_effects.size(); i++) {
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
			if (!Utils::PercentChance(to_hit)) {
				continue;
			}

			if (heals_states) {
				// RPG_RT 2k3 skills which fail due to permanent states don't "miss"
				this->success = true;
				if (State::Remove(state_id, target_states, target_perm_states)) {
					states.push_back({state_id, StateEffect::Healed});
				}
			} else if (Utils::PercentChance(GetTarget()->GetStateProbability(state_id))) {
				if (State::Add(state_id, target_states, target_perm_states, true)) {
					this->success = true;
					states.push_back({state_id, StateEffect::Inflicted});
					if (state_id == RPG::State::kDeathID) {
						lethal = true;
						break;
					}
				}
			}
		}

		// Attribute resistance / weakness + an attribute selected + can be modified
		if (skill.affect_attr_defence) {
			for (int i = 0; i < static_cast<int>(skill.attribute_effects.size()); i++) {
				if (skill.attribute_effects[i] && GetTarget()->CanShiftAttributeRate(i + 1, IsPositive() ? 1 : -1)) {
					if (!Utils::PercentChance(skill.hit))
						continue;
					shift_attributes.push_back(i + 1);
					this->success = true;
				}
			}
		}

	}
	else if (skill.type == RPG::Skill::Type_switch) {
		switch_id = skill.switch_id;
		this->success = true;
	}
	else {
		assert(false && "Unsupported skill type");
	}

	return this->success;
}

void Game_BattleAlgorithm::Skill::Apply() {
	if (IsFirstAttack()) {
		if (item) {
			Main_Data::game_party->ConsumeItemUse(item->ID);
		}
		else {
			source->ChangeSp(-source->CalculateSkillCost(skill.ID));
		}
	}

	AlgorithmBase::Apply();

	for (auto& sa: shift_attributes) {
		GetTarget()->ShiftAttributeRate(sa, healing ? 1 : -1);
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
				{'S', 'O', 'U'},
				{GetSource()->GetName(), (target ? target->GetName() : "???"), skill.name}
			);
		}
		else {
			return source->GetName() + skill.using_message1;
		}
	}
	else {
		return skill.name;
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
				{ 'S', 'O', 'U' },
				{GetSource()->GetName(), (target ? target->GetName() : "???"), skill.name}
			);
		}
		else {
			return skill.using_message2;
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

const RPG::Sound* Game_BattleAlgorithm::Skill::GetStartSe() const {
	if (skill.type == RPG::Skill::Type_switch) {
		return &skill.sound_effect;
	}
	else {
		return NULL;
	}
}

const RPG::Sound* Game_BattleAlgorithm::Skill::GetFailureSe() const {
	return skill.failure_message != 3
		? nullptr
		: AlgorithmBase::GetResultSe();
}

const RPG::Sound* Game_BattleAlgorithm::Skill::GetResultSe() const {
	return !success && skill.failure_message != 3 ? NULL : AlgorithmBase::GetResultSe();
}

std::string Game_BattleAlgorithm::Skill::GetFailureMessage() const {
	switch (skill.failure_message) {
		case 0:
			return AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_a);
		case 1:
			return AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_b);
		case 2:
			return AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_c);
		case 3:
			return AlgorithmBase::GetAttackFailureMessage(Data::terms.dodge);
		default:
			break;
	}
	return "BUG: INVALID SKILL FAIL MSG";
}

int Game_BattleAlgorithm::Skill::GetPhysicalDamageRate() const {
	return skill.physical_rate * 10;
}

bool Game_BattleAlgorithm::Skill::IsReflected() const {
	// Reflect must be stored because after "Apply" the return value for
	// reflect can be incorrect when states are added.
	if (reflect != -1) {
		return !!reflect;
	}

	if (current_target == targets.end()) {
		reflect = 0;
		return false;
	}

	auto old_current_target = current_target;
	bool old_first_attack = first_attack;

	// Only negative skills are reflected
	if (GetSource()->GetType() == (*current_target)->GetType()) {
		reflect = 0;
		return false;
	}

	std::vector<Game_Battler*> anim_targets;

	bool has_reflect = false;

	do {
		has_reflect |= (*current_target)->HasReflectState();
	} while (TargetNextInternal());

	current_target = old_current_target;
	first_attack = old_first_attack;

	reflect = has_reflect ? 1 : 0;
	return has_reflect;
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

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Battler* target, const RPG::Item& item) :
	AlgorithmBase(Type::Item, source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Party_Base* target, const RPG::Item& item) :
	AlgorithmBase(Type::Item, source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, const RPG::Item& item) :
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
	return item.type == RPG::Item::Type_medicine;
}

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	auto* target = GetTarget();

	this->success = false;

	// All other items are handled as skills because they invoke skills
	switch (item.type) {
		case RPG::Item::Type_medicine:
		case RPG::Item::Type_switch:
			break;
		default:
			assert("Unsupported battle item type");
			return false;
	}

	if (item.type == RPG::Item::Type_medicine) {
		this->healing = true;

		this->revived = !item.state_set.empty()
			&& item.state_set[RPG::State::kDeathID - 1]
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
			this->hp = std::max<int>(0, std::min<int>(item.recover_hp_rate * GetTarget()->GetMaxHp() / 100 + item.recover_hp, GetTarget()->GetMaxHp() - GetTarget()->GetHp()));
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->sp = std::max<int>(0, std::min<int>(item.recover_sp_rate * GetTarget()->GetMaxSp() / 100 + item.recover_sp, GetTarget()->GetMaxSp() - GetTarget()->GetSp()));
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
	else if (item.type == RPG::Item::Type_switch) {
		switch_id = item.switch_id;
		this->success = true;
	}

	return this->success;
}

void Game_BattleAlgorithm::Item::Apply() {
	AlgorithmBase::Apply();

	if (first_attack) {
		Main_Data::game_party->ConsumeItemUse(item.ID);
	}
}

std::string Game_BattleAlgorithm::Item::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.use_item,
			{'S', 'O'},
			{source->GetName(), item.name}
		);
	}
	else if (Player::IsRPG2k()) {
		std::string particle;
		if (Player::IsCP932())
			particle = "は";
		else
			particle = " ";
		return source->GetName() + particle + item.name + Data::terms.use_item;
	}
	else {
		return item.name;
	}
}

int Game_BattleAlgorithm::Item::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Item;
}

const RPG::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == RPG::Item::Type_medicine || item.type == RPG::Item::Type_switch) {
		return &Game_System::GetSystemSE(Game_System::SFX_UseItem);
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
			Data::terms.defending,
			{'S'},
			{source->GetName()}
		);
	}
	else if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.defending;
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::Defend::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Defending;
}

bool Game_BattleAlgorithm::Defend::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Defend::Apply() {
	ApplyActionSwitches();
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(Type::Observe, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Observe::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.observing,
			{'S'},
			{source->GetName()}
		);
	}
	else if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.observing;
	}
	else {
		return "";
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
			Data::terms.focus,
			{'S'},
			{source->GetName()}
		);
	}
	else if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.focus;
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Charge::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Charge::Apply() {
	source->SetCharged(true);
	ApplyActionSwitches();
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source, Game_Party_Base* target) :
AlgorithmBase(Type::SelfDestruct, source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::SelfDestruct::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.autodestruction,
			{'S'},
			{source->GetName()}
		);
	}
	else if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.autodestruction;
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::SelfDestruct::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_SelfDestruct;
}

const RPG::Sound* Game_BattleAlgorithm::SelfDestruct::GetStartSe() const {
	return &Game_System::GetSystemSE(Game_System::SFX_EnemyKill);
}

int Game_BattleAlgorithm::SelfDestruct::GetPhysicalDamageRate() const {
	return 100;
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Reset();

	auto* target = GetTarget();

	// Like a normal attack, but with double damage and always hitting
	// Never crits, ignores charge
	int effect = source->GetAtk() - GetTarget()->GetDef() / 2;

	if (effect < 0)
		effect = 0;

	// up to 20% stronger/weaker
	int act_perc = Utils::GetRandomNumber(-20, 20);
	int change = (int)(std::ceil(effect * act_perc / 100.0));
	effect += change;

	effect /= GetTarget()->IsDefending() ? GetTarget()->HasStrongDefense() ? 4 : 2 : 1;

	effect = Utils::Clamp(effect, 0, MaxDamageValue());

	this->hp = effect;

	if (GetTarget()->GetHp() - this->hp <= 0) {
		// Death state
		lethal = true;
		killed_by_dmg = true;
	}

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target->GetStates();
	auto target_perm_states = target->GetPermanentStates();

	// Conditions healed by physical attack:
	BattlePhysicalStateHeal(GetPhysicalDamageRate(), target_states, target_perm_states, states);

	success = true;

	return true;
}

void Game_BattleAlgorithm::SelfDestruct::Apply() {
	AlgorithmBase::Apply();

	// Only monster can self destruct
	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source, bool always_succeed) :
	AlgorithmBase(Type::Escape, source), always_succeed(always_succeed) {
	// no-op
}

std::string Game_BattleAlgorithm::Escape::GetStartMessage() const {
	// Only monsters can escape during a battle phase
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.enemy_escape,
			{'S'},
			{source->GetName()}
		);
	}
	else if (Player::IsRPG2k()) {
		if (source->GetType() == Game_Battler::Type_Enemy) {
			return source->GetName() + Data::terms.enemy_escape;
		}
	}

	return "";
}

int Game_BattleAlgorithm::Escape::GetSourceAnimationState() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetSourceAnimationState();
	}
	else {
		return Sprite_Battler::AnimationState_Dead;
	}
}

const RPG::Sound* Game_BattleAlgorithm::Escape::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetStartSe();
	}
	else {
		return &Game_System::GetSystemSE(Game_System::SFX_Escape);
	}
}

bool Game_BattleAlgorithm::Escape::Execute() {
	Reset();

	// Monsters always escape
	this->success = true;

	if (source->GetType() == Game_Battler::Type_Ally && !always_succeed) {
		int ally_agi = Main_Data::game_party->GetAverageAgility();
		int enemy_agi = Main_Data::game_enemyparty->GetAverageAgility();

		// flee chance is 0% when ally has less than 70% of enemy agi
		// 100% -> 50% flee, 200% -> 100% flee
		float to_hit = std::max(0.0f, 1.5f - ((float)enemy_agi / ally_agi));

		// Every failed escape is worth 10% higher escape chance
		to_hit += Game_Battle::GetEscapeFailureCount() * 0.1f;

		to_hit *= 100;
		this->success = Utils::PercentChance((int)to_hit);
	}

	return this->success;
}

void Game_BattleAlgorithm::Escape::Apply() {
	if (!this->success) {
		Game_Battle::IncEscapeFailureCount();
	}

	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
	ApplyActionSwitches();
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(Type::Transform, source), new_monster_id(new_monster_id) {
	// no-op
}

std::string Game_BattleAlgorithm::Transform::GetStartMessage() const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.enemy_transform,
			{'S', 'O'},
			{source->GetName(), ReaderUtil::GetElement(Data::enemies, new_monster_id)->name} // Sanity check in Game_Enemy
		);
	}
	else if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.enemy_transform;
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Transform::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Transform::Apply() {
	static_cast<Game_Enemy*>(source)->Transform(new_monster_id);
	ApplyActionSwitches();
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

void Game_BattleAlgorithm::NoMove::Apply() {
	ApplyActionSwitches();
}

