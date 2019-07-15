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

static constexpr int BLOCK_C = 3000;
static constexpr int BLOCK_C_TILES = 3;
static constexpr int BLOCK_C_INDEX = 3;

static constexpr int BLOCK_D = 4000;
static constexpr int BLOCK_D_TILES = 12;
static constexpr int BLOCK_D_INDEX = 6;

static constexpr int BLOCK_E = 5000;
static constexpr int BLOCK_E_TILES = 144;
static constexpr int BLOCK_E_INDEX = 18;

static constexpr int BLOCK_F = 10000;
static constexpr int BLOCK_F_TILES = 144;
static constexpr int BLOCK_F_INDEX = 162;

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
	if (chip_id < BLOCK_C + 50) {
		return chip_id / 1000;
	}
	else if (chip_id < BLOCK_D) {
		return 4 + (chip_id - BLOCK_C - 50) / 50;
	}
	else if (chip_id < BLOCK_E) {
		return 6 + (chip_id - BLOCK_D) / 50;
	}
	else if (chip_id < BLOCK_E + BLOCK_E_TILES) {
		return 18 + (chip_id - BLOCK_E);
	}
	else if (chip_id < BLOCK_E + BLOCK_E_TILES) {
		return 18 + (chip_id - BLOCK_E);
	}
	return 0;
}

#endif
