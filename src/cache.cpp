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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <map>

#include <boost/weak_ptr.hpp>

#include "cache.h"
#include "filefinder.h"
#include "exfont.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
typedef std::pair<std::string,std::string> string_pair;
typedef std::pair<std::string, int> tile_pair;

namespace {
std::map<string_pair, boost::weak_ptr<Bitmap> > cache;
std::map<tile_pair, boost::weak_ptr<Bitmap> > cache_tiles;
}

tSystemInfo Cache::system_info;

////////////////////////////////////////////////////////////
BitmapRef Cache::LoadBitmap(
	const std::string& folder_name,
	const std::string& filename,
	bool const transparent,
	uint32_t const flags
) {
	string_pair const key(folder_name, filename);

	std::map<string_pair, boost::weak_ptr<Bitmap> >::iterator const it = cache.find(key);

	if (it == cache.end() || it->second.expired()) {
		std::string const path = FileFinder::FindImage(folder_name, filename);
		return (cache[key] = path.empty()
				? Bitmap::Create(16, 16, Color())
				: Bitmap::Create(path, transparent, flags)
				).lock();
	} else { return it->second.lock(); }
}

////////////////////////////////////////////////////////////
BitmapRef Cache::Backdrop(const std::string& filename) {
	return LoadBitmap("Backdrop", filename, false);
}
BitmapRef Cache::Battle(const std::string& filename) {
	return LoadBitmap("Battle", filename);
}
BitmapRef Cache::Battle2(const std::string& filename) {
	return LoadBitmap("Battle2", filename);
}
BitmapRef Cache::BattleCharset(const std::string& filename) {
	return LoadBitmap("BattleCharSet", filename);
}
BitmapRef Cache::BattleWeapon(const std::string& filename) {
	return LoadBitmap("BattleWeapon", filename);
}
BitmapRef Cache::Charset(const std::string& filename) {
	return LoadBitmap("CharSet", filename);
}
BitmapRef Cache::ExFont() {
	string_pair const hash("\x00","ExFont");

	std::map<string_pair, boost::weak_ptr<Bitmap> >::iterator const it = cache.find(hash);

	if (it == cache.end() || it->second.expired()) {
		return(cache[hash] = Bitmap::Create(exfont_h, sizeof(exfont_h), true)).lock();
	} else { return it->second.lock(); }
}
BitmapRef Cache::Faceset(const std::string& filename) {
	return LoadBitmap("FaceSet", filename);
}
BitmapRef Cache::Frame(const std::string& filename) {
	return LoadBitmap("Frame", filename);
}
BitmapRef Cache::Gameover(const std::string& filename) {
	return LoadBitmap("GameOver", filename, false);
}
BitmapRef Cache::Monster(const std::string& filename) {
	return LoadBitmap("Monster", filename);
}
BitmapRef Cache::Panorama(const std::string& filename) {
	return LoadBitmap("Panorama", filename, false);
}
BitmapRef Cache::Picture(const std::string& filename) {
	return LoadBitmap("Picture", filename);
}
BitmapRef Cache::Chipset(const std::string& filename) {
	return LoadBitmap("ChipSet", filename, true, Bitmap::Chipset);
}
BitmapRef Cache::Title(const std::string& filename) {
	return LoadBitmap("Title", filename, false);
}
BitmapRef Cache::System(const std::string& filename) {
	return LoadBitmap("System", filename, true, Bitmap::System);
}
BitmapRef Cache::System2(const std::string& filename) {
	return LoadBitmap("System2", filename);
}

////////////////////////////////////////////////////////////
BitmapRef Cache::Tile(const std::string& filename, int tile_id) {
	tile_pair const key(filename, tile_id);
	std::map<tile_pair, boost::weak_ptr<Bitmap> >::iterator const it = cache_tiles.find(key);

	if (it == cache_tiles.end() || it->second.expired()) {
		BitmapRef chipset = Cache::Chipset(filename);
		Rect rect = Rect(0, 0, 16, 16);

		int sub_tile_id = 0;

		if (tile_id > 0 && tile_id < 48) {
			sub_tile_id = tile_id;
			rect.x += 288;
			rect.y += 128;
		} else if (tile_id >= 48 && tile_id < 96) {
			sub_tile_id = tile_id - 48;
			rect.x += 384;
		} else if (tile_id >= 96 && tile_id < 144) {
			sub_tile_id = tile_id - 96;
			rect.x += 384;
			rect.y += 128;
		} else { // Invalid -> Use empty file (first one)
			rect.x = 288;
			rect.y = 128;
		}

		rect.x += sub_tile_id % 6 * 16;
		rect.y += sub_tile_id / 6 * 16;

		return(cache_tiles[key] = Bitmap::Create(*chipset, rect)).lock();
	} else { return it->second.lock(); }
}

////////////////////////////////////////////////////////////
void Cache::Clear() {
	cache.clear();
	cache_tiles.clear();
}
