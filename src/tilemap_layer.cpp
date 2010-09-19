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
#include <math.h>
#include "tilemap_layer.h"
#include "player.h"
#include "graphics.h"

////////////////////////////////////////////////////////////
// Blocks subtiles IDs
////////////////////////////////////////////////////////////
static char BlockD_Subtiles_IDS[] = {
	26,	27,	32,	33,		4,	27,	32,	33,		26,	5,	32,	33,		4,	5,	32,	33,
	26,	27,	32,	11,		4,	27,	32,	11,		26,	5,	32,	11,		4,	5,	32,	11,
	26,	27,	10,	33,		4,	27,	10,	33,		26,	5,	10,	33,		4,	5,	10,	33,
	26,	27,	10,	11,		4,	27,	10,	11,		26,	5,	10,	11,		4,	5,	10,	11,
	24,	25,	30,	31,		24,	5,	30,	31,		24,	25,	30,	11,		24,	5,	30,	11,
	14,	15,	20,	21,		14,	15,	20,	11,		14,	15,	10,	21,		14,	15,	10,	11,
	28,	29,	34,	35,		28,	29,	10,	35,		4,	29,	34,	35,		4,	29,	10,	35,
	38,	39,	44,	45,		4,	39,	44,	45,		38,	5,	44,	45,		4,	5,	44,	45,
	24,	29,	30,	35,		14,	15,	44,	45,		12,	13,	18,	19,		12,	13,	18,	11,
	16,	17,	22,	23,		16,	17,	10,	23,		40,	41,	46,	47,		4,	41,	46,	47,
	36,	37,	42,	43,		36,	5,	42,	43,		12,	17,	18,	23,		12,	13,	42,	43,
	36,	41,	42,	47,		16,	17,	46,	47,		12,	17,	42,	47,		26,	27,	32,	33,
	26,	27,	32,	33,		0,	1,	6,	7
};

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////
TilemapLayer::TilemapLayer(int ilayer) {
	chipset = NULL;
	visible = true;
	ox = 0;
	oy = 0;
	width = 0;
	height = 0;

	layer = ilayer;

	ID = Graphics::ID++;
	Graphics::RegisterZObj(0, ID, true);
	Graphics::RegisterZObj(16, ID, true);
	Graphics::RegisterZObj(32, ID, true);
	Graphics::RegisterDrawable(ID, this);
}

////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////
TilemapLayer::~TilemapLayer() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
}

