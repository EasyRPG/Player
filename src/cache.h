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

#ifndef _CACHE_H_
#define _CACHE_H_

// Headers
#include <string>

#include "system.h"
#include "color.h"
#include "event_system.h"
#include "memory_management.h"

/**
 * Cache namespace.
 */
namespace Cache {
	BitmapRef Backdrop(const std::string& filename, bool& ready);
	BitmapRef Battle(const std::string& filename, bool& ready);
	BitmapRef Battle2(const std::string& filename, bool& ready);
	BitmapRef Battlecharset(const std::string& filename, bool& ready);
	BitmapRef Battleweapon(const std::string& filename, bool& ready);
	BitmapRef Charset(const std::string& filename, bool& ready);
	BitmapRef Exfont();
	BitmapRef Faceset(const std::string& filename, bool& ready);
	BitmapRef Frame(const std::string& filename, bool& ready);
	BitmapRef Gameover(const std::string& filename, bool& ready);
	BitmapRef Monster(const std::string& filename, bool& ready);
	BitmapRef Panorama(const std::string& filename, bool& ready);
	BitmapRef Picture(const std::string& filename, bool transparent, bool& ready);
	BitmapRef Chipset(const std::string& filename, bool& ready);
	BitmapRef Title(const std::string& filename, bool& ready);
	BitmapRef System(const std::string& filename, bool& ready);
	BitmapRef System2(const std::string& filename, bool& ready);
	BitmapRef Tile(const std::string& filename, int tile_id);

	void Clear();

	BitmapRef System(bool& ready);
	void SetSystemName(std::string const& filename);
}

#endif
