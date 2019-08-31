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
#include <tuple>

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
	using key_type = std::tuple<std::string,std::string,bool>;

	struct CacheItem {
		BitmapRef bitmap;
		uint32_t last_access;
	};

	using tile_pair = std::pair<std::string, int>;

	using cache_type = std::map<key_type, CacheItem>;
	cache_type cache;

	using cache_tiles_type = std::map<tile_pair, std::weak_ptr<Bitmap>>;
	cache_tiles_type cache_tiles;

	// rect, flip_x, flip_y, tone, blend
	using effect_key_type = std::tuple<BitmapRef, Rect, bool, bool, Tone, Color>;
	using cache_effect_type = std::map<effect_key_type, std::weak_ptr<Bitmap>>;
	cache_effect_type cache_effects;

	std::string system_name;
	BitmapRef default_system;

	std::string system2_name;

	constexpr int cache_limit = 10 * 1024 * 1024;
	size_t cache_size = 0;

	void FreeBitmapMemory() {
		int32_t cur_ticks = DisplayUi->GetTicks();

		for (auto& i : cache) {
			if (i.second.bitmap.use_count() != 1) {
				// Bitmap is referenced
				continue;
			}

			if (cache_size <= cache_limit && cur_ticks - i.second.last_access < 3000) {
				// Below memory limit and last access < 3s
				continue;
			}

#ifdef CACHE_DEBUG
			Output::Debug("Freeing memory of %s/%s %d %d",
						  std::get<0>(i.first).c_str(), std::get<1>(i.first).c_str(), i.second.last_access, cur_ticks);
#endif

			cache_size -= i.second.bitmap->GetSize();
			i.second.bitmap.reset();
		}

#ifdef CACHE_DEBUG
		Output::Debug("Bitmap cache size: %f", cache_size / 1024.0 / 1024);
#endif
	}

	BitmapRef AddToCache(const key_type& key, BitmapRef bmp) {
		if (bmp) {
			cache_size += bmp->GetSize();
#ifdef CACHE_DEBUG
			Output::Debug("Bitmap cache size (Add): %f", cache_size / 1024.0 / 1024.0);
#endif
		}

		return (cache[key] = {bmp, DisplayUi->GetTicks()}).bitmap;
	}

	BitmapRef LoadBitmap(const std::string& folder_name, const std::string& filename,
						 bool transparent, const uint32_t flags) {
		const key_type key(folder_name, filename, transparent);

		const cache_type::iterator it = cache.find(key);

		if (it == cache.end() || !it->second.bitmap) {
			const std::string path = FileFinder::FindImage(folder_name, filename);

			BitmapRef bmp = BitmapRef();

			FreeBitmapMemory();

			if (path.empty()) {
				Output::Warning("Image not found: %s/%s", folder_name.c_str(), filename.c_str());
			} else {
				bmp = Bitmap::Create(path, transparent, flags);
				if (!bmp) {
					Output::Warning("Invalid image: %s/%s", folder_name.c_str(), filename.c_str());
				}
			}

			return AddToCache(key, bmp);
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
		bool oob_check;
	} const spec[] = {
		{ "Backdrop", false, 320, 320, 160, 240, backdrop_dummy_func, true },
		{ "Battle", true, 480, 480, 96, 480, battle_dummy_func, true },
		{ "CharSet", true, 288, 288, 256, 256, charset_dummy_func, true },
		{ "ChipSet", true, 480, 480, 256, 256, chipset_dummy_func, true },
		{ "FaceSet", true, 192, 192, 192, 192, faceset_dummy_func, true},
		{ "GameOver", false, 320, 320, 240, 240, gameover_dummy_func, true },
		{ "Monster", true, 16, 320, 16, 160, monster_dummy_func, false },
		{ "Panorama", false, 80, 640, 80, 480, panorama_dummy_func, false },
		{ "Picture", true, 1, 640, 1, 480, picture_dummy_func, false },
		{ "System", true, 160, 160, 80, 80, &DummySystem, true },
		{ "Title", false, 320, 320, 240, 240, title_dummy_func, true },
		{ "System2", true, 80, 80, 96, 96, system2_dummy_func, true },
		{ "Battle2", true, 640, 640, 640, 640, battle2_dummy_func, true },
		{ "BattleCharSet", true, 144, 144, 384, 384, battlecharset_dummy_func, true},
		{ "BattleWeapon", true, 192, 192, 512, 512, battleweapon_dummy_func, true },
		{ "Frame", true, 320, 320, 240, 240, frame_dummy_func, true },
	};

	template<Material::Type T>
	BitmapRef DrawCheckerboard() {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

		FreeBitmapMemory();

		BitmapRef bitmap = Bitmap::Create(s.max_width, s.max_height, false);
		cache_size += bitmap->GetSize();

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
	BitmapRef LoadDummyBitmap(const std::string& folder_name, const std::string& filename) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

		const key_type key(folder_name, filename, false);

		const cache_type::iterator it = cache.find(key);

		if (it == cache.end() || !it->second.bitmap) {
			FreeBitmapMemory();

			BitmapRef bitmap = s.dummy_renderer();

			return AddToCache(key, bitmap);
		} else {
			it->second.last_access = DisplayUi->GetTicks();
			return it->second.bitmap;
		}
	}

	template<Material::Type T>
	BitmapRef LoadBitmap(const std::string& f, bool transparent) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

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

		if (s.oob_check) {
			int w = ret->GetWidth();
			int h = ret->GetHeight();
			int min_h = s.min_height;
			int max_h = s.max_height;
			int min_w = s.min_width;
			int max_w = s.max_width;

			// 240px backdrop height is 2k3 specific, for 2k is 160px
			if (T == Material::Backdrop) {
				max_h = min_h = Player::IsRPG2k() ? 160 : 240;
			}

			if (w < min_w || max_w < w || h < min_h || max_h < h) {
				Output::Debug("Image size out of bounds: %s/%s (%dx%d < %dx%d < %dx%d)",
				              s.directory, f.c_str(), min_w, min_h, w, h, max_w, max_h);
			}
		}

		return ret;
	}

	template<Material::Type T>
	BitmapRef LoadBitmap(const std::string& f) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

		return LoadBitmap<T>(f, s.transparent);
	}
}

