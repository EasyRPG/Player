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

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Cache {
	std::map<std::string, Bitmap*> cache;
	std::map<std::string, std::map<int, Bitmap*> > cache_tiles;
}

////////////////////////////////////////////////////////////
/// Load Bitmap
////////////////////////////////////////////////////////////
Bitmap* Cache::LoadBitmap(std::string folder_name, std::string filename) {
	std::string path = folder_name + filename;
	if (cache.count(path) == 0) {
		if (filename != "") {
			cache[path] = new Bitmap(path);
		} else {
			cache[path] = new Bitmap(16, 16);
		}
	}
	return cache[path];
}

////////////////////////////////////////////////////////////
/// Quick loads
////////////////////////////////////////////////////////////
Bitmap* Cache::Backdrop(std::string filename) {
	return LoadBitmap("Backdrop/", filename);
}
Bitmap* Cache::Battle(std::string filename) {
	return LoadBitmap("Battle/", filename);
}
Bitmap* Cache::Battle2(std::string filename) {
	return LoadBitmap("Battle2/", filename);
}
Bitmap* Cache::BattleCharset(std::string filename) {
	return LoadBitmap("BattleCharSet/", filename);
}
Bitmap* Cache::BattleWeapon(std::string filename) {
	return LoadBitmap("BattleWeapon/", filename);
}
Bitmap* Cache::Charset(std::string filename) {
	return LoadBitmap("CharSet/", filename);
}
Bitmap* Cache::Faceset(std::string filename) {
	return LoadBitmap("FaceSet/", filename);
}
Bitmap* Cache::Frame(std::string filename) {
	return LoadBitmap("Frame/", filename);
}
Bitmap* Cache::Gameover(std::string filename) {
	return LoadBitmap("GameOver/", filename);
}
Bitmap* Cache::Monster(std::string filename) {
	return LoadBitmap("Monster/", filename);
}
Bitmap* Cache::Panorama(std::string filename) {
	return LoadBitmap("Panorama/", filename);
}
Bitmap* Cache::Picture(std::string filename) {
	return LoadBitmap("Picture/", filename);
}
Bitmap* Cache::Chipset(std::string filename) {
	return LoadBitmap("ChipSet/", filename);
}
Bitmap* Cache::Title(std::string filename) {
	return LoadBitmap("Title/", filename);
}
Bitmap* Cache::System(std::string filename) {
	return LoadBitmap("System/", filename);
}
Bitmap* Cache::System2(std::string filename) {
	return LoadBitmap("System2/", filename);
}

////////////////////////////////////////////////////////////
/// Load tile
////////////////////////////////////////////////////////////
Bitmap* Cache::Tile(std::string filename, int tile_id) {
	if (cache_tiles.count(filename) == 0 || cache_tiles[filename].count(tile_id) == 0) {
		Bitmap* bmp = new Bitmap(16, 16);
		// TODO
		cache_tiles[filename][tile_id] = bmp;
	}
	return cache_tiles[filename][tile_id];
}

////////////////////////////////////////////////////////////
/// Clear
////////////////////////////////////////////////////////////
void Cache::Clear() {
	std::map<std::string, Bitmap*>::iterator it_cache;
	for (it_cache = cache.begin(); it_cache != cache.end(); ) {
		delete (*it_cache++).second;
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
