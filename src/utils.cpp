/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <cctype>

#include "utils.h"

////////////////////////////////////////////////////////////
std::wstring Utils::DecodeUTF(const std::string& str) {
	std::wstring wstr;
	std::string::const_iterator it;
	for (it = str.begin(); it != str.end(); ) {
		int c = (int) (unsigned char) *it++;
		if (c < 0x80)
			wstr.push_back((wchar_t) c);
		else if ((c & 0xE0) == 0xC0) {
			int c2 = (int) (unsigned char) *it++;
			wstr.push_back((wchar_t) ((c & 0x1F) << 6) + (c2 & 0x3F));
		}
		else if ((c & 0xF0) == 0xE0) {
			int c2 = (int) (unsigned char) *it++;
			int c3 = (int) (unsigned char) *it++;
			wstr.push_back((wchar_t) ((c & 0x0F) << 12) + ((c2 & 0x3F) << 6) + (c3 & 0x3F));
		}
		else
			return std::wstring(L"");
	}

	return wstr;
}

std::string Utils::EncodeUTF(const std::wstring& wstr) {
	std::string str;
	std::wstring::const_iterator it;
	for (it = wstr.begin(); it != wstr.end(); it++) {
		int c = (int) (wchar_t) *it;
		if (c < 0x80)
			str.push_back((char) c);
		else if (c < 0x800) {
			str.push_back((char) (0xC0 | (c >> 6)));
			str.push_back((char) (c & 0x3F));
		}
		else if (c < 0x10000) {
			str.push_back((char) (0xE0 | (c >> 12)));
			str.push_back((char) ((c >> 6) & 0x3F));
			str.push_back((char) (c & 0x3F));
		}
		else
			return std::string("");
	}

	return str;
}

////////////////////////////////////////////////////////////
int Utils::GetUtf8ByteSize(char glyph) {
	// Detect unicode size
	int val = (unsigned char)glyph;
	if (val < 0x80) {
		return 1;
	} else if (val & 0xC0) {
		return 2;
	} else if (val & 0xE0) {
		return 3;
	} else if (val & 0xF0) {
		return 4;
	}

	return 0;
}

////////////////////////////////////////////////////////////
std::string Utils::LowerCase(const std::string& str) {
	std::string result = str;
	std::string::iterator it;
	for (it = result.begin(); it != result.end(); it++)
		*it = tolower(*it);
	return result;
}