std::vector<uint8_t> Cache::exfont_custom;

BitmapRef Cache::Backdrop(const std::string& file) {
	return LoadBitmap<Material::Backdrop>(file);
}

BitmapRef Cache::Battle(const std::string& file) {
	return LoadBitmap<Material::Battle>(file);
}

BitmapRef Cache::Battle2(const std::string& file) {
	return LoadBitmap<Material::Battle2>(file);
}

BitmapRef Cache::Battlecharset(const std::string& file) {
	return LoadBitmap<Material::Battlecharset>(file);
}

BitmapRef Cache::Battleweapon(const std::string& file) {
	return LoadBitmap<Material::Battleweapon>(file);
}

BitmapRef Cache::Charset(const std::string& file) {
	return LoadBitmap<Material::Charset>(file);
}

BitmapRef Cache::Chipset(const std::string& file) {
	return LoadBitmap<Material::Chipset>(file);
}

BitmapRef Cache::Faceset(const std::string& file) {
	return LoadBitmap<Material::Faceset>(file);
}

BitmapRef Cache::Frame(const std::string& file, bool transparent) {
	return LoadBitmap<Material::Frame>(file, transparent);
}

BitmapRef Cache::Gameover(const std::string& file) {
	return LoadBitmap<Material::Gameover>(file);
}

BitmapRef Cache::Monster(const std::string& file) {
	return LoadBitmap<Material::Monster>(file);
}

BitmapRef Cache::Panorama(const std::string& file) {
	return LoadBitmap<Material::Panorama>(file);
}

BitmapRef Cache::Picture(const std::string& file, bool transparent) {
	return LoadBitmap<Material::Picture>(file, transparent);
}

BitmapRef Cache::System2(const std::string& file) {
	return LoadBitmap<Material::System2>(file);
}

BitmapRef Cache::Title(const std::string& file) {
	return LoadBitmap<Material::Title>(file);
}

BitmapRef Cache::System(const std::string& file) {
	return LoadBitmap<Material::System>(file);
}

BitmapRef Cache::Exfont() {
	const key_type hash("ExFont", "ExFont", false);

	cache_type::iterator const it = cache.find(hash);

	if (it == cache.end() || !it->second.bitmap) {
		// Allow overwriting of built-in exfont with a custom ExFont image file
		// exfont_custom is filled by Player::CreateGameObjects
		BitmapRef exfont_img;
		if (!exfont_custom.empty()) {
			exfont_img = Bitmap::Create(exfont_custom.data(), exfont_custom.size(), true);
		}
		// exfont_custom can contain invalid data and fail
		if (!exfont_img) {
			exfont_img = Bitmap::Create(exfont_h, sizeof(exfont_h), true);
		}

		return AddToCache(hash, exfont_img);
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

BitmapRef Cache::SpriteEffect(const BitmapRef& src_bitmap, const Rect& rect, bool flip_x, bool flip_y, const Tone& tone, const Color& blend) {
	const effect_key_type key {
		src_bitmap,
		rect,
		flip_x,
		flip_y,
		tone,
		blend
	};

	const auto it = cache_effects.find(key);

	if (it == cache_effects.end() || it->second.expired()) {
		BitmapRef bitmap_effects;

		auto create = [&rect] () -> BitmapRef {
			return Bitmap::Create(rect.width, rect.height, true);
		};

		if (tone != Tone()) {
			bitmap_effects = create();
			bitmap_effects->ToneBlit(0, 0, *src_bitmap, rect, tone, Opacity::opaque);
		}

		if (blend != Color()) {
			if (bitmap_effects) {
				// Tone blit was applied
				bitmap_effects->BlendBlit(0, 0, *bitmap_effects, bitmap_effects->GetRect(), blend, Opacity::opaque);
			} else {
				bitmap_effects = create();
				bitmap_effects->BlendBlit(0, 0, *src_bitmap, rect, blend, Opacity::opaque);
			}
		}

		if (flip_x || flip_y) {
			if (bitmap_effects) {
				// Tone or blend blit was applied
				bitmap_effects->Flip(bitmap_effects->GetRect(), flip_x, flip_y);
			} else {
				bitmap_effects = create();
				bitmap_effects->FlipBlit(rect.x, rect.y, *src_bitmap, rect, flip_x, flip_y, Opacity::opaque);
			}
		}

		assert(bitmap_effects && "Effect cache used but no effect applied!");

		return(cache_effects[key] = bitmap_effects).lock();
	} else { return it->second.lock(); }
}

void Cache::Clear() {
	cache_effects.clear();
	cache.clear();
	cache_size = 0;

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

void Cache::SetSystem2Name(std::string const& filename) {
	system2_name = filename;
}

BitmapRef Cache::System() {
	if (!system_name.empty()) {
		return Cache::System(system_name);
	} else {
		return nullptr;
	}
}

BitmapRef Cache::SystemOrBlack() {
	auto system = Cache::System();
	if (system) {
		return system;
	}
	if (!default_system) {
		default_system = Bitmap::Create(160, 80, false);
	}
	return default_system;
}

BitmapRef Cache::System2() {
	if (!system2_name.empty()) {
		return Cache::System2(system2_name);
	} else {
		return nullptr;
	}
}
