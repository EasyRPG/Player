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
#include <cstring> // GCC COMPILATION FIX
#include "output.h"

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
// [tile-id][row][col]
static signed char BlockA_Subtiles_IDS[47][2][2] = {
#define N -1
	{{N, N}, {N, N}},
	{{3, N}, {N, N}},
	{{N, 3}, {N, N}},
	{{3, 3}, {N, N}},
	{{N, N}, {N, 3}},
	{{3, N}, {N, 3}},
	{{N, 3}, {N, 3}},
	{{3, 3}, {N, 3}},
	{{N, N}, {3, N}},
	{{3, N}, {3, N}},
	{{N, 3}, {3, N}},
	{{3, 3}, {3, N}},
	{{N, N}, {3, 3}},
	{{3, N}, {3, 3}},
	{{N, 3}, {3, 3}},
	{{3, 3}, {3, 3}},
	{{1, N}, {1, N}},
	{{1, 3}, {1, N}},
	{{1, N}, {1, 3}},
	{{1, 3}, {1, 3}},
	{{2, 2}, {N, N}},
	{{2, 2}, {N, 3}},
	{{2, 2}, {3, N}},
	{{2, 2}, {3, 3}},
	{{N, 1}, {N, 1}},
	{{N, 1}, {3, 1}},
	{{3, 1}, {N, 1}},
	{{3, 1}, {3, 1}},
	{{N, N}, {2, 2}},
	{{3, N}, {2, 2}},
	{{N, 3}, {2, 2}},
	{{3, 3}, {2, 2}},
	{{1, 1}, {1, 1}},
	{{2, 2}, {2, 2}},
	{{0, 2}, {1, N}},
	{{0, 2}, {1, 3}},
	{{2, 0}, {N, 1}},
	{{2, 0}, {3, 1}},
	{{N, 1}, {2, 0}},
	{{3, 1}, {2, 0}},
	{{1, N}, {0, 2}},
	{{1, 3}, {0, 2}},
	{{0, 0}, {1, 1}},
	{{0, 2}, {0, 2}},
	{{1, 1}, {0, 0}},
	{{2, 0}, {2, 0}},
	{{0, 0}, {0, 0}}
#undef N
};

// [tile-id][row][col][x/y]
static signed char BlockD_Subtiles_IDS[50][2][2][2] = {
//     T-L     T-R       B-L     B-R
    {{{1, 2}, {1, 2}}, {{1, 2}, {1, 2}}},
    {{{2, 0}, {1, 2}}, {{1, 2}, {1, 2}}},
    {{{1, 2}, {2, 0}}, {{1, 2}, {1, 2}}},
    {{{2, 0}, {2, 0}}, {{1, 2}, {1, 2}}},
    {{{1, 2}, {1, 2}}, {{1, 2}, {2, 0}}},
    {{{2, 0}, {1, 2}}, {{1, 2}, {2, 0}}},
    {{{1, 2}, {2, 0}}, {{1, 2}, {2, 0}}},
    {{{2, 0}, {2, 0}}, {{1, 2}, {2, 0}}},
    {{{1, 2}, {1, 2}}, {{2, 0}, {1, 2}}},
    {{{2, 0}, {1, 2}}, {{2, 0}, {1, 2}}},
    {{{1, 2}, {2, 0}}, {{2, 0}, {1, 2}}},
    {{{2, 0}, {2, 0}}, {{2, 0}, {1, 2}}},
    {{{1, 2}, {1, 2}}, {{2, 0}, {2, 0}}},
    {{{2, 0}, {1, 2}}, {{2, 0}, {2, 0}}},
    {{{1, 2}, {2, 0}}, {{2, 0}, {2, 0}}},
    {{{2, 0}, {2, 0}}, {{2, 0}, {2, 0}}},
    {{{0, 2}, {0, 2}}, {{0, 2}, {0, 2}}},
    {{{0, 2}, {2, 0}}, {{0, 2}, {0, 2}}},
    {{{0, 2}, {0, 2}}, {{0, 2}, {2, 0}}},
    {{{0, 2}, {2, 0}}, {{0, 2}, {2, 0}}},
    {{{1, 1}, {1, 1}}, {{1, 1}, {1, 1}}},
    {{{1, 1}, {1, 1}}, {{1, 1}, {2, 0}}},
    {{{1, 1}, {1, 1}}, {{2, 0}, {1, 1}}},
    {{{1, 1}, {1, 1}}, {{2, 0}, {2, 0}}},
    {{{2, 2}, {2, 2}}, {{2, 2}, {2, 2}}},
    {{{2, 2}, {2, 2}}, {{2, 0}, {2, 2}}},
    {{{2, 0}, {2, 2}}, {{2, 2}, {2, 2}}},
    {{{2, 0}, {2, 2}}, {{2, 0}, {2, 2}}},
    {{{1, 3}, {1, 3}}, {{1, 3}, {1, 3}}},
    {{{2, 0}, {1, 3}}, {{1, 3}, {1, 3}}},
    {{{1, 3}, {2, 0}}, {{1, 3}, {1, 3}}},
    {{{2, 0}, {2, 0}}, {{1, 3}, {1, 3}}},
    {{{0, 2}, {2, 2}}, {{0, 2}, {2, 2}}},
    {{{1, 1}, {1, 1}}, {{1, 3}, {1, 3}}},
    {{{0, 1}, {0, 1}}, {{0, 1}, {0, 1}}},
    {{{0, 1}, {0, 1}}, {{0, 1}, {2, 0}}},
    {{{2, 1}, {2, 1}}, {{2, 1}, {2, 1}}},
    {{{2, 1}, {2, 1}}, {{2, 0}, {2, 1}}},
    {{{2, 3}, {2, 3}}, {{2, 3}, {2, 3}}},
    {{{2, 0}, {2, 3}}, {{2, 3}, {2, 3}}},
    {{{0, 3}, {0, 3}}, {{0, 3}, {0, 3}}},
    {{{0, 3}, {2, 0}}, {{0, 3}, {0, 3}}},
    {{{0, 1}, {2, 1}}, {{0, 1}, {2, 1}}},
    {{{0, 1}, {0, 1}}, {{0, 3}, {0, 3}}},
    {{{0, 3}, {2, 3}}, {{0, 3}, {2, 3}}},
    {{{2, 1}, {2, 1}}, {{2, 3}, {2, 3}}},
    {{{0, 1}, {2, 1}}, {{0, 3}, {2, 3}}},
    {{{1, 2}, {1, 2}}, {{1, 2}, {1, 2}}},
    {{{1, 2}, {1, 2}}, {{1, 2}, {1, 2}}},
    {{{0, 0}, {0, 0}}, {{0, 0}, {0, 0}}}
};

