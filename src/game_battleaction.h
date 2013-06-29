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

	ActionBase();

	virtual bool Execute();

	virtual void PreAction() = 0;
	virtual void Action() = 0;
	virtual void PostAction() = 0;
	virtual void ResultAction() = 0;
	virtual bool Again() = 0;

	void PlayAnimation(BattleAnimation* animation);

protected:
	bool result;

private:
	int state;
	BattleAnimation* animation;
	int wait;
};

class SingleTargetAction : public ActionBase {
public:
	SingleTargetAction(Game_Battler* source, Game_Battler* target);

	virtual bool Again();
	virtual void ResultAction();

protected:
	Game_Battler* source;
	Game_Battler* target;
};

class GroupTargetAction : public ActionBase {
public:
	GroupTargetAction();

	virtual bool Again();
};

class PartyTargetAction : public GroupTargetAction {

};

class AttackSingle : public SingleTargetAction {
public:
	AttackSingle(Game_Battler* source, Game_Battler* target);

	void PreAction();
	void Action();
	void PostAction();

private:
	int damage;
};

}

#endif
