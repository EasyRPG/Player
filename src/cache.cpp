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

// Headers
#ifdef _MSC_VER
#  pragma warning(disable: 4003)
#endif

#include <map>

#include "async_handler.h"
#include "cache.h"
#include "filefinder.h"
#include "exfont.h"
#include "default_graphics.h"
#include "bitmap.h"
#include "output.h"
#include "player.h"
#include "data.h"

namespace {
	typedef std::pair<std::string,std::string> string_pair;

	struct CacheItem {
		BitmapRef bitmap;
		uint32_t last_access;
	};

	typedef std::pair<std::string, int> tile_pair;

	typedef std::map<string_pair, CacheItem> cache_type;
	cache_type cache;

	typedef std::map<tile_pair, std::weak_ptr<Bitmap> > cache_tiles_type;
	cache_tiles_type cache_tiles;

	std::string system_name;

	void FreeBitmapMemory() {
		int32_t cur_ticks = DisplayUi->GetTicks();

		for (auto& i : cache) {
			if (i.second.bitmap.use_count() != 1) { continue; }

			if (cur_ticks - i.second.last_access < 5000) {
				// Last access < 5s
				continue;
			}

			//Output::Debug("Freeing memory of %s/%s %d %d",
			//			  i.first.first.c_str(), i.first.second.c_str(), i.second.last_access, cur_ticks);

			i.second.bitmap.reset();
		}
	}

	BitmapRef LoadBitmap(std::string const& folder_name, const std::string& filename,
						 bool transparent, uint32_t const flags) {
		string_pair const key(folder_name, filename);

		cache_type::iterator const it = cache.find(key);

		if (it == cache.end() || !it->second.bitmap) {
			std::string const path = FileFinder::FindImage(folder_name, filename);

			BitmapRef bmp = BitmapRef();

			if (path.empty()) {
				Output::Warning("Image not found: %s/%s", folder_name.c_str(), filename.c_str());
			} else {
				bmp = Bitmap::Create(path, transparent, flags);
				if (!bmp) {
					Output::Warning("Invalid image: %s/%s", folder_name.c_str(), filename.c_str());
				}
			}

			FreeBitmapMemory();

			return (cache[key] = {bmp, DisplayUi->GetTicks()}).bitmap;
		} else {
			it->second.last_access = DisplayUi->GetTicks();
			return it->second.bitmap;
		}
	}

	struct Material {
		enum Type {
			REND = -1,
			Backdrop,
			Battle,
			Charset,
			Chipset,
			Faceset,
			Gameover,
			Monster,
			Panorama,
			Picture,
			System,
			Title,
			System2,
			Battle2,
			Battlecharset,
			Battleweapon,
			Frame,
			END
		};

	}; // struct Material

	template<Material::Type T> BitmapRef DrawCheckerboard();

	BitmapRef DummySystem() {
		return Bitmap::Create(system_h, sizeof(system_h), true, Bitmap::Flag_System | Bitmap::Flag_ReadOnly);
	}

	std::function<BitmapRef()> backdrop_dummy_func = DrawCheckerboard<Material::Backdrop>;
	std::function<BitmapRef()> battle_dummy_func = DrawCheckerboard<Material::Battle>;
	std::function<BitmapRef()> charset_dummy_func = DrawCheckerboard<Material::Charset>;
	std::function<BitmapRef()> chipset_dummy_func = DrawCheckerboard<Material::Chipset>;
	std::function<BitmapRef()> faceset_dummy_func = DrawCheckerboard<Material::Faceset>;
	std::function<BitmapRef()> gameover_dummy_func = DrawCheckerboard<Material::Gameover>;
	std::function<BitmapRef()> monster_dummy_func = DrawCheckerboard<Material::Monster>;
	std::function<BitmapRef()> panorama_dummy_func = DrawCheckerboard<Material::Panorama>;
	std::function<BitmapRef()> picture_dummy_func = DrawCheckerboard<Material::Picture>;
	std::function<BitmapRef()> title_dummy_func = DrawCheckerboard<Material::Title>;
	std::function<BitmapRef()> system2_dummy_func = DrawCheckerboard<Material::System2>;
	std::function<BitmapRef()> battle2_dummy_func = DrawCheckerboard<Material::Battle2>;
	std::function<BitmapRef()> battlecharset_dummy_func = DrawCheckerboard<Material::Battlecharset>;
	std::function<BitmapRef()> battleweapon_dummy_func = DrawCheckerboard<Material::Battleweapon>;
	std::function<BitmapRef()> frame_dummy_func = DrawCheckerboard<Material::Frame>;

	struct Spec {
		char const* directory;
		bool transparent;
		int min_width , max_width;
		int min_height, max_height;
		std::function<BitmapRef()> dummy_renderer;
	} const spec[] = {
		{ "Backdrop", false, 320, 320, 160, 160, backdrop_dummy_func},
		{ "Battle", true, 480, 480, 96, 480, battle_dummy_func },
		{ "CharSet", true, 288, 288, 256, 256, charset_dummy_func },
		{ "ChipSet", true, 480, 480, 256, 256, chipset_dummy_func },
		{ "FaceSet", true, 192, 192, 192, 192, faceset_dummy_func },
		{ "GameOver", false, 320, 320, 240, 240, gameover_dummy_func },
		{ "Monster", true, 16, 320, 16, 160, monster_dummy_func },
		{ "Panorama", false, 80, 640, 80, 480, panorama_dummy_func },
		{ "Picture", true, 1, 640, 1, 480, picture_dummy_func },
		{ "System", true, 160, 160, 80, 80, &DummySystem },
		{ "Title", false, 320, 320, 240, 240, title_dummy_func },
		{ "System2", true, 80, 80, 96, 96, system2_dummy_func },
		{ "Battle2", true, 640, 640, 640, 640, battle2_dummy_func },
		{ "BattleCharSet", true, 144, 144, 384, 384, battlecharset_dummy_func },
		{ "BattleWeapon", true, 192, 192, 512, 512, battleweapon_dummy_func },
		{ "Frame", true, 320, 320, 240, 240, frame_dummy_func },
	};

