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
#ifdef _MSC_VER
#  pragma warning(disable: 4003)
#endif

#include <map>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/static_assert.hpp>

#include "cache.h"
#include "filefinder.h"
#include "exfont.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
namespace {

	typedef std::pair<std::string,std::string> string_pair;
	typedef std::pair<std::string, int> tile_pair;

	std::map<string_pair, EASYRPG_WEAK_PTR<Bitmap> > cache;
	std::map<tile_pair, EASYRPG_WEAK_PTR<Bitmap> > cache_tiles;

	BitmapRef LoadBitmap(std::string const& folder_name, const std::string& filename,
						 bool transparent, uint32_t const flags) {
		string_pair const key(folder_name, filename);

		std::map<string_pair, EASYRPG_WEAK_PTR<Bitmap> >::iterator const it = cache.find(key);

		if (it == cache.end() || it->second.expired()) {
			std::string const path = FileFinder::FindImage(folder_name, filename);
			return (cache[key] = path.empty()
					? Bitmap::Create(16, 16, Color())
					: Bitmap::Create(path, transparent, flags)
					).lock();
		} else { return it->second.lock(); }
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
			BattleChar,
			BattleWeapon,
			Frame,
			END,
		};

	}; // struct Material

	struct Spec {
		char const* directory;
		bool transparent;
		int min_width , max_width ;
		int min_height, max_height;
	} const spec[] = {
		{ "Backdrop", false, 320, 320, 160, 160 },
		{ "Battle", true, 480, 480, 96, 480 },
		{ "CharSet", true, 288, 288, 256, 256 },
		{ "ChipSet", true, 480, 480, 256, 256 },
		{ "FaceSet", true, 192, 192, 192, 192 },
		{ "GameOver", false, 320, 320, 240, 240 },
		{ "Monster", false, 16, 320, 16, 160 },
		{ "Panorama", false, 80, 640, 80, 480 },
		{ "Picture", true, 1, 640, 1, 480 },
		{ "System", true, 160, 160, 80, 80 },
		{ "Title", false, 320, 320, 240, 240 },
		{ "System2", true, 80, 80, 96, 96 },
		{ "Battle2", true, 640, 640, 640, 640 },
		{ "BattleChar", true, 144, 144, 384, 384 },
		{ "BattleWeapon", true, 192, 192, 512, 512 },
		{ "frame", true, 320, 320, 240, 240 },
	};

	template<Material::Type T>
	BitmapRef LoadBitmap(std::string const& f) {
		BOOST_STATIC_ASSERT(Material::REND < T && T < Material::END);

		Spec const& s = spec[T];
		BitmapRef const ret = LoadBitmap(s.directory, f, s.transparent,
										 T == Material::Chipset? Bitmap::Chipset:
										 T == Material::System? Bitmap::System:
										 0);

		assert(s.min_width  <= ret->GetWidth () && ret->GetWidth ()<= s.max_width );
		assert(s.min_height <= ret->GetHeight() && ret->GetHeight()<= s.max_height);

		return ret;
	}

}

tSystemInfo Cache::system_info;

#define macro(r, data, elem)						\
	BitmapRef Cache::elem(const std::string& f) {	\
		return LoadBitmap<Material::elem>(f);		\
	}												\

BOOST_PP_SEQ_FOR_EACH(macro, ,
					  (Backdrop)(Battle)(Battle2)(BattleChar)(BattleWeapon)
					  (Charset)(Chipset)(Faceset)(Gameover)(Monster)
					  (Panorama)(Picture)(System)(System2)(Frame)(Title)
					  )

#undef macro

BitmapRef Cache::ExFont() {
	string_pair const hash("\x00","ExFont");

	std::map<string_pair, EASYRPG_WEAK_PTR<Bitmap> >::iterator const it = cache.find(hash);

	if (it == cache.end() || it->second.expired()) {
		return(cache[hash] = Bitmap::Create(exfont_h, sizeof(exfont_h), true)).lock();
	} else { return it->second.lock(); }
}

////////////////////////////////////////////////////////////
BitmapRef Cache::Tile(const std::string& filename, int tile_id) {
	tile_pair const key(filename, tile_id);
	std::map<tile_pair, EASYRPG_WEAK_PTR<Bitmap> >::iterator const it = cache_tiles.find(key);

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
