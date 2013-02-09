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
#include "battle_actions.h"

////////////////////////////////////////////////////////////
bool Battle::WaitAction::operator()() {
	duration--;
	return duration <= 0;
}

////////////////////////////////////////////////////////////
bool Battle::SpriteAction::operator()() {
	((Scene_Battle*) Scene::instance.get())->SetAnimState(*ally, anim_state);
	return true;
}

////////////////////////////////////////////////////////////
bool Battle::CommandAction::operator()() {
	(*func)();
	return true;
}

////////////////////////////////////////////////////////////
bool Battle::CommandAction1::operator()() {
	(*func)(param);
	return true;
}

////////////////////////////////////////////////////////////
bool Battle::MoveAction::operator()() {
	int x = sprite->GetX();
	bool done = false;
	if (x0 < x1) {
		x += speed;
		done = (x >= x1);
	}
	if (x0 > x1) {
		x -= speed;
		done = (x <= x1);
	}
	if (x0 == x1)
		done = true;

	if (done)
		x = x1;

	sprite->SetX(x);

	return done;
}

////////////////////////////////////////////////////////////
Battle::AnimationAction::AnimationAction(const Sprite* target, const RPG::Animation* animation) {
	this->animation.reset(new BattleAnimation(target->GetX(), target->GetY(), animation));
}

Battle::AnimationAction::AnimationAction(int x, int y, const RPG::Animation* animation) {
	this->animation.reset(new BattleAnimation(x, y, animation));
}

bool Battle::AnimationAction::operator()() {
	if (!animation->GetVisible())
		animation->SetVisible(true);
	else
		animation->Update();

	return animation->GetFrame() >= animation->GetFrames();
}
