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
#include "game_battleaction.h"
#include "game_battler.h"
//#include "game_battlecommand.h"
#include "game_party_base.h"
#include "game_enemy.h"
#include "game_temp.h"
#include "main_data.h"
#include "game_message.h"
#include "game_actor.h"
#include "game_system.h"
#include "game_battle.h"
#include "spriteset_battle.h"

Game_BattleAction::ActionBase::ActionBase() :
	result(false),
	state(State_PreAction),
	animation(NULL),
	wait(30) {
		// no-op
}

bool Game_BattleAction::ActionBase::Execute() {
	if (animation) {
		if (!animation->GetVisible())
			animation->SetVisible(true);

		if (animation->GetFrame() >= animation->GetFrames()) {
			delete animation;
			animation = NULL;
		} else {
			animation->Update();
		}
		return false;
	}

	//if (Game_Message::message_waiting) {
	//	return false;
	//}

	switch(state) {
		case State_PreAction:
			PreAction();
			state = State_Action;
			break;
		case State_Action:
			Action();
			state = State_PostAction;
			break;
		case State_PostAction:
			if (wait--) {
				return false;
			}
			wait = 30;

			PostAction();
			state = result ? State_ResultAction : State_Finished;
			break;
		case State_ResultAction:
			if (wait--) {
				return false;
			}
			wait = 30;

			ResultAction();
			state = State_Finished;
			break;
		case State_Finished:
			if (wait--) {
				return false;
			}
			wait = 30;

			if (Again()) {
				state = State_PreAction;
			} else {
				return true;
			}
	}

	return false;
}

void Game_BattleAction::ActionBase::PlayAnimation(BattleAnimation* animation) {
	this->animation = animation;
}

Game_BattleAction::SingleTargetAction::SingleTargetAction(Game_Battler* source, Game_Battler* target) :
source(source), target(target) {
	// no-op
}

bool Game_BattleAction::SingleTargetAction::Again() {
	return false;
}

void Game_BattleAction::SingleTargetAction::ResultAction() {
	if (target->GetSignificantState()->ID == 1) {
		Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (target_sprite) {
			target_sprite->SetAnimationState(Sprite_Battler::Dead);
			Game_System::SePlay(Data::system.enemy_death_se);
		}
	}

	if (target->GetType() == Game_Battler::Type_Ally) {
		Game_Message::texts.push_back(target->GetName() + target->GetSignificantState()->message_actor);
	} else {
		Game_Message::texts.push_back(target->GetName() + target->GetSignificantState()->message_enemy);
	}
}

bool Game_BattleAction::GroupTargetAction::Again() {
	return true;
}

Game_BattleAction::AttackSingle::AttackSingle(Game_Battler* source, Game_Battler* target) :
	SingleTargetAction(source, target)
{
	// no-op
}

void Game_BattleAction::AttackSingle::PreAction() {
	Game_Message::texts.push_back("\r");
	Game_Message::texts.push_back(source->GetName() + Data::terms.attacking);
}

void Game_BattleAction::AttackSingle::Action() {
	double to_hit;

	if (target->IsDead()) {
		// Repoint to a different target if the selected one is dead
		target = target->GetParty().GetRandomAliveBattler();
	}

	Sprite_Battler* source_sprite = Game_Battle::GetSpriteset().FindBattler(source);
	if (source_sprite) {
		source_sprite->SetAnimationState(Sprite_Battler::SkillUse);
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		RPG::Animation* anim;
		int hit_chance = 80; // FIXME
		if (ally->GetWeaponId() == 0) {
			// No Weapon
			// Todo: Two Sword style
			anim = &Data::animations[Data::actors[ally->GetId() - 1].unarmed_animation - 1];
		} else {
			anim = &Data::animations[Data::items[ally->GetWeaponId() - 1].animation_id - 1];
			hit_chance = Data::items[ally->GetWeaponId() - 1].hit;
		}
		
		PlayAnimation(new BattleAnimation(target->GetBattleX(), target->GetBattleY(), anim));

		to_hit = 100 - (100 - hit_chance) * (1 + (1.0 * target->GetAgi() / ally->GetAgi() - 1) / 2);
	} else {
		// Source is Enemy

		//int hit = src->IsMissingOften() ? 70 : 90;
		int hit = 70;
		to_hit = 100 - (100 - hit) * (1 + (1.0 * target->GetAgi() / source->GetAgi() - 1) / 2);
	}

	// Damage calculation
	if (rand() % 100 < to_hit) {
		int effect = source->GetAtk() / 2 - target->GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;
		damage = effect;
		target->SetHp(target->GetHp() - effect);
		if (target->IsDead()) {
			result = true;
		}
	}
	else {
		damage = -1;
	}
}

void Game_BattleAction::AttackSingle::PostAction() {
	bool target_is_ally = target->GetType() == Game_Battler::Type_Ally;

	std::stringstream ss;
	ss << target->GetName();

	if (damage == -1) {
		ss << Data::terms.dodge;
		Game_System::SePlay(Data::system.dodge_se);
	} else {
		if (damage == 0) {
			ss << (target_is_ally ?
				Data::terms.actor_undamaged :
				Data::terms.enemy_undamaged);
		} else {
			ss << " " << damage << (target_is_ally ?
				Data::terms.actor_damaged :
				Data::terms.enemy_damaged);
		}
		Game_System::SePlay(target_is_ally ?
			Data::system.actor_damaged_se :
			Data::system.enemy_damaged_se);

		Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (target_sprite) {
			target_sprite->SetAnimationState(Sprite_Battler::Damage);
		}
	}

	Game_Message::texts.push_back(ss.str());
}
