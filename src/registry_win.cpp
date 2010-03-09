//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
    #error This build doesn't target windows platforms
#endif

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "registry_win.h"

////////////////////////////////////////////////////////////
/// MSVC Unicode std::string to LPCWSTR
////////////////////////////////////////////////////////////
#ifdef MSVC
static std::wstring s2ws2(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
#endif

////////////////////////////////////////////////////////////
/// Read String value
////////////////////////////////////////////////////////////
std::string Registry::ReadStrValue(HKEY hkey, std::string key, std::string val) {
    char value[1024]; 
    DWORD size = 1024;
    DWORD type = REG_SZ;
    HKEY key_handle;
#if MSVC
    std::wstring wkey = s2ws2(key.c_str());
#else
    std::string wkey  = key;
#endif
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, wkey.c_str(), NULL, KEY_QUERY_VALUE, &key_handle)) {
        return "";
    }
#if MSVC
    std::wstring wval = s2ws2(val.c_str());
#else
    std::string wval  = val;
#endif
    if (RegQueryValueEx(key_handle, wval.c_str(), NULL, &type, (LPBYTE)&value, &size)) {
        return "";
    }
    RegCloseKey(key_handle);

    std::string string_value = "";
    for(unsigned int i = 0; i < size; i++) {
        if (value[i] != '\0' ) {
            string_value += value[i];
        }
    }
    return string_value;
}

////////////////////////////////////////////////////////////
/// Read Binary value
////////////////////////////////////////////////////////////
int Registry::ReadBinValue(HKEY hkey, std::string key, std::string val, unsigned char* bin) {
    DWORD size = 1024;
    DWORD type = REG_BINARY;
    HKEY key_handle;
#if MSVC
    std::wstring wkey = s2ws2(key.c_str());
#else
    std::string wkey  = key;
#endif
    if (RegOpenKeyEx(hkey, wkey.c_str(), NULL, KEY_QUERY_VALUE, &key_handle)) {
        return 0;
    }
#if MSVC
    std::wstring wval = s2ws2(val.c_str());
#else
    std::string wval  = val;
#endif
    if (RegQueryValueEx(key_handle, wval.c_str(), NULL, &type, bin, &size)) {
        return 0;
    }
    RegCloseKey(key_handle);

    return size;
}
