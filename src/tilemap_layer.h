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

#ifndef _TILEMAP_LAYER_H_
#define _TILEMAP_LAYER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <map>
#include "drawable.h"
#include "bitmap.h"
#include "bitmap_screen.h"

////////////////////////////////////////////////////////////
/// TilemapLayer class.
////////////////////////////////////////////////////////////
class TilemapLayer : public Drawable {
public:
	TilemapLayer(int ilayer);
	~TilemapLayer();

	void Draw(int z_order);

	void Update();

	Bitmap* GetChipset() const;
	void SetChipset(Bitmap* nchipset);
	std::vector<short> GetMapData() const;
	void SetMapData(std::vector<short> nmap_data);
	std::vector<unsigned char> GetPassable() const;
	void SetPassable(std::vector<unsigned char> npassable);
	bool GetVisible() const;
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

	int GetZ() const;
	unsigned long GetId() const;
	DrawableType GetType() const;
	void Substitute(int old_id, int new_id);

private:
	Bitmap* chipset;
	BitmapScreen* chipset_screen;
	std::vector<short> map_data;
	std::vector<unsigned char> passable;
	bool visible;
	int ox;
	int oy;
	int width;
	int height;
	char animation_frame;
	char animation_step_ab;
	char animation_step_c;
	int animation_speed;
	int animation_type;

	unsigned long ID;
	DrawableType type;
	int layer;

	bool have_invisible_tile;

	void GenerateAutotileAB(short ID, short animID);
	void GenerateAutotileD(short ID);

	static const int TILES_PER_ROW = 64;

	struct TileXY {
		uint8 x;
		uint8 y;
		bool valid;
		TileXY() : valid(false) {}
		TileXY(int x, int y) : x(x), y(y), valid(true) {}
	};

	BitmapScreen* GenerateAutotiles(int count, const std::map<uint32, TileXY>& map);

	TileXY GetCachedAutotileAB(short ID, short animID);
	TileXY GetCachedAutotileD(short ID);

	BitmapScreen* autotiles_ab_screen;
	BitmapScreen* autotiles_d_screen;

	int autotiles_ab_next;
	int autotiles_d_next;

	TileXY autotiles_ab[3][3][16][47];
	TileXY autotiles_d[12][50];

	std::map<uint32, TileXY> autotiles_ab_map;
	std::map<uint32, TileXY> autotiles_d_map;

	struct TileData {
		short ID;
		int z;
	};
	std::vector<std::vector<TileData> > data_cache;
	uint8 substitutions[144];
};

#endif
