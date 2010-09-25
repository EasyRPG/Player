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
// BlockD subtiles IDs
////////////////////////////////////////////////////////////
static const int BLOCK_C = 3000;

static const int BLOCK_D = 4000;
static const int BLOCK_D_BLOCKS = 12;

static const int BLOCK_E = 5000;
static const int BLOCK_E_TILES = 144;

static const int BLOCK_F = 10000;
static const int BLOCK_F_TILES = 144;

////////////////////////////////////////////////////////////
// Blocks subtiles IDs
// Mess with this code and you will die in 3 days...
////////////////////////////////////////////////////////////
static char BlockA_Subtiles_IDS[] = {
	-1,	-1,	-1,	-1,		12,	-1,	-1,	-1,		-1,	13,	-1,	-1,		12,	13,	-1,	-1,
	-1,	-1,	-1,	15,		12,	-1,	-1,	15,		-1,	13,	-1,	15,		12,	13,	-1,	15,
	-1,	-1,	14,	-1,		12,	-1,	14,	-1,		-1,	13,	14,	-1,		12,	13,	14,	-1,
	-1,	-1,	14,	15,		12,	-1,	14,	15,		-1,	13,	14,	15,		12,	13,	14,	15,
	4,	-1,	6,	-1,		4,	13,	6,	-1,		4,	-1,	6,	15,		4,	13,	6,	15,
	8,	9,	-1,	-1,		8,	9,	-1,	15,		8,	9,	14,	-1,		8,	9,	14,	15,
	-1,	5,	-1,	7,		-1,	5,	14,	7,		12,	5,	-1,	7,		12,	5,	14,	7,
	-1,	-1,	10,	11,		12,	-1,	10,	11,		-1,	13,	10,	11,		12,	13,	10,	11,
	4,	5,	6,	7,		8,	9,	10,	11,		0,	9,	6,	-1,		0,	9,	6,	15,
	8,	1,	-1,	7,		8,	1,	14,	7,		-1,	5,	10,	3,		12,	5,	10,	3,
	4,	-1,	2,	11,		4,	13,	2,	11,		0,	1,	6,	7,		0,	9,	2,	11,
	4,	5,	2,	3,		8,	1,	10,	3,		0,	1,	2,	3
};

static char BlockB_Subtiles_IDS[] = {
	0, 	1, 	2, 	3,		4, 	1, 	2, 	3,		0, 	5, 	2, 	3,		4, 	5, 	2, 	3,
	0, 	1, 	6, 	3,		4, 	1, 	6, 	3,		0, 	5, 	6, 	3,		4, 	5, 	6, 	3,
	0, 	1, 	2, 	7,		4, 	1, 	2, 	7,		0, 	5, 	2, 	7,		4, 	5, 	2, 	7,
	0, 	1, 	6, 	7,		4, 	1, 	6, 	7,		0, 	5, 	6, 	7,		4, 	5, 	6, 	7,
	12,	13,	14,	15,		8,	13,	14,	15,		12,	9,	14,	15,		8,	9,	14,	15,
	12,	13,	10,	15,		8,	13,	10,	15,		12,	9,	10,	15,		8,	9,	10,	15,
	12,	13,	14,	11,		8,	13,	14,	11,		12,	9,	14,	11,		8,	9,	14,	11,
	12,	13,	10,	11,		8,	13,	10,	11,		12,	9,	10,	11,		8,	9,	10,	11
};

