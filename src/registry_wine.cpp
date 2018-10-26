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

#if !(defined(_WIN32) || defined(GEKKO) || defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(_3DS) || defined(SWITCH))

#include <cstdlib>
#include "registry.h"
#include "filefinder.h"
#include "output.h"
#include "inireader.h"

std::string Registry::ReadStrValue(HKEY hkey, std::string const& key, std::string const& val, REGVIEW view) {
	std::string prefix =
			getenv("WINEPREFIX")? getenv("WINEPREFIX"):
			getenv("HOME")? std::string(getenv("HOME")).append("/.wine"):
			std::string();

	std::string registry_file = "";
	std::string string_value = "";
	std::string formatted_key = key;

	// Replaces key backslashes with double backslashes
	size_t pos = 0;
	while ((pos = formatted_key.find("\\", pos)) != std::string::npos) {
		formatted_key.replace(pos, 1, "\\\\");
		pos += 2;
	}

	// Puts value between quotes
	std::string formatted_val = "\"" + val + "\"";

	if(prefix.empty() || !FileFinder::Exists(prefix)) {
		Output::Debug("wine prefix not found: \"%s\"", prefix.c_str());
		return std::string("");
	}

	switch(hkey) {
		case HKEY_LOCAL_MACHINE:
			registry_file = prefix + "/system.reg";
			break;
		case HKEY_CURRENT_USER:
			registry_file = prefix + "/user.reg";
			break;
	}

	bool is_wine64 = FileFinder::Exists(prefix + "/drive_c/windows/syswow64");
	bool use_redirect = (view == KEY32 && is_wine64);
	/* FIXME: Actual registry redirection behaviour on 64 bit Windows is much more complex,
	 * for reference see: https://msdn.microsoft.com/en-us/library/aa384253(v=vs.85).aspx
	 * We just care for the simple case where the "Software" Key inside HKLM is redirected
	 * to the "Software\Wow6432Node" Key in 64 bit wine prefixes.
	 */
	if (hkey == HKEY_LOCAL_MACHINE && use_redirect && (formatted_key.rfind("Software\\\\", 0) == 0)) {
		int pos = formatted_key.find("\\\\", 0);
		formatted_key.insert(pos, "\\\\Wow6432Node");
	}

	INIReader registry(registry_file);
	std::string path;

	if(registry.ParseError() != -1) {
		string_value = registry.Get(formatted_key, formatted_val, "");

		// Removes begin and end quotes but keeps all other inner just in case
		if (string_value.size()) string_value.erase(0, 1);
		if (string_value.size()) string_value.erase(string_value.size() - 1, 1);

		if(string_value.size() < 3
			|| !std::isupper(*string_value.begin())
			|| std::string(string_value.begin() + 1, string_value.begin() + 3) != ":\\")
		{ return string_value; }

		// Replaces double backslashes with single backslashes
		pos = 0;
		while ((pos = string_value.find("\\\\", pos)) != std::string::npos) {
			string_value.replace(pos, 2, "/");
			pos += 1;
		}

		char const drive = std::tolower(*string_value.begin());
		switch(drive) {
			default:
				path.assign(prefix.append("/drive_"))
						.append(&drive, 1).append(string_value.begin() + 2, string_value.end());
				break;
			case 'z': path.assign(string_value.begin() + 2, string_value.end()); break;
		}
	}

	return path;
}

int Registry::ReadBinValue(HKEY, std::string const&, std::string const&, unsigned char*, REGVIEW) {
	return 0; // not really used yet
}

#endif
