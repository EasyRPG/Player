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

#ifdef BOOST_NO_EXCEPTIONS
#  include <exception>
#  include <boost/throw_exception.hpp>
#  include "output.h"

	void boost::throw_exception(std::exception const& exp) {
		Output::Error("exception: %s", exp.what());
	}
#endif

using boost::u8_to_u32_iterator;
using boost::u16_to_u32_iterator;
using boost::u32_to_u16_iterator;
using boost::u32_to_u8_iterator;

std::string Utils::LowerCase(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), tolower);
	return result;
}

std::string Utils::UpperCase(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), toupper);
	return result;
}

std::u16string Utils::DecodeUTF16(const std::string& str) {
	std::u32string const tmp = DecodeUTF32(str);
	return std::u16string(u32_to_u16_iterator<std::u32string::const_iterator>(tmp.begin()),
						u32_to_u16_iterator<std::u32string::const_iterator>(tmp.end  ()));
}

std::u32string Utils::DecodeUTF32(const std::string& str) {
	return std::u32string(u8_to_u32_iterator<std::string::const_iterator>(str.begin(), str.begin(), str.end()),
						u8_to_u32_iterator<std::string::const_iterator>(str.end  (), str.begin(), str.end()));
}

std::string Utils::EncodeUTF(const std::u16string& str) {
	std::u32string const tmp(u16_to_u32_iterator<std::u16string::const_iterator>(str.begin(), str.begin(), str.end()),
						   u16_to_u32_iterator<std::u16string::const_iterator>(str.end  (), str.begin(), str.end()));
	return std::string(u32_to_u8_iterator<std::u32string::const_iterator>(tmp.begin()),
					   u32_to_u8_iterator<std::u32string::const_iterator>(tmp.end  ()));
}

std::string Utils::EncodeUTF(const std::u32string& str) {
	return std::string(u32_to_u8_iterator<std::u32string::const_iterator>(str.begin()),
					   u32_to_u8_iterator<std::u32string::const_iterator>(str.end  ()));
}

template<size_t WideSize>
static std::wstring ToWideStringImpl(const std::string&);
template<> // utf16
std::wstring ToWideStringImpl<2>(const std::string& str) {
	std::u16string const tmp = Utils::DecodeUTF16(str);
	return std::wstring(tmp.begin(), tmp.end());
}
template<> // utf32
std::wstring ToWideStringImpl<4>(const std::string& str) {
	std::u32string const tmp = Utils::DecodeUTF32(str);
	return std::wstring(tmp.begin(), tmp.end());
}

std::wstring Utils::ToWideString(const std::string& str) {
	return ToWideStringImpl<sizeof(wchar_t)>(str);
}

template<size_t WideSize>
static std::string FromWideStringImpl(const std::wstring&);
template<> // utf16
std::string FromWideStringImpl<2>(const std::wstring& str) {
	return Utils::EncodeUTF(std::u16string(str.begin(), str.end()));
}
template<> // utf32
std::string FromWideStringImpl<4>(const std::wstring& str) {
	return Utils::EncodeUTF(std::u32string(str.begin(), str.end()));
}

std::string Utils::FromWideString(const std::wstring& str) {
	return FromWideStringImpl<sizeof(wchar_t)>(str);
}

bool Utils::IsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } d = {0x01020304};

    return(d.c[0] == 1);
}
