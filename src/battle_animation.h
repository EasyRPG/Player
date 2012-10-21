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

#ifndef _BATTLE_ANIMATION_H_
#define _BATTLE_ANIMATION_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "system.h"
#include "rpg_animation.h"
#include "drawable.h"
#include "zobj.h"

////////////////////////////////////////////////////////////

class BattleAnimation : public Drawable {
public:
	BattleAnimation(int x, int y, const RPG::Animation* animation);
	~BattleAnimation();

	void Draw(int z_order);
	unsigned long GetId() const;
	int GetZ() const;
	DrawableType GetType() const;

	void Setup();
	void Update();
	int GetFrame() const;
	int GetFrames() const;
	void SetFrame(int);
	void SetVisible(bool visible);
	bool GetVisible();
	bool IsDone() const;

protected:
	int x;
	int y;
	const RPG::Animation* animation;
	int frame;
	bool initialized;
	bool visible;
	bool large;
	BitmapScreenRef screen;
	unsigned long ID;
	ZObj* zobj;
};

#endif
