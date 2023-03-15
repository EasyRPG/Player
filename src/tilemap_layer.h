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

#ifndef EP_TILEMAP_LAYER_H
#define EP_TILEMAP_LAYER_H

// Headers
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "system.h"
#include "drawable.h"
#include "tone.h"
#include "opacity.h"
#include "span.h"

class TilemapLayer;

/**
 * TilemapSubLayer class.
 */
class TilemapSubLayer : public Drawable {
public:
	TilemapSubLayer(TilemapLayer* tilemap, Drawable::Z_t z);

	void Draw(Bitmap& dst) override;

private:
	TilemapLayer* tilemap = nullptr;

	// z value truncated to lower 8 bits for tile cache
	uint8_t internal_z = 0;
};

/**
 * TilemapLayer class.
 */
class TilemapLayer {
public:
	// For performance reasons the tile cache only uses 8-bit z-layer because it only contains 4 values
	// Do not change anything here to Drawable::Z_t
	static constexpr uint8_t TileBelow = 0;
	static constexpr uint8_t TileAbove = 100;

	TilemapLayer(int ilayer);

	void Draw(Bitmap& dst, uint8_t z_order);

	BitmapRef const& GetChipset() const;
	void SetChipset(BitmapRef const& nchipset);
	const std::vector<short>& GetMapData() const;
	void SetMapData(std::vector<short> nmap_data);
	const std::vector<unsigned char>& GetPassable() const;
	void SetPassable(std::vector<unsigned char> npassable);
	bool IsVisible() const;
	void SetVisible(bool nvisible);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);
	int GetWidth() const;
	void SetWidth(int nwidth);
	int GetHeight() const;
	void SetHeight(int nheight);
	int GetAnimationSpeed() const;
	void SetAnimationSpeed(int speed);
	int GetAnimationType() const;
	void SetAnimationType(int type);
	void OnSubstitute();
	/**
	 * Influences how tiles of the tilemap are blitted.
	 * When enabled the opacity information of the tile is ignored and a opaque
	 * tile is assumed (Faster).
	 *
	 * @param fast true: enable fast blit (ignores alpha)
	 */
	void SetFastBlit(bool fast);

	void SetTone(Tone tone);

private:
	BitmapRef chipset;
	BitmapRef chipset_effect;
	std::unordered_set<uint32_t> chipset_tone_tiles;
	std::vector<short> map_data;
	std::vector<uint8_t> passable;
	std::vector<uint8_t> substitutions;
	int ox = 0;
	int oy = 0;
	int width = 0;
	int height = 0;
	int animation_speed = 1;
	int animation_type = 0;
	int layer = 0;
	bool fast_blit = false;

	void CreateTileCache(const std::vector<short>& nmap_data);
	void GenerateAutotileAB(short ID, short animID);
	void GenerateAutotileD(short ID);
	void DrawTile(Bitmap& dst, Bitmap& tile, Bitmap& tone_tile, int x, int y, int row, int col, uint32_t tone_hash, bool allow_fast_blit = true);
	void DrawTileImpl(Bitmap& dst, Bitmap& tile, Bitmap& tone_tile, int x, int y, int row, int col, uint32_t tone_hash, ImageOpacity op, bool allow_fast_blit);

	static const int TILES_PER_ROW = 64;

	struct TileXY {
		uint8_t x;
		uint8_t y;
		bool valid;
		TileXY() : valid(false) {}
		TileXY(uint8_t x, uint8_t y) : x(x), y(y), valid(true) {}
	};

	BitmapRef GenerateAutotiles(int count, const std::unordered_map<uint32_t, TileXY>& map);

	TileXY GetCachedAutotileAB(short ID, short animID);
	TileXY GetCachedAutotileD(short ID);
	BitmapRef autotiles_ab_screen;
	BitmapRef autotiles_ab_screen_effect;
	BitmapRef autotiles_d_screen;
	BitmapRef autotiles_d_screen_effect;

	int autotiles_ab_next = -1;
	int autotiles_d_next = -1;

	TileXY autotiles_ab[3][3][16][47] = {};
	TileXY autotiles_d[12][50] = {};

	std::unordered_map<uint32_t, TileXY> autotiles_ab_map;
	std::unordered_map<uint32_t, TileXY> autotiles_d_map;

	struct TileData {
		short ID;
		uint8_t z;
	};

	TileData& GetDataCache(int x, int y);

	std::vector<TileData> data_cache_vec;

	TilemapSubLayer lower_layer;
	TilemapSubLayer upper_layer;

	Tone tone;
};

inline BitmapRef const& TilemapLayer::GetChipset() const {
	return chipset;
}

inline const std::vector<short>& TilemapLayer::GetMapData() const {
	return map_data;
}

inline const std::vector<unsigned char>& TilemapLayer::GetPassable() const {
	return passable;
}

inline bool TilemapLayer::IsVisible() const {
	// Both layers always the same, so just return 1.
	return lower_layer.IsVisible();
}

inline void TilemapLayer::SetVisible(bool nvisible) {
	lower_layer.SetVisible(nvisible);
	upper_layer.SetVisible(nvisible);
}

inline int TilemapLayer::GetOx() const {
	return ox;
}

inline void TilemapLayer::SetOx(int nox) {
	ox = nox;
}

inline int TilemapLayer::GetOy() const {
	return oy;
}

inline void TilemapLayer::SetOy(int noy) {
	oy = noy;
}

inline int TilemapLayer::GetWidth() const {
	return width;
}

inline void TilemapLayer::SetWidth(int nwidth) {
	width = nwidth;
}

inline int TilemapLayer::GetHeight() const {
	return height;
}

inline void TilemapLayer::SetHeight(int nheight) {
	height = nheight;
}

inline int TilemapLayer::GetAnimationSpeed() const {
	return animation_speed;
}

inline void TilemapLayer::SetAnimationSpeed(int speed) {
	animation_speed = std::max(1, speed);
}

inline int TilemapLayer::GetAnimationType() const {
	return animation_type;
}

inline void TilemapLayer::SetAnimationType(int type) {
	animation_type = type;
}

inline void TilemapLayer::SetFastBlit(bool fast) {
	fast_blit = fast;
}

inline TilemapLayer::TileData& TilemapLayer::GetDataCache(int x, int y) {
	return data_cache_vec[x + y * width];
}


#endif
