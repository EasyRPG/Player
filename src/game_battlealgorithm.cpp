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
#include "game_temp.h"
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

static inline int to_hit_physical(Game_Battler *i_source, Game_Battler *i_target, int current_to_hit) {
	int to_hit = current_to_hit;

	// If target has Restriction "do_nothing", the attack always hits
	if (i_target->GetSignificantRestriction() == RPG::State::Restriction_do_nothing) {
		return 100;
	}

	// Modify hit chance for each state the source has
	for (const auto state : i_source->GetInflictedStates()) {
		if (state > 0) {
			to_hit = to_hit * ReaderUtil::GetElement(Data::states, state)->reduce_hit_ratio / 100;
		}
	}

	// If target has physical dodge evasion:
	if (i_target->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(i_target);
		for (const auto equipment : ally->GetWholeEquipment()) {
			RPG::Item* weapon = ReaderUtil::GetElement(Data::items, equipment);
			if (weapon != nullptr && (weapon->type == RPG::Item::Type_shield || weapon->type == RPG::Item::Type_armor
				|| weapon->type == RPG::Item::Type_helmet || weapon->type == RPG::Item::Type_accessory) && weapon->raise_evasion) {
				to_hit = to_hit * (100 - 25) / 100;
				break;
			}
		}
	}

	return to_hit;
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source) :
	source(source), no_target(true), first_attack(true) {
	Reset();

	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Battler* target) :
	source(source), no_target(false), first_attack(true) {
	Reset();

	SetTarget(target);
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Party_Base* target) :
	source(source), no_target(false), first_attack(true) {
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
	killed_by_attack_damage = false;
	critical_hit = false;
	absorb = false;
	reflect = -1;
	animation = nullptr;
	animation2 = nullptr;
	has_animation_played = false;
	has_animation2_played = false;
	conditions.clear();
	healed_conditions.clear();
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

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSwitch() const {
	return switch_id;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsPositive() const {
	return healing;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsAbsorb() const {
	return absorb;
}

const RPG::Item* Game_BattleAlgorithm::AlgorithmBase::GetItem() const {
	return nullptr;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetType() const {
	return "Base";
}

const std::vector<RPG::State>& Game_BattleAlgorithm::AlgorithmBase::GetAffectedConditions() const {
	return conditions;
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
		std::vector<Game_Battler*> anim_targets = { GetSource() };
		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, anim_targets);
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
		std::vector<Game_Battler*> anim_targets = { GetSource() };
		Game_Battle::ShowBattleAnimation(GetSecondAnimation()->ID, anim_targets);
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
		std::vector<Game_Battler*> anim_targets = { GetSource() };
		Game_Battle::ShowBattleAnimation(GetAnimation()->ID, anim_targets, false, true, cutoff);
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
		anim_targets, false, true, cutoff);

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

bool Game_BattleAlgorithm::AlgorithmBase::IsKilledByAttack() const {
	return killed_by_attack_damage;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsCriticalHit() const {
	return critical_hit;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsFirstAttack() const {
	return first_attack;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsSecondStartMessage() const {
	return false;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetSecondStartMessage() const {
	return "";
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetDeathMessage() const {
	if (!killed_by_attack_damage) {
		return "";
	}

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
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << value;
		return Utils::ReplacePlaceholders(
			Data::terms.hp_recovery,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), ss.str(), points}
		);
	}
	else {
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
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << value;
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'O', 'V', 'U'},
			{GetSource()->GetName(), GetTarget()->GetName(), ss.str(), points}
		);
	}
	else {
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

	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << value;
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), ss.str(), Data::terms.health_points}
		);
	}
	else {
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
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << value;
		return Utils::ReplacePlaceholders(
			message,
			{'S', 'V', 'U'},
			{GetTarget()->GetName(), ss.str(), points}
		);
	}
	else {
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

std::string Game_BattleAlgorithm::AlgorithmBase::GetShiftAttributeMessage(bool is_positive, const std::string& attribute) const {
	const std::string& message = is_positive ?
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
			// FIXME?? Verify if these particle are correct
			particle = "の ";
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

void Game_BattleAlgorithm::AlgorithmBase::GetResultMessages(std::vector<std::string>& out, std::vector<int>& out_replace) const {
	if (current_target == targets.end()) {
		return;
	}

	if (!success) {
		out_replace.push_back(0);
		out.push_back(GetAttackFailureMessage(Data::terms.dodge));
		return;
	}

	if (GetAffectedHp() != -1) {

		if (IsPositive()) {
			if (!GetTarget()->IsDead()) {
				out_replace.push_back(0);
				out.push_back(GetHpSpRecoveredMessage(GetAffectedHp(), Data::terms.health_points));
			}
		}
		else {
			if (critical_hit) {
				out_replace.push_back(0);
				out.push_back(GetCriticalHitMessage());
			}

			out_replace.push_back(0);
			if (GetAffectedHp() == 0) {
				out.push_back(GetUndamagedMessage());
			}
			else {
				if (IsAbsorb()) {
					out.push_back(GetHpSpAbsorbedMessage(GetAffectedHp(), Data::terms.health_points));
				}
				else {
					out.push_back(GetDamagedMessage());
				}
			}
		}

		// If enemy is killed, it ends here
		if (killed_by_attack_damage) {
			out_replace.push_back(1);
			out.push_back(GetDeathMessage());
			return;
		}

		// Healed conditions messages
		std::vector<int16_t>::const_iterator it_healed = healed_conditions.begin();
		for (; it_healed != healed_conditions.end(); it_healed++) {
			out_replace.push_back(1);
			out.push_back(GetStateMessage(ReaderUtil::GetElement(Data::states, *it_healed)->message_recovery));
		}
	}

	if (GetAffectedSp() != -1) {
		out_replace.push_back(0);
		if (IsPositive()) {
			out.push_back(GetHpSpRecoveredMessage(GetAffectedSp(), Data::terms.spirit_points));
		}
		else if (GetAffectedSp() > 0) {
			if (IsAbsorb()) {
				out.push_back(GetHpSpAbsorbedMessage(GetAffectedSp(), Data::terms.spirit_points));
			}
			else {
				out.push_back(GetParameterChangeMessage(false, GetAffectedSp(), Data::terms.spirit_points));
			}
		}
	}

	if (GetAffectedAttack() > 0) {
		out_replace.push_back(0);
		out.push_back(GetParameterChangeMessage(IsPositive(), GetAffectedAttack(), Data::terms.attack));
	}

	if (GetAffectedDefense() > 0) {
		out_replace.push_back(0);
		out.push_back(GetParameterChangeMessage(IsPositive(), GetAffectedDefense(), Data::terms.defense));
	}

	if (GetAffectedSpirit() > 0) {
		out_replace.push_back(0);
		out.push_back(GetParameterChangeMessage(IsPositive(), GetAffectedSpirit(), Data::terms.spirit));
	}

	if (GetAffectedAgility() > 0) {
		out_replace.push_back(0);
		out.push_back(GetParameterChangeMessage(IsPositive(), GetAffectedAgility(), Data::terms.agility));
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	for (; it != conditions.end(); ++it) {
		if (GetTarget()->HasState(it->ID) && std::find(healed_conditions.begin(), healed_conditions.end(), it->ID) == healed_conditions.end()) {
			if (IsPositive()) {
				out_replace.push_back(0);
				out.push_back(GetStateMessage(it->message_recovery));
			}
			else if (!it->message_already.empty()) {
				out_replace.push_back(0);
				out.push_back(GetStateMessage(it->message_already));
			}
		} else {
			// Positive case doesn't report anything in case of uselessness
			if (IsPositive()) {
				continue;
			}

			bool is_actor = GetTarget()->GetType() == Game_Battler::Type_Ally;
			out_replace.push_back(0);
			out.push_back(GetStateMessage(is_actor ? it->message_actor : it->message_enemy));

			// Reporting ends with death state
			if (it->ID == 1) {
				return;
			}
		}
	}
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

float Game_BattleAlgorithm::AlgorithmBase::GetAttributeMultiplier(const std::vector<bool>& attributes_set) const {
	float physical = -1001, magical = -1001;
	for (unsigned int i = 0; i < attributes_set.size(); i++) {
		if (attributes_set[i]) {
			if (ReaderUtil::GetElement(Data::attributes, i + 1)->type == RPG::Attribute::Type_physical) {
				physical = std::max<float>(physical, GetTarget()->GetAttributeModifier(i + 1));
			}
			else {
				magical = std::max<float>(magical, GetTarget()->GetAttributeModifier(i + 1));
			}
		}
	}
	physical = physical < -1000 ? 100 : physical;
	magical = magical < -1000 ? 100 : magical;

	return physical * magical / 10000;
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
	if (!success)
		return;

	if (GetAffectedHp() != -1 && !GetTarget()->IsDead()) {
		int hp = GetAffectedHp();
		int target_hp = GetTarget()->GetHp();
		GetTarget()->ChangeHp(IsPositive() ? hp : -hp);
		if (IsAbsorb()) {
			// Only absorb the hp that were left
			int src_hp = std::min(target_hp, hp);
			source->ChangeHp(src_hp);
		}
	}

	if (GetAffectedSp() != -1) {
		int sp = GetAffectedSp();
		int target_sp = GetTarget()->GetSp();
		GetTarget()->SetSp(GetTarget()->GetSp() + (IsPositive() ? sp : -sp));
		if (IsAbsorb()) {
			int src_sp = std::min(target_sp, sp);
			source->ChangeSp(src_sp);
		}
	}

	if (GetAffectedAttack() != -1) {
		int atk = GetAffectedAttack();
		GetTarget()->ChangeAtkModifier(IsPositive() ? atk : -atk);
		if (IsAbsorb()) {
			atk = std::max<int>(0, std::min<int>(atk, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeAtkModifier(atk);
		}
	}

	if (GetAffectedDefense() != -1) {
		int def = GetAffectedDefense();
		GetTarget()->ChangeDefModifier(IsPositive() ? def : -def);
		if (IsAbsorb()) {
			def = std::max<int>(0, std::min<int>(def, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeDefModifier(def);
		}
	}

	if (GetAffectedSpirit() != -1) {
		int spi = GetAffectedSpirit();
		GetTarget()->ChangeSpiModifier(IsPositive() ? spi : -spi);
		if (IsAbsorb()) {
			spi = std::max<int>(0, std::min<int>(spi, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeSpiModifier(spi);
		}
	}

	if (GetAffectedAgility() != -1) {
		int agi = GetAffectedAgility();
		GetTarget()->ChangeAgiModifier(IsPositive() ? agi : -agi);
		if (IsAbsorb()) {
			agi = std::max<int>(0, std::min<int>(agi, std::min<int>(source->MaxStatBattleValue(), source->GetBaseAtk() * 2) - source->GetAtk()));
			source->ChangeAgiModifier(agi);
		}
	}

	if (GetAffectedSwitch() != -1) {
		Game_Switches[GetAffectedSwitch()] = true;
	}

	// Conditions healed by physical attack:
	std::vector<int16_t>::const_iterator it_healed = healed_conditions.begin();
	for (; it_healed != healed_conditions.end(); ++it_healed) {
		GetTarget()->RemoveState(*it_healed);
	}

	// Conditions healed/caused:
	std::vector<RPG::State>::const_iterator it = conditions.begin();
	for (; it != conditions.end(); ++it) {
		if (IsPositive()) {
			if (GetTarget()->IsDead() && it->ID == 1) {
				// Was a revive skill with an effect rating of 0
				GetTarget()->ChangeHp(std::max<int>(1, GetAffectedHp()));
			}

			GetTarget()->RemoveState(it->ID);
		}
		else {
			if (it->ID == 1) {
				GetTarget()->ChangeHp(-(GetTarget()->GetHp()));
			} else {
				GetTarget()->AddState(it->ID);
			}
		}
	}

	source->SetDefending(false);

	for (int s : switch_on) {
		Game_Switches[s] = true;
	}

	for (int s : switch_off) {
		Game_Switches[s] = false;
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyFirst() {
	// no-op
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

	return (!GetTarget()->IsDead() && !GetTarget()->IsHidden());
}

int Game_BattleAlgorithm::AlgorithmBase::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Idle;
}

int Game_BattleAlgorithm::AlgorithmBase::GetSourceAnimationStateApply() const {
	return NULL;
}

void Game_BattleAlgorithm::AlgorithmBase::TargetFirst() {
	current_target = targets.begin();

	if (!IsTargetValid()) {
		AlgorithmBase::TargetNext();
	}

	first_attack = true;
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	if (IsReflected()) {
		// Only source available, can't target again
		return false;
	}

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

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchEnable(int switch_id) {
	switch_on.push_back(switch_id);
}

void Game_BattleAlgorithm::AlgorithmBase::SetSwitchDisable(int switch_id) {
	switch_off.push_back(switch_id);
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	return NULL;
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

std::vector<RPG::State> Game_BattleAlgorithm::AlgorithmBase::FilterStatesByPriority(std::vector<RPG::State> filter_states) {
	if (filter_states.empty())
		return filter_states;

	std::vector<RPG::State> return_states;
	int new_priority = -1;
	for (auto state : filter_states) {
		if (state.ID == 1) {
			return_states.push_back(state);
			return return_states;
		}
		new_priority = std::max(new_priority, state.priority);
	}
	for (auto state_id : GetTarget()->GetInflictedStates()) {
		if (std::find(healed_conditions.begin(), healed_conditions.end(), state_id) == healed_conditions.end()) {
			new_priority = std::max(new_priority, ReaderUtil::GetElement(Data::states, state_id)->priority);
		}
	}

	for (auto state : filter_states) {
		if (state.priority >= new_priority - 9) {
			return_states.push_back(state);
		}
	}
	return return_states;
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(source, target) {
	// no-op
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Party_Base* target) :
	AlgorithmBase(source, target) {
	// no-op
}

bool Game_BattleAlgorithm::Normal::Execute() {
	Reset();

	int to_hit;
	float multiplier = 1;
	float crit_chance = source->GetCriticalHitChance();
	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		int hit_chance = source->GetHitChance();
		const RPG::Item* weapon = ReaderUtil::GetElement(Data::items, ally->GetWeaponId());
		const RPG::Item* weapon2 = ally->HasTwoWeapons() ? ally->GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;

		if (!weapon && !weapon2) {
			// No Weapon
			// TODO: Different behavior for 2k3
			const RPG::Actor& actor = *ReaderUtil::GetElement(Data::actors, ally->GetId());
			animation = ReaderUtil::GetElement(Data::animations, actor.unarmed_animation);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid unarmed animation ID %d", actor.unarmed_animation);
			}
		}
		else if (!weapon) {
			animation = ReaderUtil::GetElement(Data::animations, weapon->animation_id);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon->animation_id);
			}

			hit_chance = weapon2->hit;
			crit_chance += (1.0 - crit_chance) * weapon2->critical_hit / 100.0f;
			multiplier = GetAttributeMultiplier(weapon2->attribute_set);
		}
		else if (!weapon2) {
			animation = ReaderUtil::GetElement(Data::animations, weapon->animation_id);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon->animation_id);
			}

			hit_chance = weapon->hit;
			crit_chance += (1.0 - crit_chance) * weapon->critical_hit / 100.0f;
			multiplier = GetAttributeMultiplier(weapon->attribute_set);
		}
		else {
			animation = ReaderUtil::GetElement(Data::animations, weapon->animation_id);
			animation2 = ReaderUtil::GetElement(Data::animations, weapon2->animation_id);
			if (!animation) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon->animation_id);
			}
			if (!animation2) {
				Output::Warning("Algorithm Normal: Invalid weapon animation ID %d", weapon2->animation_id);
			}

			hit_chance = (weapon->hit + weapon2->hit) / 2;
			crit_chance += (1.0 - crit_chance) * (weapon->critical_hit + weapon2->critical_hit) / 200.0f;

			std::vector<bool> weapon12_attribute_set;
			for (int i = 0; i < Data::attributes.size(); ++i) {
				weapon12_attribute_set.push_back((weapon->attribute_set.size() < i + 1 ? false : weapon->attribute_set[i])
					|| (weapon2->attribute_set.size() < i + 1 ? false : weapon2->attribute_set[i]));
			}
			multiplier = GetAttributeMultiplier(weapon12_attribute_set);
		}

		if ((weapon && weapon->ignore_evasion) || (weapon2 && weapon2->ignore_evasion)) {
			to_hit = (int)(100 - (100 - hit_chance));
		} else {
			to_hit = (int)(100 - (100 - hit_chance) * (1 + (1.0 * GetTarget()->GetAgi() / ally->GetAgi() - 1) / 2));
		}
	} else {
		// Source is Enemy
		int hit = source->GetHitChance();
		to_hit = (int)(100 - (100 - hit) * (1 + (1.0 * GetTarget()->GetAgi() / source->GetAgi() - 1) / 2));
		if (!Data::animations.empty()) {
			animation = &Data::animations[0];
		}
	}

	to_hit = to_hit_physical(GetSource(), GetTarget(), to_hit);

	// If the target actor has a "Prevent Critic Hits" armor, there's no critic:
	if (GetTarget()->GetType() == Game_Battler::Type_Ally) {
		for (auto object_id : static_cast<Game_Actor*>(GetTarget())->GetWholeEquipment()) {
			RPG::Item *object = ReaderUtil::GetElement(Data::items, object_id);
			if (object != nullptr && (object->type == RPG::Item::Type_shield || object->type == RPG::Item::Type_armor
				|| object->type == RPG::Item::Type_helmet || object->type == RPG::Item::Type_accessory) && object->prevent_critical) {
				crit_chance = 0.0;
				break;
			}
		}
	}

	// Damage calculation
	if (Utils::GetRandomNumber(0, 99) < to_hit) {
		if (!source->IsCharged() && Utils::GetRandomNumber(0, 99) < (int)ceil(crit_chance * 100)) {
			critical_hit = true;
		}

		int effect = (source->GetAtk() / 2 - GetTarget()->GetDef() / 4);

		if (effect < 0)
			effect = 0;

		// up to 20% stronger/weaker
		int act_perc = Utils::GetRandomNumber(-20, 20);
		// Change rounded up
		int change = (int)(std::ceil(effect * act_perc / 100.0));
		effect += change;
		effect *= multiplier;
		if(effect < 0) {
			effect = 0;
		}
		this->hp = (effect * (critical_hit ? 3 : 1) * (source->IsCharged() ? 2 : 1)) /
			(GetTarget()->IsDefending() ? GetTarget()->HasStrongDefense() ? 4 : 2 : 1);

		if (GetTarget()->GetHp() - this->hp <= 0) {
			// Death state
			killed_by_attack_damage = true;
		}
		else {
			// Conditions healed by physical attack:
			if (!IsPositive())
				healed_conditions = GetTarget()->BattlePhysicalStateHeal(GetPhysicalDamageRate());

			// Conditions caused:
			if (source->GetType() == Game_Battler::Type_Ally) {
				Game_Actor* actor = static_cast<Game_Actor*>(source);
				
				const RPG::Item* weapon = actor->GetEquipment(RPG::Item::Type_weapon);
				const RPG::Item* weapon2 = actor->HasTwoWeapons()? actor->GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;

				if (weapon) {
					for (unsigned int i = 0; i < weapon->state_set.size(); i++) {
						if (weapon->state_set[i]) {
							RPG::State* state = ReaderUtil::GetElement(Data::states, i + 1);
							if (!state) {
								Output::Warning("Algorithm Normal: Weapon %d causes invalid state %d", weapon->ID, weapon->state_set[i]);
								continue;
							}

							if (Utils::GetRandomNumber(0, 99) < (weapon->state_chance * GetTarget()->GetStateProbability(state->ID) / 100)) {
								if (weapon->state_effect) {
									healing = true;
								}
								conditions.push_back(*state);
							}
						}
					}
				}

				if (weapon2) {
					struct checker {
						checker(const RPG::State* compare_to) : compare_to_(compare_to) { }
						bool operator()(RPG::State elem) const { return compare_to_->ID == elem.ID; }
					private:
						const RPG::State* compare_to_;
					};

					for (unsigned int i = 0; i < weapon2->state_set.size(); i++) {
						if (weapon2->state_set[i]) {
							RPG::State* state = ReaderUtil::GetElement(Data::states, i + 1);
							if (!state) {
								Output::Warning("Algorithm Normal: Weapon %d causes invalid state %d", weapon->ID, weapon->state_set[i]);
								continue;
							}

							if (std::find_if(conditions.begin(), conditions.end(), checker(state)) == conditions.end()
								&& Utils::GetRandomNumber(0, 99) < (weapon2->state_chance * GetTarget()->GetStateProbability(state->ID) / 100)) {
								if (weapon2->state_effect) {
									healing = true;
								}
								
								conditions.push_back(*state);
							}
						}
					}
				}

				conditions = FilterStatesByPriority(conditions);
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
		Game_Actor* src = static_cast<Game_Actor*>(source);
		const RPG::Item* weapon = ReaderUtil::GetElement(Data::items, src->GetWeaponId());
		if (weapon) {
			source->ChangeSp(std::ceil(-weapon->sp_cost / (float) src->GetSpCostModifier()));
		}
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

std::string Game_BattleAlgorithm::Normal::GetType() const {
	return "Normal";
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source, target), skill(skill), item(item) {
	// no-op
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Party_Base* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source, target), skill(skill), item(item) {
	// no-op
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source), skill(skill), item(item) {
	// no-op
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
	int effect;

	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	animation = nullptr;
	if (skill.animation_id != 0) {
		animation = ReaderUtil::GetElement(Data::animations, skill.animation_id);
		if (!animation) {
			Output::Warning("Algorithm Skill: Invalid skill animation ID %d", skill.animation_id);
		}
	}

	absorb = false;
	this->success = false;

	this->healing =
		skill.scope == RPG::Skill::Scope_ally ||
		skill.scope == RPG::Skill::Scope_party ||
		skill.scope == RPG::Skill::Scope_self;

	if (skill.type == RPG::Skill::Type_normal ||
		skill.type >= RPG::Skill::Type_subskill) {
		if (this->healing) {
			float mul = GetAttributeMultiplier(skill.attribute_effects);
			if (mul < 0.5f) {
				// Determined via testing, the heal is always at least 50%
				mul = 0.5f;
			}
			effect = skill.power +
				source->GetAtk() * skill.physical_rate / 20 +
				source->GetSpi() * skill.magical_rate / 40;
			effect *= mul;

			effect += (effect * Utils::GetRandomNumber(-skill.variance * 10, skill.variance * 10) / 100);

			if (effect < 0)
				effect = 0;

			if (skill.affect_hp)
				this->hp = std::max<int>(0, std::min<int>(effect, GetTarget()->GetMaxHp() - GetTarget()->GetHp()));
			if (skill.affect_sp) {
				int prov_sp = GetSource() == GetTarget() ? source->CalculateSkillCost(skill.ID) : 0;
				this->sp = std::max<int>(0, std::min<int>(effect, GetTarget()->GetMaxSp() - GetTarget()->GetSp() + prov_sp));
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
		}
		else {
			absorb = skill.absorb_damage;

			int to_hit = skill.hit;

			//If Physical technique, apply physical restrictions
			if (skill.failure_message == 3) {
				to_hit = to_hit_physical(GetSource(), GetTarget(), to_hit);
			}

			if (Utils::GetRandomNumber(0, 99) < to_hit) {
				effect = skill.power +
					source->GetAtk() * skill.physical_rate / 20 +
					source->GetSpi() * skill.magical_rate / 40;
				if (!skill.ignore_defense) {
					effect -= GetTarget()->GetDef() * skill.physical_rate / 40;
					effect -= GetTarget()->GetSpi() * skill.magical_rate / 80;
				}
				effect *= GetAttributeMultiplier(skill.attribute_effects);

				effect += (effect * Utils::GetRandomNumber(-skill.variance * 10, skill.variance * 10) / 100);

				if (effect < 0)
					effect = 0;

				if (skill.affect_hp) {
					this->hp = effect /
						(GetTarget()->IsDefending() ? GetTarget()->HasStrongDefense() ? 4 : 2 : 1);

					if (IsAbsorb())
						this->hp = std::min<int>(hp, GetTarget()->GetHp());

					if (GetTarget()->GetHp() - this->hp <= 0) {
						// Death state
						killed_by_attack_damage = true;
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
					(IsAbsorb() && ((GetAffectedHp() == 0 && GetAffectedSp() <= 0) || (GetAffectedHp() <= 0 && GetAffectedSp() == 0))) ||
					(!IsAbsorb() && GetAffectedSp() == 0 && GetAffectedHp() == -1))
					return this->success;
			}
		}

		// Conditions:
		for (int i = 0; i < (int) skill.state_effects.size(); i++) {
			if (!skill.state_effects[i])
				continue;
			if (!healing && GetTarget()->HasState(i + 1)) {
				this->success = true;
				conditions.push_back(Data::states[i]);
				continue;
			}
			if (healing && !GetTarget()->HasState(i + 1)) {
				continue;
			}
			if (Utils::GetRandomNumber(0, 99) >= skill.hit)
				continue;

			if (healing || Utils::GetRandomNumber(0, 99) <= GetTarget()->GetStateProbability(Data::states[i].ID)) {
				conditions.push_back(Data::states[i]);
			}
		}
		conditions = FilterStatesByPriority(conditions);
		if (!conditions.empty()) {
			this->success = true;
		}

		//If resurrected and no HP selected, the effect value is a percentage:
		if (healing && GetTarget()->IsDead() && !skill.affect_hp) {
			for (auto condition : conditions) {
				if (condition.ID == 1) {
					hp = GetTarget()->GetMaxHp() * effect / 100;
					this->success = true;
					break;
				}
			}
		}

		// Attribute resistance / weakness + an attribute selected + can be modified
		if (!success && skill.affect_attr_defence) {
			for (int i = 0; i < skill.attribute_effects.size(); i++) {
				if (skill.attribute_effects[i] && GetTarget()->GetShiftAttributeRate(i + 1) * (IsPositive() ? 1 : -1) != 1) {
					if (Utils::GetRandomNumber(0, 99) >= skill.hit)
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

	std::vector<int16_t>::const_iterator it_shift = shift_attributes.begin();
	for (; it_shift != shift_attributes.end(); ++it_shift) {
		GetTarget()->ShiftAttributeRate(*it_shift, healing ? 1 : -1);
	}
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		if (item && item->using_message == 0) {
			// Use item message
			return Item(source, *item).GetStartMessage();
		}
		if (Player::IsRPG2kE()) {
			return Utils::ReplacePlaceholders(
				skill.using_message1,
				{'S', 'O', 'U'},
				{GetSource()->GetName(), GetTarget()->GetName(), skill.name}
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

bool Game_BattleAlgorithm::Skill::IsSecondStartMessage() const {
	return Player::IsRPG2k() && (!item || item->using_message != 0) && !skill.using_message2.empty();
}

std::string Game_BattleAlgorithm::Skill::GetSecondStartMessage() const {
	if (Player::IsRPG2k()) {
		if (item && item->using_message == 0) {
			return "";
		}
		if (Player::IsRPG2kE()) {
			return Utils::ReplacePlaceholders(
				skill.using_message2,
				{ 'S', 'O', 'U' },
				{ GetSource()->GetName(), GetTarget()->GetName(), skill.name }
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

const RPG::Sound* Game_BattleAlgorithm::Skill::GetResultSe() const {
	return !success && skill.failure_message != 3 ? NULL : AlgorithmBase::GetResultSe();
}

void Game_BattleAlgorithm::Skill::GetResultMessages(std::vector<std::string>& out, std::vector<int>& out_replace) const {
	if (!success) {
		out_replace.push_back(0);
		switch (skill.failure_message) {
			case 0:
				out.push_back(AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_a));
				break;
			case 1:
				out.push_back(AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_b));
				break;
			case 2:
				out.push_back(AlgorithmBase::GetAttackFailureMessage(Data::terms.skill_failure_c));
				break;
			case 3:
				out.push_back(AlgorithmBase::GetAttackFailureMessage(Data::terms.dodge));
				break;
			default:
				out.push_back("BUG: INVALID SKILL FAIL MSG");
		}
		return;
	}

	AlgorithmBase::GetResultMessages(out, out_replace);

	// Attribute resistance / weakness + an attribute selected + can be modified
	std::vector<int16_t>::const_iterator it_shift = shift_attributes.begin();
	for (; it_shift != shift_attributes.end(); ++it_shift) {
		out_replace.push_back(0);
		out.push_back(GetShiftAttributeMessage(IsPositive(), ReaderUtil::GetElement(Data::attributes, *it_shift)->name));
	}
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

int Game_BattleAlgorithm::Skill::GetSpCost() const {
	return item? -1 : source->CalculateSkillCost(skill.ID);
}

const RPG::Item* Game_BattleAlgorithm::Skill::GetItem() const {
	return item;
}

std::string Game_BattleAlgorithm::Skill::GetType() const {
	return "Skill";
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Battler* target, const RPG::Item& item) :
	AlgorithmBase(source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Party_Base* target, const RPG::Item& item) :
	AlgorithmBase(source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, const RPG::Item& item) :
AlgorithmBase(source), item(item) {
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

	// All other items are handled as skills because they invoke skills
	switch (item.type) {
		case RPG::Item::Type_medicine:
		case RPG::Item::Type_switch:
			break;
		default:
			assert("Unsupported battle item type");
	}

	this->success = false;

	if (item.type == RPG::Item::Type_medicine) {
		if (GetTarget()->GetType() == Game_Battler::Type_Ally && !item.actor_set[GetTarget()->GetId() - 1]) {
			return this->success;
		}
		if (item.ko_only && !GetTarget()->IsDead()) {
			return this->success;
		}
		this->healing = true;

		// HP recovery
		if (item.recover_hp != 0 || item.recover_hp_rate != 0) {
			this->hp = std::max<int>(0, std::min<int>(item.recover_hp_rate * GetTarget()->GetMaxHp() / 100 + item.recover_hp, GetTarget()->GetMaxHp() - GetTarget()->GetHp()));
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->sp = std::max<int>(0, std::min<int>(item.recover_sp_rate * GetTarget()->GetMaxSp() / 100 + item.recover_sp, GetTarget()->GetMaxSp() - GetTarget()->GetSp()));
		}

		bool is_dead_cured = false;
		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				if (i == 0)
					is_dead_cured = true;
				if (GetTarget()->HasState(i + 1))
					this->conditions.push_back(Data::states[i]);
			}
		}

		if (GetTarget()->IsDead() && !is_dead_cured)
			this->hp = -1;

		this->success = this->hp > -1 || this->sp > -1 || !conditions.empty();
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

void Game_BattleAlgorithm::Item::GetResultMessages(std::vector<std::string>& out, std::vector<int>& out_replace) const {
	if (success)
		AlgorithmBase::GetResultMessages(out, out_replace);
}

const RPG::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == RPG::Item::Type_medicine || item.type == RPG::Item::Type_switch) {
		return &Game_System::GetSystemSE(Game_System::SFX_UseItem);
	}
	else {
		return NULL;
	}
}

const RPG::Item* Game_BattleAlgorithm::Item::GetItem() const {
	return &item;
}

std::string Game_BattleAlgorithm::Item::GetType() const {
	return "Item";
}

Game_BattleAlgorithm::NormalDual::NormalDual(Game_Battler* source, Game_Battler* target) :
	Normal(source, target), second_attack(false) {
	// no-op
}

Game_BattleAlgorithm::NormalDual::NormalDual(Game_Battler* source, Game_Party_Base* target) :
	Normal(source, target), second_attack(false) {
	// no-op
}

bool Game_BattleAlgorithm::NormalDual::TargetNext() {
	if (IsReflected()) {
		// Only source available, can't target again
		second_attack = false;
		return false;
	}
	if (!second_attack && !GetTarget()->IsDead()) {
		first_attack = false;
		second_attack = true;
		return true;
	}
	else {
		second_attack = false;
	}

	return TargetNextInternal();
}

std::string Game_BattleAlgorithm::NormalDual::GetType() const {
	return "NormalDual";
}

Game_BattleAlgorithm::Defend::Defend(Game_Battler* source) :
	AlgorithmBase(source) {
	// no-op
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
	return true;
}

void Game_BattleAlgorithm::Defend::Apply() {
	source->SetDefending(true);
}

void Game_BattleAlgorithm::Defend::ApplyFirst() {
	source->SetDefending(true);
}

std::string Game_BattleAlgorithm::Defend::GetType() const {
	return "Defend";
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(source) {
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
	return true;
}

std::string Game_BattleAlgorithm::Observe::GetType() const {
	return "Observe";
}

Game_BattleAlgorithm::Charge::Charge(Game_Battler* source) :
AlgorithmBase(source) {
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
	return true;
}

void Game_BattleAlgorithm::Charge::Apply() {
	source->SetCharged(true);
}

std::string Game_BattleAlgorithm::Charge::GetType() const {
	return "Charge";
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source, Game_Party_Base* target) :
AlgorithmBase(source, target) {
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

int Game_BattleAlgorithm::SelfDestruct::GetSourceAnimationStateApply() const {
	return Sprite_Battler::AnimationState_DeadSelfDestruct;
}

const RPG::Sound* Game_BattleAlgorithm::SelfDestruct::GetStartSe() const {
	return &Game_System::GetSystemSE(Game_System::SFX_EnemyKill);
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Reset();

	// Like a normal attack, but with double damage and always hitting
	// Never crits, ignores charge
	int effect = source->GetAtk() - GetTarget()->GetDef() / 2;

	if (effect < 0)
		effect = 0;

	// up to 20% stronger/weaker
	int act_perc = Utils::GetRandomNumber(-20, 20);
	int change = (int)(std::ceil(effect * act_perc / 100.0));
	effect += change;

	if (effect < 0)
		effect = 0;

	this->hp = effect / (
		GetTarget()->IsDefending() ? GetTarget()->HasStrongDefense() ? 4 : 2 : 1);

	if (GetTarget()->GetHp() - this->hp <= 0) {
		// Death state
		killed_by_attack_damage = true;
	}

	// Conditions healed by physical attack:
	std::vector<int16_t>::const_iterator it_healed = healed_conditions.begin();
	for (; it_healed != healed_conditions.end(); ++it_healed) {
		GetTarget()->RemoveState(*it_healed);
	}

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

std::string Game_BattleAlgorithm::SelfDestruct::GetType() const {
	return "SelfDestruct";
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source) :
	AlgorithmBase(source) {
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
		return Sprite_Battler::AnimationState_Escape;
	}
}

int Game_BattleAlgorithm::Escape::GetSourceAnimationStateApply() const {
	return Sprite_Battler::AnimationState_Dead;
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

	// Preemptive attack has 100% escape ratio
	if (source->GetType() == Game_Battler::Type_Ally && !(Game_Temp::battle_first_strike && Game_Battle::GetTurn() == 0)) {
		int ally_agi = Main_Data::game_party->GetAverageAgility();
		int enemy_agi = Main_Data::game_enemyparty->GetAverageAgility();

		// flee chance is 0% when ally has less than 70% of enemy agi
		// 100% -> 50% flee, 200% -> 100% flee
		float to_hit = std::max(0.0f, 1.5f - ((float)enemy_agi / ally_agi));

		// Every failed escape is worth 10% higher escape chance
		to_hit += Game_Battle::escape_fail_count * 0.1f;

		to_hit *= 100;
		this->success = Utils::GetRandomNumber(0, 99) < (int)to_hit;
	}

	return this->success;
}

void Game_BattleAlgorithm::Escape::Apply() {
	if (!this->success) {
		Game_Battle::escape_fail_count += 1;
	}

	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
}

void Game_BattleAlgorithm::Escape::GetResultMessages(std::vector<std::string>& out, std::vector<int>& out_replace) const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		out_replace.push_back(0);
		if (this->success) {
			out.push_back(Data::terms.escape_success);
		}
		else {
			out.push_back(Data::terms.escape_failure);
		}
	}
}

std::string Game_BattleAlgorithm::Escape::GetType() const {
	return "Escape";
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(source), new_monster_id(new_monster_id) {
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
	return true;
}

void Game_BattleAlgorithm::Transform::Apply() {
	static_cast<Game_Enemy*>(source)->Transform(new_monster_id);
}

std::string Game_BattleAlgorithm::Transform::GetType() const {
	return "Transform";
}

Game_BattleAlgorithm::NoMove::NoMove(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::NoMove::GetStartMessage() const {
	const std::vector<int16_t>& states = source->GetStates();

	for (std::vector<int16_t>::const_iterator it = states.begin();
		it != states.end(); ++it) {
		if (Data::states[*it].restriction == RPG::State::Restriction_do_nothing) {
			std::string msg = Data::states[*it].message_affected;
			if (!msg.empty()) {
				return source->GetName() + msg;
			}
			return "";
		}
	}

	// State was healed before the actor got his turn
	return "";
}

bool Game_BattleAlgorithm::NoMove::Execute() {
	// no-op
	return true;
}

void Game_BattleAlgorithm::NoMove::Apply() {
	// no-op
}

std::string Game_BattleAlgorithm::NoMove::GetType() const {
	return "NoMove";
}
