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

	REQUIRE_EQ(draw(0, 0, ""), Rect(0, 0, 0, 0));
	REQUIRE_EQ(draw(0, 0, "abc"), Rect(0, 0, cwh * 3, ch));
	REQUIRE_EQ(draw(3, 17, "$A"), Rect(3, 17, cwf, ch));
	REQUIRE_EQ(draw(3, 17, "$A $B"), Rect(3, 17, cwf * 2 + cwh, ch));
}

TEST_CASE("TextDrawColorStrReturn") {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto font = Font::Default();
	auto surface = Bitmap::Create(width, height);
	auto color = Color(255,255,255,255);

	auto draw = [&](int x, int y, const auto& text) {
		return Text::Draw(*surface, x, y, *font, color, text);
	};

	REQUIRE_EQ(draw(0, 0, ""), Rect(0, 0, 0, 0));
	REQUIRE_EQ(draw(0, 0, "abc"), Rect(0, 0, cwh * 3, ch));
	REQUIRE_EQ(draw(3, 17, "\n"), Rect(3, 17, 0, ch));
	REQUIRE_EQ(draw(3, 17, "x\nyz"), Rect(3, 17, cwh * 2, ch *2));
	REQUIRE_EQ(draw(10, 0, "xy\nz"), Rect(10, 0, cwh * 2, ch *2));
}

TEST_SUITE_END();