////////////////////////////////////////////////////////////
TilemapLayer::TilemapLayer(int ilayer) :
	chipset(NULL),
	visible(true),
	ox(0),
	oy(0),
	width(0),
	height(0),
	animation_frame(0),
	animation_step_ab(0),
	animation_step_c(0),
	animation_speed(24),
	animation_type(1),
	ID(Graphics::drawable_id++),
	type(TypeTilemap),
	layer(ilayer),
	have_invisible_tile(false) {

	chipset_screen = BitmapScreen::CreateBitmapScreen(false);

	memset(autotiles_ab, NULL, sizeof(autotiles_ab));
	memset(autotiles_d, NULL, sizeof(autotiles_d));
	
	int tiles_y = (int)ceil(DisplayUi->GetHeight() / 16.0) + 1;
	for (int i = 0; i <= tiles_y; i++) {
		Graphics::RegisterZObj(16 * i, ID, true);
	}
	Graphics::RegisterZObj(9999, ID, true);
	Graphics::RegisterDrawable(ID, this);

	for (uint8 i = 0; i < 144; i++)
		substitutions[i] = i;
}

////////////////////////////////////////////////////////////
TilemapLayer::~TilemapLayer() {
	Graphics::RemoveZObj(ID, true);
	Graphics::RemoveDrawable(ID);

	for (int e = 0; e < 3; e++)
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 50; j++)
				for (int m = 0; m < 50; m++)
					delete autotiles_ab[e][i][j][m];
				
	for (int i = 0; i < 12; i++)
		for (int j = 0; j < 50; j++)
				delete autotiles_d[i][j];

	delete chipset_screen;
}

