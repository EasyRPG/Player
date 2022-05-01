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

#ifndef EP_VERSION_H
#define EP_VERSION_H

#include <cstring>
#include <string>

/**
 * Version of Player.
 */
namespace Version {

	extern const int MAJOR;
	extern const int MINOR;
	extern const int PATCH;
	extern const int TWEAK;
	extern const char STRING[];
	extern const char GIT[];
	extern const char APPEND[];

	/**
	 * Generates a version string.
	 *
	 * @param with_git Append git information (if available)
	 * @param with_append Append additional information (if available, by default the build date)
	 */
	std::string GetVersionString(bool with_git = true, bool with_append = true);
}

/**
 * Version written to the easyrpg_data.version field in savegames.
 * Increment the last digit (0) only when having a good reason.
 */
#define PLAYER_SAVEGAME_VERSION (Version::MAJOR * 1000 + Version::MINOR * 100 + Version::PATCH * 10 + 0)

#endif
