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

// Headers
#include "utils.h"
#include <algorithm>
#include <cctype>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/static_assert.hpp>

using boost::u8_to_u32_iterator;
using boost::u16_to_u32_iterator;
using boost::u32_to_u16_iterator;
using boost::u32_to_u8_iterator;

template <class T>
static T tolower_(T const& v) {
	return tolower((unsigned char)v);
}
template <class T>
static T toupper_(T const& v) {
	return toupper((unsigned char)v);
}

std::string Utils::LowerCase(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), tolower_<char>);
	return result;
}

std::string Utils::UpperCase(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), toupper_<char>);
	return result;
}

Utils::utf16_string Utils::DecodeUTF16(const std::string& str) {
	utf32_string const tmp = DecodeUTF32(str);
	return utf16_string(u32_to_u16_iterator<utf32_string::const_iterator>(tmp.begin()),
	                    u32_to_u16_iterator<utf32_string::const_iterator>(tmp.end()));
}

Utils::utf32_string Utils::DecodeUTF32(const std::string& str) {
	return utf32_string(
	    u8_to_u32_iterator<std::string::const_iterator>(str.begin(), str.begin(), str.end()),
	    u8_to_u32_iterator<std::string::const_iterator>(str.end(), str.begin(), str.end()));
}

std::string Utils::EncodeUTF(const Utils::utf16_string& str) {
	utf32_string const tmp(
	    u16_to_u32_iterator<utf16_string::const_iterator>(str.begin(), str.begin(), str.end()),
	    u16_to_u32_iterator<utf16_string::const_iterator>(str.end(), str.begin(), str.end()));
	return std::string(u32_to_u8_iterator<utf32_string::const_iterator>(tmp.begin()),
	                   u32_to_u8_iterator<utf32_string::const_iterator>(tmp.end()));
}

std::string Utils::EncodeUTF(const Utils::utf32_string& str) {
	return std::string(u32_to_u8_iterator<utf32_string::const_iterator>(str.begin()),
	                   u32_to_u8_iterator<utf32_string::const_iterator>(str.end()));
}

template <size_t WideSize>
static Utils::wstring ToWideStringImpl(const std::string&);
template <> // utf16
Utils::wstring ToWideStringImpl<2>(const std::string& str) {
	Utils::utf16_string const tmp = Utils::DecodeUTF16(str);
	return Utils::wstring(tmp.begin(), tmp.end());
}
template <> // utf32
Utils::wstring ToWideStringImpl<4>(const std::string& str) {
	Utils::utf32_string const tmp = Utils::DecodeUTF32(str);
	return Utils::wstring(tmp.begin(), tmp.end());
}

Utils::wstring Utils::ToWideString(const std::string& str) {
	return ToWideStringImpl<sizeof(wchar_t)>(str);
}

template <size_t WideSize>
static std::string FromWideStringImpl(const Utils::wstring&);
template <> // utf16
std::string FromWideStringImpl<2>(const Utils::wstring& str) {
	return Utils::EncodeUTF(Utils::utf16_string(str.begin(), str.end()));
}
template <> // utf32
std::string FromWideStringImpl<4>(const Utils::wstring& str) {
	return Utils::EncodeUTF(Utils::utf32_string(str.begin(), str.end()));
}

std::string Utils::FromWideString(const Utils::wstring& str) {
	return FromWideStringImpl<sizeof(wchar_t)>(str);
}

bool Utils::IsBigEndian() {
	union {
		uint32_t i;
		char c[4];
	} d = {0x01020304};

	return (d.c[0] == 1);
}
