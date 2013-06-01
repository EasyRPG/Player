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

class Action {
public:
	virtual bool Execute() = 0;
};

class AttackSingle : public Action {
public:
	AttackSingle(Game_Battler* source, Game_Battler* target);
	bool Execute();

protected:
	Game_Battler* source;
	Game_Battler* target;
	BattleAnimation* animation;
};

}

#endif
