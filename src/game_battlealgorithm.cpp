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
#include "battle_message.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <lcf/rpg/animation.h>
#include <lcf/rpg/state.h>
#include <lcf/rpg/skill.h>
#include <lcf/rpg/item.h>
#include <lcf/rpg/battleranimation.h>
#include "sprite_battler.h"
#include "utils.h"
#include "rand.h"
#include "state.h"
#include "enemyai.h"
#include "algo.h"
#include "attribute.h"
#include "spriteset_battle.h"

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

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(ty, source, std::vector<Game_Battler*>{ target }) {}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, std::vector<Game_Battler*> in_targets) :
	type(ty), source(source), targets(std::move(in_targets))
{
	assert(source != nullptr);
	for (auto* t: targets) {
		assert(t != nullptr);
	}

	Reset();

	source->SetIsDefending(false);
	physical_charged = source->IsCharged();

	num_original_targets = targets.size();
	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Type ty, Game_Battler* source, Game_Party_Base* target) :
	type(ty), source(source)
{
	assert(source != nullptr);
	assert(target != nullptr);

	Reset();

	source->SetIsDefending(false);
	physical_charged = source->IsCharged();

	current_target = targets.end();
	party_target = target;
}

void Game_BattleAlgorithm::AlgorithmBase::Reset() {
	hp = 0;
	sp = 0;
	attack = 0;
	defense = 0;
	spirit = 0;
	agility = 0;
	switch_id = 0;
	affect_hp = false;
	affect_sp = false;
	affect_atk = false;
	affect_def = false;
	affect_spi = false;
	affect_agi = false;
	positive = false;
	success = false;
	critical_hit = false;
	absorb = false;
	revived = false;
	states.clear();
	attributes.clear();
}

int Game_BattleAlgorithm::AlgorithmBase::PlayAnimation(int anim_id, bool only_sound, int cutoff, bool invert) {
	if (anim_id == 0) {
		return 0;
	}

	std::vector<Game_Battler*> anim_targets;
	for (auto iter = current_target; iter != targets.end(); ++iter) {
		if (IsTargetValid(**iter)) {
			anim_targets.push_back(*iter);
		}
	}

	return Game_Battle::ShowBattleAnimation(anim_id, anim_targets, only_sound, cutoff, invert);
}

