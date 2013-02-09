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

#ifndef _BATTLE_ACTIONS_H_
#define _BATTLE_ACTIONS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "rpg_animation.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include "scene_battle.h"
#include <boost/scoped_ptr.hpp>

////////////////////////////////////////////////////////////

namespace Battle {

class Action {
public:
	virtual bool operator()() = 0;
	virtual ~Action() {}
};

class WaitAction : public Action {
public:
	WaitAction(int duration) :
		duration(duration) {}
	bool operator()();
protected:
	int duration;
};

class SpriteAction : public Action {
public:
	SpriteAction(Battle::Ally* ally, int anim_state) :
		ally(ally), anim_state(anim_state) {}
	bool operator()();
protected:
	Battle::Ally* ally;
	int anim_state;
};

class AnimationAction : public Action {
public:
	AnimationAction(const Sprite* target, const RPG::Animation* animation);
	AnimationAction(int x, int y, const RPG::Animation* animation);

	bool operator()();

protected:
	boost::scoped_ptr<BattleAnimation> animation;
};

class MoveAction : public Action {
public:
	MoveAction(Sprite* sprite, int x0, int x1, int speed) :
		sprite(sprite), x0(x0), x1(x1), speed(speed) {}
	bool operator()();
protected:
	Sprite* sprite;
	int x0;
	int x1;
	int speed;
};

class CommandAction : public Action {
public:
	CommandAction(void (*func)()) :
		func(func) {}
	bool operator()();
protected:
	void (*func)();
};

class CommandAction1 : public Action {
public:
	CommandAction1(void (*func)(void*), void* param) :
		func(func), param(param) {}
	bool operator()();
protected:
	void (*func)(void*);
	void* param;
};

}

#endif
