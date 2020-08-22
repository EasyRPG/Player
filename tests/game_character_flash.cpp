#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "mock_game.h"

TEST_SUITE_BEGIN("Game_Character_Flash");

static void testChar(const Game_Character& ch, Color color, double power, int time_left) {
	CAPTURE(color);
	CAPTURE(power);
	CAPTURE(time_left);

	REQUIRE_EQ(ch.GetFlashColor(), color);
	REQUIRE_EQ(ch.GetFlashLevel(), power);
	REQUIRE_EQ(ch.GetFlashTimeLeft(), time_left);
}

static void testFlash(int r, int g, int b, double power, int frames) {
	const MockGame mg(MockMap::ePassBlock20x15);

	auto& ch = *mg.GetEvent(1);

	CAPTURE(r);
	CAPTURE(g);
	CAPTURE(b);
	CAPTURE(power);
	CAPTURE(frames);

	ch.Flash(r, g, b, power, frames);

	testChar(ch, Flash::MakeColor(r, g, b, power), power, frames);

	while (power > 0) {
		ForceUpdate(ch);
		Flash::Update(power, frames);

		testChar(ch, Flash::MakeColor(r, g, b, power), power, frames);
	}

	for (int i = 0; i < 8; ++i) {
		ForceUpdate(ch);
		testChar(ch, Flash::MakeColor(r, g, b, 0.0), 0.0, frames);
	}
}

TEST_CASE("Flash") {
	for (auto r: {0, 7}) {
		for (auto g: {9, 31}) {
			for (auto b: {3, 28}) {
				for (auto p = 0; p < 32; ++p) {
					for (auto t: {0, 1, 21, 61}) {
						testFlash(r, g, b, p, t);
					}
				}
			}
		}
	}
}

TEST_SUITE_END();
