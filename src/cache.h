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

#ifndef EP_CACHE_H
#define EP_CACHE_H

// Headers
#include <cstdint>
#include <string>
#include <vector>

#include "system.h"
#include "memory_management.h"
#include "string_view.h"

#define CACHE_DEFAULT_BITMAP "\x01"

class Color;
class Rect;
class Tone;

/**
 * Cache namespace.
 */
namespace Cache {
	BitmapRef Backdrop(StringView filename);
	BitmapRef Battle(StringView filename);
	BitmapRef Battle2(StringView filename);
	BitmapRef Battlecharset(StringView filename);
	BitmapRef Battleweapon(StringView filename);
	BitmapRef Charset(StringView filename);
	BitmapRef Exfont();
	BitmapRef Faceset(StringView filename);
	BitmapRef Frame(StringView filename, bool transparent = true);
	BitmapRef Gameover(StringView filename);
	BitmapRef Monster(StringView filename);
	BitmapRef Panorama(StringView filename);
	BitmapRef Picture(StringView filename, bool transparent);
	BitmapRef Chipset(StringView filename);
	BitmapRef Title(StringView filename);
	BitmapRef System(StringView filename);
	BitmapRef System2(StringView filename);

	BitmapRef Tile(StringView filename, int tile_id);
	BitmapRef SpriteEffect(const BitmapRef& src_bitmap, const Rect& rect, bool flip_x, bool flip_y, const Tone& tone, const Color& blend);

	void Clear();
	void ClearAll();

	/** @return the configured system bitmap, or nullptr if there is no system */
	BitmapRef System();

	/** @return a black system graphic bitmap */
	BitmapRef SysBlack();

	/** @return the configured system bitmap, or a default black bitmap if there is no system */
	BitmapRef SystemOrBlack();

	/** @return the configured system2 bitmap, or nullptr if there is no system2 */
	BitmapRef System2();

	void SetSystemName(std::string filename);
	void SetSystem2Name(std::string filename);

	extern std::vector<uint8_t> exfont_custom;
}

#endif