bool Game_BattleAlgorithm::AlgorithmBase::IsSuccess() const {
	return success;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsCriticalHit() const {
	return critical_hit;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetFailureMessage() const {
	return BattleMessage::GetPhysicalFailureMessage(*GetSource(), *GetTarget());
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

void Game_BattleAlgorithm::AlgorithmBase::ApplyCustomEffect() {
}

void Game_BattleAlgorithm::AlgorithmBase::ApplySwitchEffect() {
	const auto sw = GetAffectedSwitch();
	if (sw > 0) {
		Main_Data::game_switches->Set(sw, true);
	}
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyHpEffect() {
	auto* target = GetTarget();
	assert(target);

	if (target->IsDead()) {
		return 0;
	}
	int hp = GetAffectedHp();
	if (hp != 0) {
		hp = target->ChangeHp(hp, true);
		if (IsAbsorb()) {
			// Only absorb the hp that were left
			source->ChangeHp(-hp, false);
		}
	}
	return hp;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplySpEffect() {
	auto* target = GetTarget();
	assert(target);
	auto sp = GetAffectedSp();
	if (sp != 0) {
		sp = target->ChangeSp(sp);
		if (IsAbsorb()) {
			// Only absorb the sp that were left
			source->ChangeSp(-sp);
		}
	}
	return sp;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAtkEffect() {
	auto* target = GetTarget();
	assert(target);
	auto atk = GetAffectedAttack();
	if (atk) {
		atk = target->ChangeAtkModifier(atk);
	}
	return atk;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyDefEffect() {
	auto* target = GetTarget();
	assert(target);
	auto def = GetAffectedDefense();
	if (def) {
		def = target->ChangeDefModifier(def);
	}
	return def;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplySpiEffect() {
	auto* target = GetTarget();
	assert(target);
	auto spi = GetAffectedSpirit();
	if (spi) {
		spi = target->ChangeSpiModifier(spi);
	}
	return spi;
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAgiEffect() {
	auto* target = GetTarget();
	assert(target);
	auto agi = GetAffectedAgility();
	if (agi) {
		agi = target->ChangeAgiModifier(agi);
	}
	return agi;
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
	if (was_dead && !target->IsDead()) {
		auto hp = GetAffectedHp();
		target->ChangeHp(hp - 1, false);
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyStateEffects() {
	// Apply states
	for (auto& se: states) {
		ApplyStateEffect(se);
	}
}

int Game_BattleAlgorithm::AlgorithmBase::ApplyAttributeShiftEffect(AttributeEffect ae) {
	auto* target = GetTarget();
	if (target) {
		return target->ShiftAttributeRate(ae.attr_id, ae.shift);
	}
	return 0;
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyAttributeShiftEffects() {
	for (auto& ae: attributes) {
		ApplyAttributeShiftEffect(ae);
	}
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyAll() {
	ApplyCustomEffect();
	ApplySwitchEffect();
	ApplyHpEffect();
	ApplySpEffect();
	ApplyAtkEffect();
	ApplyDefEffect();
	ApplySpiEffect();
	ApplyAgiEffect();
	ApplyStateEffects();
	ApplyAttributeShiftEffects();
}

void Game_BattleAlgorithm::AlgorithmBase::ProcessPostActionSwitches() {
	for (int s : switch_on) {
		Main_Data::game_switches->Set(s, true);
	}

	for (int s : switch_off) {
		Main_Data::game_switches->Set(s, false);
	}
}

bool Game_BattleAlgorithm::AlgorithmBase::IsTargetValid(const Game_Battler& target) const {
	return target.Exists();
}

int Game_BattleAlgorithm::AlgorithmBase::GetSourcePose() const {
	return lcf::rpg::BattlerAnimation::Pose_Idle;
}

void Game_BattleAlgorithm::AlgorithmBase::Start() {
	reflect_target = nullptr;

	if (party_target) {
		targets.clear();
		party_target->GetBattlers(targets);
		num_original_targets = targets.size();
	} else {
		// Remove any previously set reflect targets
		targets.resize(num_original_targets);
	}
	current_target = targets.begin();

	// Call any custom start logic, then check if we need to retarget
	bool allow_retarget = vStart();

	if (!IsCurrentTargetValid()) {
		if (!TargetNext() && allow_retarget && !party_target && !targets.empty()) {
			auto* last_target = targets.back();
			auto* next_target = last_target->GetParty().GetNextActiveBattler(last_target);
			if (next_target) {
				current_target = targets.insert(targets.end(), next_target);
			}

			if (!IsCurrentTargetValid()) {
				TargetNext();
			}
		}
	}

	// This case must be true before returning.
	assert(current_target == targets.end() || IsCurrentTargetValid());
}

bool Game_BattleAlgorithm::AlgorithmBase::vStart() {
	return true;
}

bool Game_BattleAlgorithm::AlgorithmBase::ReflectTargets() {
	auto iter = std::find_if(current_target, targets.end(), [this](auto* target) { return IsReflected(*target); });

	// No reflect
	if (iter == targets.end()) {
		return false;
	}

	reflect_target = *iter;

	if (party_target) {
		// Reflect back on source party
		const auto offset = targets.size();
		source->GetParty().GetBattlers(targets);
		current_target = targets.begin() + offset;
	} else {
		// Reflect back on source
		current_target = targets.insert(targets.end(), source);
	}

	if (!IsCurrentTargetValid()) {
		TargetNext();
	}
	return true;
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	return TargetNextInternal();
}

bool Game_BattleAlgorithm::AlgorithmBase::RepeatNext(bool require_valid) {
	++cur_repeat;
	if (cur_repeat >= repeat || (require_valid && !IsCurrentTargetValid())) {
		cur_repeat = 0;
		return false;
	}
	return true;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsCurrentTargetValid() const {
	if (current_target == targets.end()) {
		return false;
	}
	return IsTargetValid(**current_target);
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNextInternal() {
	do {
		if (current_target == targets.end() || ++current_target == targets.end()) {
			return false;
		}
	} while (!IsCurrentTargetValid());

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

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	return NULL;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetStartMessage(int) const {
	return "";
}

const lcf::rpg::Sound* Game_BattleAlgorithm::AlgorithmBase::GetFailureSe() const {
	return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Evasion);
}

bool Game_BattleAlgorithm::AlgorithmBase::IsReflected(const Game_Battler&) const {
	return false;
}

void Game_BattleAlgorithm::AlgorithmBase::ApplyComboHitsMultiplier(int hits) {
	repeat *= hits;
}

Game_BattleAlgorithm::None::None(Game_Battler* source) :
AlgorithmBase(Type::None, source, source) {
	// no-op
}

bool Game_BattleAlgorithm::None::Execute() {
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Battler* target, int hits_multiplier, Style style) :
	AlgorithmBase(Type::Normal, source, target), hits_multiplier(hits_multiplier)
{
	Init(style);
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Party_Base* target, int hits_multiplier, Style style) :
	AlgorithmBase(Type::Normal, source, target), hits_multiplier(hits_multiplier)
{
	Init(style);
}

Game_BattleAlgorithm::Normal::Style Game_BattleAlgorithm::Normal::GetDefaultStyle() {
	return Player::IsRPG2k3() ? Style_MultiHit : Style_Combined;
}

Game_Battler::Weapon Game_BattleAlgorithm::Normal::GetWeapon() const {
	if (weapon_style < 0) {
		return Game_Battler::WeaponAll;
	}
	return cur_repeat >= weapon_style ? Game_Battler::WeaponSecondary : Game_Battler::WeaponPrimary;
}

void Game_BattleAlgorithm::Normal::Init(Style style) {
	weapon_style = -1;
	if (GetSource()->GetType() == Game_Battler::Type_Ally && style == Style_MultiHit) {
		auto* ally = static_cast<Game_Actor*>(GetSource());
		if (ally->GetWeapon() && ally->Get2ndWeapon()) {
			auto hits = hits_multiplier * ally->GetNumberOfAttacks(Game_Battler::WeaponPrimary);
			weapon_style = hits;
			hits += hits_multiplier * ally->GetNumberOfAttacks(Game_Battler::WeaponSecondary);
			SetRepeat(hits);
			return;
		}
	}
	SetRepeat(hits_multiplier * source->GetNumberOfAttacks(GetWeapon()));
}

void Game_BattleAlgorithm::Normal::ApplyComboHitsMultiplier(int hits) {
	AlgorithmBase::ApplyComboHitsMultiplier(hits);
	if (weapon_style > 0) {
		// For dual wield normal attack, the first weapon gets combo'd then the second weapon.
		weapon_style *= hits;
	}
}


bool Game_BattleAlgorithm::Normal::vStart() {
	// If this weapon attacks all, then attack all enemies regardless of original targetting.
	const auto weapon = GetWeapon();
	if (!party_target && source->HasAttackAll(weapon)) {
		auto* target = targets.back();
		auto idx = targets.size();
		target->GetParty().GetBattlers(targets);
		current_target = targets.begin() + idx;
	}

	source->ChangeSp(-source->CalculateWeaponSpCost(weapon));

	return true;
}

int Game_BattleAlgorithm::Normal::GetAnimationId(int idx) const {
	const auto weapon = GetWeapon();
	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		auto weapons = ally->GetWeapons(weapon);
		auto* item = (idx >= 0 && idx < static_cast<int>(weapons.size()))
			? weapons[idx] : nullptr;
		if (item) {
			return item->animation_id;
		} else if (idx == 0) {
			return ally->GetUnarmedBattleAnimationId();
		}
		return 0;
	}
	if (source->GetType() == Game_Battler::Type_Enemy
			&& Player::IsRPG2k3()
			&& !lcf::Data::animations.empty()) {
		return 1;
	}
	return 0;
}

bool Game_BattleAlgorithm::Normal::Execute() {
	Reset();

	const auto weapon = GetWeapon();
	auto& source = *GetSource();
	auto& target = *GetTarget();

	auto to_hit = Algo::CalcNormalAttackToHit(source, target, weapon, Game_Battle::GetBattleCondition(), true);
	auto crit_chance = Algo::CalcCriticalHitChance(source, target, weapon);

	// Damage calculation
	if (!Rand::PercentChance(to_hit)) {
		this->success = false;
		return this->success;
	}

	if (Rand::PercentChance(crit_chance)) {
		critical_hit = true;
	}

	// FIXME: Differentiate the cases where 2k3 back attack bug applies
	auto effect = Algo::CalcNormalAttackEffect(source, target, weapon, critical_hit, true, Game_Battle::GetBattleCondition(), true);
	effect = Algo::AdjustDamageForDefend(effect, target);

	effect = Utils::Clamp(effect, -MaxDamageValue(), MaxDamageValue());

	this->hp = -effect;
	this->affect_hp = true;

	// RPG_RT BUG: 2k3 has a bug where a negative attribute reversal of damage causes hp effect to double
	// RPG_RT applies the affect but it doesn't appear in the floating numbers.
	if (this->hp > 0) {
		this->hp = std::min(this->hp * 2, MaxDamageValue());
	}

	// If target is killed, states not applied
	if (target.GetHp() + this->hp <= 0) {
		this->success = true;
		return this->success;
	}

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target.GetStates();
	auto target_perm_states = target.GetPermanentStates();

	// Conditions healed by physical attack:
	BattlePhysicalStateHeal(100, target_states, target_perm_states, states);

	// Conditions caused / healed by weapon.
	if (source.GetType() == Game_Battler::Type_Ally) {
		auto& ally = static_cast<Game_Actor&>(source);
		const bool is2k3 = Player::IsRPG2k3();
		auto weapons = ally.GetWeapons(weapon);

		if (weapons[0]) {
			int num_states = 0;
			for (auto* w: weapons) {
				if (w) {
					num_states = std::max(num_states, static_cast<int>(w->state_set.size()));
				}
			}

			for (int i = 0; i < num_states; ++i) {
				// EasyRPG extension: This logic allows heal/inflict to work properly with a combined weapon attack.
				// If the first weapon heals and the second inflicts, then this will do then in the right order.
				// If both heal or both inflict, we take the max probability as RPG_RT does.
				int heal_pct = 0;
				int inflict_pct = 0;
				for (auto* w: weapons) {
					if (w && i < static_cast<int>(w->state_set.size()) && w->state_set[i]) {
						if (is2k3 && w->reverse_state_effect) {
							heal_pct = std::max(heal_pct, static_cast<int>(w->state_chance));
						} else {
							inflict_pct = std::max(inflict_pct, static_cast<int>(w->state_chance));
						}
					}
				}
				auto state_id = (i + 1);

				for (auto* w: weapons) {
					if (is2k3 && w->reverse_state_effect) {
						if (heal_pct > 0 && Rand::PercentChance(heal_pct)) {
							if (State::Remove(state_id, target_states, target_perm_states)) {
								states.push_back(StateEffect(state_id, StateEffect::Healed));
							}
						}
						heal_pct = 0;
					} else {
						if (inflict_pct > 0) {
							inflict_pct = inflict_pct * target.GetStateProbability(state_id) / 100;
							if (Rand::PercentChance(inflict_pct)) {
								// Unlike skills, weapons do not try to reinflict states already present
								if (!State::Has(state_id, target_states) && State::Add(state_id, target_states, target_perm_states, true)) {
									states.push_back(StateEffect(state_id, StateEffect::Inflicted));
								}
							}
							inflict_pct = 0;
						}
					}
				}
			}
		}
	}

	this->success = true;
	return this->success;
}

std::string Game_BattleAlgorithm::Normal::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetNormalAttackStartMessage2k(*GetSource());
		}
		if (GetSource()->GetType() == Game_Battler::Type_Enemy && hits_multiplier == 2) {
			return BattleMessage::GetDoubleAttackStartMessage2k3();
		}
	}
	return "";
}

int Game_BattleAlgorithm::Normal::GetSourcePose() const {
	auto weapon = GetWeapon();
	return weapon == Game_Battler::WeaponSecondary
		? lcf::rpg::BattlerAnimation::Pose_AttackLeft
		: lcf::rpg::BattlerAnimation::Pose_AttackRight;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Normal::GetStartSe() const {
	if (Player::IsRPG2k() && source->GetType() == Game_Battler::Type_Enemy) {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyAttacks);
	}
	return nullptr;
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
	Skill(source, source, skill, item)
{
}

void Game_BattleAlgorithm::Skill::Init() {
}

bool Game_BattleAlgorithm::Skill::vStart() {
	if (item) {
		Main_Data::game_party->ConsumeItemUse(item->ID);
	} else {
		source->ChangeSp(-source->CalculateSkillCost(skill.ID));
	}
	return true;
}

int Game_BattleAlgorithm::Skill::GetAnimationId(int idx) const {
	return idx == 0 && Algo::IsNormalOrSubskill(skill) ? skill.animation_id : 0;
}

bool Game_BattleAlgorithm::Skill::IsTargetValid(const Game_Battler& target) const {
	if (target.IsHidden()) {
		return false;
	}

	if (Algo::SkillTargetsAllies(skill) && target.IsDead()) {
		// Cures death
		// NOTE: RPG_RT 2k3 also allows this targetting if reverse_state_effect.
		return !skill.state_effects.empty() && skill.state_effects[0];
	}

	return true;
}


bool Game_BattleAlgorithm::Skill::Execute() {
	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	absorb = false;
	this->success = false;

	auto* source = GetSource();
	assert(source);
	auto* target = GetTarget();
	assert(target);

	this->positive = Algo::SkillTargetsAllies(skill);

	if (skill.type == lcf::rpg::Skill::Type_switch) {
		switch_id = skill.switch_id;
		this->success = true;
		return this->success;
	}

	if (!Algo::IsNormalOrSubskill(skill)) {
		this->success = false;
		return this->success;
	}

	auto to_hit = Algo::CalcSkillToHit(*source, *target, skill);
	auto effect = Algo::CalcSkillEffect(*source, *target, skill, true);
	effect = Utils::Clamp(effect, -MaxDamageValue(), MaxDamageValue());

	if (!IsPositive()) {
		effect = -effect;
	}

	const bool is_dead = target->IsDead();
	const bool cures_death = IsPositive()
		&& !skill.state_effects.empty()
		&& skill.state_effects[lcf::rpg::State::kDeathID - 1]
		&& is_dead;

	// Dead targets only allowed if this skill revives later
	if (is_dead && (!IsPositive() || !cures_death)) {
		this->success = false;
		return false;
	}

	// Absorb only works on offensive skills.
	this->absorb = skill.absorb_damage && !IsPositive();

	// Make a copy of the target's state set and see what we can apply.
	auto target_states = target->GetStates();
	auto target_perm_states = target->GetPermanentStates();

	if (skill.affect_hp && Rand::PercentChance(to_hit)) {
		const auto cur_hp = target->GetHp();
		this->affect_hp = true;

		if (IsAbsorb()) {
			// Cannot aborb more hp than the target has.
			this->hp = -std::max<int>(-this->hp, cur_hp);

			// Absorb requires damage to be successful
			this->success = (hp != 0);
		} else {
			if (IsPositive()) {
				// RPG_RT attribute inverted healing effects are non-lethal
				this->hp = std::max(-(cur_hp - 1), effect);
				// HP recovery is sucessful if the effect is non-zero, even at full hp.
				this->success |= (effect != 0);
			} else {
				this->hp = effect;

				// RPG_RT BUG: 2k3 has a bug where a negative attribute reversal of damage causes hp effect to double
				// RPG_RT applies the affect but it doesn't appear in the floating numbers.
				if (this->hp > 0) {
					this->hp = std::min(this->hp * 2, MaxDamageValue());
				}

				// Conditions healed by physical attack:
				BattlePhysicalStateHeal(skill.physical_rate * 10, target_states, target_perm_states, states);

				// Hp damage always successful, even if 0
				this->success = true;
			}
		}
	}

	// If target will be killed, no further affects are applied.
	if (!is_dead && GetTarget()->GetHp() + this->hp <= 0) {
		return this->success;
	}

	if (skill.affect_sp && Rand::PercentChance(to_hit)) {
		const auto sp_cost = (source == target) ? source->CalculateSkillCost(skill.ID) : 0;
		const auto max_sp = target->GetMaxSp();
		const auto cur_sp = target->GetSp() - sp_cost;

		this->affect_sp = true;
		if (IsAbsorb()) {
			// FIXME: Verify behavior on negative attributes
			this->sp = -std::min(cur_sp, -effect);
		} else {
			this->sp = Utils::Clamp(cur_sp + effect, 0, max_sp) - cur_sp;
		}

		this->success |= this->sp;
	}

	if (!IsPositive() && !this->success && (skill.affect_hp || skill.affect_sp)) {
		return this->success;
	}

	if (skill.affect_attack && Rand::PercentChance(to_hit)) {
		this->affect_atk = true;
		this->attack = target->CanChangeAtkModifier(effect);
		this->success |= this->attack;
	}
	if (skill.affect_defense && Rand::PercentChance(to_hit)) {
		this->affect_def = true;
		this->defense = target->CanChangeDefModifier(effect);
		this->success |= this->defense;
	}
	if (skill.affect_spirit && Rand::PercentChance(to_hit)) {
		this->affect_spi = true;
		this->spirit = target->CanChangeSpiModifier(effect);
		this->success |= this->spirit;
	}
	if (skill.affect_agility && Rand::PercentChance(to_hit)) {
		this->affect_agi = true;
		this->agility = target->CanChangeAgiModifier(effect);
		this->success |= this->agility;
	}

	bool heals_states = IsPositive() ^ (Player::IsRPG2k3() && skill.reverse_state_effect);
	bool affected_death = false;
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

		if (!Rand::PercentChance(to_hit)) {
			continue;
		}

		if (heals_states) {
			if (target_has_state) {
				// RPG_RT 2k3 skills which fail due to permanent states don't "miss"
				this->success = true;
				if (State::Remove(state_id, target_states, target_perm_states)) {
					states.push_back({state_id, StateEffect::Healed});
					affected_death |= (state_id == lcf::rpg::State::kDeathID);
				}
			}
		} else if (Rand::PercentChance(GetTarget()->GetStateProbability(state_id))) {
			if (State::Add(state_id, target_states, target_perm_states, true)) {
				this->success = true;
				states.push_back({state_id, StateEffect::Inflicted});
				affected_death |= (state_id == lcf::rpg::State::kDeathID);
			}
		}
	}

	if (heals_states && affected_death) {
		this->affect_hp = true;
		// If resurrected and no HP selected, the effect value is a percentage:
		if (skill.affect_hp) {
			this->hp = std::max(0, effect);
		} else {
			this->hp = target->GetMaxHp() * effect / 100;
		}
		this->revived = true;
	}

	// When a skill inflicts death state, other states can also be inflicted, but attributes will be skipped
	if (!heals_states && affected_death) {
		return this->success;
	}

	// Attribute resistance / weakness + an attribute selected + can be modified
	if (skill.affect_attr_defence) {
		auto shift = IsPositive() ? 1 : -1;
		for (int i = 0; i < static_cast<int>(skill.attribute_effects.size()); i++) {
			auto id = i + 1;
			if (skill.attribute_effects[i]
					&& GetTarget()->CanShiftAttributeRate(id, shift)
					&& Rand::PercentChance(to_hit)
					)
			{
				AttributeEffect ae;
				ae.attr_id = id;
				ae.shift = shift;
				attributes.push_back(ae);
				this->success = true;
			}
		}
	}

	return this->success;
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage(int line) const {
	if (item && item->using_message == 0) {
		if (line == 0) {
			if (Player::IsRPG2k()) {
				return BattleMessage::GetItemStartMessage2k(*GetSource(), *item);
			} else {
				return BattleMessage::GetItemStartMessage2k3(*item);
			}
		}
		return "";
	}

	const auto* target = (targets.size() == 1) ? targets.front() : nullptr;

	if (line == 0) {
		if (Player::IsRPG2k()) {
			if (!skill.using_message1.empty()) {
				return BattleMessage::GetSkillFirstStartMessage2k(*GetSource(), target, skill);
			} else {
				return BattleMessage::GetSkillSecondStartMessage2k(*GetSource(), target, skill);
			}
		} else {
			return BattleMessage::GetSkillStartMessage2k3(skill);
		}
	}
	if (line == 1 && Player::IsRPG2k() && !skill.using_message1.empty()) {
		return BattleMessage::GetSkillSecondStartMessage2k(*GetSource(), target, skill);
	}
	return "";
}

int Game_BattleAlgorithm::Skill::GetSourcePose() const {
	if (source->GetType() == Game_Battler::Type_Ally && skill.animation_id > 0) {
		if (static_cast<int>(skill.battler_animation_data.size()) > source->GetId() - 1) {
			return skill.battler_animation_data[source->GetId() - 1].pose;
		}
	}

	return lcf::rpg::BattlerAnimation::Pose_Skill;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Skill::GetStartSe() const {
	if (skill.type == lcf::rpg::Skill::Type_switch) {
		return &skill.sound_effect;
	}
	return nullptr;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Skill::GetFailureSe() const {
	return skill.failure_message != 3
		? nullptr
		: &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Evasion);
}

std::string Game_BattleAlgorithm::Skill::GetFailureMessage() const {
	return BattleMessage::GetSkillFailureMessage(*GetSource(), *GetTarget(), skill);
}

bool Game_BattleAlgorithm::Skill::IsReflected(const Game_Battler& target) const {
	// Skills invoked by items ignore reflect
	if (item) {
		return false;
	}
	return IsTargetValid(target) && target.HasReflectState() && target.GetType() != GetSource()->GetType();
}

bool Game_BattleAlgorithm::Skill::ActionIsPossible() const {
	if (item) {
		return Main_Data::game_party->GetItemTotalCount(item->ID) > 0;
	}
	// RPG_RT performs this check only for enemies and if skill is single target
	if (source->GetType() == Game_Battler::Type_Enemy && targets.size() == 1 && targets.front()->GetType() == source->GetType()) {
		if (!EnemyAi::IsSkillEffectiveOn(skill, *targets.front(), true)) {
			return false;
		}
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

bool Game_BattleAlgorithm::Item::vStart() {
	Main_Data::game_party->ConsumeItemUse(item.ID);
	return true;
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, const lcf::rpg::Item& item) :
	Item(source, source, item) {}

bool Game_BattleAlgorithm::Item::IsTargetValid(const Game_Battler&) const {
	return item.type == lcf::rpg::Item::Type_medicine;
}

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	auto* target = GetTarget();

	this->success = false;

	if (item.type == lcf::rpg::Item::Type_switch) {
		switch_id = item.switch_id;
		this->success = true;
		return true;
	}

	if (item.type == lcf::rpg::Item::Type_medicine) {
		this->success = true;
		this->positive = true;

		this->revived = !item.state_set.empty()
			&& item.state_set[lcf::rpg::State::kDeathID - 1]
			&& GetTarget()->IsDead();

		// RM2k3 BUG: In rm2k3 battle system, this IsItemUsable() check is only applied when equipment_setting == actor, not for class.
		if (GetTarget()->GetType() == Game_Battler::Type_Ally && !static_cast<Game_Actor*>(GetTarget())->IsItemUsable(item.ID)) {
			// No effect, but doesn't behave like a dodge or damage to set healing and success to true.
			return this->success;
		}

		if (item.ko_only && !GetTarget()->IsDead()) {
			return this->success;
		}

		// Make a copy of the target's state set and see what we can apply.
		auto target_states = target->GetStates();
		auto target_perm_states = target->GetPermanentStates();

		bool is_dead_cured = false;
		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				is_dead_cured |= (i == 0);
				if (State::Remove(i + 1, target_states, target_perm_states)) {
					states.push_back({i+1, StateEffect::Healed});
				}
			}
		}

		// HP recovery
		if ((item.recover_hp != 0 || item.recover_hp_rate != 0) && (!target->IsDead() || is_dead_cured)) {
			this->affect_hp = true;
			this->hp = item.recover_hp_rate * GetTarget()->GetMaxHp() / 100 + item.recover_hp;
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->affect_sp = true;
			this->sp = item.recover_sp_rate * GetTarget()->GetMaxSp() / 100 + item.recover_sp;
		}

		return this->success;
	}

	assert("Unsupported battle item type");
	this->success = false;
	return false;
}

std::string Game_BattleAlgorithm::Item::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetItemStartMessage2k(*GetSource(), item);
		} else {
			return BattleMessage::GetItemStartMessage2k3(item);
		}
	}
	return "";
}

int Game_BattleAlgorithm::Item::GetSourcePose() const {
	return lcf::rpg::BattlerAnimation::Pose_Item;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == lcf::rpg::Item::Type_medicine || item.type == lcf::rpg::Item::Type_switch) {
		return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_UseItem);
	}
	return nullptr;
}

bool Game_BattleAlgorithm::Item::ActionIsPossible() const {
	return Main_Data::game_party->GetItemTotalCount(item.ID) > 0;
}

Game_BattleAlgorithm::Defend::Defend(Game_Battler* source) :
	AlgorithmBase(Type::Defend, source, source) {
		source->SetIsDefending(true);
}

std::string Game_BattleAlgorithm::Defend::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetDefendStartMessage2k(*GetSource());
		} else if (GetSource()->GetType() == Game_Battler::Type_Enemy) {
			return BattleMessage::GetDefendStartMessage2k3();
		}
	}
	return "";
}

int Game_BattleAlgorithm::Defend::GetSourcePose() const {
	return lcf::rpg::BattlerAnimation::Pose_Defend;
}

bool Game_BattleAlgorithm::Defend::Execute() {
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(Type::Observe, source, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Observe::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetObserveStartMessage2k(*GetSource());
		} else if (GetSource()->GetType() == Game_Battler::Type_Enemy) {
			return BattleMessage::GetObserveStartMessage2k3();
		}
	}
	return "";
}

bool Game_BattleAlgorithm::Observe::Execute() {
	// Observe only prints the start message
	this->success = true;
	return true;
}

Game_BattleAlgorithm::Charge::Charge(Game_Battler* source) :
AlgorithmBase(Type::Charge, source, source) {
	// no-op
}

bool Game_BattleAlgorithm::Charge::vStart() {
	source->SetCharged(true);
	return true;
}

std::string Game_BattleAlgorithm::Charge::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetChargeUpStartMessage2k(*GetSource());
		} else if (GetSource()->GetType() == Game_Battler::Type_Enemy) {
			return BattleMessage::GetChargeUpStartMessage2k3();
		}
	}
	return "";
}