////////////////////////////////////////////////////////////
void TilemapLayer::Draw(int z_order) {
	if (!visible) return;

	// Get the number of tiles that can be displayed on window
	int tiles_x = (int)ceil(DisplayUi->GetWidth() / 16.0);
	int tiles_y = (int)ceil(DisplayUi->GetHeight() / 16.0);

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
			int map_x = ox / 16 + x;
			int map_y = oy / 16 + y;

			if (width <= map_x || height <= map_y) continue;

			int map_draw_x = x * 16 - ox % 16;
			int map_draw_y = y * 16 - oy % 16;

			// Get the tile data
			TileData &tile = data_cache[map_x][map_y];

			int map_draw_z = tile.z;

			if (map_draw_z > 0) {
				if (map_draw_z < 9999) {
					map_draw_z += y * 16;
					if (y == 0) map_draw_z += 16;
				}
			}
			
			// Draw the tile if its z is being draw now
			if (z_order == map_draw_z) {
				if (layer == 0) {
					// If lower layer

					if (tile.ID >= BLOCK_E && tile.ID < BLOCK_E + BLOCK_E_TILES) {
						int id = substitutions[tile.ID - BLOCK_E];
						// If Block E

						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						if (id < 96) {
							// If from first column of the block
							rect.x = 192 + (id % 6) * 16;
							rect.y = (id / 6) * 16;
						} else {
							// If from second column of the block
							rect.x = 288 + ((id - 96) % 6) * 16;
							rect.y = ((id - 96) / 6) * 16;
						}

						// Draw the tile
						chipset_screen->BlitScreen(map_draw_x, map_draw_y, rect);
					} else if (tile.ID >= BLOCK_C && tile.ID < BLOCK_D) {
						// If Block C

						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						rect.x = 48 + ((tile.ID - BLOCK_C) / 50) * 16;
						rect.y = 64 + animation_step_c * 16;

						// Draw the tile
						chipset_screen->BlitScreen(map_draw_x, map_draw_y, rect);
					} else if (tile.ID < BLOCK_C) {
						// If Blocks A1, A2, B

						// Draw the tile from autile cache
						GetCachedAutotileAB(tile.ID, animation_step_ab)->BlitScreen(map_draw_x, map_draw_y);
					} else {
						// If blocks D1-D12

						// Draw the tile from autile cache
						GetCachedAutotileD(tile.ID)->BlitScreen(map_draw_x, map_draw_y);
					}
				} else {
					// If upper layer

					// Check that block F is being drawn
					if (tile.ID >= BLOCK_F && tile.ID < BLOCK_F + BLOCK_F_TILES) {
						if (tile.ID == BLOCK_F && have_invisible_tile)
							continue;

						int id = substitutions[tile.ID - BLOCK_F];
						Rect rect;
						rect.width = 16;
						rect.height = 16;

						// Get the tile coordinates from chipset
						if (id < 48) {
							// If from first column of the block
							rect.x = 288 + (id % 6) * 16;
							rect.y = 128 + (id / 6) * 16;
						} else {
							// If from second column of the block
							rect.x = 384 + ((id - 48) % 6) * 16;
							rect.y = ((id - 48) / 6) * 16;
						}
						
						// Draw the tile
						chipset_screen->BlitScreen(map_draw_x, map_draw_y, rect);
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////
BitmapScreen* TilemapLayer::GetCachedAutotileAB(short ID, short animID) {
	short block = ID / 1000;
	short b_subtile = (ID - block * 1000) / 50;
	short a_subtile = ID - block * 1000 - b_subtile * 50;
	return autotiles_ab[animID][block][b_subtile][a_subtile];
}

BitmapScreen* TilemapLayer::GetCachedAutotileD(short ID) {
	short block = (ID - 4000) / 50;
	short subtile = ID - 4000 - block * 50;
	return autotiles_d[block][subtile];
}

////////////////////////////////////////////////////////////
void TilemapLayer::GenerateAutotileAB(short ID, short animID) {
	// Calculate the block to use
	//	1: A1 + Upper B (Grass + Coast)
	//	2: A2 + Upper B (Snow + Coast)
	//	3: A1 + Lower B (Grass + Ocean/Deep water)
	short block = ID / 1000;

	// Calculate the B block combination
	short b_subtile = (ID - block * 1000) / 50;

	// Calculate the A block combination
	short a_subtile = ID - block * 1000 - b_subtile * 50;

	if (autotiles_ab[animID][block][b_subtile][a_subtile])
		return;

	Bitmap* tile = Bitmap::CreateBitmap(16, 16);

	Rect rect;
	rect.width = 8;
	rect.height = 8;

	short block_x, block_y;

	// Get Block B chipset coords
	block_x = animID * 16;
	block_y = 64;

	// Blit B block subtiles
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Skip the subtile if it will be used one from A block instead
			if (BlockA_Subtiles_IDS[a_subtile][j][i] != -1) continue;

			// Get the block B subtiles ids and get their coordinates on the chipset
			int t = (b_subtile >> (j * 2 + i)) & 1;
			if (block == 2) t ^= 3;
			rect.x = block_x + i * 8;
			rect.y = block_y + (t * 2 + j) * 8;

			// Blit the subtile
			tile->Blit(i * 8, j * 8, chipset, rect, 255);
		}
	}

	// Get Block A chipset coords
	block_x = animID * 16 + (block == 1 ? 48 : 0);
	block_y = 0;

	// Blit A block subtiles
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Skip the subtile if it was used one from B block
			if (BlockA_Subtiles_IDS[a_subtile][j][i] == -1) continue;

			// Get the block A subtiles ids and get their coordinates on the chipset
			rect.x = block_x + i * 8;
			rect.y = block_y + (BlockA_Subtiles_IDS[a_subtile][j][i] * 2 + j) * 8;

			// Blit the subtile
			tile->Blit(i * 8, j * 8, chipset, rect, 255);
		}
	}

	// Get Block B chipset coords
	block_x = animID * 16;
	block_y = 64;

	// Blit B block subtiles when combining A and B
	if (b_subtile != 0 && a_subtile != 0) {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				// calculate tile (row 0..3)
				int t = (b_subtile >> (j * 2 + i)) & 1;
				if (block == 2) t *= 2;

				// Skip the subtile if not used
				if (t == 0) continue;

				// Get the coordinates on the chipset
				rect.x = block_x + i * 8;
				rect.y = block_y + j * 8 + t * 16;

				// Blit the subtile
				tile->Blit(i * 8, j * 8, chipset, rect, 255);
			}
		}
	}

	autotiles_ab[animID][block][b_subtile][a_subtile] = BitmapScreen::CreateBitmapScreen(tile);
}

