#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "main_data.h"
#include "test_move_route.h"
#include "game_system.h"
#include <climits>

TEST_SUITE_BEGIN("Game_Player Input");

// FIXME: Test conditions which disable input

static void testPos(Game_Player& ch, int x, int y,
		int dir, int face,
		int remaining_step, bool jumping,
		int stop_count, int max_stop_count,
		int encounter_steps, bool encounter_calling,
		bool menu_calling,
		int vehicle, bool boarding, bool aboard)
{

	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_EQ(ch.GetDirection(), dir);
	REQUIRE_EQ(ch.GetSpriteDirection(), face);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE_EQ(ch.IsJumping(), false);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetEncounterSteps(), encounter_steps);
	REQUIRE_EQ(ch.IsEncounterCalling(), encounter_calling);
	REQUIRE_EQ(ch.IsMenuCalling(), menu_calling);
	REQUIRE_EQ(ch.GetVehicleType(),vehicle);
	REQUIRE_EQ(ch.IsBoardingOrUnboarding(), boarding);
	REQUIRE_EQ(ch.IsAboard(), aboard);
}

static void testMove(int input_dir, int dir, int x, int y) {
	const MapGuard mg;

	auto& ch = *Main_Data::game_player;
	ch.SetX(8);
	ch.SetY(8);

	Input::ResetKeys();

	Input::dir4 = input_dir;
	// FIXME: Test collision
	Input::press_time[Input::DEBUG_THROUGH] = 1;

	ForceUpdate(ch);
	testPos(ch, x, y, dir, dir, 224, false, 0, 0, 100, false, false, 0, false, false);
}

TEST_CASE("Move") {
	testMove(8, Up, 8, 7);
	testMove(6, Right, 9, 8);
	testMove(2, Down, 8, 9);
	testMove(4, Left, 7, 8);
}

static void testDecision(bool moved, int vehicle, bool boarding, bool aboard) {
	auto& ch = *Main_Data::game_player;
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
	const MapGuard mg;

	testDecision(false, 0, false, false);
}

TEST_CASE("DecisionEvent") {
	const MapGuard mg;

	auto* event = Game_Map::GetEvent(1);
	REQUIRE(event);
	event->MoveTo(1, 8, 9);

	testDecision(false, 0, false, false);

	// FIXME: Add map page which triggers
}

TEST_CASE("DecisionBoard") {
	const MapGuard mg;
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
	const MapGuard mg;

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
	const MapGuard mg;

	testMenu(true);
}

TEST_CASE("CallMenuDisabled") {
	const MapGuard mg;

	Game_System::SetAllowMenu(false);

	testMenu(false);
}

TEST_SUITE_END();