bool Game_BattleAlgorithm::Charge::Execute() {
	this->success = true;
	return true;
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source, Game_Party_Base* target) :
AlgorithmBase(Type::SelfDestruct, source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::SelfDestruct::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetSelfDestructStartMessage2k(*GetSource());
		} else if (GetSource()->GetType() == Game_Battler::Type_Enemy) {
			return BattleMessage::GetSelfDestructStartMessage2k3();
		}
	}
	return "";
}

const lcf::rpg::Sound* Game_BattleAlgorithm::SelfDestruct::GetStartSe() const {
	return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill);
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Reset();

	auto& source = *GetSource();
	auto& target = *GetTarget();

	auto effect = Algo::CalcSelfDestructEffect(source, target, true);
	effect = Algo::AdjustDamageForDefend(effect, target);
	effect = Utils::Clamp(effect, -MaxDamageValue(), MaxDamageValue());

	this->hp = -effect;
	this->affect_hp = true;

	// Recover physical states only if not killed.
	if (target.GetHp() + this->hp > 0) {
		// Make a copy of the target's state set and see what we can apply.
		auto target_states = target.GetStates();
		auto target_perm_states = target.GetPermanentStates();

		// Conditions healed by physical attack:
		BattlePhysicalStateHeal(100, target_states, target_perm_states, states);
	}

	success = true;

	return true;
}

