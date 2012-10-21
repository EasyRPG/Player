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
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include "color.h"

class Bitmap;

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
	boost::shared_ptr<Bitmap> LoadBitmap(const std::string& folder_name, const std::string& filename,
					   bool transparent = true, uint32_t flags = 0);
	boost::shared_ptr<Bitmap> Backdrop(const std::string& filename);
	boost::shared_ptr<Bitmap> Battle(const std::string& filename);
	boost::shared_ptr<Bitmap> Battle2(const std::string& filename);
	boost::shared_ptr<Bitmap> BattleCharset(const std::string& filename);
	boost::shared_ptr<Bitmap> BattleWeapon(const std::string& filename);
	boost::shared_ptr<Bitmap> Charset(const std::string& filename);
	boost::shared_ptr<Bitmap> ExFont();
	boost::shared_ptr<Bitmap> Faceset(const std::string& filename);
	boost::shared_ptr<Bitmap> Frame(const std::string& filename);
	boost::shared_ptr<Bitmap> Gameover(const std::string& filename);
	boost::shared_ptr<Bitmap> Monster(const std::string& filename);
	boost::shared_ptr<Bitmap> Panorama(const std::string& filename);
	boost::shared_ptr<Bitmap> Picture(const std::string& filename);
	boost::shared_ptr<Bitmap> Chipset(const std::string& filename);
	boost::shared_ptr<Bitmap> Title(const std::string& filename);
	boost::shared_ptr<Bitmap> System(const std::string& filename);
	boost::shared_ptr<Bitmap> System2(const std::string& filename);
	boost::shared_ptr<Bitmap> Tile(const std::string& filename, int tile_id);
	void Clear();

	extern tSystemInfo system_info;
}

#endif