////////////////////////////////////////////////////////////
void TilemapLayer::GenerateAutotileD(short ID) {
	// Calculate the D block id
	short block = (ID - 4000) / 50;

	// Calculate the D block combination
	short subtile = ID - 4000 - block * 50;

	if(autotiles_d[block][subtile])
		return;

	Bitmap* tile = Bitmap::CreateBitmap(16, 16);

	// Get Block chipset coords
	short block_x, block_y;
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
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Get the block D subtiles ids and get their coordinates on the chipset
			rect.x = block_x + (BlockD_Subtiles_IDS[subtile][j][i][0] * 2 + i) * 8;
			rect.y = block_y + (BlockD_Subtiles_IDS[subtile][j][i][1] * 2 + j) * 8;

			// Blit the subtile
			tile->Blit(i * 8, j * 8, chipset, rect, 255);
		}
	}

	autotiles_d[block][subtile] = BitmapScreen::CreateBitmapScreen(tile);
}


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
Bitmap* TilemapLayer::GetChipset() const {
	return chipset;
}
void TilemapLayer::SetChipset(Bitmap* nchipset) {
	chipset = nchipset;
	chipset_screen->SetBitmap(chipset);
	chipset_screen->SetSrcRect(chipset->GetRect());
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

				tile.z = 0;

				// Calculate the tile Z
				if (!passable.empty()) {
					if (tile.ID >= BLOCK_F) {
						if ((passable[substitutions[tile.ID - BLOCK_F]] & (1 << 4)) == (1 << 4)) tile.z = 32;

					} else if (tile.ID >= BLOCK_E) {
						if ((passable[substitutions[tile.ID - BLOCK_E]] & (1 << 4)) == (1 << 4)) tile.z = 16;

					} else if (tile.ID >= BLOCK_D) {
						if ((passable[(tile.ID - BLOCK_D) / 50 + 6] & (1 << 5)) == (1 << 5)) tile.z = 9999;
						else if ((passable[(tile.ID - BLOCK_D) / 50 + 6] & (1 << 4)) == (1 << 4)) tile.z = 16;

					} else if (tile.ID >= BLOCK_C) {
						if ((passable[(tile.ID - BLOCK_C) / 50 + 3] & (1 << 4)) == (1 << 4)) tile.z = 16;

					} else {
						if ((passable[tile.ID / 1000] & (1 << 4)) == (1 << 4)) tile.z = 16;
					}
				}
				data_cache[x][y] = tile;
			}
		}

		if (layer == 0) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {

					if (data_cache[x][y].ID < BLOCK_C) {
						// If blocks A and B

						GenerateAutotileAB(data_cache[x][y].ID, 0);
						GenerateAutotileAB(data_cache[x][y].ID, 1);
						GenerateAutotileAB(data_cache[x][y].ID, 2);
					} else if (data_cache[x][y].ID >= BLOCK_D && data_cache[x][y].ID < BLOCK_E) {
						// If block D

						GenerateAutotileD(data_cache[x][y].ID);
					}
				}
			}
		} else {
			have_invisible_tile = true;
			for (int x = 288; x < 288 + 16; x++) {
				for (int y = 128; y < 128 + 16; y++) {
					if (chipset->GetPixel(x, y).alpha > 0) {
						have_invisible_tile = false;
						x = 288 + 16;
						break;
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
int TilemapLayer::GetZ() const {
	return -1;
}

////////////////////////////////////////////////////////////
unsigned long TilemapLayer::GetId() const {
	return ID;
}

DrawableType TilemapLayer::GetType() const {
	return type;
}

void TilemapLayer::Substitute(int old_id, int new_id) {
	substitutions[old_id] = (uint8) new_id;
}
