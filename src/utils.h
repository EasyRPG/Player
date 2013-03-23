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

#ifndef _EASYRPG_UTILS_H_
#define _EASYRPG_UTILS_H_

#include <string>
#include "system.h"

namespace Utils {
	typedef std::basic_string<uint16_t> utf16_string;
	typedef std::basic_string<uint32_t> utf32_string;

#ifdef BOOST_NO_CWCHAR
	typedef std::basic_string<uint32_t> wstring;
#else
	using std::wstring;
#endif

	/**
	 * Converts a string to lower case.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string LowerCase(const std::string& str);

	/**
	 * Converts a string to upper case.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string UpperCase(const std::string& str);

	/**
	 * Converts Utf8 to UTF-16.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	utf16_string DecodeUTF16(const std::string& str);

	/**
	 * Converts UTF-8 to UTF-32.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	utf32_string DecodeUTF32(const std::string& str);

	/**
	 * Converts UTF-16 to UTF-8.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string EncodeUTF(const utf16_string& str);

	/**
	 * Converts UTF-32 to UTF-8.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string EncodeUTF(const utf32_string& str);

	/**
	 * Converts UTF-8 string to wstring.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	wstring ToWideString(const std::string& str);

	/**
	 * Converts wstring to UTF-8 string.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string FromWideString(const wstring& str);


	bool IsBigEndian();

} // namespace Utils

#endif
