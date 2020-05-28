#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "test_move_route.h"

TEST_SUITE_BEGIN("Game_Character_Move");

static void testChar(
		const Game_Character& ch,
		int x, int y,
		int dir, int face,
		int remaining_step,
		bool is_jumping,
		int begin_jump_x, int begin_jump_y,
		int frequency, int speed,
		int stop_count, int max_stop_count)
{
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(dir);
	CAPTURE(face);
	CAPTURE(remaining_step);
	CAPTURE(is_jumping);
	CAPTURE(begin_jump_x);
	CAPTURE(begin_jump_y);
	CAPTURE(frequency);
	CAPTURE(speed);
	CAPTURE(stop_count);
	CAPTURE(max_stop_count);

	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_EQ(ch.GetDirection(), dir);
	REQUIRE_EQ(ch.GetSpriteDirection(), face);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE_EQ(ch.IsJumping(), is_jumping);
	REQUIRE_EQ(ch.GetBeginJumpX(), begin_jump_x);
	REQUIRE_EQ(ch.GetBeginJumpY(), begin_jump_y);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE_EQ(ch.GetMoveFrequency(), frequency);
	REQUIRE_EQ(ch.GetMoveSpeed(), speed);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetMoveRouteIndex(), 0);
	REQUIRE_EQ(ch.IsMoveRouteOverwritten(), false);
	REQUIRE_EQ(ch.IsMoveRouteRepeated(), false);
	REQUIRE_EQ(ch.GetMoveRoute(), lcf::rpg::MoveRoute());

	REQUIRE_EQ(ch.IsStopCountActive(), stop_count < max_stop_count);

	if (remaining_step > 0) {
		REQUIRE(!ch.IsStopping());
		REQUIRE_EQ(ch.IsJumping(), is_jumping);
		REQUIRE_EQ(ch.IsMoving(), !is_jumping);
	} else {
		REQUIRE(ch.IsStopping());
		REQUIRE(!ch.IsJumping());
		REQUIRE(!ch.IsMoving());
	}
}

template <bool success>
static void testMove(int move_dir, int x, int y, int dir, int face,
		int tx, int ty, int tdir, int tface,
		int freq, int speed) {

	auto ch = MakeCharacter();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetSpriteDirection(face);
	ch.SetAllowMovement(success);
	ch.SetMoveSpeed(speed);
	ch.SetMoveFrequency(freq);
	ch.SetAllowMovement(success);

	REQUIRE_EQ(ch.Move(move_dir), success);

	if (!success) {
		testChar(ch, tx, ty, tdir, tface, 0, false, 0, 0, freq, speed, 0, 0);
		return;
	}
	// FIXME: Verify stop_count and max_stop_count setting / frequency
	testChar(ch, tx, ty, tdir, tface, 256, false, 0, 0, freq, speed, 0, 0);

	const auto dt = 1 << (1 + speed);

	for(int i = 256 - dt; i > 0; i -= dt) {
		ForceUpdate(ch);
		testChar(ch, tx, ty, tdir, tface, i, false, 0, 0, freq, speed, 0, 0);
	}

	ForceUpdate(ch);
	testChar(ch, tx, ty, tdir, tface, 0, false, 0, 0, freq, speed, 0, 0);
}

TEST_CASE("Move") {
	const MapGuard mg;

	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<true>(Up, 8, 8, Down, Down, 8, 7, Up, Up, freq, speed);
			testMove<true>(Right, 8, 8, Down, Down, 9, 8, Right, Right, freq, speed);
			testMove<true>(Down, 8, 8, Down, Down, 8, 9, Down, Down, freq, speed);
			testMove<true>(Left, 8, 8, Down, Down, 7, 8, Left, Left, freq, speed);
		}
	}
}

TEST_CASE("MoveFail") {
	const MapGuard mg;

	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<false>(Up, 8, 8, Down, Down, 8, 8, Up, Up, freq, speed);
			testMove<false>(Right, 8, 8, Down, Down, 8, 8, Right, Right, freq, speed);
			testMove<false>(Down, 8, 8, Down, Down, 8, 8, Down, Down, freq, speed);
			testMove<false>(Left, 8, 8, Down, Down, 8, 8, Left, Left, freq, speed);
		}
	}
}

