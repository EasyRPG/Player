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

#ifndef _EASYRPG_GAME_BATTLEACTION_H_
#define _EASYRPG_GAME_BATTLEACTION_H_

#include <vector>
#include "rpg_animation.h"
#include "rpg_item.h"
#include "rpg_skill.h"
#include "sprite.h"

#include "battle_animation.h"

class Game_Battler;
class Game_Party_Base;

/**
 * Game Battle Action
 */
namespace Game_BattleAction {

class ActionBase {
public:
	enum ActionState {
		State_PreAction,
		State_Action,
		State_PostAction,
		State_ResultAction,
		State_Finished
	};

	ActionBase(Game_Battler* source);

	virtual bool Execute() = 0;

	virtual void PreAction();
	virtual void Action() = 0;
	virtual void PostAction() = 0;
	virtual void ResultAction() = 0;

	void PlayAnimation(BattleAnimation* animation);

	Game_Battler* GetSource();

protected:
	bool result;
	Game_Battler* source;
	int state;
	BattleAnimation* animation;
	int wait;
};

class SingleTargetAction : public ActionBase {
public:
	SingleTargetAction(Game_Battler* source, Game_Battler* target);

	virtual bool Execute();

	virtual void ResultAction();

protected:
	Game_Battler* target;
};

class PartyTargetAction : public ActionBase {
public:
	PartyTargetAction(Game_Battler* source, Game_Party_Base* target);

	virtual bool Execute();
	virtual void ResultAction();

protected:
	Game_Party_Base* target;

	std::vector<Game_Battler*> alive;
	std::vector<Game_Battler*>::iterator current_target;
};

class AttackPartyNormal : public PartyTargetAction {
public:
	AttackPartyNormal(Game_Battler* source, Game_Party_Base* target);

	void Action();
	void PostAction();
	void ResultAction();
};

class AttackSingleNormal : public SingleTargetAction {
public:
	AttackSingleNormal(Game_Battler* source, Game_Battler* target);

	void Action();
	void PostAction();

private:
	int damage;
};

class AttackSingleSkill : public SingleTargetAction {
public:
	AttackSingleSkill(Game_Battler* source, Game_Battler* target, RPG::Skill* skill);

	void Action();
	void PostAction();

private:
	int damage;
	RPG::Skill* skill;
};

class AttackPartySkill : public PartyTargetAction {
public:
	AttackPartySkill(Game_Battler* source, Game_Party_Base* target, RPG::Skill* skill);

	void Action();
	void PostAction();

private:
	int damage;
	RPG::Skill* skill;
};

}

#endif
