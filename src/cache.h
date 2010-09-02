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
/// Cache namespace
////////////////////////////////////////////////////////////
namespace Cache {
	Bitmap* LoadBitmap(std::string folder_name, std::string filename);
	Bitmap* Backdrop(std::string filename);
	Bitmap* Battle(std::string filename);
	Bitmap* Battle2(std::string filename);
	Bitmap* BattleCharset(std::string filename);
	Bitmap* BattleWeapon(std::string filename);
	Bitmap* Charset(std::string filename);
	Bitmap* Faceset(std::string filename);
	Bitmap* Frame(std::string filename);
	Bitmap* Gameover(std::string filename);
	Bitmap* Monster(std::string filename);
	Bitmap* Panorama(std::string filename);
	Bitmap* Picture(std::string filename);
	Bitmap* Chipset(std::string filename);
	Bitmap* Title(std::string filename);
	Bitmap* System(std::string filename);
	Bitmap* System2(std::string filename);
	Bitmap* Tile(std::string filename, int tile_id);
	void Clear();

	extern std::map<std::string, Bitmap*> cache;
	extern std::map<std::string, std::map<int, Bitmap*> > cache_tiles;
}

#endif
