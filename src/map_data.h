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

#ifndef EP_MAP_DATA_H
#define EP_MAP_DATA_H

// Tile IDs
static constexpr int BLOCK_A = 0;
static constexpr int BLOCK_A_TILES = 2;
static constexpr int BLOCK_A_STRIDE = 1000;
static constexpr int BLOCK_A_INDEX = 0;
static constexpr int BLOCK_A_END = BLOCK_A + BLOCK_A_TILES * BLOCK_A_STRIDE;

static constexpr int BLOCK_B = 2000;
static constexpr int BLOCK_B_TILES = 1;
static constexpr int BLOCK_B_STRIDE = 1000;
static constexpr int BLOCK_B_INDEX = 2;
static constexpr int BLOCK_B_END = BLOCK_B + BLOCK_B_TILES * BLOCK_B_STRIDE;

static constexpr int BLOCK_C = 3000;
static constexpr int BLOCK_C_TILES = 3;
static constexpr int BLOCK_C_STRIDE = 50;
static constexpr int BLOCK_C_INDEX = 3;
static constexpr int BLOCK_C_END = BLOCK_C + BLOCK_C_TILES * BLOCK_C_STRIDE;

static constexpr int BLOCK_D = 4000;
static constexpr int BLOCK_D_TILES = 12;
static constexpr int BLOCK_D_STRIDE = 50;
static constexpr int BLOCK_D_INDEX = 6;
static constexpr int BLOCK_D_END = BLOCK_D + BLOCK_D_TILES * BLOCK_D_STRIDE;

static constexpr int BLOCK_E = 5000;
static constexpr int BLOCK_E_TILES = 144;
static constexpr int BLOCK_E_STRIDE = 1;
static constexpr int BLOCK_E_INDEX = 18;
static constexpr int BLOCK_E_END = BLOCK_E + BLOCK_E_TILES * BLOCK_E_STRIDE;

static constexpr int BLOCK_F = 10000;
static constexpr int BLOCK_F_TILES = 144;
static constexpr int BLOCK_F_STRIDE = 1;
static constexpr int BLOCK_F_INDEX = 162;
static constexpr int BLOCK_F_END = BLOCK_F + BLOCK_F_TILES * BLOCK_F_STRIDE;

static constexpr int NUM_LOWER_TILES = BLOCK_F_INDEX;
static constexpr int NUM_UPPER_TILES = BLOCK_F_TILES;
static constexpr int NUM_TILES = NUM_LOWER_TILES + NUM_UPPER_TILES;

/** Passability flags. */
namespace Passable {
	enum Passable {
		Down		= 0x01,
		Left		= 0x02,
		Right		= 0x04,
		Up		= 0x08,
		Above		= 0x10,
		Wall		= 0x20,
		Counter		= 0x40
	};
}

inline int ChipIdToIndex(int chip_id) {
	if (chip_id >= BLOCK_A && chip_id < BLOCK_A_END) {
		return BLOCK_A_INDEX + (chip_id - BLOCK_A) / BLOCK_A_STRIDE;
	}
	else if (chip_id >= BLOCK_B && chip_id < BLOCK_B_END) {
		return BLOCK_B_INDEX + (chip_id - BLOCK_B) / BLOCK_B_STRIDE;
	}
	else if (chip_id >= BLOCK_C && chip_id < BLOCK_C_END) {
		return BLOCK_C_INDEX + (chip_id - BLOCK_C) / BLOCK_C_STRIDE;
	}
	else if (chip_id >= BLOCK_D && chip_id < BLOCK_D_END) {
		return BLOCK_D_INDEX + (chip_id - BLOCK_D) / BLOCK_D_STRIDE;
	}
	else if (chip_id >= BLOCK_E && chip_id < BLOCK_E_END) {
		return BLOCK_E_INDEX + (chip_id - BLOCK_E) / BLOCK_E_STRIDE;
	}
	else if (chip_id >= BLOCK_F && chip_id < BLOCK_F_END) {
		return BLOCK_F_INDEX + (chip_id - BLOCK_F) / BLOCK_F_STRIDE;
	}
	return 0;
}

#endif