void Game_BattleAlgorithm::SelfDestruct::ApplyCustomEffect() {
	// Only monster can self destruct
	if (animate && source->GetType() == Game_Battler::Type_Enemy) {
		auto* enemy = static_cast<Game_Enemy*>(source);
		enemy->SetHidden(true);
		enemy->SetExplodeTimer();
	}
	animate = false;
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source) :
	AlgorithmBase(Type::Escape, source, source) {
	// no-op
}

std::string Game_BattleAlgorithm::Escape::GetStartMessage(int line) const {
	if (line == 0) {
		if (Player::IsRPG2k()) {
			return BattleMessage::GetEscapeStartMessage2k(*GetSource());
		} else if (GetSource()->GetType() == Game_Battler::Type_Enemy) {
			return BattleMessage::GetEscapeStartMessage2k3();
		}
	}
	return "";
}

int Game_BattleAlgorithm::Escape::GetSourcePose() const {
	return lcf::rpg::BattlerAnimation::Pose_WalkRight;
}

const lcf::rpg::Sound* Game_BattleAlgorithm::Escape::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetStartSe();
	}
	return &Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Escape);
}

bool Game_BattleAlgorithm::Escape::Execute() {
	Reset();

	this->success = true;

	return this->success;
}

void Game_BattleAlgorithm::Escape::ApplyCustomEffect() {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		auto* enemy = static_cast<Game_Enemy*>(source);
		enemy->SetHidden(true);
		enemy->SetDeathTimer();
	}
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(Type::Transform, source, source), new_monster_id(new_monster_id) {
	// no-op
}

std::string Game_BattleAlgorithm::Transform::GetStartMessage(int line) const {
	if (line == 0 && Player::IsRPG2k()) {
		auto* enemy = lcf::ReaderUtil::GetElement(lcf::Data::enemies, new_monster_id);
		return BattleMessage::GetTransformStartMessage(*GetSource(), *enemy);
	}
	return "";
}

bool Game_BattleAlgorithm::Transform::Execute() {
	this->success = true;
	return true;
}

void Game_BattleAlgorithm::Transform::ApplyCustomEffect() {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		auto* enemy = static_cast<Game_Enemy*>(source);
		enemy->Transform(new_monster_id);
		enemy->Flash(31,31,31,31,20);
	}
}

Game_BattleAlgorithm::DoNothing::DoNothing(Game_Battler* source) :
AlgorithmBase(Type::DoNothing, source, source) {
	// no-op
}

bool Game_BattleAlgorithm::DoNothing::Execute() {
	this->success = true;
	return true;
}

