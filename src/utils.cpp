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
#include <random>

namespace {
	std::mt19937 rng;
}

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

bool Utils::EndsWith(const std::string &str, const std::string &end) {
	return str.length() >= end.length() &&
		0 == str.compare(str.length() - end.length(), end.length(), end);
}

std::u16string Utils::DecodeUTF16(const std::string& str) {
	std::u16string result;
	for (auto it = str.begin(), str_end = str.end(); it < str_end; ++it) {
		uint8_t c1 = *it;
		if (c1 < 0x80) {
			result.push_back(static_cast<uint16_t>(c1));
		}
		else if (c1 < 0xC2) {
			continue;
		}
		else if (c1 < 0xE0) {
			if (str_end-it < 2)
				break;
			uint8_t c2 = *(++it);
			if ((c2 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint16_t>(((c1 & 0x1F) << 6) | (c2 & 0x3F)));
		}
		else if (c1 < 0xF0) {
			if (str_end-it < 3)
				break;
			uint8_t c2 = *(++it);
			uint8_t c3 = *(++it);
			if (c1 == 0xE0) {
				if ((c2 & 0xE0) != 0xA0)
					continue;
			} else if (c1 == 0xED) {
				if ((c2 & 0xE0) != 0x80)
					continue;
			} else {
				if ((c2 & 0xC0) != 0x80)
					continue;
			}
			if ((c3 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint16_t>(((c1 & 0x0F) << 12)
												 | ((c2 & 0x3F) << 6)
												 |  (c3 & 0x3F)));
		}
		else if (c1 < 0xF5) {
			if (str_end-it < 4)
				break;
			uint8_t c2 = *(++it);
			uint8_t c3 = *(++it);
			uint8_t c4 = *(++it);
			if (c1 == 0xF0) {
				if (!(0x90 <= c2 && c2 <= 0xBF))
					continue;
			} else if (c1 == 0xF4) {
				if ((c2 & 0xF0) != 0x80)
					continue;
			} else {
				if ((c2 & 0xC0) != 0x80)
					continue;
			}
			if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
				continue;
			if ((((c1 & 7UL) << 18) +
				((c2 & 0x3FUL) << 12) +
				((c3 & 0x3FUL) << 6) + (c4 & 0x3F)) > 0x10FFFF)
				continue;
			result.push_back(static_cast<uint16_t>(
					0xD800
				  | (((((c1 & 0x07) << 2) | ((c2 & 0x30) >> 4)) - 1) << 6)
				  | ((c2 & 0x0F) << 2)
				  | ((c3 & 0x30) >> 4)));
			result.push_back(static_cast<uint16_t>(
					0xDC00
				  | ((c3 & 0x0F) << 6)
				  |  (c4 & 0x3F)));
		}
	}
	return result;
}

std::u32string Utils::DecodeUTF32(const std::string& str) {
	std::u32string result;
	for (auto it = str.begin(), str_end = str.end(); it < str_end; ++it) {
		uint8_t c1 = *it;
		if (c1 < 0x80) {
			result.push_back(static_cast<uint32_t>(c1));
		}
		else if (c1 < 0xC2) {
			continue;
		}
		else if (c1 < 0xE0) {
			if (str_end-it < 2)
				break;
			uint8_t c2 = it[1];
			if ((c2 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint32_t>(((c1 & 0x1F) << 6)
												  | (c2 & 0x3F)));
		}
		else if (c1 < 0xF0) {
			if (str_end-it < 3)
				break;
			uint8_t c2 = *(++it);
			uint8_t c3 = *(++it);
			if (c1 == 0xE0) {
				if ((c2 & 0xE0) != 0xA0)
					continue;
			} else if (c1 == 0xED) {
				if ((c2 & 0xE0) != 0x80)
					continue;
			} else {
				if ((c2 & 0xC0) != 0x80)
					continue;
			}
			if ((c3 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint32_t>(((c1 & 0x0F) << 12)
												 | ((c2 & 0x3F) << 6)
												 |  (c3 & 0x3F)));
		}
		else if (c1 < 0xF5) {
			if (str_end-it < 4)
				break;
			uint8_t c2 = *(++it);
			uint8_t c3 = *(++it);
			uint8_t c4 = *(++it);
			if (c1 == 0xF0) {
				if (!(0x90 <= c2 && c2 <= 0xBF))
					continue;
			} else if (c1 == 0xF4) {
				if ((c2 & 0xF0) != 0x80)
					continue;
			} else {
				if ((c2 & 0xC0) != 0x80)
					continue;
			}
			if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint32_t>(((c1 & 0x07) << 18)
												 | ((c2 & 0x3F) << 12)
												 | ((c3 & 0x3F) << 6)
												 |  (c4 & 0x3F)));
		}
	}
	return result;
}

std::string Utils::EncodeUTF(const std::u16string& str) {
	std::string result;
	for (auto it = str.begin(), str_end = str.end(); it < str_end; ++it) {
		uint16_t wc1 = *it;
		if (wc1 < 0x0080) {
			result.push_back(static_cast<uint8_t>(wc1));
		}
		else if (wc1 < 0x0800) {
			result.push_back(static_cast<uint8_t>(0xC0 | (wc1 >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 | (wc1 & 0x03F)));
		}
		else if (wc1 < 0xD800) {
			result.push_back(static_cast<uint8_t>(0xE0 |  (wc1 >> 12)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 |  (wc1 & 0x003F)));
		}
		else if (wc1 < 0xDC00) {
			if (str_end-it < 2)
				break;
			uint16_t wc2 = *(++it);
			if ((wc2 & 0xFC00) != 0xDC00)
				continue;
			if (((((wc1 & 0x03C0UL) >> 6) + 1) << 16) +
				((wc1 & 0x003FUL) << 10) + (wc2 & 0x03FF) > 0x10FFFF)
				continue;
			uint8_t z = ((wc1 & 0x03C0) >> 6) + 1;
			result.push_back(static_cast<uint8_t>(0xF0 | (z >> 2)));
			result.push_back(static_cast<uint8_t>(0x80 | ((z & 0x03) << 4)     | ((wc1 & 0x003C) >> 2)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc1 & 0x0003) << 4) | ((wc2 & 0x03C0) >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 |  (wc2 & 0x003F)));
		}
		else if (wc1 < 0xE000) {
			continue;
		}
		else {
			result.push_back(static_cast<uint8_t>(0xE0 |  (wc1 >> 12)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 |  (wc1 & 0x003F)));
		}
	}
	return result;
}

std::string Utils::EncodeUTF(const std::u32string& str) {
	std::string result;
	for (const char32_t& wc : str) {
		if ((wc & 0xFFFFF800) == 0x00D800 || wc > 0x10FFFF)
			break;
		if (wc < 0x000080) {
			result.push_back(static_cast<uint8_t>(wc));
		}
		else if (wc < 0x000800) {
			result.push_back(static_cast<uint8_t>(0xC0 | (wc >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 | (wc & 0x03F)));
		}
		else if (wc < 0x010000) {
			result.push_back(static_cast<uint8_t>(0xE0 |  (wc >> 12)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 |  (wc & 0x003F)));
		}
		else {
			result.push_back(static_cast<uint8_t>(0xF0 |  (wc >> 18)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc & 0x03F000) >> 12)));
			result.push_back(static_cast<uint8_t>(0x80 | ((wc & 0x000FC0) >> 6)));
			result.push_back(static_cast<uint8_t>(0x80 |  (wc & 0x00003F)));
		}
	}
	return result;
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
	static bool ran_once = false;
	static bool is_big = false;

	if (ran_once) {
		return is_big;
	}

	union {
		uint32_t i;
		char c[4];
	} d = {0x01020304};

	ran_once = true;
	is_big = d.c[0] == 1;

	return is_big;
}

void Utils::SwapByteOrder(uint16_t& us) {
	if (!IsBigEndian()) {
		return;
	}

	us =	(us >> 8) |
			(us << 8);
}

void Utils::SwapByteOrder(uint32_t& ui) {
	if (!IsBigEndian()) {
		return;
	}

	ui =	(ui >> 24) |
			((ui<<8) & 0x00FF0000) |
			((ui>>8) & 0x0000FF00) |
			(ui << 24);
}

void Utils::SwapByteOrder(double& d) {
	if (!IsBigEndian()) {
		return;
	}

	uint32_t *p = reinterpret_cast<uint32_t *>(&d);
	SwapByteOrder(p[0]);
	SwapByteOrder(p[1]);
	uint32_t tmp = p[0];
	p[0] = p[1];
	p[1] = tmp;
}

int32_t Utils::GetRandomNumber(int32_t from, int32_t to) {
	std::uniform_int_distribution<int32_t> dist(from, to);
	return dist(rng);
}

void Utils::SeedRandomNumberGenerator(int32_t seed) {
	rng.seed(seed);
}

#if (defined(_3DS) || defined(PSP2))
#  define EOF -1
#endif

// via https://stackoverflow.com/questions/6089231/
std::string Utils::ReadLine(std::istream &is) {
	std::string out;

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
        	int c = sb->sbumpc();
		switch (c) {
			case '\n':
			return out;
		case '\r':
			if (sb->sgetc() == '\n') {
				sb->sbumpc();
			}
			return out;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (out.empty()) {
				is.setstate(std::ios::eofbit);
			}
			return out;
		default:
			out += (char)c;
		}
	}
}

std::vector<std::string> Utils::Tokenize(const std::string &str_to_tokenize, const std::function<bool(char32_t)> predicate) {
	std::u32string text = DecodeUTF32(str_to_tokenize);
	std::vector<std::string> tokens;
	std::u32string cur_token;	

	for (char32_t& c : text) {
		if (predicate(c)) {
			tokens.push_back(EncodeUTF(cur_token));
			cur_token.clear();
			continue;
		}

		cur_token.push_back(c);
	}

	tokens.push_back(EncodeUTF(cur_token));

	return tokens;
}
