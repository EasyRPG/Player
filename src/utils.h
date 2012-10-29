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
#include <algorithm>
#include <cctype>

#include "system.h"

namespace Utils {
	typedef std::basic_string<uint16_t> utf16_string;
	typedef std::basic_string<uint32_t> utf32_string;

#ifdef BOOST_NO_CWCHAR
	typedef std::basic_string<uint32_t> wstring;
#else
	using std::wstring;
#endif

	////////////////////////////////////////////////////////
	/// Converts a string to lower case.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string LowerCase(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts a string to upper case.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string UpperCase(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf8 to Utf16.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	utf16_string DecodeUTF16(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf8 to Utf32.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	utf32_string DecodeUTF32(const std::string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf16 to Utf8.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string EncodeUTF(const utf16_string& str);

	////////////////////////////////////////////////////////
	/// Converts Utf32 to Utf8.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	std::string EncodeUTF(const utf32_string& str);

	////////////////////////////////////////////////////////
	/// Converts utf8 string to wstring.
	/// @param str : String to convert.
	/// @return The converted string
	////////////////////////////////////////////////////////
	wstring ToWideString(const std::string& str);
} // namespace Utils

#endif
