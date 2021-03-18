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

#include "system.h"

#ifdef USE_WINE_REGISTRY

#include <cstdlib>
#include <fstream>
#include "registry.h"
#include "filefinder.h"
#include "output.h"
#include "utils.h"

/*
Wine registry file example:

[Software\\Wow6432Node\\ASCII\\RPG2000] 1554665942 9542200
#time=1d4ed798dfc6938
"FullScreenFlag"="0"
"RuntimePackagePath"="C:\\Program Files (x86)\\ASCII\\RPG2000\\RTP"
 */

std::string Registry::ReadStrValue(HKEY hkey, StringView key, StringView val, REGVIEW view) {
	std::string prefix =
			getenv("WINEPREFIX")? getenv("WINEPREFIX"):
			getenv("HOME")? std::string(getenv("HOME")).append("/.wine"):
			std::string();

	std::string registry_file;
	std::string string_value;
	std::string formatted_key = ToString(key);

	// Replaces key backslashes with double backslashes
	size_t pos = 0;
	while ((pos = formatted_key.find('\\', pos)) != std::string::npos) {
		formatted_key.replace(pos, 1, R"(\\)");
		pos += 2;
	}

	// Puts value between quotes and add =
	std::string formatted_val = "\"" + ToString(val) + "\""  + "=";

	if (prefix.empty() || !FileFinder::Root().Exists(prefix)) {
		Output::Debug("wine prefix not found: \"{}\"", prefix.c_str());
		return std::string("");
	}

	switch (hkey) {
		case HKEY_LOCAL_MACHINE:
			registry_file = prefix + "/system.reg";
			break;
		case HKEY_CURRENT_USER:
			registry_file = prefix + "/user.reg";
			break;
	}

	bool is_wine64 = FileFinder::Root().Exists(prefix + "/drive_c/windows/syswow64");
	bool use_redirect = (view == KEY32 && is_wine64);

	/* On 64bit Windows 32bit keys are redirected in some cases, see:
	 * https://msdn.microsoft.com/en-us/library/aa384253(v=vs.85).aspx
	 * We only support redirecting "Software" in HKLM.
	 */
	if (hkey == HKEY_LOCAL_MACHINE &&
		use_redirect && (formatted_key.rfind(R"(Software\\)", 0) == 0)) {
		pos = formatted_key.find(R"(\\)", 0);
		formatted_key.insert(pos, R"(\\Wow6432Node)");
	}

	// Custom, simple INI parser because liblcf ini is not efficient enough
	// (lcf ini stores all keys/values but we only need one)
	std::string formatted_key_search = "[" + Utils::LowerCaseInPlace(formatted_key) + "]";
	Utils::LowerCaseInPlace(formatted_val);
	std::string path;
	std::ifstream registry(registry_file);
	if (!registry) {
		return path;
	}

	bool in_section = false;
	std::string line;
	line.reserve(1024);
	do {
		std::getline(registry, line);
		if (!in_section) {
			if (line.empty() || line[0] != '[') {
				continue;
			} else if (ToStringView(Utils::LowerCaseInPlace(line)).starts_with(formatted_key_search)) {
				// Found the section
				in_section = true;
			}
		} else {
			if (!line.empty() && line[0] == '[') {
				// value not found
				break;
			}

			if (ToStringView(Utils::LowerCase(line)).starts_with(formatted_val)) {
				// value found
				string_value = line.substr(formatted_val.length());
				break;
			}
		}
	} while (!registry.eof());

	if (!string_value.empty()) {
		// Removes begin and end quotes but keeps all other inner just in case
		if (!string_value.empty()) {
			string_value.erase(0, 1);
		}
		if (!string_value.empty()) {
			string_value.erase(string_value.size() - 1, 1);
		}

		if (string_value.size() < 3
				|| !std::isupper(*string_value.begin())
				|| std::string(string_value.begin() + 1, string_value.begin() + 3) != R"(:\)") {
			return string_value;
		}

		// Replaces double backslashes with single backslashes
		pos = 0;
		while ((pos = string_value.find(R"(\\)", pos)) != std::string::npos) {
			string_value.replace(pos, 2, "/");
			pos += 1;
		}

		const char drive = std::tolower(*string_value.begin());

		if (drive == 'z') {
			path.assign(string_value.begin() + 2, string_value.end());
		} else {
			path.assign(prefix.append("/drive_"))
					.append(&drive, 1).append(string_value.begin() + 2, string_value.end());
		}
	}

	return path;
}

int Registry::ReadBinValue(HKEY, StringView, StringView, unsigned char*, REGVIEW) {
	return 0; // not really used yet
}

#endif
