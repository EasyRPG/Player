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
#include "game_party_base.h"
#include "game_system.h"
#include "game_temp.h"
#include "main_data.h"
#include "rpg_animation.h"
#include "rpg_state.h"
#include "rpg_skill.h"
#include "rpg_item.h"

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
	success = false;
	killed_by_attack_damage = false;
	critical_hit = false;
	animation = NULL;
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedHp() const {
	return hp != -1 ? hp : boost::optional<int>();
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedSp() const {
	return sp != -1 ? hp : boost::optional<int>();
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedAttack() const {
	return attack != -1 ? hp : boost::optional<int>();
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedDefense() const {
	return defense != -1 ? hp : boost::optional<int>();
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedSpirit() const {
	return spirit != -1 ? hp : boost::optional<int>();
}

boost::optional<int> Game_BattleAlgorithm::AlgorithmBase::GetAffectedAgility() const {
	return agility != -1 ? hp : boost::optional<int>();
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

	if ((*current_target)->GetType() == Game_Battler::Type_Ally) {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_actor;
	} else {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_enemy;
	}
}

void Game_BattleAlgorithm::AlgorithmBase::GetResultMessages(std::vector<std::string>& out) const {
	if (!success) {
		out.push_back((*current_target)->GetName() + Data::terms.dodge);
	}

	bool target_is_ally = (*current_target)->GetType() == Game_Battler::Type_Ally;

	if (GetAffectedHp()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if (*GetAffectedHp() == 0) {
			ss << (target_is_ally ?
				Data::terms.actor_undamaged :
			Data::terms.enemy_undamaged);
		} else {
			ss << " " << *GetAffectedHp() << (target_is_ally ?
				Data::terms.actor_damaged :
			Data::terms.enemy_damaged);
		}
		out.push_back(ss.str());
	}

	if (GetAffectedSp()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.attack << " " << *GetAffectedSp();
		out.push_back(ss.str());
	}

	if (GetAffectedAttack()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.attack << " " << *GetAffectedSp();
		out.push_back(ss.str());
	}

	if (GetAffectedDefense()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.defense << " " << *GetAffectedDefense();
		out.push_back(ss.str());
	}

	if (GetAffectedSpirit()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.spirit << " " << *GetAffectedSpirit();
		out.push_back(ss.str());
	}

	if (GetAffectedAgility()) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.agility << " " << *GetAffectedAgility();
		out.push_back(ss.str());
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	// TODO: Handle healing

	for (; it != conditions.end(); ++it) {
		if ((*current_target)->HasState(it->ID)) {
			if (!it->message_already.empty()) {
				out.push_back(it->message_already);
			}
		} else {
			std::stringstream ss;
			ss << (*current_target)->GetName();

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
	return *current_target;
}

void Game_BattleAlgorithm::AlgorithmBase::SetTarget(Game_Battler* target) {
	targets.clear();
	targets.push_back(target);
	current_target = targets.begin();
}

void Game_BattleAlgorithm::AlgorithmBase::Apply() {
	if (GetAffectedHp()) {
		(*current_target)->ChangeHp(-*GetAffectedHp());
	}

	if (GetAffectedSp()) {
		(*current_target)->SetSp((*current_target)->GetSp() -*GetAffectedSp());
	}

	// TODO
	if (GetAffectedAttack()) {
	}

	if (GetAffectedDefense()) {
	}

	if (GetAffectedSpirit()) {
	}

	if (GetAffectedAgility()) {
	}
	// End TODO

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	// TODO: Handle healing

	for (; it != conditions.end(); ++it) {
		(*current_target)->AddState(it->ID);
	}
}

bool Game_BattleAlgorithm::AlgorithmBase::IsDeadTargetValid() {
	return (!(*current_target)->IsDead());
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
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

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, const RPG::Skill& skill) :
	AlgorithmBase(source, target), skill(skill) {
		// no-op
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Party_Base* target, const RPG::Skill& skill) :
	AlgorithmBase(source, target), skill(skill) {
		// no-op
}

bool Game_BattleAlgorithm::Skill::Execute() {
	Reset();

	animation = &Data::animations[skill.animation_id == 0 ? 0 : skill.animation_id - 1];

	this->success = false;

	if (skill.type == RPG::Skill::Type_normal) {
		if (skill.power > 0) {
			if (rand() % 100 < skill.hit) {
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

			// TODO
			//if (skill.state_effect)
				conditions.push_back(Data::states[i]);
			//	actor->AddState(i + 1);
			//else
			//	actor->RemoveState(i + 1);
		}

		return this->success;
	}

	return this->success;
}

void Game_BattleAlgorithm::Skill::Apply() {
	AlgorithmBase::Apply();

	source->SetSp(source->GetSp() - source->CalculateSkillCost(skill.ID));
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage() const {
	// TODO: How to handle using_message2?
	return source->GetName() + " " + skill.using_message1;
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

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	success = false;
	return success;
}

void Game_BattleAlgorithm::Item::Apply() {
	AlgorithmBase::Apply();
}

std::string Game_BattleAlgorithm::Item::GetStartMessage() const {
	return source->GetName() + " TODO USE ITEM MSG";
}

void Game_BattleAlgorithm::Item::GetResultMessages(std::vector<std::string>& out) const {

}
