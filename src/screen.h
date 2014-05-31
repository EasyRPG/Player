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

#ifndef _EASYRPG_GRAPHIC_SCREEN_H_
#define _EASYRPG_GRAPHIC_SCREEN_H_

// Headers
#include <string>
#include "bitmap.h"
#include "drawable.h"
#include "system.h"

namespace RPG {
	class SaveScreen;
}

/**
 * A special drawable for handling screen effects.
 * This basically works by taking the screen surface and drawing on itself.
 * Sounds a bit dirty, but works.
 * 
 * The z index is chosen in a way that battle animations and message boxes
 * are not colorized.
 */
class Screen : public Drawable {
public:
	Screen();
	~Screen();

	void Draw();
	void Update();

	int GetZ() const;
	DrawableType GetType() const;

private:
	static const int z = 1050;
	static const DrawableType type = TypeScreen;

	Tone default_tone;
	BitmapRef flash;
};

#endif
