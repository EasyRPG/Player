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
#include <cstring>
#include <math.h>
#include "tilemap_layer.h"
#include "graphics.h"
#include "output.h"
#include "player.h"
#include "map_data.h"
#include "bitmap.h"

// Blocks subtiles IDs
// Mess with this code and you will die in 3 days...
// [tile-id][row][col]
static const int8_t BlockA_Subtiles_IDS[47][2][2] = {
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
static const uint8_t BlockD_Subtiles_IDS[50][2][2][2] = {
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

TilemapLayer::TilemapLayer(int ilayer) :
	visible(true),
	ox(0),
	oy(0),
	width(0),
	height(0),
	animation_frame(0),
	animation_step_ab(0),
	animation_step_c(0),
	animation_speed(24),
	animation_type(0),
	layer(ilayer) {

	memset(autotiles_ab, 0, sizeof(autotiles_ab));
	memset(autotiles_d, 0, sizeof(autotiles_d));

	int tiles_y = (int)ceil(DisplayUi->GetHeight() / (float)TILE_SIZE) + 1;
	for (int i = 0; i < tiles_y + 2; i++) {
		tilemap_tiles.push_back(EASYRPG_MAKE_SHARED<TilemapTile>(this, TILE_SIZE * i));
	}
}

void TilemapLayer::DrawTile(Bitmap& screen, int x, int y, int row, int col, bool autotile) {
	if (!autotile && screen.GetTileOpacity(row, col) == Bitmap::Transparent)
		return;
	Rect rect(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);

	BitmapRef dst = DisplayUi->GetDisplaySurface();
	dst->Blit(x, y, screen, rect, 255);
}

void TilemapLayer::Draw(int z_order) {
	if (!visible) return;

	// FIXME: 
	// When the map is looping and the camera is over a map boundary the tiles
	// on one half of the map are rendered too late.

	// Get the number of tiles that can be displayed on window
	int tiles_x = (int)ceil(DisplayUi->GetWidth() / (float)TILE_SIZE);
	int tiles_y = (int)ceil(DisplayUi->GetHeight() / (float)TILE_SIZE);

	// If ox or oy are not equal to the tile size draw the next tile too
	// to prevent black (empty) tiles at the borders
	if (ox % TILE_SIZE != 0) {
		++tiles_x;
	}
	if (oy % TILE_SIZE != 0) {
		++tiles_y;
	}

	for (int x = 0; x < tiles_x; x++) {
		for (int y = 0; y < tiles_y; y++) {
			// Get the real maps tile coordinates
			int map_x = (ox / TILE_SIZE + x) % width;
			int map_y = (oy / TILE_SIZE + y) % height;

			if (map_x < 0) {
				map_x += width;
			}
			if (map_y < 0) {
				map_y += height;
			}

			int map_draw_x = x * TILE_SIZE - ox % TILE_SIZE;
			int map_draw_y = y * TILE_SIZE - oy % TILE_SIZE;

			// Get the tile data
			TileData &tile = data_cache[map_x][map_y];

			int map_draw_z = tile.z;

			if (map_draw_z > 0) {
				if (map_draw_z < 9999) {
					map_draw_z += y * TILE_SIZE;
					if (y == 0) map_draw_z += TILE_SIZE;
				}
			}

			// Draw the tile if its z is being draw now
			if (z_order == map_draw_z) {
				if (layer == 0) {
					// If lower layer

					if (tile.ID >= BLOCK_E && tile.ID < BLOCK_E + BLOCK_E_TILES) {
						int id = substitutions[tile.ID - BLOCK_E];
						// If Block E

						int row, col;

						// Get the tile coordinates from chipset
						if (id < 96) {
							// If from first column of the block
							col = 12 + id % 6;
							row = id / 6;
						} else {
							// If from second column of the block
							col = 18 + (id - 96) % 6;
							row = (id - 96) / 6;
						}

						DrawTile(*chipset, map_draw_x, map_draw_y, row, col, false);
					} else if (tile.ID >= BLOCK_C && tile.ID < BLOCK_D) {
						// If Block C

						// Get the tile coordinates from chipset
						int col = 3 + (tile.ID - BLOCK_C) / 50;
						int row = 4 + animation_step_c;

						// Draw the tile
						DrawTile(*chipset, map_draw_x, map_draw_y, row, col, false);
					} else if (tile.ID < BLOCK_C) {
						// If Blocks A1, A2, B

						// Draw the tile from autotile cache
						TileXY pos = GetCachedAutotileAB(tile.ID, animation_step_ab);
						DrawTile(*autotiles_ab_screen, map_draw_x, map_draw_y, pos.y, pos.x, true);
					} else {
						// If blocks D1-D12

						// Draw the tile from autotile cache
						TileXY pos = GetCachedAutotileD(tile.ID);
						DrawTile(*autotiles_d_screen, map_draw_x, map_draw_y, pos.y, pos.x, true);
					}
				} else {
					// If upper layer

					// Check that block F is being drawn
					if (tile.ID >= BLOCK_F && tile.ID < BLOCK_F + BLOCK_F_TILES) {
						int id = substitutions[tile.ID - BLOCK_F];
						int row, col;

						// Get the tile coordinates from chipset
						if (id < 48) {
							// If from first column of the block
							col = 18 + id % 6;
							row = 8 + id / 6;
						} else {
							// If from second column of the block
							col = 24 + (id - 48) % 6;
							row = (id - 48) / 6;
						}

						// Draw the tile
						DrawTile(*chipset, map_draw_x, map_draw_y, row, col, false);
					}
				}
			}
		}
	}
}

TilemapLayer::TileXY TilemapLayer::GetCachedAutotileAB(short ID, short animID) {
	short block = ID / 1000;
	short b_subtile = (ID - block * 1000) / 50;
	short a_subtile = ID - block * 1000 - b_subtile * 50;
	return autotiles_ab[animID][block][b_subtile][a_subtile];
}

TilemapLayer::TileXY TilemapLayer::GetCachedAutotileD(short ID) {
	short block = (ID - 4000) / 50;
	short subtile = ID - 4000 - block * 50;
	return autotiles_d[block][subtile];
}

void TilemapLayer::CreateTileCache(const std::vector<short>& nmap_data) {
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
					if ((passable[substitutions[tile.ID - BLOCK_F]] & Passable::Above) != 0)
						tile.z = 32;

				}
				else if (tile.ID >= BLOCK_E) {
					if ((passable[substitutions[tile.ID - BLOCK_E + 18]] & Passable::Above) != 0)
						tile.z = 32;

				}
				else if (tile.ID >= BLOCK_D) {
					if ((passable[(tile.ID - BLOCK_D) / 50 + 6] & (Passable::Wall | Passable::Above)) != 0)
						tile.z = 32;

				}
				else if (tile.ID >= BLOCK_C) {
					if ((passable[(tile.ID - BLOCK_C) / 50 + 3] & Passable::Above) != 0)
						tile.z = 32;

				}
				else {
					if ((passable[tile.ID / 1000] & Passable::Above) != 0)
						tile.z = 32;
				}
			}
			data_cache[x][y] = tile;
		}
	}
}

