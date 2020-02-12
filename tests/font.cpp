#include "text.h"
#include "pixel_format.h"
#include "cache.h"
#include "bitmap.h"
#include "font.h"
#include <iostream>
#include "doctest.h"

TEST_SUITE_BEGIN("Font");

constexpr char32_t escape = '\\';
constexpr int width = 240;
constexpr int height = 80;
constexpr int ch = 12;
constexpr int cwh = 6;
constexpr int cwf = 12;

TEST_CASE("FontSizeStr") {
	auto font = Font::Default();

	REQUIRE_EQ(font->GetSize(""), Rect(0, 0, 0, ch));
	REQUIRE_EQ(font->GetSize(" "), Rect(0, 0, cwh, ch));
	REQUIRE_EQ(font->GetSize("\n"), Rect(0, 0, 0, ch));
	REQUIRE_EQ(font->GetSize("$A"), Rect(0, 0, cwh * 2, ch));
	REQUIRE_EQ(font->GetSize("X\nX"), Rect(0, 0, cwh * 2, ch));
}

TEST_CASE("FontSizeChar") {
	auto font = Font::Default();

	REQUIRE_EQ(font->GetSize(0), Rect(0, 0, 0, ch));
	REQUIRE_EQ(font->GetSize(U' '), Rect(0, 0, cwh, ch));
	REQUIRE_EQ(font->GetSize(U'\n'), Rect(0, 0, 0, ch));
	REQUIRE_EQ(font->GetSize(U'X'), Rect(0, 0, cwh, ch));
	REQUIRE_EQ(font->GetSize(U'ぽ'), Rect(0, 0, cwf, ch));
}

TEST_CASE("FontSizeStrEx") {
	auto font = Font::exfont;

	for (char i = 0; i < 52; ++i) {
		REQUIRE_EQ(font->GetSize(std::string(1,i)), Rect(0, 0, cwf, cwf));
	}
}

TEST_CASE("FontSizeCharEx") {
	auto font = Font::exfont;

	for (char32_t i = 0; i < 52; ++i) {
		REQUIRE_EQ(font->GetSize(i), Rect(0, 0, cwf, cwf));
	}
}

TEST_CASE("FontGlyphChar") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto check = [&](char32_t ch, Rect r) {
		auto ret = font->Glyph(ch);
		REQUIRE(ret.bitmap != nullptr);
		REQUIRE_EQ(ret.rect, r);
	};

	check(0, Rect(0, 0, 0, ch));
	check(U' ', Rect(0, 0, cwh, ch));
	check(U'\n', Rect(0, 0, 0, ch));
	check(U'X', Rect(0, 0, cwh, ch));
	check(U'ぽ', Rect(0, 0, cwf, ch));
}

TEST_CASE("FontGlyphCharEx") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::exfont;

	for (char32_t i = 0; i < 52; ++i) {
		auto ret = font->Glyph(i);
		REQUIRE(ret.bitmap != nullptr);
		REQUIRE_EQ(ret.rect, Rect(0, 0, cwf, ch));
	}
}

TEST_CASE("FontGlyphChar") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto system = Cache::SysBlack();
	auto surface = Bitmap::Create(width, height);
	int color = 0;
	auto check = [&](char32_t ch, Rect r) {
		REQUIRE_EQ(font->Render(*surface, 0, 0, *system, color, ch), r);
	};

	check(0, Rect(0, 0, 0, ch));
	check(U' ', Rect(0, 0, cwh, ch));
	check(U'\n', Rect(0, 0, 0, ch));
	check(U'X', Rect(0, 0, cwh, ch));
	check(U'ぽ', Rect(0, 0, cwf, ch));
}

TEST_CASE("FontGlyphCharEx") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::exfont;
	auto system = Cache::SysBlack();
	auto surface = Bitmap::Create(width, height);
	int color = 0;

	for (char32_t i = 0; i < 52; ++i) {
		REQUIRE_EQ(font->Render(*surface, 0, 0, *system, color, ch), Rect(0, 0, cwf, ch));
	}
}

TEST_SUITE_END();
