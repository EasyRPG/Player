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
	BitmapRef Backdrop(std::string_view filename);
	BitmapRef Battle(std::string_view filename);
	BitmapRef Battle2(std::string_view filename);
	BitmapRef Battlecharset(std::string_view filename);
	BitmapRef Battleweapon(std::string_view filename);
	BitmapRef Charset(std::string_view filename);
	BitmapRef Exfont();
	BitmapRef Faceset(std::string_view filename);
	BitmapRef Frame(std::string_view filename, bool transparent = true);
	BitmapRef Gameover(std::string_view filename);
	BitmapRef Monster(std::string_view filename);
	BitmapRef Panorama(std::string_view filename);
	BitmapRef Picture(std::string_view filename, bool transparent);
	BitmapRef Chipset(std::string_view filename);
	BitmapRef Title(std::string_view filename);
	BitmapRef System(std::string_view filename);
	BitmapRef System2(std::string_view filename);

	BitmapRef Tile(std::string_view filename, int tile_id);
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