void TilemapLayer::GenerateAutotileAB(short ID, short animID) {
	// Calculate the block to use
	//	1: A1 + Upper B (Grass + Coast)
	//	2: A2 + Upper B (Snow + Coast)
	//	3: A1 + Lower B (Grass + Ocean/Deep water)
	short block = ID / 1000;

	// Calculate the B block combination
	short b_subtile = (ID - block * 1000) / 50;
	if (b_subtile >= TILE_SIZE) {
		Output::Warning("Invalid AB autotile ID: %d (b_subtile = %d)",
						ID, b_subtile);
		return;
	}

	// Calculate the A block combination
	short a_subtile = ID - block * 1000 - b_subtile * 50;
	if (a_subtile >= 47) {
		Output::Warning("Invalid AB autotile ID: %d (a_subtile = %d)",
						ID, a_subtile);
		return;
	}

	if (autotiles_ab[animID][block][b_subtile][a_subtile].valid)
		return;

	uint8_t quarters[2][2][2];

	// Determine block B subtiles
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Skip the subtile if it will be used one from A block instead
			if (BlockA_Subtiles_IDS[a_subtile][j][i] != -1) continue;

			// Get the block B subtiles ids and get their coordinates on the chipset
			int t = (b_subtile >> (j * 2 + i)) & 1;
			if (block == 2) t ^= 3;

			quarters[j][i][0] = animID;
			quarters[j][i][1] = 4 + t;
		}
	}

	// Determine block A subtiles
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Skip the subtile if it was used one from B block
			if (BlockA_Subtiles_IDS[a_subtile][j][i] == -1) continue;

			// Get the block A subtiles ids and get their coordinates on the chipset
			quarters[j][i][0] = animID + (block == 1 ? 3 : 0);
			quarters[j][i][1] = BlockA_Subtiles_IDS[a_subtile][j][i];
		}
	}

	// Determine block B subtiles when combining A and B
	if (b_subtile != 0 && a_subtile != 0) {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				// calculate tile (row 0..3)
				int t = (b_subtile >> (j * 2 + i)) & 1;
				if (block == 2) t *= 2;

				// Skip the subtile if not used
				if (t == 0) continue;

				// Get the coordinates on the chipset
				quarters[j][i][0] = animID;
				quarters[j][i][1] = 4 + t;
			}
		}
	}

	// pack the quarters data into a word
	uint32_t quarters_hash = 0;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 2; i++)
			for (int k = 0; k < 2; k++) {
				quarters_hash <<= 4;
				quarters_hash |= quarters[j][i][k];
			}

	// check whether we have already generated this tile
	std::map<uint32_t, TileXY>::iterator it;
	it = autotiles_ab_map.find(quarters_hash);
	if (it != autotiles_ab_map.end()) {
		autotiles_ab[animID][block][b_subtile][a_subtile] = it->second;
		return;
	}

	int id = autotiles_ab_next++;
	int dst_x = id % TILES_PER_ROW;
	int dst_y = id / TILES_PER_ROW;

	TileXY tile_xy(dst_x, dst_y);
	autotiles_ab_map[quarters_hash] = tile_xy;
	autotiles_ab[animID][block][b_subtile][a_subtile] = tile_xy;
}

