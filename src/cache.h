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
#include <map>
#include "bitmap.h"

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
	Bitmap* LoadBitmap(const std::string& folder_name, const std::string& filename, bool transparent, bool read_only = true);
	Bitmap* Backdrop(const std::string& filename);
	Bitmap* Battle(const std::string& filename);
	Bitmap* Battle2(const std::string& filename);
	Bitmap* BattleCharset(const std::string& filename);
	Bitmap* BattleWeapon(const std::string& filename);
	Bitmap* Charset(const std::string& filename);
	Bitmap* ExFont();
	Bitmap* Faceset(const std::string& filename);
	Bitmap* Frame(const std::string& filename);
	Bitmap* Gameover(const std::string& filename);
	Bitmap* Monster(const std::string& filename);
	Bitmap* Panorama(const std::string& filename);
	Bitmap* Picture(const std::string& filename);
	Bitmap* Chipset(const std::string& filename);
	Bitmap* Title(const std::string& filename);
	Bitmap* System(const std::string& filename);
	Bitmap* System2(const std::string& filename);
	Bitmap* Tile(const std::string& filename, int tile_id);
	void Clear();

	extern tSystemInfo system_info;
}

#endif
