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
#include "output.h"
#include "compiler.h"
#include <cassert>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <algorithm>
#include <random>
#include <cctype>
#include <zlib.h>

namespace {
	char Lower(char c) {
		if (c >= 'A' && c <= 'Z') {
			return c + 'a' - 'A';
		} else {
			return c;
		}
	}

	char Upper(char c) {
		if (c >= 'a' && c <= 'z') {
			return c + 'A' - 'a';
		} else {
			return c;
		}
	};

}

std::string Utils::LowerCase(StringView str) {
	auto result = std::string(str);
	LowerCaseInPlace(result);
	return result;
}

std::string& Utils::LowerCaseInPlace(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(), Lower);
	return str;
}

std::string Utils::UpperCase(StringView str) {
	auto result = std::string(str);
	UpperCaseInPlace(result);
	return result;
}

std::string& Utils::UpperCaseInPlace(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(), Upper);
	return str;
}

int Utils::StrICmp(const char* l, const char* r) {
	assert(l != nullptr);
	assert(r != nullptr);
	while (*l != '\0' && *r != '\0') {
		auto d = Lower(*l) - Lower(*r);
		if (d != 0) {
			return d;
		}
		++l;
		++r;
	}
	return *l - *r;
}

int Utils::StrICmp(StringView l, StringView r) {
	for (size_t i = 0; i < std::min(l.size(), r.size()); ++i) {
		auto d = Lower(l[i]) - Lower(r[i]);
		if (d != 0) {
			return d;
		}
	}
	return l.size() - r.size();
}