void TilemapLayer::GenerateAutotileD(short ID) {
	// Calculate the D block id
	short block = (ID - 4000) / 50;

	// Calculate the D block combination
	short subtile = ID - 4000 - block * 50;

	if (block >= 12 || subtile >= 50 || block < 0 || subtile < 0)
		Output::Error("Index out of range: %d %d", block, subtile);

	if (autotiles_d[block][subtile].valid)
		return;

	uint8_t quarters[2][2][2];

	// Get Block chipset coords
	short block_x, block_y;
	if (block < 4) {
		// If from first column
		block_x = (block % 2) * 3;
		block_y = 8 + (block / 2) * 4;
	} else {
		// If from second column
		block_x = 6 + (block % 2) * 3;
		block_y = ((block - 4) / 2) * 4;
	}

	// Calculate D block subtiles
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			// Get the block D subtiles ids and get their coordinates on the chipset
			quarters[j][i][0] = block_x + BlockD_Subtiles_IDS[subtile][j][i][0];
			quarters[j][i][1] = block_y + BlockD_Subtiles_IDS[subtile][j][i][1];
		}
	}

	// pack the quarters data into a word
	uint32_t quarters_hash = 0;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 2; i++)
			for (int k = 0; k < 2; k++) {
				quarters_hash <<= 4;//multiply 16
				quarters_hash |= quarters[j][i][k];
			}



	// check whether we have already generated this tile
	std::map<uint32_t, TileXY>::iterator it;
	it = autotiles_d_map.find(quarters_hash);
	if (it != autotiles_d_map.end()) {
		autotiles_d[block][subtile] = it->second;
		return;
	}

	int id = autotiles_d_next++;
	int dst_x = id % TILES_PER_ROW;
	int dst_y = id / TILES_PER_ROW;

	TileXY tile_xy(dst_x, dst_y);
	autotiles_d_map[quarters_hash] = tile_xy;
	autotiles_d[block][subtile] = tile_xy;
}


