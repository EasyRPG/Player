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

#ifndef EP_UTILS_H
#define EP_UTILS_H

#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <random>
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
	 * Tests if a string starts with a substring.
	 *
	 * @param str String to search in
	 * @param end Substring to check at the start of str
	 * @return true when the start matches
	 */
	bool StartsWith(const std::string& str, const std::string& end);

	/**
	 * Tests if a string ends with a substring.
	 *
	 * @param str String to search in
	 * @param end Substring to check at the end of str
	 * @return true when the end matches
	 */
	bool EndsWith(const std::string& str, const std::string& end);

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

	struct UtfNextResult {
		uint32_t ch;
		const char* iter;
	};

	/**
	 * Decodes the next UTF-8 character and returns it and the iterator to the next
	 *
	 * @param iter begginning of the range to convert from
	 * @param end end of the range to convert from
	 * @return the converted string.
	 */
	UtfNextResult UTF8Next(const char* iter, const char* end);

#if !defined(__amigaos4__) && !defined(__AROS__)
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
#endif

	/**
	 * Calculates the modulo of number i ensuring the result is non-negative
	 * for all values of i when m > 0.
	 *
	 * via: https://stackoverflow.com/q/14997165
	 *
	 * @param i Number
	 * @param m Modulo
	 * @return positive modulo of i % m
	 */
	int PositiveModulo(int i, int m);

	/**
	 * Checks if the platform is big endian
	 *
	 * @return true if big, false if little endian
	 */
	bool IsBigEndian();

	/**
	 * Swaps the byte order of the passed number when on big endian systems.
	 * Does nothing otherwise.
	 *
	 * @param us Number to swap
	 */
	void SwapByteOrder(uint16_t& us);

	/**
	 * Swaps the byte order of the passed number when on big endian systems.
	 * Does nothing otherwise.
	 *
	 * @param ui Number to swap
	 */
	void SwapByteOrder(uint32_t& ui);

	/**
	 * Swaps the byte order of the passed number when on big endian systems.
	 * Does nothing otherwise.
	 *
	 * @param d Number to swap
	 */
	void SwapByteOrder(double& d);

	/**
	 * Gets a random number in the inclusive range from - to.
	 *
	 * @param from Interval start
	 * @param to Interval end
	 * @return Random number in inclusive interval
	 */
	int32_t GetRandomNumber(int32_t from, int32_t to);

	/**
	 * Gets the seeded Random Number Generator (RNG).
	 *
	 * @return the random number generator
	 */
	std::mt19937 &GetRNG();

	/**
	 * Has an n/m chance of returning true. If n>m, always returns true.
	 *
	 * @param n number of times out of m to return true (non-negative)
	 * @param m denominator of the probability (positive)
	 * @return true with probability n/m, false with probability 1-n/m
	 */
	bool ChanceOf(int32_t n, int32_t m);


	/**
	 * Rolls a random number in [0.0f, 1.0f) returns true if it's less than rate.
	 *
	 * @param rate a value in [0.0f, 1.0f]. Values out of this range are clamped.
	 * @return true with probability rate.
	 */
	bool PercentChance(float rate);

	/**
	 * Rolls a random number in [0, 99] and returns true if it's less than rate.
	 *
	 * @param rate a value in [0, 100]. Values out of this range are clamped.
	 * @return true with probability rate.
	 */
	bool PercentChance(int rate);
	bool PercentChance(long rate);

	/**
	 * Seeds the RNG used by GetRandomNumber and ChanceOf.
	 *
	 * @param seed Seed to use
	 */
	void SeedRandomNumberGenerator(int32_t seed);

	/**
	 * Reads a line from a stream and returns it.
	 * Same as std::getline but handles linebreaks independent of the platform
	 * correctly.
	 *
	 * @param is Input stream to read
	 * @return Content of the read line
	 */
	std::string ReadLine(std::istream& is);

	/**
	 * Splits a string into tokens specified by a predicate function.
	 *
	 * @param str_to_tokenize String that is tokenized
	 * @param predicate Predicate function, must return true when the character is used for splitting.
	 * @return vector containing the elements between the tokens
	 */
	std::vector<std::string> Tokenize(const std::string& str_to_tokenize, const std::function<bool(char32_t)> predicate);

	/*
	 * Searches for newlines and calls f(const std::string&) for each line.
	 *
	 * @param line the line to parse.
	 * @param f function of type void(const std::string&)
	 */
	template <typename F>
	void ForEachLine(const std::string& line, F&& f);


	/**
	 * Reads a stream until EOF and returns the read bytes.
	 *
	 * @param stream input stream to read
	 * @return content read from stream
	 */
	std::vector<uint8_t> ReadStream(std::istream& stream);

	/**
	 * Replaces placeholders (like %S, %O, %V, %U) in strings.
	 *
	 * @param text_template String with placeholders to replace.
	 * @param types Vector of uppercase characters like 'S',
	 * 'O', 'V', 'U'. Should have the same number of elements
	 * as the values param.
	 * @param values Vector of replacements strings,
	 * should match types in number of elements and order.
	 * @return A new string with placeholders replaced.
	 */
	std::string ReplacePlaceholders(const std::string& text_template, std::vector<char> types, std::vector<std::string> values);

	/**
	 * @return value clamped between min and max
	 */
	template <typename T> T Clamp(T value, const T& minv, const T& maxv);

} // namespace Utils

template <typename T>
inline T Utils::Clamp(T value, const T& minv, const T& maxv) {
	return (value < minv) ? (minv) : ((value > maxv) ? maxv : value);
}

inline bool Utils::PercentChance(long rate) {
	return Utils::PercentChance(static_cast<int>(rate));
}

template <typename F>
inline void Utils::ForEachLine(const std::string& line, F&& f) {
	int next = 0;
	do {
		auto idx = line.find('\n', next);
		if (idx == std::string::npos) {
			if (next == 0) {
				// Optimize the common case
				f(line);
				break;
			}
			idx = line.size();
		}
		f(line.substr(next, idx - next));
		next = idx + 1;
	} while(next < line.size());
}

#endif