static char BlockB2_Subtiles_IDS[] = {
	-1,	-1,	-1,	-1,		4,	-1,	-1,	-1,		-1,	5,	-1,	-1,		4,	5,	-1,	-1,
	-1,	-1,	6,	-1,		4,	-1,	6,	-1,		-1,	5,	6,	-1,		4,	5,	6,	-1,
	-1,	-1,	-1,	7,		4,	-1,	-1,	7,		-1,	5,	-1,	7,		4,	5,	-1,	7,
	-1,	-1,	6,	7,		4,	-1,	6,	7,		-1,	5,	6,	7,		4,	5,	6,	7
};

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
	animation_frame = 0;
	animation_step_ab = 0;
	animation_step_c = 0;
	animation_speed = 24;
	animation_type = 1;

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

	// Get the number of tiles that can be displayed on window
	int tiles_x = (int)ceil(Player::GetWidth() / 16.0);
	int tiles_y = (int)ceil(Player::GetHeight() / 16.0);

	// If ox or oy are not equal to the tile size draw the next tile too
	// to prevent black (empty) tiles at the borders
	if (ox % 16 != 0) {
		++tiles_x;
	}
	if (oy % 16 != 0) {
		++tiles_y;
	}

	for (int x = 0; x < tiles_x; x++) {
		for (int y = 0; y < tiles_y; y++) {

			// Get the real maps tile coordinates
			unsigned int map_x = ox / 16 + x;
			unsigned int map_y = oy / 16 + y;
			int map_draw_x = x * 16 - ox % 16;
			int map_draw_y = y * 16 - oy % 16;

			if (data_cache.size() - 1 < map_x || data_cache[map_x].size() - 1 < map_y) continue;

			// Get the tile data
			TileData tile = data_cache[map_x][map_y];
			
			// Draw the tile if its z is being draw now
			if (z_order == tile.z) {
				if (layer == 0) {
					// If lower layer

					if (tile.ID >= BLOCK_E && tile.ID < BLOCK_E + BLOCK_E_TILES) {
						// If Block E

						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						if (tile.ID < BLOCK_E + 96) {
							// If from first column of the block
							rect.x = 192 + ((tile.ID - BLOCK_E) % 6) * 16;
							rect.y = ((tile.ID - BLOCK_E) / 6) * 16;
						} else {
							// If from second column of the block
							rect.x = 288 + ((tile.ID - BLOCK_E - 96) % 6) * 16;
							rect.y = ((tile.ID - BLOCK_E - 96) / 6) * 16;
						}

						// Draw the tile
						chipset->BlitScreen(map_draw_x, map_draw_y, rect);
					} else if (tile.ID >= BLOCK_C && tile.ID < BLOCK_D) {
						// If Block C

						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						rect.x = 48 + ((tile.ID - BLOCK_C) / 50) * 16;
						rect.y = 64 + animation_step_c * 16;

						// Draw the tile
						chipset->BlitScreen(map_draw_x, map_draw_y, rect);
					} else if (tile.ID < BLOCK_C) {
						// If Blocks A1, A2, B

						// Get the autotile id for the current animation
						int autotile_id = tile.ID;
						if (animation_step_ab == 1)
							autotile_id += 20000;
						if (animation_step_ab == 2)
							autotile_id += 30000;

						// Draw the tile from autile cache
						autotiles[autotile_id]->BlitScreen(map_draw_x, map_draw_y);
					} else {
						// If blocks D1-D12

						// Draw the tile from autile cache
						autotiles[tile.ID]->BlitScreen(map_draw_x, map_draw_y);
					}
				} else {
					// If upper layer

					// Check that block F is being drawn
					if (tile.ID >= BLOCK_F && tile.ID < BLOCK_F + BLOCK_F_TILES) {
						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						if (tile.ID < BLOCK_F + 48) {
							// If from first column of the block
							rect.x = 288 + ((tile.ID - BLOCK_F) % 6) * 16;
							rect.y = 128 + ((tile.ID - BLOCK_F) / 6) * 16;
						} else {
							// If from second column of the block
							rect.x = 384 + ((tile.ID - BLOCK_F - 48) % 6) * 16;
							rect.y = ((tile.ID - BLOCK_F - 48) / 6) * 16;
						}
						
						// Draw the tile
						chipset->BlitScreen(map_draw_x, map_draw_y, rect);
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////
// GenerateAutotileAB
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GenerateAutotileAB(short ID, short animID) {
	Bitmap* tile = new Bitmap(16, 16);
	
	// Calculate the block to use
	//	1: A1 + Upper B (Grass + Coast)
	//	2: A2 + Upper B (Snow + Coast)
	//	3: A1 + Lower B (Grass + Ocean/Deep water)
	int block = ID / 1000;

	// Calculate the B block combination
	int b_subtile = (ID - block * 1000) / 50;

	// Calculate the A block combination
	int a_subtile = ID - block * 1000 - b_subtile * 50;

	Rect rect;
	rect.width = 8;
	rect.height = 8;

	int block_x, block_y;

	// Get Block B chipset coords
	block_x = animID * 16;
	block_y = 64;

	// Blit B block subtiles
	//	1 -> Upper right
	//	2 -> Upper left
	//	3 -> Lower right
	//	4 -> Lower left
	for (int i = 0; i < 4; i++) {
		// Skip the subtile if it will be used one from A block instead
		if (BlockA_Subtiles_IDS[a_subtile * 4 + i] != -1) continue;

		// Get the block B subtiles ids and get their coordinates on the chipset
		rect.x = block_x + (BlockB_Subtiles_IDS[(block == 2 ? 64 : 0) + b_subtile * 4 + i] % 2) * 8;
		rect.y = block_y + (BlockB_Subtiles_IDS[(block == 2 ? 64 : 0) + b_subtile * 4 + i] / 2) * 8;

		// Blit the subtile
		tile->Blit((i % 2) * 8, (i / 2) * 8, chipset, rect, 255);
	}

	// Get Block A chipset coords
	block_x = animID * 16 + (block == 1 ? 48 : 0);
	block_y = 0;

	// Blit A block subtiles
	//	1 -> Upper right
	//	2 -> Upper left
	//	3 -> Lower right
	//	4 -> Lower left
	for (int i = 0; i < 4; i++) {
		// Skip the subtile if it was used one from B block
		if (BlockA_Subtiles_IDS[a_subtile * 4 + i] == -1) continue;

		// Get the block A subtiles ids and get their coordinates on the chipset
		rect.x = block_x + (BlockA_Subtiles_IDS[a_subtile * 4 + i] % 2) * 8;
		rect.y = block_y + (BlockA_Subtiles_IDS[a_subtile * 4 + i] / 2) * 8;

		// Blit the subtile
		tile->Blit((i % 2) * 8, (i / 2) * 8, chipset, rect, 255);
	}

	// Get Block B chipset coords
	block_x = animID * 16;
	block_y = 64;

	// Blit B block subtiles when combining A and B
	//	1 -> Upper right
	//	2 -> Upper left
	//	3 -> Lower right
	//	4 -> Lower left
	if (b_subtile != 0 && a_subtile != 0) {
		for (int i = 0; i < 4; i++) {
			// Skip the subtile if it was used one from A or B block
			if (BlockB2_Subtiles_IDS[b_subtile * 4 + i] == -1) continue;

			// Get the block B subtiles ids and get their coordinates on the chipset
			rect.x = block_x + (BlockB2_Subtiles_IDS[b_subtile * 4 + i] % 2) * 8;
			rect.y = block_y + (BlockB2_Subtiles_IDS[b_subtile * 4 + i] / 2) * 8;

			// Blit the subtile
			tile->Blit((i % 2) * 8, (i / 2) * 8, chipset, rect, 255);
		}
	}

	return tile;
}

////////////////////////////////////////////////////////////
// GenerateAutotileD
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GenerateAutotileD(short ID) {
	Bitmap* tile = new Bitmap(16, 16);

	// Calculate the D block id
	int block = (ID - 4000) / 50;

	// Calculate the D block combination
	int subtile = ID - 4000 - block * 50;

	// Get Block chipset coords
	int block_x, block_y;
	if (block < 4) {
		// If from first column
		block_x = (block % 2) * 48;
		block_y = 128 + (block / 2) * 64;
	} else {
		// If from second column
		block_x = 96 + (block % 2) * 48;
		block_y = ((block - 4) / 2) * 64;
	}

	Rect rect;
	rect.width = 8;
	rect.height = 8;

	// Blit D block subtiles
	//	1 -> Upper right
	//	2 -> Upper left
	//	3 -> Lower right
	//	4 -> Lower left
	for (int i = 0; i < 4; i++) {
		// Get the block D subtiles ids and get their coordinates on the chipset
		rect.x = block_x + (BlockD_Subtiles_IDS[subtile * 4 + i] % 6) * 8;
		rect.y = block_y + (BlockD_Subtiles_IDS[subtile * 4 + i] / 6) * 8;

		// Blit the subtile
		tile->Blit((i % 2) * 8, (i / 2) * 8, chipset, rect, 255);
	}

	return tile;
}


////////////////////////////////////////////////////////////
// Update
////////////////////////////////////////////////////////////
void TilemapLayer::Update() {
	animation_frame += 1;

	// Step to the next animation frame
	if (animation_frame == animation_speed) {
		animation_step_ab = 1;
		animation_step_c = 1;
	} else if (animation_frame == animation_speed * 2) {
		animation_step_ab = 2;
		animation_step_c = 2;
	} else if (animation_frame == animation_speed * 3) {
		if (animation_type == 1) {
			// If animation type is 1-2-3-2
			animation_step_ab = 1;
		} else {
			// If animation type is 1-2-3
			animation_step_ab = 0;
			animation_frame = 0;
		}
		animation_step_c = 3;
	} else if (animation_frame >= animation_speed * 4) {
		animation_step_ab = 0;
		animation_step_c = 0;
		animation_frame = 0;
	}
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
		// Create the tiles data cache
		data_cache.resize(width);
		for (int x = 0; x < width; x++) {
			data_cache[x].resize(height);
			for (int y = 0; y < height; y++) {
				TileData tile;

				// Get the tile ID
				tile.ID = nmap_data[x + y * width];

				// Calculate the tile Z
				tile.z = 0;

				// Check if passable data was set
				if (passable.size() > 0) {
					if (tile.ID >= BLOCK_E && tile.ID < BLOCK_E + BLOCK_E_TILES) {
						// If tile is from block E

						// Check if the over property is set
						if (passable[18 + tile.ID - BLOCK_E] & (1 << 4)) tile.z = 16;
					} else if (tile.ID >= BLOCK_F && tile.ID < BLOCK_F + BLOCK_F_TILES) {
						// If tile is from block F

						// Check if the over property is set
						if (passable[18 + tile.ID - BLOCK_F] & (1 << 4)) tile.z = 32;
					}
				}
				data_cache[x][y] = tile;
			}
		}

		// Check if lower layer, and generate the autotiles cache
		if (layer == 0) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {

					if (data_cache[x][y].ID < BLOCK_C) {
						// If blocks A and B

						// Check if autotile cache does not exists
						if (autotiles.count(data_cache[x][y].ID) == 0) {
							// Generate the autotiles for all 3 animation frames
							autotiles[data_cache[x][y].ID] = GenerateAutotileAB(data_cache[x][y].ID, 0);
							autotiles[data_cache[x][y].ID + 20000] = GenerateAutotileAB(data_cache[x][y].ID, 1);
							autotiles[data_cache[x][y].ID + 30000] = GenerateAutotileAB(data_cache[x][y].ID, 2);
						}
					} else if (data_cache[x][y].ID >= BLOCK_D && data_cache[x][y].ID < BLOCK_E) {
						// If block D

						// Check if autotile cache does not exists
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
int TilemapLayer::GetAnimationSpeed() const {
	return animation_speed;
}
void TilemapLayer::SetAnimationSpeed(int speed) {
	animation_speed = speed;
}
int TilemapLayer::GetAnimationType() const {
	return animation_type;
}
void TilemapLayer::SetAnimationType(int type) {
	animation_type = type;
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
