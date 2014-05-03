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

#include <sstream>
#include "game_actor.h"
#include "game_battlealgorithm.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_temp.h"
#include "main_data.h"
#include "rpg_animation.h"
#include "rpg_state.h"
#include "rpg_skill.h"
#include "rpg_item.h"

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source) :
source(source) {
	Reset();

	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Battler* target) :
	source(source) {
	Reset();

	SetTarget(target);
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Party_Base* target) :
	source(source) {
	Reset();

	target->GetAliveBattlers(targets);
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
	animation = NULL;
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

const std::vector<RPG::State>& Game_BattleAlgorithm::AlgorithmBase::GetAffectedConditions() const {
	return conditions;
}

const RPG::Animation* Game_BattleAlgorithm::AlgorithmBase::GetAnimation() const {
	return animation;
}

bool Game_BattleAlgorithm::AlgorithmBase::GetSuccess() const {
	return success;
}

bool Game_BattleAlgorithm::AlgorithmBase::GetKilledByAttack() const {
	return killed_by_attack_damage;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetDeathMessage() const {
	if (!killed_by_attack_damage) {
		return "";
	}

	if (current_target == targets.end()) {
		return "";
	}

	if ((*current_target)->GetType() == Game_Battler::Type_Ally) {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_actor;
	} else {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_enemy;
	}
}

void Game_BattleAlgorithm::AlgorithmBase::GetResultMessages(std::vector<std::string>& out) const {
	if (current_target == targets.end()) {
		return;
	}

	if (!success) {
		out.push_back((*current_target)->GetName() + Data::terms.dodge);
	}

	bool target_is_ally = (*current_target)->GetType() == Game_Battler::Type_Ally;

	if (GetAffectedHp() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if (IsPositive()) {
			if (!(*current_target)->IsDead()) {
				ss << " ";
				ss << Data::terms.health_points << " " << GetAffectedHp();
				ss << Data::terms.hp_recovery;
				out.push_back(ss.str());
			}
		}
		else {
			if (GetAffectedHp() == 0) {
				ss << (target_is_ally ?
					Data::terms.actor_undamaged :
					Data::terms.enemy_undamaged);
			}
			else {
				ss << " " << GetAffectedHp() << (target_is_ally ?
					Data::terms.actor_damaged :
					Data::terms.enemy_damaged);
			}
			out.push_back(ss.str());
		}
	}

	if (GetAffectedSp() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if (IsPositive()) {
			ss << " ";
			ss << Data::terms.spirit_points << " " << GetAffectedSp();
			ss << Data::terms.hp_recovery;
		}
		else {
			ss << " " << Data::terms.attack << " " << GetAffectedSp();
		}
		out.push_back(ss.str());
	}

	if (GetAffectedAttack() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.attack << " " << GetAffectedSp();
		out.push_back(ss.str());
	}

	if (GetAffectedDefense() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.defense << " " << GetAffectedDefense();
		out.push_back(ss.str());
	}

	if (GetAffectedSpirit() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.spirit << " " << GetAffectedSpirit();
		out.push_back(ss.str());
	}

	if (GetAffectedAgility() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.agility << " " << GetAffectedAgility();
		out.push_back(ss.str());
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	// TODO: Handle healing

	for (; it != conditions.end(); ++it) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if ((*current_target)->HasState(it->ID)) {
			if (IsPositive()) {
				ss << it->message_recovery;
				out.push_back(ss.str());
			}
			if (!it->message_already.empty()) {
				ss << it->message_already;
				out.push_back(ss.str());
			}
		} else {
			// Positive case doesn't report anything in case of uselessness
			if (IsPositive()) {
				continue;
			}

			if ((*current_target)->GetType() == Game_Battler::Type_Ally) {
				ss << it->message_actor;
			} else {
				ss << it->message_enemy;
			}
			out.push_back(ss.str());

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
	if (current_target == targets.end()) {
		return NULL;
	}

	return *current_target;
}

void Game_BattleAlgorithm::AlgorithmBase::SetTarget(Game_Battler* target) {
	targets.clear();
	targets.push_back(target);
	current_target = targets.begin();
}

void Game_BattleAlgorithm::AlgorithmBase::Apply() {
	if (GetAffectedHp() != -1) {
		int hp = GetAffectedHp();
		(*current_target)->ChangeHp(IsPositive() ? hp : -hp);
	}

	if (GetAffectedSp() != -1) {
		int sp = GetAffectedSp();
		(*current_target)->SetSp((*current_target)->GetSp() + (IsPositive() ? sp : -sp));
	}

	// TODO
	if (GetAffectedAttack() != -1) {
	}

	if (GetAffectedDefense() != -1) {
	}

	if (GetAffectedSpirit() != -1) {
	}

	if (GetAffectedAgility() != -1) {
	}
	// End TODO
	if (GetAffectedSwitch() != -1) {
		Game_Switches[GetAffectedSwitch()] = true;
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	for (; it != conditions.end(); ++it) {
		if (IsPositive()) {
			(*current_target)->RemoveState(it->ID);
		}
		else {
			(*current_target)->AddState(it->ID);
		}
	}
}

bool Game_BattleAlgorithm::AlgorithmBase::IsTargetValid() {
	if (current_target == targets.end()) {
		return true;
	}

	return (!(*current_target)->IsDead());
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	if (current_target == targets.end()) {
		return false;
	}

	if (current_target + 1 != targets.end()) {
		++current_target;
		return true;
	}
	return false;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		return &Data::system.enemy_attack_se;
	} else {
		return NULL;
	}
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetResultSe() const {
	if (healing) {
		return NULL;
	}

	if (!success) {
		return &Data::system.dodge_se;
	} else {
		return ((*current_target)->GetType() == Game_Battler::Type_Ally ?
			&Data::system.actor_damaged_se :
		&Data::system.enemy_damaged_se);
	}
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetDeathSe() const {
	return ((*current_target)->GetType() == Game_Battler::Type_Ally ?
		NULL : &Data::system.enemy_death_se);
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

	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		int hit_chance = 80; // FIXME
		if (ally->GetWeaponId() == 0) {
			// No Weapon
			// Todo: Two Sword style
			animation = &Data::animations[Data::actors[ally->GetId() - 1].unarmed_animation - 1];
		} else {
			animation = &Data::animations[Data::items[ally->GetWeaponId() - 1].animation_id - 1];
			hit_chance = Data::items[ally->GetWeaponId() - 1].hit;
		}
		to_hit = (int)(100 - (100 - hit_chance) * (1 + (1.0 * (*current_target)->GetAgi() / ally->GetAgi() - 1) / 2));
	} else {
		// Source is Enemy

		//int hit = src->IsMissingOften() ? 70 : 90;
		int hit = 70;
		to_hit = (int)(100 - (100 - hit) * (1 + (1.0 * (*current_target)->GetAgi() / source->GetAgi() - 1) / 2));
	}

	// Damage calculation
	if (rand() % 100 < to_hit) {
		int effect = source->GetAtk() / 2 - (*current_target)->GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;
		this->hp = effect;

		if ((*current_target)->GetHp() - this->hp <= 0) {
			// Death state
			killed_by_attack_damage = true;
			conditions.push_back(Data::states[0]);
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
}

std::string Game_BattleAlgorithm::Normal::GetStartMessage() const {
	return source->GetName() + Data::terms.attacking;
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

bool Game_BattleAlgorithm::Skill::IsTargetValid() {
	if (current_target == targets.end()) {
		return true;
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		if (skill.scope == RPG::Skill::Scope_ally ||
			skill.scope == RPG::Skill::Scope_party) {
			return true;
		}
	}

	return (!(*current_target)->IsDead());
}

bool Game_BattleAlgorithm::Skill::Execute() {
	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	animation = &Data::animations[skill.animation_id == 0 ? NULL : skill.animation_id - 1];

	this->success = false;

	if (source->GetType() == Game_Battler::Type_Ally) {
		this->healing =
			skill.scope == RPG::Skill::Scope_ally ||
			skill.scope == RPG::Skill::Scope_party ||
			skill.scope == RPG::Skill::Scope_self;
	}

	if (skill.type == RPG::Skill::Type_normal) {
		if (skill.power > 0) {
			if (healing || rand() % 100 < skill.hit) {
				this->success = true;

				// FIXME: is this still affected by stats for allies?
				// FIXME: This is what the help file says, but it doesn't look right
				int effect = skill.power +
					source->GetAtk() * skill.pdef_f / 20 +
					(*current_target)->GetDef() * skill.mdef_f / 40;

				if (skill.variance > 0) {
					int var_perc = skill.variance * 5;
					int act_perc = rand() % (var_perc * 2) - var_perc;
					int change = effect * act_perc / 100;
					effect += change;
				}

				if (skill.affect_hp) {
					this->hp = effect;

					if ((*current_target)->GetHp() - this->hp <= 0) {
						// Death state
						killed_by_attack_damage = true;
						conditions.push_back(Data::states[0]);
					}
				}
				if (skill.affect_sp)
					this->sp = effect;
				if (skill.affect_attack)
					this->attack = effect;
				if (skill.affect_defense)
					this->defense = effect;
				if (skill.affect_spirit)
					this->spirit = effect;
				if (skill.affect_agility)
					this->agility = agility;
			}
		}

		for (int i = 0; i < (int) skill.state_effects.size(); i++) {
			if (!skill.state_effects[i])
				continue;
			if (rand() % 100 >= skill.hit)
				continue;

			this->success = true;

			conditions.push_back(Data::states[i]);
		}

		return this->success;
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
	AlgorithmBase::Apply();

	if (item) {
		Main_Data::game_party->GainItem(item->ID, -1);
	}
	else {
		source->SetSp(source->GetSp() - source->CalculateSkillCost(skill.ID));
	}
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage() const {
	// TODO: How to handle using_message2?
	if (item && item->using_message == 0) {
		// Use item message
		return Item(source, *item).GetStartMessage();
	}
	return source->GetName() + skill.using_message1;
}

const RPG::Sound* Game_BattleAlgorithm::Skill::GetStartSe() const {
	if (skill.type == RPG::Skill::Type_switch) {
		return &skill.sound_effect;
	}
	else {
		return AlgorithmBase::GetStartSe();
	}
}


void Game_BattleAlgorithm::Skill::GetResultMessages(std::vector<std::string>& out) const {
	if (!success) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		switch (skill.failure_message) {
			case 1:
				ss << Data::terms.skill_failure_a;
				break;
			case 2:
				ss << Data::terms.skill_failure_b;
				break;
			case 3:
				ss << Data::terms.skill_failure_c;
				break;
			case 4:
				ss << Data::terms.dodge;
				break;
			default:
				ss << " BUG: INVALID SKILL FAIL MSG";
		}
		out.push_back(ss.str());
		return;
	}

	AlgorithmBase::GetResultMessages(out);
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

bool Game_BattleAlgorithm::Item::IsTargetValid() {
	if (current_target == targets.end()) {
		return true;
	}

	return item.type == RPG::Item::Type_medicine;
}

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	switch (item.type) {
		case RPG::Item::Type_medicine:
		case RPG::Item::Type_switch:
			break;
		default:
			assert("Unsupported battle item type");
	}

	this->success = false;

	if (item.type == RPG::Item::Type_medicine) {
		this->healing = true;

		// HP recovery
		if (item.recover_hp != 0 || item.recover_hp_rate != 0) {
			this->hp = item.recover_hp_rate * (*current_target)->GetMaxHp() / 100 + item.recover_hp;
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->sp = item.recover_sp_rate * (*current_target)->GetMaxSp() / 100 + item.recover_sp;
		}

		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				this->conditions.push_back(Data::states[i]);
			}
		}

		this->success = true;
	}
	else if (item.type == RPG::Item::Type_switch) {
		switch_id = item.switch_id;
		this->success = true;
	}

	return this->success;
}

void Game_BattleAlgorithm::Item::Apply() {
	AlgorithmBase::Apply();

	Main_Data::game_party->GainItem(item.ID, -1);
}

std::string Game_BattleAlgorithm::Item::GetStartMessage() const {
	return source->GetName() + " " + item.name + Data::terms.use_item;
}

void Game_BattleAlgorithm::Item::GetResultMessages(std::vector<std::string>& out) const {
	AlgorithmBase::GetResultMessages(out);
}

const RPG::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == RPG::Item::Type_switch) {
		return &Data::system.item_se;
	}
	else {
		return AlgorithmBase::GetStartSe();
	}
}

Game_BattleAlgorithm::NormalDual::NormalDual(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::NormalDual::GetStartMessage() const {
	return source->GetName() + " TODO DUAL";
}

bool Game_BattleAlgorithm::NormalDual::Execute() {
	Output::Warning("Battle: Enemy Double Attack not implemented");
	return true;
}

Game_BattleAlgorithm::Defend::Defend(Game_Battler* source) :
	AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Defend::GetStartMessage() const {
	return source->GetName() + Data::terms.defending;
}

bool Game_BattleAlgorithm::Defend::Execute() {
	Output::Warning("Battle: Defend not implemented");
	return true;
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Observe::GetStartMessage() const {
	return source->GetName() + Data::terms.observing;
}

bool Game_BattleAlgorithm::Observe::Execute() {
	// Observe only prints the start message
	return true;
}

Game_BattleAlgorithm::Charge::Charge(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Charge::GetStartMessage() const {
	return source->GetName() + Data::terms.focus;
}

bool Game_BattleAlgorithm::Charge::Execute() {
	Output::Warning("Battle: Enemy Charge not implemented");
	return true;
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::SelfDestruct::GetStartMessage() const {
	return source->GetName() + Data::terms.autodestruction;
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Output::Warning("Battle: Enemy SelfDestruct not implemented");
	return true;
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Escape::GetStartMessage() const {
	return source->GetName() + Data::terms.enemy_escape;
}

bool Game_BattleAlgorithm::Escape::Execute() {
	Output::Warning("Battle: Enemy Escape not implemented");
	return true;
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(source), new_monster_id(new_monster_id) {
	// no-op
}

std::string Game_BattleAlgorithm::Transform::GetStartMessage() const {
	return source->GetName() + Data::terms.enemy_transform;
}

bool Game_BattleAlgorithm::Transform::Execute() {
	return true;
}

void Game_BattleAlgorithm::Transform::Apply() {
	static_cast<Game_Enemy*>(source)->Transform(new_monster_id);
}
