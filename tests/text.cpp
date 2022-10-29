#include "text.h"
#include "pixel_format.h"
#include "cache.h"
#include "bitmap.h"
#include "font.h"
#include <iostream>
#include "doctest.h"

TEST_SUITE_BEGIN("Text");

constexpr char32_t escape = '\\';
constexpr int width = 240;
constexpr int height = 80;
constexpr int ch = 12;
constexpr int cwh = 6;
constexpr int cwf = 12;

TEST_CASE("TextDrawSystemStrReturn") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);
	auto system = Cache::SysBlack();

	auto draw = [&](int x, int y, const auto& text) {
		return Text::Draw(*surface, x, y, *font, *system, 0, text);
	};

	REQUIRE_EQ(draw(0, 0, ""), Point(0, 0));
	REQUIRE_EQ(draw(0, 0, "abc"), Point(cwh * 3, ch));
	REQUIRE_EQ(draw(3, 17, "$A"), Point(cwf, ch));
	REQUIRE_EQ(draw(3, 17, "$A $B"), Point(cwf * 2 + cwh, ch));
}

TEST_CASE("TextDrawColorStrReturn") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);
	auto color = Color(255,255,255,255);

	auto draw = [&](int x, int y, const auto& text) {
		return Text::Draw(*surface, x, y, *font, color, text);
	};

	REQUIRE_EQ(draw(0, 0, ""), Point(0, 0));
	REQUIRE_EQ(draw(0, 0, "abc"), Point(cwh * 3, 0));
	REQUIRE_EQ(draw(3, 17, "\n"), Point(0, 12));
	REQUIRE_EQ(draw(3, 17, "x\nyz"), Point(cwh * 2, 12));
	REQUIRE_EQ(draw(10, 0, "xy\nz"), Point(cwh * 2, 12));
}

TEST_SUITE_END();