////////////////////////////////////////////////////////////
// Draw
////////////////////////////////////////////////////////////
void TilemapLayer::Draw(int z_order) {
	if (!visible) return;

	int tiles_x = (int)ceil(Player::GetWidth() / 16.0);
	int tiles_y = (int)ceil(Player::GetHeight() / 16.0);

	for (int i = 0; i < tiles_x; i++) {
		for (int j = 0; j < tiles_y; j++) {

			int map_x = ox / 16 + i;
			int map_y = oy / 16 + j;

			TileData tile = data_cache[map_x][map_y];
			
			if (z_order == tile.z) {
				if (layer == 0) {
					if (tile.ID >= 5000 && tile.ID <= 5143) {
						Rect rect;
						rect.x = 192 + ((tile.ID - 5000) % 6) * 16 + ((tile.ID - 5000) / 96) * 16;
						rect.y = (((tile.ID - 5000) / 6) % 16) * 16;
						rect.width = 16;
						rect.height = 16;

						chipset->BlitScreen(map_x * 16, map_y * 16, rect);
					} else if (tile.ID >= 4000 && tile.ID < 5000) {
						autotiles[tile.ID]->BlitScreen(map_x * 16, map_y * 16);
					}
				} else {
					if (tile.ID >= 10000 && tile.ID <= 10143) {
						Rect rect;
						if (tile.ID < 10048) {
							rect.x = 288 + ((tile.ID - 10000) % 6) * 16;
							rect.y = 128 + ((tile.ID - 10000) / 6) * 16;
						} else {
							rect.x = 384 + ((tile.ID - 10048) % 6) * 16;
							rect.y = ((tile.ID - 10048) / 6) * 16;
						}
						rect.width = 16;
						rect.height = 16;

						chipset->BlitScreen(map_x * 16, map_y * 16, rect);
					} else {
						
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////
// GenerateAutotileA
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GenerateAutotileAB(short ID) {
	Bitmap* tile = new Bitmap(16, 16);


	return tile;
}

Bitmap* TilemapLayer::GenerateAutotileC(short ID) {
	Bitmap* tile = new Bitmap(16, 16);


	return tile;
}

Bitmap* TilemapLayer::GenerateAutotileD(short ID) {
	Bitmap* tile = new Bitmap(16, 16);
	int block = (ID - 4000) / 50;
	int subtile = ID - 4000 - block * 50;

	int block_x, block_y;
	
	if (block < 4) {
		block_x = (block % 2) * 48;
		block_y = 128 + (block / 2) * 64;
	} else {
		block_x = 96 + (block % 2) * 48;
		block_y = ((block - 4) / 2) * 64;
	}

	Rect rect;
	rect.width = 8;
	rect.height = 8;
	for (int i = 0; i < 4; i++) {
		rect.x = block_x + (BlockD_Subtiles_IDS[subtile * 4 + i] % 6) * 8;
		rect.y = block_y + (BlockD_Subtiles_IDS[subtile * 4 + i] / 6) * 8;

		tile->Blit((i % 2) * 8, (i / 2) * 8, chipset, rect, 255);
	}

	return tile;
}

////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GetChipset() const {
	return chipset;
}
void TilemapLayer::SetChipset(Bitmap* nchipset) {
	chipset = nchipset;
}
std::vector<short> TilemapLayer::GetMapData() const {
	return map_data;
}
void TilemapLayer::SetMapData(std::vector<short> nmap_data) {
	if (map_data != nmap_data) {
		data_cache.resize(width);
		for (int x = 0; x < width; x++) {
			data_cache[x].resize(height);
			for (int y = 0; y < height; y++) {
				TileData tile;
				tile.ID = nmap_data[x + y * width];
				tile.z = 0;
				if (passable.size() > 0) {
					if (tile.ID >= 5000 && tile.ID <= 5143) {
						if (passable[18 + tile.ID - 5000] & (1 << 4)) {
							tile.z += 16;
							if (layer == 1) tile.z += 16;
						}
					} else if (tile.ID >= 10000 && tile.ID <= 10143) {
						if (passable[18 + tile.ID - 10000] & (1 << 4)) {
							tile.z += 16;
							if (layer == 1) tile.z += 16;
						}
					}
				}
				data_cache[x][y] = tile;
			}
		}
		if (layer == 0) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (data_cache[x][y].ID < 1000) {
						//autotiles[data_cache[x][y].ID] = GenerateAutotileAB(data_cache[x][y].ID);
					} else if (data_cache[x][y].ID < 4000) {
						//autotiles[data_cache[x][y].ID] = GenerateAutotileC(data_cache[x][y].ID);
					} else if (data_cache[x][y].ID < 5000) {
						if (autotiles.count(data_cache[x][y].ID) == 0)
							autotiles[data_cache[x][y].ID] = GenerateAutotileD(data_cache[x][y].ID);
					}
				}
			}
		}
	}
	map_data = nmap_data;
}
std::vector<unsigned char> TilemapLayer::GetPassable() const {
	return passable;
}
void TilemapLayer::SetPassable(std::vector<unsigned char> npassable) {
	passable = npassable;
}
bool TilemapLayer::GetVisible() const {
	return visible;
}
void TilemapLayer::SetVisible(bool nvisible) {
	visible = nvisible;
}
int TilemapLayer::GetOx() const {
	return ox;
}
void TilemapLayer::SetOx(int nox) {
	ox = nox;
}
int TilemapLayer::GetOy() const {
	return oy;
}
void TilemapLayer::SetOy(int noy) {
	oy = noy;
}
int TilemapLayer::GetWidth() const {
	return width;
}
void TilemapLayer::SetWidth(int nwidth) {
	width = nwidth;
}
int TilemapLayer::GetHeight() const {
	return height;
}
void TilemapLayer::SetHeight(int nheight) {
	height = nheight;
}

////////////////////////////////////////////////////////////
// Get z
////////////////////////////////////////////////////////////
int TilemapLayer::GetZ() const {
	return -1;
}

////////////////////////////////////////////////////////////
// Get ID
////////////////////////////////////////////////////////////
unsigned long TilemapLayer::GetId() const {
	return ID;
}