BitmapRef TilemapLayer::GenerateAutotiles(int count, const std::map<uint32_t, TileXY>& map) {
	int rows = (count + TILES_PER_ROW - 1) / TILES_PER_ROW;
	BitmapRef tiles = Bitmap::Create(TILES_PER_ROW * TILE_SIZE, rows * TILE_SIZE);
	tiles->Clear();
	Rect rect(0, 0, TILE_SIZE/2, TILE_SIZE/2);

	std::map<uint32_t, TileXY>::const_iterator it;
	for (it = map.begin(); it != map.end(); ++it) {
		uint32_t quarters_hash = it->first;
		TileXY dst = it->second;

		// unpack the quarters data
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				int x = quarters_hash >> 28;
				quarters_hash <<= 4;
				
				int y = quarters_hash >> 28;
				quarters_hash <<= 4;
				
				rect.x = (x * 2 + i) * (TILE_SIZE/2);
				rect.y = (y * 2 + j) * (TILE_SIZE/2);
				tiles->Blit((dst.x * 2 + i) * (TILE_SIZE/2), (dst.y * 2 + j) * (TILE_SIZE/2), *chipset, rect, 255);
			}
		}
	}

	return tiles;
}

void TilemapLayer::Update() {
	animation_frame += 1;

	// Step to the next animation frame
	if (animation_frame % 6 == 0) {
		animation_step_c = (animation_step_c + 1) % 4;
	}
	if (animation_frame == animation_speed) {
		animation_step_ab = 1;
	} else if (animation_frame == animation_speed * 2) {
		animation_step_ab = 2;
	} else if (animation_frame == animation_speed * 3) {
		if (animation_type == 0) {
			// If animation type is 1-2-3-2
			animation_step_ab = 1;
		} else {
			// If animation type is 1-2-3
			animation_step_ab = 0;
			animation_frame = 0;
		}
	} else if (animation_frame >= animation_speed * 4) {
		animation_step_ab = 0;
		animation_frame = 0;
	}
}

BitmapRef const& TilemapLayer::GetChipset() const {
	return chipset;
}

void TilemapLayer::SetChipset(BitmapRef const& nchipset) {
	chipset = nchipset;
	if (autotiles_ab_next != 0 && autotiles_d_screen != 0 && layer == 0) {
		autotiles_ab_screen = GenerateAutotiles(autotiles_ab_next, autotiles_ab_map);
		autotiles_d_screen = GenerateAutotiles(autotiles_d_next, autotiles_d_map);
	}
}

std::vector<short> TilemapLayer::GetMapData() const {
	return map_data;
}

void TilemapLayer::SetMapData(const std::vector<short>& nmap_data) {
	if (map_data != nmap_data) {
		// Create the tiles data cache
		CreateTileCache(nmap_data);

		if (layer == 0) {
			autotiles_ab_map.clear();
			autotiles_d_map.clear();
			autotiles_ab_next = 0;
			autotiles_d_next = 0;
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
			autotiles_ab_screen = GenerateAutotiles(autotiles_ab_next, autotiles_ab_map);
			autotiles_d_screen = GenerateAutotiles(autotiles_d_next, autotiles_d_map);
		}
	}
	map_data = nmap_data;
}

std::vector<unsigned char> TilemapLayer::GetPassable() const {
	return passable;
}

void TilemapLayer::SetPassable(const std::vector<unsigned char>& npassable) {
	passable = npassable;

	if (substitutions.size() < passable.size())
	{
		substitutions.resize(passable.size());
		for (uint8_t i = 0; i < substitutions.size(); i++)
			substitutions[i] = i;
	}
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

void TilemapLayer::Substitute(int old_id, int new_id) {
	int subst_count = 0;

	for (size_t i = 0; i < substitutions.size(); ++i) {
		if (substitutions[i] == old_id) {
			++subst_count;
			substitutions[i] = (uint8_t) new_id;
		}
	}

	if (subst_count > 0) {
		// Recalculate z values of all tiles
		CreateTileCache(map_data);
	}
}

TilemapTile::TilemapTile(TilemapLayer* tilemap, int z) :
	type(TypeTilemap),
	tilemap(tilemap),
	z(z)
{
	Graphics::RegisterDrawable(this);
}

TilemapTile::~TilemapTile() {
	Graphics::RemoveDrawable(this);
}

void TilemapTile::Draw() {
	tilemap->Draw(GetZ());
}

int TilemapTile::GetZ() const {
	return z;
}

DrawableType TilemapTile::GetType() const {
	return type;
}
