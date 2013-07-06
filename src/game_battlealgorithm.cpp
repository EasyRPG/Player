/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <sstream>
#include "game_actor.h"
#include "game_battlealgorithm.h"
#include "game_battleaction.h"
#include "game_battler.h"
#include "game_system.h"
#include "game_temp.h"
#include "main_data.h"
#include "rpg_animation.h"
#include "rpg_state.h"
#include "rpg_skill.h"
#include "rpg_item.h"

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Battler* target) :
	source(source), target(target), hp(-1), sp(-1), attack(-1), defense(-1), spirit(-1), agility(-1), animation(NULL) {
		// no-op
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

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(source, target) {
	// no-op
}

bool Game_BattleAlgorithm::Normal::Execute() {
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
		to_hit = (int)(100 - (100 - hit_chance) * (1 + (1.0 * target->GetAgi() / ally->GetAgi() - 1) / 2));
	} else {
		// Source is Enemy

		//int hit = src->IsMissingOften() ? 70 : 90;
		int hit = 70;
		to_hit = (int)(100 - (100 - hit) * (1 + (1.0 * target->GetAgi() / source->GetAgi() - 1) / 2));
	}

	// Damage calculation
	if (rand() % 100 < to_hit) {
		int effect = source->GetAtk() / 2 - target->GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;
		this->hp = effect;

		if (target->GetHp() - this->hp <= 0) {
			// Death state
			conditions.push_back(Data::states[0]);
		}
	}
	else {
		return false;
	}

	return true;
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, RPG::Skill& skill) :
	AlgorithmBase(source, target), skill(skill) {
		// no-op
}

bool Game_BattleAlgorithm::Skill::Execute() {
	animation = &Data::animations[skill.animation_id == 0 ? 0 : skill.animation_id - 1];

	if (skill.type == RPG::Skill::Type_normal) {
		bool miss = true;

		if (skill.power > 0) {
			if (rand() % 100 < skill.hit) {
				miss = false;

				// FIXME: is this still affected by stats for allies?
				int effect = skill.power;

				if (skill.variance > 0) {
					int var_perc = skill.variance * 5;
					int act_perc = rand() % (var_perc * 2) - var_perc;
					int change = effect * act_perc / 100;
					effect += change;
				}

				if (skill.affect_hp) {
					this->hp = effect;

					if (target->GetHp() - this->hp <= 0) {
						// Death state
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

			miss = false;

			//if (skill.state_effect)
				conditions.push_back(Data::states[i]);
			//	actor->AddState(i + 1);
			//else
			//	actor->RemoveState(i + 1);
		}

		return !miss;
	}

	return false;
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Battler* target, RPG::Item& item) :
	AlgorithmBase(source, target), item(item) {
		// no-op
}

bool Game_BattleAlgorithm::Item::Execute() {
	return false;
}
