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

#ifndef _VERSION_H_
#define _VERSION_H_

// Helper Macros
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

/**
 * Version of Player.
 * FIXME: Make this a proper, generated version header redefined by the build system.
 */
#define PLAYER_MAJOR 0
#define PLAYER_MINOR 5
#define PLAYER_PATCH 0
#define PLAYER_ADDTL ""
#define PLAYER_VERSION TO_STRING(PLAYER_MAJOR) "." TO_STRING(PLAYER_MINOR) "." TO_STRING(PLAYER_PATCH)

#endif /* _VERSION_H_ */