	template<Material::Type T>
	BitmapRef DrawCheckerboard() {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		Spec const& s = spec[T];

		BitmapRef bitmap = Bitmap::Create(s.max_width, s.max_height, false);

		// ToDo: Maybe use different renderers depending on material
		// Will look ugly for some image types

		// Draw chess board
		Color color[2] = { Color(255, 255, 255, 255), Color(128, 128, 128, 255) };
		for (int i = 0; i < s.max_width / 16; ++i) {
			for (int j = 0; j < s.max_height / 16; ++j) {
				bitmap->FillRect(Rect(i * 16, j * 16, 16, 16), color[(i + j) % 2]);
			}
		}

		return bitmap;
	}

	template<Material::Type T>
	BitmapRef LoadDummyBitmap(std::string const& folder_name, const std::string& filename) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		Spec const& s = spec[T];

		string_pair const key(folder_name, filename);

		BitmapRef bitmap = s.dummy_renderer();

		return (cache[key] = {bitmap, DisplayUi->GetTicks()}).bitmap;
	}

	template<Material::Type T>
	BitmapRef LoadBitmap(std::string const& f, bool transparent) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		Spec const& s = spec[T];

		if (f == CACHE_DEFAULT_BITMAP) {
			return LoadDummyBitmap<T>(s.directory, f);
		}

		// Test if the file was requested asynchronously before.
		// If not the file can't be expected to exist -> bug.
		FileRequestAsync* request = AsyncHandler::RequestFile(s.directory, f);
		if (!request->IsReady()) {
			Output::Debug("BUG: File Not Requested: %s/%s", s.directory, f.c_str());
			return BitmapRef();
		}

		BitmapRef ret = LoadBitmap(s.directory, f, transparent, Bitmap::Flag_ReadOnly | (
										 T == Material::Chipset? Bitmap::Flag_Chipset:
										 T == Material::System? Bitmap::Flag_System:
										 0));

		if (!ret) {
			Output::Warning("Image not found: %s/%s", s.directory, f.c_str());

			return LoadDummyBitmap<T>(s.directory, f);
		}

		if(ret->GetWidth() < s.min_width   || s.max_width  < ret->GetWidth() ||
		   ret->GetHeight() < s.min_height || s.max_height < ret->GetHeight()) {
			Output::Debug("Image size out of bounds: %s/%s (%dx%d < %dx%d < %dx%d)",
						  s.directory, f.c_str(), s.min_width, s.min_height,
						  ret->GetWidth(), ret->GetHeight(), s.max_width, s.max_height);
		}

		return ret;
	}
}

#define cache(elem) \
	BitmapRef Cache::elem(const std::string& f) { \
		bool trans = spec[Material::elem].transparent; \
		return LoadBitmap<Material::elem>(f, trans); \
	}
	cache(Backdrop)
	cache(Battle)
	cache(Battle2)
	cache(Battlecharset)
	cache(Battleweapon)
	cache(Charset)
	cache(Chipset)
	cache(Faceset)
	cache(Gameover)
	cache(Monster)
	cache(Panorama)
	cache(System2)
	cache(Title)
	cache(System)
#undef cache

BitmapRef Cache::Frame(const std::string& f, bool trans) {
	return LoadBitmap<Material::Frame>(f, trans);
}

BitmapRef Cache::Picture(const std::string& f, bool trans) {
	return LoadBitmap<Material::Picture>(f, trans);
}

BitmapRef Cache::Exfont() {
	string_pair const hash("ExFont","ExFont");

	cache_type::iterator const it = cache.find(hash);

	if (it == cache.end() || !it->second.bitmap) {
		return(cache[hash] = {Bitmap::Create(exfont_h, sizeof(exfont_h), true), DisplayUi->GetTicks()}).bitmap;
	} else {
		it->second.last_access = DisplayUi->GetTicks();
		return it->second.bitmap;
	}
}

BitmapRef Cache::Tile(const std::string& filename, int tile_id) {
	tile_pair const key(filename, tile_id);
	cache_tiles_type::const_iterator const it = cache_tiles.find(key);

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
		} else { // Invalid -> Use empty tile (first one)
			rect.x = 288;
			rect.y = 128;
		}

		rect.x += sub_tile_id % 6 * 16;
		rect.y += sub_tile_id / 6 * 16;

		return(cache_tiles[key] = Bitmap::Create(*chipset, rect)).lock();
	} else { return it->second.lock(); }
}

void Cache::Clear() {
	cache.clear();

	for (cache_tiles_type::const_iterator i = cache_tiles.begin(); i != cache_tiles.end(); ++i) {
		if (i->second.expired()) { continue; }
		Output::Debug("possible leak in cached tilemap %s/%d",
					  i->first.first.c_str(), i->first.second);
	}

	cache_tiles.clear();
}

void Cache::SetSystemName(std::string const& filename) {
	system_name = filename;
}

BitmapRef Cache::System() {
	if (!system_name.empty()) {
		return Cache::System(system_name);
	} else {
		if (!Data::system.system_name.empty()) {
			// Load the system file for the shadow and text color
			return Cache::System(Data::system.system_name);
		} else {
			return Bitmap::Create(160, 80, false);
		}
	}
}
