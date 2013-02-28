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

#ifndef _PICTURE_H_
#define _PICTURE_H_

// Headers
#include <string>
#include <vector>
#include "system.h"
#include "rpg_save.h"
#include "sprite.h"
#include <boost/scoped_ptr.hpp>

/**
 * Picture class.
 */
class Sprite;

class Picture {
public:
	Picture(int ID);

	void Show(const std::string& name);
	void Erase();
	void UseTransparent(bool flag);
	void Scrolls(bool flag);
	void Move(int x, int y);
	void Color(int r, int g, int b, int s);
	void Magnify(int scale);
	void Transparency(int t, int b);
	void Rotate(int speed);
	void Waver(int depth);
	void StopEffects();
	void Transition(int tenths);

	void Update();

private:
	RPG::SavePicture& data;

	static const int waver_speed = 10;

	boost::scoped_ptr<Sprite> sprite;

	void UpdateSprite();
};

#endif
