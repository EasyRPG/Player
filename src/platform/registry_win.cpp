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

#ifdef _WIN32

// Headers
#include <string>
#include "registry.h"
#include "utils.h"

/**
 * Adds Manifest depending on architecture.
 */
#ifdef _MSC_VER
	#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#endif
#endif

std::string Registry::ReadStrValue(HKEY hkey, std::string const& key, std::string const& val) {
	char value[1024];
	DWORD size = 1024;
	DWORD type = REG_SZ;
	HKEY key_handle;

	Utils::wstring wkey = Utils::ToWideString(key.c_str());

	if (RegOpenKeyEx(hkey, wkey.c_str(), NULL, KEY_QUERY_VALUE, &key_handle)) {
		return "";
	}

	Utils::wstring wval = Utils::ToWideString(val.c_str());

	if (RegQueryValueEx(key_handle, wval.c_str(), NULL, &type, (LPBYTE)&value, &size)) {
		return "";
	}
	RegCloseKey(key_handle);

	std::string string_value = "";
	for (unsigned int i = 0; i < size; i++) {
		if (value[i] != '\0' ) {
			string_value += value[i];
		}
	}
	return string_value;
}

int Registry::ReadBinValue(HKEY hkey, std::string const& key, std::string const& val, unsigned char* bin) {
	DWORD size = 1024;
	DWORD type = REG_BINARY;
	HKEY key_handle;

	Utils::wstring wkey = Utils::ToWideString(key.c_str());

	if (RegOpenKeyEx(hkey, wkey.c_str(), NULL, KEY_QUERY_VALUE, &key_handle)) {
		return 0;
	}

	Utils::wstring wval = Utils::ToWideString(val.c_str());

	if (RegQueryValueEx(key_handle, wval.c_str(), NULL, &type, bin, &size)) {
		return 0;
	}
	RegCloseKey(key_handle);

	return size;
}

#endif
