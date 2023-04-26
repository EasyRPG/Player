#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "main_data.h"
#include "game_system.h"
#include "rand.h"
#include <climits>

#include "mock_game.h"

TEST_SUITE_BEGIN("Game_Player Input");

static constexpr auto map_id = MockMap::ePassBlock20x15;

// FIXME: Test conditions which disable input

static void testPos(Game_Player& ch, int x, int y,
		int dir, int face,
		int remaining_step, bool jumping,
		int stop_count, int max_stop_count,
		int total_encounter_rate, bool encounter_calling,
		bool menu_calling,
		int vehicle, bool boarding, bool aboard)
{

	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_EQ(ch.GetDirection(), dir);
	REQUIRE_EQ(ch.GetFacing(), face);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE_EQ(ch.IsJumping(), jumping);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetTotalEncounterRate(), total_encounter_rate);
	REQUIRE_EQ(ch.IsEncounterCalling(), encounter_calling);
	REQUIRE_EQ(ch.IsMenuCalling(), menu_calling);
	REQUIRE_EQ(ch.GetVehicleType(),vehicle);
	REQUIRE_EQ(ch.IsBoardingOrUnboarding(), boarding);
	REQUIRE_EQ(ch.IsAboard(), aboard);
}

static void testMove(bool success, int input_dir, int dir, int x, int y, int dx, int dy, bool cheat, bool debug) {
	DebugGuard dg(debug);
	const MockGame mg(map_id);
	auto& ch = *mg.GetPlayer();

	CAPTURE(cheat);
	CAPTURE(debug);
	CAPTURE(input_dir);
	CAPTURE(dir);
	CAPTURE(x);
	CAPTURE(y);

	ch.SetX(x);
	ch.SetY(y);

	Input::ResetKeys();

	Input::dir4 = input_dir;

	if (cheat) {
		Input::press_time[Input::DEBUG_THROUGH] = 1;
	}

	const auto tx = x + dx;
	const auto ty = y + dy;

	// Force RNG to always fail to generate a random encounter
	Rand::LockGuard lk(INT32_MAX);
	ForceUpdate(ch);
	if (success) {
		int acc_rate = (cheat && debug) ? 0 : 100;
		testPos(ch, tx, ty, dir, dir, 224, false, 0, 0, acc_rate, false, false, 0, false, false);
	} else {
		testPos(ch, tx, ty, dir, dir, 0, false, 1, 0, 0, false, false, 0, false, false);
	}
}

TEST_CASE("Move") {
	testMove(true, 8, Up, 4, 8, 0, -1, false, false);
	testMove(true, 6, Right, 4, 8, 1, 0, false, false);
	testMove(true, 2, Down, 4, 8, 0, 1, false, false);
	testMove(true, 4, Left, 4, 8, -1, 0, false, false);

	testMove(false, 8, Up, 16, 8, 0, 0, false, false);
	testMove(false, 6, Right, 16, 8, 0, 0, false, false);
	testMove(false, 2, Down, 16, 8, 0, 0, false, false);
	testMove(false, 4, Left, 16, 8, 0, 0, false, false);

	testMove(false, 8, Up, 16, 8, 0, 0, true, false);
	testMove(false, 6, Right, 16, 8, 0, 0, true, false);
	testMove(false, 2, Down, 16, 8, 0, 0, true, false);
	testMove(false, 4, Left, 16, 8, 0, 0, true, false);

	testMove(true, 8, Up, 16, 8, 0, -1, true, true);
	testMove(true, 6, Right, 16, 8, 1, 0, true, true);
	testMove(true, 2, Down, 16, 8, 0, 1, true, true);
	testMove(true, 4, Left, 16, 8, -1, 0, true, true);
}

static void testDecision(bool moved, int vehicle, bool boarding, bool aboard) {
	auto& ch = *MockGame::GetPlayer();

	ch.SetX(8);
	ch.SetY(8);

	Input::ResetKeys();

	Input::triggered[Input::DECISION] = true;

	ForceUpdate(ch);

	int y = moved ? 9 : 8;
	int rs = moved ? 224 : 0;
	int stp = moved ? 0 : 1;
	testPos(ch, 8, y, Down, Down, rs, false, stp, 0, 0, false, false, vehicle, boarding, aboard);
}

TEST_CASE("DecisionNone") {
	const MockGame mg(map_id);

	testDecision(false, 0, false, false);
}

TEST_CASE("DecisionEvent") {
	const MockGame mg(map_id);

	auto* event = Game_Map::GetEvent(1);
	REQUIRE(event);
	event->MoveTo(1, 8, 9);

	testDecision(false, 0, false, false);

	// FIXME: Add map page which triggers
}

TEST_CASE("DecisionBoard") {
	const MockGame mg(map_id);
	auto& ch = *Main_Data::game_player;

	auto* vh = Game_Map::GetVehicle(Game_Vehicle::Boat);
	REQUIRE(vh);
	vh->MoveTo(1, 8, 9);

	testDecision(true, Game_Vehicle::Boat, true, false);

	while (!ch.IsStopping()) {
		ForceUpdate(ch);
	}

	testPos(ch, 8, 9, Down, Left, 0, false, 0, 0, 0, false, false, Game_Vehicle::Boat, 0, true);
}

TEST_CASE("DecisionUnboard") {
	const MockGame mg(map_id);

	// FIXME: Implement
}

static void testMenu(bool success) {
	auto& ch = *Main_Data::game_player;
	ch.SetX(8);
	ch.SetY(8);

	Input::ResetKeys();

	Input::triggered[Input::CANCEL] = true;

	ForceUpdate(ch);
	testPos(ch, 8, 8, Down, Down, 0, false, 1, 0, 0, false, success, 0, false, false);

	// FIXME: Test conditions which disable menu calling
}

TEST_CASE("CallMenu") {
	const MockGame mg(map_id);

	testMenu(true);
}

TEST_CASE("CallMenuDisabled") {
	const MockGame mg(map_id);

	Main_Data::game_system->SetAllowMenu(false);

	testMenu(false);
}

TEST_SUITE_END();
