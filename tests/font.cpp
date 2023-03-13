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

TEST_CASE("FontSizeChar") {
	auto font = Font::Default();

	REQUIRE_EQ(font->GetSize(0), Rect(0, 0, 0, 0));
	REQUIRE_EQ(font->GetSize(U' '), Rect(0, 0, cwh, ch));
	REQUIRE_EQ(font->GetSize(U'\n'), Rect(0, 0, 0, ch));
	REQUIRE_EQ(font->GetSize(U'X'), Rect(0, 0, cwh, ch));
	REQUIRE_EQ(font->GetSize(U'ぽ'), Rect(0, 0, cwf, ch));
	REQUIRE_EQ(font->GetSize(U'下'), Rect(0, 0, cwf, ch));
}

TEST_CASE("FontSizeCharEx") {
	auto font = Font::exfont;

	for (char32_t i = 'a'; i <= 'z'; ++i) {
		REQUIRE_EQ(font->GetSize(i), Rect(0, 0, cwf, cwf));
	}

	for (char32_t i = 'A'; i <= 'Z'; ++i) {
		REQUIRE_EQ(font->GetSize(i), Rect(0, 0, cwf, cwf));
	}
}

TEST_CASE("FontGlyphChar") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto check = [&](char32_t ch, Point p) {
		auto ret = font->vRender(ch);
		REQUIRE(ret.bitmap != nullptr);
		REQUIRE_EQ(ret.advance, p);
	};

	check(U' ', Point(cwh, 0));
	check(U'\n', Point(cwh, 0));
	check(U'X', Point(cwh, 0));
	check(U'ぽ', Point(cwf, 0));
	check(U'下', Point(cwf, 0));
}

TEST_CASE("FontGlyphCharEx") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::exfont;

	for (char32_t i = 'a'; i <= 'z'; ++i) {
		auto ret = font->vRender(i);
		REQUIRE(ret.bitmap != nullptr);
		REQUIRE_EQ(ret.advance, Point(cwf, 0));
	}

	for (char32_t i = 'A'; i <= 'Z'; ++i) {
		auto ret = font->vRender(i);
		REQUIRE(ret.bitmap != nullptr);
		REQUIRE_EQ(ret.advance, Point(cwf, 0));
	}
}

TEST_CASE("FontGlyphChar") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto system = Cache::SysBlack();
	auto surface = Bitmap::Create(width, height);
	int color = 0;
	auto check = [&](char32_t ch, Point p) {
		REQUIRE_EQ(font->Render(*surface, 0, 0, *system, color, ch), p);
	};

	check(0, Point(0, 0));
	check(U' ', Point(cwh, 0));
	check(U'\n', Point(0, 0));
	check(U'X', Point(cwh, 0));
	check(U'ぽ', Point(cwf, 0));
	check(U'下', Point(cwf, 0));
}

TEST_CASE("FontGlyphCharEx") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::exfont;
	auto system = Cache::SysBlack();
	auto surface = Bitmap::Create(width, height);
	int color = 0;

	for (char32_t i = 'a'; i <= 'z'; ++i) {
		REQUIRE_EQ(font->Render(*surface, 0, 0, *system, color, i), Point(cwf, 0));
	}

	for (char32_t i = 'A'; i <= 'Z'; ++i) {
		REQUIRE_EQ(font->Render(*surface, 0, 0, *system, color, i), Point(cwf, 0));
	}
}

TEST_SUITE_END();
