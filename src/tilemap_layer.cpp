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
#include <cmath>
#include "tilemap_layer.h"
#include "output.h"
#include "player.h"
#include "map_data.h"
#include "main_data.h"
#include "bitmap.h"
#include "compiler.h"
#include "game_map.h"
#include "game_system.h"
#include "drawable_mgr.h"
#include "baseui.h"

// Blocks subtiles IDs
// Mess with this code and you will die in 3 days...
// [tile-id][row][col]
static constexpr int8_t BlockA_Subtiles_IDS[47][2][2] = {
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
static constexpr uint8_t BlockD_Subtiles_IDS[50][2][2][2] = {
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
	substitutions(Game_Map::GetTilesLayer(ilayer)),
	layer(ilayer),
	// SubLayer for the tiles without Wall or Above passability
	// Its z-value should be under z of the events in the lower layer
	lower_layer(this, Priority_TilesetBelow + TileBelow + layer),
	// SubLayer for the tiles with Wall or Above passability
	// Its z-value should be between the z of the events in the upper layer and the hero
	upper_layer(this, Priority_TilesetAbove + TileAbove + layer)
{
}

// This setup of having an always inlined DrawTile() which dispatches to DrawTileImpl()
// was created intentionally. Inlining the transparency check was measured and shown
// to provide a performance improvement
EP_ALWAYS_INLINE
void TilemapLayer::DrawTile(Bitmap& dst, Bitmap& tileset, Bitmap& tone_tileset, int x, int y, int row, int col, uint32_t tone_hash, bool allow_fast_blit) {
	auto op = tileset.GetTileOpacity(col, row);
	if (op != ImageOpacity::Transparent) {
		DrawTileImpl(dst, tileset, tone_tileset, x, y, row, col, tone_hash, op, allow_fast_blit);
	}
}

void TilemapLayer::DrawTileImpl(Bitmap& dst, Bitmap& tileset, Bitmap& tone_tileset, int x, int y, int row, int col, uint32_t tone_hash, ImageOpacity op, bool allow_fast_blit) {

	auto rect = Rect{ col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };

	auto* src = &tileset;

	// Create tone changed tile
	if (tone != Tone()) {
		if (chipset_tone_tiles.insert(tone_hash).second) {
			tone_tileset.ToneBlit(col * TILE_SIZE, row * TILE_SIZE, tileset, rect, tone, Opacity::Opaque());
		}
		src = &tone_tileset;
	}

	bool use_fast_blit = fast_blit && allow_fast_blit;
	if (op == ImageOpacity::Opaque || use_fast_blit) {
		dst.BlitFast(x, y, *src, rect, 255);
	} else {
		dst.Blit(x, y, *src, rect, 255);
	}
}

static uint32_t MakeFTileHash(int id) {
	return static_cast<uint32_t>(id);
}

static uint32_t MakeETileHash(int id) {
	return static_cast<uint32_t>(id | (1 << 24));
}

static uint32_t MakeDTileHash(int id) {
	return static_cast<uint32_t>(id | (2 << 24));
}

static uint32_t MakeCTileHash(int id, int anim_step) {
	return static_cast<uint32_t>((id + (anim_step << 12)) | (3 << 24));
}

static uint32_t MakeAbTileHash(int id, int anim_step) {
	return static_cast<uint32_t>((id + (anim_step << 12)) | (4 << 24));
}

void TilemapLayer::Draw(Bitmap& dst, uint8_t z_order) {
	// Get the number of tiles that can be displayed on window
	int tiles_x = (int)ceil(Player::screen_width / (float)TILE_SIZE);
	int tiles_y = (int)ceil(Player::screen_height / (float)TILE_SIZE);

	// If ox or oy are not equal to the tile size draw the next tile too
	// to prevent black (empty) tiles at the borders
	if (ox % TILE_SIZE != 0) {
		++tiles_x;
	}
	if (oy % TILE_SIZE != 0) {
		++tiles_y;
	}

	const bool loop_h = Game_Map::LoopHorizontal();
	const bool loop_v = Game_Map::LoopVertical();

	auto div_rounding_down = [](int n, int m) {
		if (n >= 0) return n / m;
		return (n - m + 1) / m;
	};
	auto mod = [](int n, int m) {
		int rem = n % m;
		return rem >= 0 ? rem : m + rem;
	};

	// FIXME: When Game_Map singleton is made an object we can remove this null check
	const auto frames = Main_Data::game_system ? Main_Data::game_system->GetFrameCounter() : 0;
	auto animation_step_c = (frames / 6) % 4;
	auto animation_step_ab = frames / animation_speed;
	if (animation_type) {
		animation_step_ab %= 3;
	} else {
		animation_step_ab %= 4;
		if (animation_step_ab == 3) {
			animation_step_ab = 1;
		}
	}

	const int div_ox = div_rounding_down(ox, TILE_SIZE);
	const int div_oy = div_rounding_down(oy, TILE_SIZE);

	const int mod_ox = mod(ox, TILE_SIZE);
	const int mod_oy = mod(oy, TILE_SIZE);

	for (int y = 0; y < tiles_y; y++) {
		for (int x = 0; x < tiles_x; x++) {

			// Get the real maps tile coordinates
			int map_x = div_ox + x;
			int map_y = div_oy + y;
			if (loop_h) map_x = mod(map_x, width);
			if (loop_v) map_y = mod(map_y, height);

			int map_draw_x = x * TILE_SIZE - mod_ox;
			int map_draw_y = y * TILE_SIZE - mod_oy;

			bool out_of_bounds =
				map_x < 0 || map_x >= width ||
				map_y < 0 || map_y >= height;

			if (out_of_bounds) {
				continue;
			}

			// Get the tile data
			TileData &tile = GetDataCache(map_x, map_y);

			// Draw the sublayer if its z is being draw now
			if (z_order == tile.z) {
				if (layer == 0) {
					// If lower layer
					bool allow_fast_blit = (tile.z == TileBelow);

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

						auto tone_hash = MakeETileHash(id);
						DrawTile(dst, *chipset, *chipset_effect, map_draw_x, map_draw_y, row, col, tone_hash, allow_fast_blit);
					} else if (tile.ID >= BLOCK_C && tile.ID < BLOCK_D) {
						// If Block C

						// Get the tile coordinates from chipset
						int col = 3 + (tile.ID - BLOCK_C) / 50;
						int row = 4 + animation_step_c;

						auto tone_hash = MakeCTileHash(tile.ID, animation_step_c);
						DrawTile(dst, *chipset, *chipset_effect, map_draw_x, map_draw_y, row, col, tone_hash, allow_fast_blit);
					} else if (tile.ID < BLOCK_C) {
						// If Blocks A1, A2, B

						// Draw the tile from autotile cache
						TileXY pos = GetCachedAutotileAB(tile.ID, animation_step_ab);

						int col = pos.x;
						int row = pos.y;

						// Create tone changed tile
						auto tone_hash = MakeAbTileHash(tile.ID,  animation_step_ab);
						DrawTile(dst, *autotiles_ab_screen, *autotiles_ab_screen_effect, map_draw_x, map_draw_y, row, col, tone_hash, allow_fast_blit);
					} else {
						// If blocks D1-D12

						// Draw the tile from autotile cache
						TileXY pos = GetCachedAutotileD(tile.ID);

						int col = pos.x;
						int row = pos.y;

						auto tone_hash = MakeDTileHash(tile.ID);
						DrawTile(dst, *autotiles_d_screen, *autotiles_d_screen_effect, map_draw_x, map_draw_y, row, col, tone_hash, allow_fast_blit);
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

						auto tone_hash = MakeFTileHash(id);
						DrawTile(dst, *chipset, *chipset_effect, map_draw_x, map_draw_y, row, col, tone_hash);
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
	data_cache_vec.resize(width * height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			TileData tile;

			// Get the tile ID
			tile.ID = nmap_data[x + y * width];

			tile.z = TileBelow;

			// Calculate the tile Z
			if (!passable.empty()) {
				if (tile.ID >= BLOCK_F) { // Upper layer
					if ((passable[substitutions[tile.ID - BLOCK_F]] & Passable::Above) != 0)
						tile.z = TileAbove + 1; // Upper sublayer
					else
						tile.z = TileBelow + 1; // Lower sublayer

				} else { // Lower layer
					int chip_index =
						tile.ID >= BLOCK_E ? substitutions[tile.ID - BLOCK_E] + 18 :
						tile.ID >= BLOCK_D ? (tile.ID - BLOCK_D) / 50 + 6 :
						tile.ID >= BLOCK_C ? (tile.ID - BLOCK_C) / 50 + 3 :
						tile.ID / 1000;
					if ((passable[chip_index] & (Passable::Wall | Passable::Above)) != 0)
						tile.z = TileAbove; // Upper sublayer
					else
						tile.z = TileBelow; // Lower sublayer

				}
			}
			GetDataCache(x, y) = tile;
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
		Output::Warning("Invalid AB autotile ID: {} (b_subtile = {})",
						ID, b_subtile);
		return;
	}

	// Calculate the A block combination
	short a_subtile = ID - block * 1000 - b_subtile * 50;
	if (a_subtile >= 47) {
		Output::Warning("Invalid AB autotile ID: {} (a_subtile = {})",
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
	auto it = autotiles_ab_map.find(quarters_hash);
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

	if (block >= 12 || subtile >= 50 || block < 0 || subtile < 0) {
		Output::Warning("Tilemap index out of range: {} {}", block, subtile);
		return;
	}

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
	auto it = autotiles_d_map.find(quarters_hash);
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

BitmapRef TilemapLayer::GenerateAutotiles(int count, const std::unordered_map<uint32_t, TileXY>& map) {
	int rows = (count + TILES_PER_ROW - 1) / TILES_PER_ROW;
	BitmapRef tiles = Bitmap::Create(TILES_PER_ROW * TILE_SIZE, rows * TILE_SIZE);
	tiles->Clear();
	Rect rect(0, 0, TILE_SIZE/2, TILE_SIZE/2);

	for (auto& p: map) {
		uint32_t quarters_hash = p.first;
		TileXY dst = p.second;

		// unpack the quarters data
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				constexpr int mask = ~(0xFu << 28);

				int x = quarters_hash >> 28;
				quarters_hash &= mask;
				quarters_hash <<= 4;

				int y = quarters_hash >> 28;
				quarters_hash &= mask;
				quarters_hash <<= 4;

				rect.x = (x * 2 + i) * (TILE_SIZE/2);
				rect.y = (y * 2 + j) * (TILE_SIZE/2);

				tiles->BlitFast((dst.x * 2 + i) * (TILE_SIZE / 2), (dst.y * 2 + j) * (TILE_SIZE / 2), *chipset, rect, 255);
			}
		}
	}

	if (rows > 0) {
		tiles->CheckPixels(Bitmap::Flag_Chipset | Bitmap::Flag_ReadOnly);
	}

	return tiles;
}

void TilemapLayer::SetChipset(BitmapRef const& nchipset) {
	chipset = nchipset;
	chipset_effect = Bitmap::Create(chipset->width(), chipset->height());
	chipset_tone_tiles.clear();

	if (autotiles_ab_next != 0 && autotiles_d_screen != nullptr && layer == 0) {
		autotiles_ab_screen = GenerateAutotiles(autotiles_ab_next, autotiles_ab_map);
		autotiles_d_screen = GenerateAutotiles(autotiles_d_next, autotiles_d_map);

		autotiles_ab_screen_effect = Bitmap::Create(autotiles_ab_screen->width(), autotiles_ab_screen->height());
		autotiles_d_screen_effect = Bitmap::Create(autotiles_d_screen->width(), autotiles_d_screen->height());
	}
}

void TilemapLayer::SetMapData(std::vector<short> nmap_data) {
	// Create the tiles data cache
	CreateTileCache(nmap_data);
	memset(autotiles_ab, 0, sizeof(autotiles_ab));
	memset(autotiles_d, 0, sizeof(autotiles_d));

	if (layer == 0) {
		autotiles_ab_map.clear();
		autotiles_d_map.clear();
		autotiles_ab_next = 0;
		autotiles_d_next = 0;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {

				if (GetDataCache(x, y).ID < BLOCK_C) {
					// If blocks A and B

					GenerateAutotileAB(GetDataCache(x, y).ID, 0);
					GenerateAutotileAB(GetDataCache(x, y).ID, 1);
					GenerateAutotileAB(GetDataCache(x, y).ID, 2);
				} else if (GetDataCache(x, y).ID >= BLOCK_D && GetDataCache(x, y).ID < BLOCK_E) {
					// If block D

					GenerateAutotileD(GetDataCache(x, y).ID);
				}
			}
		}
		autotiles_ab_screen = GenerateAutotiles(autotiles_ab_next, autotiles_ab_map);
		autotiles_d_screen = GenerateAutotiles(autotiles_d_next, autotiles_d_map);

		autotiles_ab_screen_effect = Bitmap::Create(autotiles_ab_screen->width(), autotiles_ab_screen->height());
		autotiles_d_screen_effect = Bitmap::Create(autotiles_d_screen->width(), autotiles_d_screen->height());

		chipset_tone_tiles.clear();
	}

	map_data = std::move(nmap_data);
}

void TilemapLayer::SetPassable(std::vector<unsigned char> npassable) {
	passable = std::move(npassable);

	// Recalculate z values of all tiles
	CreateTileCache(map_data);
}

void TilemapLayer::OnSubstitute() {
	substitutions = Game_Map::GetTilesLayer(layer);

	// Recalculate z values of all tiles
	CreateTileCache(map_data);
}

TilemapSubLayer::TilemapSubLayer(TilemapLayer* tilemap, Drawable::Z_t z) :
	Drawable(z),
	tilemap(tilemap),
	internal_z(static_cast<uint8_t>(z))
{
	DrawableMgr::Register(this);
}

void TilemapSubLayer::Draw(Bitmap& dst) {
	if (!tilemap->GetChipset()) {
		return;
	}

	tilemap->Draw(dst, internal_z);
}

void TilemapLayer::SetTone(Tone tone) {
	if (tone == this->tone) {
		return;
	}

	this->tone = tone;

	if (autotiles_d_screen_effect) {
		autotiles_d_screen_effect->Clear();
	}
	if (autotiles_ab_screen_effect) {
		autotiles_ab_screen_effect->Clear();
	}
	if (chipset_effect) {
		chipset_effect->Clear();
	}
	chipset_tone_tiles.clear();
}
