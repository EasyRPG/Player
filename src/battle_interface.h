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

#ifndef _BATTLE_INTERFACE_H_
#define _BATTLE_INTERFACE_H_

// Headers
#include <string>
#include "sprite.h"
#include "battle_battler.h"

/**
 * Battle interface class.
 * Provides an interface between Scene_Battle and Game_Battle.
 */
class Battle_Interface {

public:
	virtual void Restart() = 0;
	virtual void Message(const std::string& msg, bool pause = true) = 0;
	virtual void Floater(const Sprite* ref, int color, const std::string& text, int duration) = 0;
	virtual void Floater(const Sprite* ref, int color, int value, int duration) = 0;
	virtual void ShowAnimation(int animation_id, bool allies, Battle::Ally* ally, Battle::Enemy* enemy, bool wait) = 0;
	virtual void UpdateAnimations() = 0;
	virtual bool IsAnimationWaiting() = 0;
};

#endif

