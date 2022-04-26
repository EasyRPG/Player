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

#include "version.h"

// Helper Macros
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

// Fallback if buildsystem has not provided a version
#ifndef EP_VERSION
	#define EP_VERSION_MAJOR 0
	#define EP_VERSION_MINOR 7
	#define EP_VERSION_PATCH 0
	#define EP_VERSION TO_STRING(EP_VERSION_MAJOR) "." \
			TO_STRING(EP_VERSION_MINOR) "." TO_STRING(EP_VERSION_PATCH)
#endif

// These can be undefined
#ifndef EP_VERSION_GIT
	#define EP_VERSION_GIT ""
#endif
#ifndef EP_VERSION_APPEND
	#define EP_VERSION_APPEND ""
#endif

namespace Version {

	const char STRING[] = EP_VERSION;
	const int MAJOR = EP_VERSION_MAJOR;
	const int MINOR = EP_VERSION_MINOR;
	const int PATCH = EP_VERSION_PATCH;
	const char GIT[] = EP_VERSION_GIT;
	const char APPEND[] = EP_VERSION_APPEND;

	std::string GetVersionString(bool with_git, bool with_append) {
		std::string ver = Version::STRING;
		if (with_git && std::strlen(GIT) > 0) {
			ver += std::string(" ") + GIT;
		}
		if (with_append && std::strlen(APPEND) > 0) {
			ver += std::string(" ") + APPEND;
		}
		return ver;
	}
}
