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

#ifndef _PLANE_H_
#define _PLANE_H_

// Headers
#include "system.h"
#include "color.h"
#include "drawable.h"
#include "tone.h"

/**
 * Plane class.
 */
class Plane : public Drawable {
public:
	Plane();
	virtual ~Plane();

	void Draw();

	BitmapRef const& GetBitmap() const;
	void SetBitmap(BitmapRef const& bitmap);
	bool GetVisible() const;
	void SetVisible(bool visible);
	int GetZ() const;
	void SetZ(int z);
	int GetOx() const;
	void SetOx(int ox);
	int GetOy() const;
	void SetOy(int oy);

	DrawableType GetType() const;

private:
	DrawableType type;

	BitmapRef bitmap;
	BitmapScreenRef bitmap_screen;

	bool visible;
	int z;
	int ox;
	int oy;
};

#endif
