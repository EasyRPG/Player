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
#include "cache.h"
#include "filefinder.h"
#include "exfont.h"

////////////////////////////////////////////////////////////
typedef std::pair<std::string,std::string> string_pair;

namespace {
	std::map<string_pair, Bitmap*> cache;
	std::map<std::string, std::map<int, Bitmap*> > cache_tiles;
}

tSystemInfo Cache::system_info;

bool IsCached(const string_pair& key) {
	return cache.count(key) != 0;
}

bool IsCached(const std::string& folder_name, const std::string& filename) {
	string_pair key = string_pair(folder_name, filename);
	return cache.count(key) != 0;
}

////////////////////////////////////////////////////////////
Bitmap* Cache::LoadBitmap(
	const std::string& folder_name, 
	const std::string& filename,
	bool transparent, 
	bool read_only
) {
	string_pair key = string_pair(folder_name, filename);

	if ( !IsCached(key) ) {
		std::string path = FileFinder::FindImage(folder_name, filename);
		if (!path.empty()) {
			if (!read_only) {
				return Bitmap::CreateBitmap(path, transparent, read_only);
			}
			cache[key] = Bitmap::CreateBitmap(path, transparent);
		} else {
			if (!read_only) {
				return 0;
			}
			cache[key] = Bitmap::CreateBitmap(16, 16);
		}
	}

	return cache[key];
}

////////////////////////////////////////////////////////////
Bitmap* Cache::Backdrop(const std::string& filename) {
	return LoadBitmap("Backdrop", filename, false);
}
Bitmap* Cache::Battle(const std::string& filename) {
	return LoadBitmap("Battle", filename, true);
}
Bitmap* Cache::Battle2(const std::string& filename) {
	return LoadBitmap("Battle2", filename, true);
}
Bitmap* Cache::BattleCharset(const std::string& filename) {
	return LoadBitmap("BattleCharSet", filename, true);
}
Bitmap* Cache::BattleWeapon(const std::string& filename) {
	return LoadBitmap("BattleWeapon", filename, true);
}
Bitmap* Cache::Charset(const std::string& filename) {
	return LoadBitmap("CharSet", filename, true);
}
Bitmap* Cache::ExFont() {
	string_pair hash = string_pair("\x00","ExFont");

	if (cache.count(hash) == 0) {
		cache[hash] = Bitmap::CreateBitmap(exfont_h, sizeof(exfont_h), true);
	}

	return cache[hash];
}
Bitmap* Cache::Faceset(const std::string& filename) {
	return LoadBitmap("FaceSet", filename, true);
}
Bitmap* Cache::Frame(const std::string& filename) {
	return LoadBitmap("Frame", filename, true);
}
Bitmap* Cache::Gameover(const std::string& filename) {
	return LoadBitmap("GameOver", filename, false);
}
Bitmap* Cache::Monster(const std::string& filename) {
	return LoadBitmap("Monster", filename, true);
}
Bitmap* Cache::Panorama(const std::string& filename) {
	return LoadBitmap("Panorama", filename, false);
}
Bitmap* Cache::Picture(const std::string& filename) {
	return LoadBitmap("Picture", filename, true);
}
Bitmap* Cache::Chipset(const std::string& filename) {
	return LoadBitmap("ChipSet", filename, true);
}
Bitmap* Cache::Title(const std::string& filename) {
	return LoadBitmap("Title", filename, false);
}
Bitmap* Cache::System(const std::string& filename) {
	const char* sys_name = "System";
	if (!IsCached(sys_name, filename)) {
		// Load bitmap in system memory for direct pixel access
		Bitmap* sram_b = LoadBitmap(sys_name, filename, true, false);
		if (!sram_b) {
			return 0;
		}
		// Overwrite previous colours. Probably FIXME...
		system_info.bg_color = sram_b->GetPixel(0, 32);
		system_info.sh_color = sram_b->GetPixel(16, 32);

		// FIXME: Load new bitmap from system memory
		// instead of deleting and loading again from disk
		delete sram_b;
	}
	return LoadBitmap(sys_name, filename, true);
}
Bitmap* Cache::System2(const std::string& filename) {
	return LoadBitmap("System2", filename, true);
}

////////////////////////////////////////////////////////////
Bitmap* Cache::Tile(const std::string& filename, int tile_id) {
	if (cache_tiles.count(filename) == 0 || cache_tiles[filename].count(tile_id) == 0) {
		Bitmap* tile = Bitmap::CreateBitmap(16, 16);
		Bitmap* chipset = Cache::Chipset(filename);
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


		tile = Bitmap::CreateBitmap(chipset, rect);
		cache_tiles[filename][tile_id] = tile;
	}
	return cache_tiles[filename][tile_id];
}

////////////////////////////////////////////////////////////
void Cache::Clear() {
	std::map<string_pair, Bitmap*>::iterator it_cache;
	for (it_cache = cache.begin(); it_cache != cache.end(); it_cache++) {
		delete it_cache->second;
	}
	cache.clear();
	// TODO Why is this clean up commented out?
	/*for (it1_cache_tiles = cache_tiles.begin(); it1_cache_tiles != cache_tiles.end(); it1_cache_tiles++) {
		for (it2_cache_tiles = it1_cache_tiles.begin(); it2_cache_tiles != it1_cache_tiles.end(); it2_cache_tiles++) {
			delete it2_cache_tiles->second;
		}
	}
	cache_tiles.clear();*/
}
