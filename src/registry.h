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

#ifndef _REGISTRY_H_
#define _REGISTRY_H_

// Headers
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
enum HKEY {
	HKEY_LOCAL_MACHINE,
	HKEY_CURRENT_USER
};
#endif
enum REGVIEW {
	NATIVE,
	KEY64,
	KEY32
};

/**
 * Registry namespace
 */
namespace Registry {
	/**
	 * Reads string value.
	 */
	std::string ReadStrValue(HKEY hkey, std::string const& key, std::string const& val, REGVIEW view = NATIVE);

	/**
	 * Reads binary value.
	 */
	int ReadBinValue(HKEY hkey, std::string const& key, std::string const& val, unsigned char* bin, REGVIEW view = NATIVE);
}

#endif
