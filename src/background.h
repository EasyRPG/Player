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

#ifndef _EASYRPG_BACKGROUND_H_
#define _EASYRPG_BACKGROUND_H_

// Headers
#include <string>
#include "system.h"
#include "drawable.h"

class Background : public Drawable {
public:
	Background(const std::string& name);
	Background(int terrain_id);
	~Background();

	void Draw();
	void Update();

	int GetZ() const;
	DrawableType GetType() const;

private:
	static const int z = -1000;
	static const DrawableType type = TypeBackground;

	static void Update(int& rate, int& value);
	static int Scale(int x);

	bool visible;

	BitmapScreenRef bg_screen;
	int bg_hscroll;
	int bg_vscroll;
	int bg_x;
	int bg_y;
	BitmapScreenRef fg_screen;
	int fg_hscroll;
	int fg_vscroll;
	int fg_x;
	int fg_y;
};

#endif