std::u16string Utils::DecodeUTF16(StringView str) {
	std::u16string result;
	for (auto it = str.begin(), str_end = str.end(); it < str_end; ++it) {
		uint8_t c1 = static_cast<uint8_t>(*it);
		if (c1 < 0x80) {
			result.push_back(static_cast<uint16_t>(c1));
		}
		else if (c1 < 0xC2) {
			continue;
		}
		else if (c1 < 0xE0) {
			if (str_end-it < 2)
				break;
			uint8_t c2 = static_cast<uint8_t>(*(++it));
			if ((c2 & 0xC0) != 0x80)
				continue;
			result.push_back(static_cast<uint16_t>(((c1 & 0x1F) << 6) | (c2 & 0x3F)));
		}
		else if (c1 < 0xF0) {
			if (str_end-it < 3)
				break;
			uint8_t c2 = static_cast<uint8_t>(*(++it));
			uint8_t c3 = static_cast<uint8_t>(*(++it));
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
			uint8_t c2 = static_cast<uint8_t>(*(++it));
			uint8_t c3 = static_cast<uint8_t>(*(++it));
			uint8_t c4 = static_cast<uint8_t>(*(++it));
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

std::u32string Utils::DecodeUTF32(StringView str) {
	std::u32string result;
	for (auto it = str.begin(), str_end = str.end(); it < str_end; ++it) {
		uint8_t c1 = static_cast<uint8_t>(*it);
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
			uint8_t c2 = static_cast<uint8_t>(*(++it));
			uint8_t c3 = static_cast<uint8_t>(*(++it));
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
			uint8_t c2 = static_cast<uint8_t>(*(++it));
			uint8_t c3 = static_cast<uint8_t>(*(++it));
			uint8_t c4 = static_cast<uint8_t>(*(++it));
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
			result.push_back(static_cast<char>(wc1));
		}
		else if (wc1 < 0x0800) {
			result.push_back(static_cast<char>(0xC0 | (wc1 >> 6)));
			result.push_back(static_cast<char>(0x80 | (wc1 & 0x03F)));
		}
		else if (wc1 < 0xD800) {
			result.push_back(static_cast<char>(0xE0 |  (wc1 >> 12)));
			result.push_back(static_cast<char>(0x80 | ((wc1 & 0x0FC0) >> 6)));
			result.push_back(static_cast<char>(0x80 |  (wc1 & 0x003F)));
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
			result.push_back(static_cast<char>(0xF0 | (z >> 2)));
			result.push_back(static_cast<char>(0x80 | ((z & 0x03) << 4)     | ((wc1 & 0x003C) >> 2)));
			result.push_back(static_cast<char>(0x80 | ((wc1 & 0x0003) << 4) | ((wc2 & 0x03C0) >> 6)));
			result.push_back(static_cast<char>(0x80 |  (wc2 & 0x003F)));
		}
		else if (wc1 < 0xE000) {
			continue;
		}
		else {
			result.push_back(static_cast<char>(0xE0 |  (wc1 >> 12)));
			result.push_back(static_cast<char>(0x80 | ((wc1 & 0x0FC0) >> 6)));
			result.push_back(static_cast<char>(0x80 |  (wc1 & 0x003F)));
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
			result.push_back(static_cast<char>(wc));
		}
		else if (wc < 0x000800) {
			result.push_back(static_cast<char>(0xC0 | (wc >> 6)));
			result.push_back(static_cast<char>(0x80 | (wc & 0x03F)));
		}
		else if (wc < 0x010000) {
			result.push_back(static_cast<char>(0xE0 |  (wc >> 12)));
			result.push_back(static_cast<char>(0x80 | ((wc & 0x0FC0) >> 6)));
			result.push_back(static_cast<char>(0x80 |  (wc & 0x003F)));
		}
		else {
			result.push_back(static_cast<char>(0xF0 |  (wc >> 18)));
			result.push_back(static_cast<char>(0x80 | ((wc & 0x03F000) >> 12)));
			result.push_back(static_cast<char>(0x80 | ((wc & 0x000FC0) >> 6)));
			result.push_back(static_cast<char>(0x80 |  (wc & 0x00003F)));
		}
	}
	return result;
}

Utils::UtfNextResult Utils::UTF8Next(const char* iter, const char* const end) {
	while (iter != end) {
		uint8_t c1 = static_cast<uint8_t>(*iter);
		++iter;
		if (c1 < 0x80) {
			return { iter, static_cast<uint32_t>(c1) };
		}
		if (c1 < 0xC2) {
			continue;
		}
		if (iter == end) {
			break;
		}
		uint8_t c2 = static_cast<uint8_t>(*iter);
		++iter;
		if (c1 < 0xE0) {
			if ((c2 & 0xC0) != 0x80)
				continue;
			auto ch = (static_cast<uint32_t>(((c1 & 0x1F) << 6) | (c2 & 0x3F)));
			return { iter, ch };
		}
		if (iter == end) {
			break;
		}
		uint8_t c3 = static_cast<uint8_t>(*iter);
		++iter;
		if (c1 < 0xF0) {
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
			auto ch = (static_cast<uint32_t>(((c1 & 0x0F) << 12)
						| ((c2 & 0x3F) << 6)
						|  (c3 & 0x3F)));
			return { iter, ch };
		}
		if (iter == end) {
			break;
		}
		uint8_t c4 = static_cast<uint8_t>(*iter);
		++iter;
		if (c1 < 0xF5) {
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
		auto ch = static_cast<uint32_t>(((c1 & 0x07) << 18)
				| ((c2 & 0x3F) << 12)
				| ((c3 & 0x3F) << 6)
				|  (c4 & 0x3F));
		return { iter, ch };
		}
	}
	return { iter, 0 };
}

Utils::ExFontRet Utils::ExFontNext(const char* iter, const char* end) {
	ExFontRet ret;
	if (end - iter >= 2 && *iter == '$') {
		auto next_ch = *(iter + 1);
		// Don't use std::isalpha, because it's indirects based on locale.
		bool is_lower = (next_ch >= 'a' && next_ch <= 'z');
		bool is_upper = (next_ch >= 'A' && next_ch <= 'Z');
		if (is_lower || is_upper) {
			ret.next = iter + 2;
			ret.value = next_ch;
			ret.is_valid = true;
		}
	}
	return ret;
}

Utils::TextRet Utils::TextNext(const char* iter, const char* end, char32_t escape) {
	TextRet ret;

	if (EP_UNLIKELY(iter == end)) {
		ret.next = iter;
		return ret;
	}

	auto ex_ret = ExFontNext(iter, end);
	if (ex_ret) {
		ret.next = ex_ret.next;
		ret.ch = ex_ret.value;
		ret.is_exfont = true;
		return ret;
	}

	auto utf8_ret = UTF8Next(iter, end);
	ret.next = utf8_ret.next;
	ret.ch = utf8_ret.ch;

	if (escape != 0 && ret.ch == escape && ret.next != end) {
		auto eret = UTF8Next(ret.next, end);
		ret.next = eret.next;
		ret.ch = eret.ch;
		ret.is_escape = true;
	}
	ret.is_exfont = false;

	return ret;
}

#if _WIN32

template<size_t WideSize>
static std::wstring ToWideStringImpl(StringView);
#if __SIZEOF_WCHAR_T__ == 4 || __WCHAR_MAX__ > 0x10000
template<> // utf32
std::wstring ToWideStringImpl<4>(StringView str) {
	const auto tmp = Utils::DecodeUTF32(str);
	return std::wstring(tmp.begin(), tmp.end());
}
#else
template<> // utf16
std::wstring ToWideStringImpl<2>(StringView str) {
	const auto tmp = Utils::DecodeUTF16(str);
	return std::wstring(tmp.begin(), tmp.end());
}
#endif

std::wstring Utils::ToWideString(StringView str) {
	return ToWideStringImpl<sizeof(wchar_t)>(str);
}

template<size_t WideSize>
static std::string FromWideStringImpl(const std::wstring&);
#if __SIZEOF_WCHAR_T__ == 4 || __WCHAR_MAX__ > 0x10000
template<> // utf32
std::string FromWideStringImpl<4>(const std::wstring& str) {
	return Utils::EncodeUTF(std::u32string(str.begin(), str.end()));
}
#else
template<> // utf16
std::string FromWideStringImpl<2>(const std::wstring& str) {
	return Utils::EncodeUTF(std::u16string(str.begin(), str.end()));
}
#endif

std::string Utils::FromWideString(const std::wstring& str) {
	return FromWideStringImpl<sizeof(wchar_t)>(str);
}

#endif

int Utils::PositiveModulo(int i, int m) {
	return (i % m + m) % m;
}

bool Utils::IsBigEndian() {
	union {
		uint32_t i;
		char c[4];
	} d = {0x01020304};

	return d.c[0] == 1;
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

// based on https://stackoverflow.com/questions/6089231/
bool Utils::ReadLine(std::istream& is, std::string& line_out) {
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	if (!is) {
		return false;
	}

	line_out.clear();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return true;
		case '\r':
			if (sb->sgetc() == '\n') {
				sb->sbumpc();
			}
			return true;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (line_out.empty()) {
				is.setstate(std::ios::eofbit);
				return false;
			}
			return true;
		default:
			line_out += (char)c;
		}
	}
}

std::vector<std::string> Utils::Tokenize(StringView str_to_tokenize, const std::function<bool(char32_t)> predicate) {
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

std::vector<uint8_t> Utils::ReadStream(std::istream& stream) {
	constexpr int buffer_incr = 8192;
	std::vector<uint8_t> outbuf;

	do {
		outbuf.resize(outbuf.size() + buffer_incr);
		stream.read(reinterpret_cast<char*>(outbuf.data() + outbuf.size() - buffer_incr), buffer_incr);
	} while (stream.gcount() == buffer_incr);

	outbuf.resize(outbuf.size() - buffer_incr + stream.gcount());

	return outbuf;
}

uint32_t Utils::CRC32(std::istream& stream) {
	uLong crc = crc32(0L, Z_NULL, 0);
	std::array<uint8_t, 8192> buffer = {};
	do {
		stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		crc = crc32(crc, buffer.data(), stream.gcount());
	} while (stream.gcount() == static_cast<std::streamsize>(buffer.size()));
	return crc;
}

// via https://stackoverflow.com/q/3418231/
std::string Utils::ReplaceAll(std::string str, const std::string& search, const std::string& replace) {
    size_t start_pos = 0;
    while((start_pos = str.find(search, start_pos)) != std::string::npos) {
        str.replace(start_pos, search.length(), replace);
        start_pos += replace.length(); // Handles case where 'replace' is a substring of 'search'
    }
    return str;
}

std::string Utils::ReplacePlaceholders(StringView text_template, Span<const char> types, Span<const StringView> values) {
	auto str = std::string(text_template);
	size_t index = str.find("%");
	while (index != std::string::npos) {
		if (index + 1 < str.length()) {
			char type = str[index + 1];
			if (type != '%') {
				auto v_it = values.begin();
				for (auto t_it = types.begin();
					t_it != types.end() && v_it != values.end();
					++t_it, ++v_it) {
					if (std::toupper(type) == *t_it) {
						str.replace(index, 2, v_it->data(), v_it->size());
						index += (*v_it).length() - 2;
						break;
					}
				}
			}
		}

		index = str.find("%", index + 1);
	}

	return str;
}

StringView Utils::TrimWhitespace(StringView s) {
	size_t left = 0;
	for (auto& c: s) {
		if (std::isspace(static_cast<int>(c))) {
			++left;
		} else {
			break;
		}
	}
	s.remove_prefix(left);

	size_t right = 0;
	for (auto it = s.crbegin(); it != s.crend(); ++it) {
		if (std::isspace(static_cast<int>(*it))) {
			++right;
		} else {
			break;
		}
	}
	s.remove_suffix(right);

	return s;
}

std::string Utils::FormatDate(const std::tm *tm, StringView format) {
	constexpr int buf_size = 128;
	char buffer[buf_size];

	auto res = strftime(buffer, buf_size, ToString(format).c_str(), tm);

	return std::string(buffer, res);
}
