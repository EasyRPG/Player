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
#include <chrono>
#include <cassert>

#include "async_handler.h"
#include "cache.h"
#include "filefinder.h"
#include "exfont.h"
#include "default_graphics.h"
#include "bitmap.h"
#include "output.h"
#include "player.h"
#include <lcf/data.h>
#include "game_clock.h"

using namespace std::chrono_literals;

namespace {
	std::string MakeHashKey(const std::string& folder_name, const std::string& filename, bool transparent) {
		return folder_name + ":" + filename + ":" + (transparent ? "T" : " ");
	}

	std::string MakeTileHashKey(const std::string& chipset_name, int id) {
		std::string key;
		key.reserve(chipset_name.size() + sizeof(int) + 2);
		key.append(reinterpret_cast<char*>(&id), sizeof(id));
		key.append(1, ':');
		key.append(chipset_name);

		return key;
	}

	int IdFromTileHash(const std::string& key) {
		int id = 0;
		if (key.size() > sizeof(id)) {
			std::memcpy(&id, key.data(), sizeof(id));
		}
		return id;
	}

	const char* NameFromTileHash(const std::string& key) {
		int offset = sizeof(int) + 1;
		if (static_cast<int>(key.size()) < offset) {
			return "";
		}
		return key.data() + offset;
	}

	struct CacheItem {
		BitmapRef bitmap;
		Game_Clock::time_point last_access;
	};

	using key_type = std::string;
	std::unordered_map<key_type, CacheItem> cache;

	using tile_key_type = std::string;
	std::unordered_map<tile_key_type, std::weak_ptr<Bitmap>> cache_tiles;

	// rect, flip_x, flip_y, tone, blend
	using effect_key_type = std::tuple<BitmapRef, Rect, bool, bool, Tone, Color>;
	std::map<effect_key_type, std::weak_ptr<Bitmap>> cache_effects;

	std::string system_name;

	std::string system2_name;

	constexpr int cache_limit = 10 * 1024 * 1024;
	size_t cache_size = 0;

	void FreeBitmapMemory() {
		auto cur_ticks = Game_Clock::GetFrameTime();

		for (auto it = cache.begin(); it != cache.end();) {
			if (it->second.bitmap.use_count() != 1) {
				// Bitmap is referenced
				++it;
				continue;
			}

			if (cache_size <= cache_limit && cur_ticks - it->second.last_access < 3s) {
				// Below memory limit and last access < 3s
				++it;
				continue;
			}

#ifdef CACHE_DEBUG
			Output::Debug("Freeing memory of {}/{}", std::get<0>(i.first), std::get<1>(i.first));
#endif

			cache_size -= it->second.bitmap->GetSize();

			it = cache.erase(it);
		}

#ifdef CACHE_DEBUG
		Output::Debug("Bitmap cache size: {}", cache_size / 1024.0 / 1024);
#endif
	}

	BitmapRef AddToCache(const std::string& key, BitmapRef bmp) {
		if (bmp) {
			cache_size += bmp->GetSize();
#ifdef CACHE_DEBUG
			Output::Debug("Bitmap cache size (Add): {}", cache_size / 1024.0 / 1024.0);
#endif
		}

		return (cache[key] = {bmp, Game_Clock::GetFrameTime()}).bitmap;
	}

