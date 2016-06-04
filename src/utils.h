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
#include <sstream>
#include "system.h"

namespace Utils {
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
	std::u16string DecodeUTF16(const std::string& str);

	/**
	 * Converts UTF-8 to UTF-32.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::u32string DecodeUTF32(const std::string& str);

	/**
	 * Converts UTF-16 to UTF-8.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string EncodeUTF(const std::u16string& str);

	/**
	 * Converts UTF-32 to UTF-8.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string EncodeUTF(const std::u32string& str);

	/**
	 * Converts UTF-8 string to std::wstring.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::wstring ToWideString(const std::string& str);

	/**
	 * Converts std::wstring to UTF-8 string.
	 *
	 * @param str string to convert.
	 * @return the converted string.
	 */
	std::string FromWideString(const std::wstring& str);


	/**
	 * Converts arithmetic types to a string.
	 *
	 * @param n arithmetic type to convert.
	 * @return the converted string
	 */
	template<typename T>
	std::string ToString(const T& n) {
		std::ostringstream stm ;
		stm << n ;
		return stm.str();
	}

	bool IsBigEndian();

	/**
	 * Gets a random number in the inclusive range from - to.
	 *
	 * @param from Interval start
	 * @param to Interval end
	 * @return Random number in inclusive interval
	 */
	int32_t GetRandomNumber(int32_t from, int32_t to);

	/**
	 * Seeds the RNG used by GetRandomNumber
	 *
	 * @param seed Seed to use
	 */
	void SeedRandomNumberGenerator(int32_t seed);

} // namespace Utils

#endif
