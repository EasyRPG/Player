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

static const int BLOCK_C = 3000;

static const int BLOCK_D = 4000;
static const int BLOCK_D_BLOCKS = 12;

static const int BLOCK_E = 5000;
static const int BLOCK_E_TILES = 144;

static const int BLOCK_F = 10000;
static const int BLOCK_F_TILES = 144;

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

#endif