	BitmapRef LoadBitmap(const std::string& folder_name, const std::string& filename,
						 bool transparent, const uint32_t flags) {
		const auto key = MakeHashKey(folder_name, filename, transparent);

		auto it = cache.find(key);

		if (it == cache.end()) {
			const std::string path = FileFinder::FindImage(folder_name, filename);

			BitmapRef bmp = BitmapRef();

			FreeBitmapMemory();

			if (path.empty()) {
				Output::Warning("Image not found: {}/{}", folder_name, filename);
			} else {
				bmp = Bitmap::Create(path, transparent, flags);
				if (!bmp) {
					Output::Warning("Invalid image: {}/{}", folder_name, filename);
				}
			}

			if (bmp) {
				return AddToCache(key, bmp);
			}
			return nullptr;
		} else {
			it->second.last_access = Game_Clock::GetFrameTime();
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

	using DummyRenderer = BitmapRef(*)(void);

	template<Material::Type T> BitmapRef DrawCheckerboard();

	BitmapRef DummySystem() {
		return Bitmap::Create(system_h, sizeof(system_h), true, Bitmap::Flag_System | Bitmap::Flag_ReadOnly);
	}

	struct Spec {
		char const* directory;
		bool transparent;
		int min_width , max_width;
		int min_height, max_height;
		DummyRenderer dummy_renderer;
		bool oob_check;
	};
	constexpr Spec spec[] = {
		{ "Backdrop", false, 320, 320, 160, 240, DrawCheckerboard<Material::Backdrop>, true },
		{ "Battle", true, 480, 480, 96, 480, DrawCheckerboard<Material::Battle>, true },
		{ "CharSet", true, 288, 288, 256, 256, DrawCheckerboard<Material::Charset>, true },
		{ "ChipSet", true, 480, 480, 256, 256, DrawCheckerboard<Material::Chipset>, true },
		{ "FaceSet", true, 192, 192, 192, 192, DrawCheckerboard<Material::Faceset>, true},
		{ "GameOver", false, 320, 320, 240, 240, DrawCheckerboard<Material::Gameover>, true },
		{ "Monster", true, 16, 320, 16, 160, DrawCheckerboard<Material::Monster>, false },
		{ "Panorama", false, 80, 640, 80, 480, DrawCheckerboard<Material::Panorama>, false },
		{ "Picture", true, 1, 640, 1, 480, DrawCheckerboard<Material::Picture>, false },
		{ "System", true, 160, 160, 80, 80, DummySystem, true },
		{ "Title", false, 320, 320, 240, 240, DrawCheckerboard<Material::Title>, true },
		{ "System2", true, 80, 80, 96, 96, DrawCheckerboard<Material::System2>, true },
		{ "Battle2", true, 640, 640, 640, 640, DrawCheckerboard<Material::Battle2>, true },
		{ "BattleCharSet", true, 144, 144, 384, 384, DrawCheckerboard<Material::Battlecharset>, true},
		{ "BattleWeapon", true, 192, 192, 512, 512, DrawCheckerboard<Material::Battleweapon>, true },
		{ "Frame", true, 320, 320, 240, 240, DrawCheckerboard<Material::Frame>, true },
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
	BitmapRef LoadDummyBitmap(const std::string& folder_name, const std::string& filename, bool transparent) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

		const auto key = MakeHashKey(folder_name, filename, transparent);

		auto it = cache.find(key);

		if (it == cache.end()) {
			FreeBitmapMemory();

			BitmapRef bitmap = s.dummy_renderer();

			return AddToCache(key, bitmap);
		} else {
			it->second.last_access = Game_Clock::GetFrameTime();
			return it->second.bitmap;
		}
	}

	template<Material::Type T>
	BitmapRef LoadBitmap(const std::string& f, bool transparent) {
		static_assert(Material::REND < T && T < Material::END, "Invalid material.");

		const Spec& s = spec[T];

		if (f == CACHE_DEFAULT_BITMAP) {
			return LoadDummyBitmap<T>(s.directory, f, true);
		}

#ifndef NDEBUG
		// Test if the file was requested asynchronously before.
		// If not the file can't be expected to exist -> bug.
		// This test is expensive and turned off in release builds.
		auto* req = AsyncHandler::RequestFile(s.directory, f);
		assert(req != nullptr && req->IsReady());
#endif

		BitmapRef ret = LoadBitmap(s.directory, f, transparent, Bitmap::Flag_ReadOnly | (
										 T == Material::Chipset? Bitmap::Flag_Chipset:
										 T == Material::System? Bitmap::Flag_System:
										 0));

		if (!ret) {
			return LoadDummyBitmap<T>(s.directory, f, transparent);
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
				Output::Debug("Image size out of bounds: {}/{} ({}x{} < {}x{} < {}x{})",
				              s.directory, f, min_w, min_h, w, h, max_w, max_h);
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
	const auto key = MakeHashKey("ExFont", "ExFont", false);

	auto it = cache.find(key);

	if (it == cache.end()) {
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

		return AddToCache(key, exfont_img);
	} else {
		it->second.last_access = Game_Clock::GetFrameTime();
		return it->second.bitmap;
	}
}

BitmapRef Cache::Tile(const std::string& filename, int tile_id) {
	const auto key = MakeTileHashKey(filename, tile_id);
	auto it = cache_tiles.find(key);

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
			bitmap_effects->ToneBlit(0, 0, *src_bitmap, rect, tone, Opacity::Opaque());
		}

		if (blend != Color()) {
			if (bitmap_effects) {
				// Tone blit was applied
				bitmap_effects->BlendBlit(0, 0, *bitmap_effects, bitmap_effects->GetRect(), blend, Opacity::Opaque());
			} else {
				bitmap_effects = create();
				bitmap_effects->BlendBlit(0, 0, *src_bitmap, rect, blend, Opacity::Opaque());
			}
		}

		if (flip_x || flip_y) {
			if (bitmap_effects) {
				// Tone or blend blit was applied
				bitmap_effects->Flip(flip_x, flip_y);
			} else {
				bitmap_effects = create();
				bitmap_effects->FlipBlit(rect.x, rect.y, *src_bitmap, rect, flip_x, flip_y, Opacity::Opaque());
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

	for (auto& kv : cache_tiles) {
		auto& key = kv.first;
		if (kv.second.expired()) {
			continue;
		}
		Output::Debug("possible leak in cached tilemap {}/{}",
				NameFromTileHash(key), IdFromTileHash(key));
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

BitmapRef Cache::SysBlack() {
	static auto system_black = Bitmap::Create(160, 80, false);
	return system_black;
}

BitmapRef Cache::SystemOrBlack() {
	auto system = Cache::System();
	if (system) {
		return system;
	}
	return SysBlack();
}

BitmapRef Cache::System2() {
	if (!system2_name.empty()) {
		return Cache::System2(system2_name);
	} else {
		return nullptr;
	}
}