TEST_CASE("MoveDiagonal") {
	const MapGuard mg;

	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<true>(UpRight, 8, 8, Up, Up, 9, 7, UpRight, Up, freq, speed);
			testMove<true>(UpRight, 8, 8, Right, Right, 9, 7, UpRight, Right, freq, speed);
			testMove<true>(UpRight, 8, 8, Down, Down, 9, 7, UpRight, Up, freq, speed);
			testMove<true>(UpRight, 8, 8, Left, Left, 9, 7, UpRight, Right, freq, speed);

			testMove<true>(DownRight, 8, 8, Up, Up, 9, 9, DownRight, Down, freq, speed);
			testMove<true>(DownRight, 8, 8, Right, Right, 9, 9, DownRight, Right, freq, speed);
			testMove<true>(DownRight, 8, 8, Down, Down, 9, 9, DownRight, Down, freq, speed);
			testMove<true>(DownRight, 8, 8, Left, Left, 9, 9, DownRight, Right, freq, speed);

			testMove<true>(UpLeft, 8, 8, Up, Up, 7, 7, UpLeft, Up, freq, speed);
			testMove<true>(UpLeft, 8, 8, Right, Right, 7, 7, UpLeft, Left, freq, speed);
			testMove<true>(UpLeft, 8, 8, Down, Down, 7, 7, UpLeft, Up, freq, speed);
			testMove<true>(UpLeft, 8, 8, Left, Left, 7, 7, UpLeft, Left, freq, speed);

			testMove<true>(DownLeft, 8, 8, Up, Up, 7, 9, DownLeft, Down, freq, speed);
			testMove<true>(DownLeft, 8, 8, Right, Right, 7, 9, DownLeft, Left, freq, speed);
			testMove<true>(DownLeft, 8, 8, Down, Down, 7, 9, DownLeft, Down, freq, speed);
			testMove<true>(DownLeft, 8, 8, Left, Left, 7, 9, DownLeft, Left, freq, speed);
		}
	}
}

static void testJump(bool success, int x, int y, int dir, int face,
		int tx, int ty, int freq, int speed) {

	auto ch = MakeCharacter();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetSpriteDirection(face);
	ch.SetAllowMovement(success);
	ch.SetMoveSpeed(speed);
	ch.SetMoveFrequency(freq);
	ch.SetAllowMovement(success);

	CAPTURE(success);
	CAPTURE(speed);
	CAPTURE(freq);

	auto dx = tx - x;
	auto dy = ty - y;

	// Expected direction after the jump finishes.
	auto tdir = Down;
	if (std::abs(dy) >= std::abs(dx)) {
		tdir = dy >= 0 ? Down : Up;
	} else {
		tdir = dx >= 0 ? Right : Left;
	}
	auto tface = tdir;

	// Jump in place always succeeds
	if (x == tx && y == ty) {
		success = true;
	}

	INFO("BEFORE JUMP");

	REQUIRE_EQ(ch.Jump(tx, ty), success);

	if (!success) {
		INFO("FAIL JUMP");
		testChar(ch, x, y, tdir, tface, 0, false, 0, 0, freq, speed, 0, 0);
		return;
	}

	testChar(ch, tx, ty, tdir, tface, 256, true, x, y, freq, speed, 0, 0);

	// FIXME: Verify all speeds
	static const int jump_speed[] = {8, 12, 16, 24, 32, 64};
	const auto dt = jump_speed[speed - 1];

	INFO("ANIMATE JUMP");

	for(int i = 256 - dt; i > 0; i -= dt) {
		ForceUpdate(ch);
		testChar(ch, tx, ty, tdir, tface, i, true, x, y, freq, speed, 0, 0);
	}

	INFO("AFTER JUMP");

	ForceUpdate(ch);
	testChar(ch, tx, ty, tdir, tface, 0, false, x, y, freq, speed, 0, 0);
}

TEST_CASE("Jump") {
	const MapGuard mg;

	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			for (int dx = -2; dx <= 2; ++dx) {
				for (int dy = -2; dy <= 2; ++dy) {
					testJump(true, 8, 8, Down, Down, 8 + dx, 8 + dy, freq, speed);
					testJump(false, 8, 8, Down, Down, 8 + dx, 8 + dy, freq, speed);
				}
			}
		}
	}
}

static void testStop(bool success, bool jump) {
	const MapGuard mg;

	auto ch = MakeCharacter();
	ch.SetX(8);
	ch.SetY(8);
	ch.SetStopCount(99);
	ch.SetMaxStopCount(200);
	ch.SetAllowMovement(success);

	if (jump) {
		REQUIRE_EQ(ch.Jump(9, 9), success);
	} else {
		REQUIRE_EQ(ch.Move(Up), success);
	}

	if (success) {
		REQUIRE_EQ(ch.GetStopCount(), 0);
	} else {
		REQUIRE_EQ(ch.GetStopCount(), 99);
	}
	REQUIRE_EQ(ch.GetMaxStopCount(), 200);
}

TEST_CASE("StopCountMove") { testStop(true, false); }
TEST_CASE("StopCountMoveFail") { testStop(false, false); }
TEST_CASE("StopCountJump") { testStop(true, true); }
TEST_CASE("StopCountJumpFail") { testStop(false, true); }

TEST_SUITE_END();
