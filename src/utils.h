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

#ifndef _EASYRPG_UTILS_H_
#define _EASYRPG_UTILS_H_

#include <string>

#if defined(DINGOO)
#define wstring basic_string<wchar_t>
#endif

namespace Utils {
	////////////////////////////////////////////////////////
	/// Converts a string to lower case.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string LowerCase(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf8 to Utf16 (Windows) or Utf32 (Unix).
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::wstring DecodeUTF(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf16 (Windows) or Utf32 (Unix) to Utf8.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string EncodeUTF(const std::wstring& wstr);
}

#endif

