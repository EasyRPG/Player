#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "mock_game.h"

TEST_SUITE_BEGIN("Game_Character_MoveTo");

static constexpr auto map_id = MockMap::ePassBlock20x15;

static void testChar(const Game_Character& ch, int map_id, int x, int y,
		int remaining_step, bool jumping,
		int stop_count, int max_stop_count,
		int anim_count, int anim_frame,
		const std::string& sprite_graphic, int sprite_id) {
	CAPTURE(map_id);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(remaining_step);
	CAPTURE(jumping);
	CAPTURE(stop_count);
	CAPTURE(max_stop_count);
	CAPTURE(anim_count);
	CAPTURE(anim_frame);
	CAPTURE(sprite_graphic);
	CAPTURE(sprite_id);

	REQUIRE_EQ(ch.GetMapId(), map_id);
	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE_EQ(ch.IsJumping(), jumping);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetAnimCount(), anim_count);
	REQUIRE_EQ(ch.GetAnimFrame(), anim_frame);
	REQUIRE_EQ(ch.GetSpriteName(), sprite_graphic);
	REQUIRE_EQ(ch.GetSpriteIndex(), sprite_id);
}

static void testPlayer(Game_Player& ch) {
	ch.SetRemainingStep(256);
	ch.SetJumping(true);
	ch.SetStopCount(77);
	ch.SetMaxStopCount(88);
	ch.SetAnimCount(9);
	ch.SetAnimFrame(2);
	ch.SetTransparency(7);
	ch.SetSpriteGraphic("GRAPHIC", 1);

	ch.SetTotalEncounterRate(50);
	ch.SetMenuCalling(true);
	ch.SetEncounterCalling(true);

	ch.MoveTo(static_cast<int>(map_id), 2, 3);

	testChar(ch, 1, 2, 3, 0, true, 77, 88, 9, 2, "", 0);

	REQUIRE_EQ(ch.GetTotalEncounterRate(), 0);
	REQUIRE_EQ(ch.GetTransparency(), 0);
	REQUIRE(!ch.IsMenuCalling());

	// This flag is not cleared.
	REQUIRE(ch.IsEncounterCalling());
}

template <typename T>
static void testNonPlayer(T& ch) {
	ch.SetRemainingStep(256);
	ch.SetJumping(true);
	ch.SetJumping(true);
	ch.SetStopCount(77);
	ch.SetMaxStopCount(88);
	ch.SetAnimCount(9);
	ch.SetAnimFrame(2);
	ch.SetSpriteGraphic("GRAPHIC", 1);

	ch.MoveTo(static_cast<int>(map_id), 2, 3);

	testChar(ch, 1, 2, 3, 0, true, 77, 88, 9, 2, "GRAPHIC", 1);
}

TEST_CASE("Vehicle") {
	const MockGame mg(map_id);

	testNonPlayer(*mg.GetVehicle(Game_Vehicle::Boat));
	testNonPlayer(*mg.GetVehicle(Game_Vehicle::Ship));
	testNonPlayer(*mg.GetVehicle(Game_Vehicle::Airship));
}

TEST_CASE("Event") {
	const MockGame mg(map_id);

	testNonPlayer(*mg.GetEvent(1));
}

TEST_CASE("Player") {
	const MockGame mg(map_id);

	// This tests a limited case
	// * NO vehicle
	// * No map change
	auto& ch = *mg.GetPlayer();
	ch.SetMapId(static_cast<int>(map_id));

	testPlayer(ch);

	// FIXME: Test map change
	// FIXME: Test in vehicle
}

TEST_SUITE_END();
