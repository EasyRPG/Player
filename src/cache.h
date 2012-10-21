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

#ifndef _CACHE_H_
#define _CACHE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

#include "system.h"
#include "color.h"

////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////
typedef struct {
	Color bg_color;
	Color sh_color;
} tSystemInfo;

////////////////////////////////////////////////////////////
/// Cache namespace
////////////////////////////////////////////////////////////
namespace Cache {
	BitmapRef LoadBitmap(const std::string& folder_name, const std::string& filename,
					   bool transparent = true, uint32_t flags = 0);
	BitmapRef Backdrop(const std::string& filename);
	BitmapRef Battle(const std::string& filename);
	BitmapRef Battle2(const std::string& filename);
	BitmapRef BattleCharset(const std::string& filename);
	BitmapRef BattleWeapon(const std::string& filename);
	BitmapRef Charset(const std::string& filename);
	BitmapRef ExFont();
	BitmapRef Faceset(const std::string& filename);
	BitmapRef Frame(const std::string& filename);
	BitmapRef Gameover(const std::string& filename);
	BitmapRef Monster(const std::string& filename);
	BitmapRef Panorama(const std::string& filename);
	BitmapRef Picture(const std::string& filename);
	BitmapRef Chipset(const std::string& filename);
	BitmapRef Title(const std::string& filename);
	BitmapRef System(const std::string& filename);
	BitmapRef System2(const std::string& filename);
	BitmapRef Tile(const std::string& filename, int tile_id);
	void Clear();

	extern tSystemInfo system_info;
}

#endif
