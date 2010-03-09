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
#include "msgbox.h"
#include <windows.h>

////////////////////////////////////////////////////////////
/// MSVC Unicode std::string to LPCWSTR
////////////////////////////////////////////////////////////
#ifdef MSVC
static std::wstring s2ws(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
#else
static std::string s2ws(const std::string& s) {
    return s;
}
#endif

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string msg, std::string title) {
    MessageBox(NULL, s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK);
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string msg, std::string title) {
    MessageBox(NULL, s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK | MB_ICONERROR);
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string msg, std::string title) {
    MessageBox(NULL, s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK | MB_ICONEXCLAMATION);
}
