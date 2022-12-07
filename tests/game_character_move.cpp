#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "game_vehicle.h"
#include "main_data.h"
#include "game_switches.h"
#include <climits>
#include <initializer_list>

#include "mock_game.h"

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
	REQUIRE_EQ(ch.GetFacing(), face);
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
	REQUIRE_EQ(ch.IsMoveRouteFinished(), false);
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
		int dx, int dy, int tdir, int tface,
		int freq, int speed, bool facing_locked) {
	const MockGame mg(MockMap::ePassBlock20x15);

	auto& ch = *mg.GetEvent(1);
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetFacing(face);
	ch.SetMoveSpeed(speed);
	ch.SetMoveFrequency(freq);
	const auto maxstop = ch.GetMaxStopCount();
	if (facing_locked) {
		ch.SetFacingLocked(facing_locked);
	}

	const auto tx = x + dx;
	const auto ty = y + dy;

	CAPTURE(success);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(dx);
	CAPTURE(dy);

	REQUIRE_EQ(ch.Move(move_dir), success);

	if (!success) {
		testChar(ch, tx, ty, tdir, tface, 0, false, 0, 0, freq, speed, 0, maxstop);
		return;
	}
	testChar(ch, tx, ty, tdir, tface, 256, false, 0, 0, freq, speed, 0, maxstop);

	const auto dt = 1 << (1 + speed);

	CAPTURE(dt);
	for(int i = 256 - dt; i > 0; i -= dt) {
		CAPTURE(i);
		ForceUpdate(ch);
		testChar(ch, tx, ty, tdir, tface, i, false, 0, 0, freq, speed, 0, maxstop);
	}

	ForceUpdate(ch);
	testChar(ch, tx, ty, tdir, tface, 0, false, 0, 0, freq, speed, 0, maxstop);
}

TEST_CASE("Move") {
	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<true>(Up, 4, 4, Down, Down, 0, -1, Up, Up, freq, speed, false);
			testMove<true>(Right, 4, 4, Down, Down, 1, 0, Right, Right, freq, speed, false);
			testMove<true>(Down, 4, 4, Down, Down, 0, 1, Down, Down, freq, speed, false);
			testMove<true>(Left, 4, 4, Down, Down, -1, 0, Left, Left, freq, speed, false);

			testMove<true>(Up, 4, 4, Down, Down, 0, -1, Up, Down, freq, speed, true);
			testMove<true>(Right, 4, 4, Down, Down, 1, 0, Right, Down, freq, speed, true);
			testMove<true>(Down, 4, 4, Down, Down, 0, 1, Down, Down, freq, speed, true);
			testMove<true>(Left, 4, 4, Down, Down, -1, 0, Left, Down, freq, speed, true);
		}
	}
}

TEST_CASE("MoveFail") {
	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<false>(Up, 16, 16, Down, Down, 0, 0, Up, Up, freq, speed, false);
			testMove<false>(Right, 16, 16, Down, Down, 0, 0, Right, Right, freq, speed, false);
			testMove<false>(Down, 16, 16, Down, Down, 0, 0, Down, Down, freq, speed, false);
			testMove<false>(Left, 16, 16, Down, Down, 0, 0, Left, Left, freq, speed, false);

			testMove<false>(Up, 16, 16, Down, Down, 0, 0, Up, Down, freq, speed, true);
			testMove<false>(Right, 16, 16, Down, Down, 0, 0, Right, Down, freq, speed, true);
			testMove<false>(Down, 16, 16, Down, Down, 0, 0, Down, Down, freq, speed, true);
			testMove<false>(Left, 16, 16, Down, Down, 0, 0, Left, Down, freq, speed, true);
		}
	}
}

TEST_CASE("MoveDiagonal") {
	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			testMove<true>(UpRight, 4, 4, Up, Up, 1, -1, UpRight, Up, freq, speed, false);
			testMove<true>(UpRight, 4, 4, Right, Right, 1, -1, UpRight, Right, freq, speed, false);
			testMove<true>(UpRight, 4, 4, Down, Down, 1, -1, UpRight, Up, freq, speed, false);
			testMove<true>(UpRight, 4, 4, Left, Left, 1, -1, UpRight, Right, freq, speed, false);

			testMove<true>(UpRight, 4, 4, Up, Up, 1, -1, UpRight, Up, freq, speed, true);
			testMove<true>(UpRight, 4, 4, Right, Right, 1, -1, UpRight, Right, freq, speed, true);
			testMove<true>(UpRight, 4, 4, Down, Down, 1, -1, UpRight, Down, freq, speed, true);
			testMove<true>(UpRight, 4, 4, Left, Left, 1, -1, UpRight, Left, freq, speed, true);

			testMove<true>(DownRight, 4, 4, Up, Up, 1, 1, DownRight, Down, freq, speed, false);
			testMove<true>(DownRight, 4, 4, Right, Right, 1, 1, DownRight, Right, freq, speed, false);
			testMove<true>(DownRight, 4, 4, Down, Down, 1, 1, DownRight, Down, freq, speed, false);
			testMove<true>(DownRight, 4, 4, Left, Left, 1, 1, DownRight, Right, freq, speed, false);

			testMove<true>(DownRight, 4, 4, Up, Up, 1, 1, DownRight, Up, freq, speed, true);
			testMove<true>(DownRight, 4, 4, Right, Right, 1, 1, DownRight, Right, freq, speed, true);
			testMove<true>(DownRight, 4, 4, Down, Down, 1, 1, DownRight, Down, freq, speed, true);
			testMove<true>(DownRight, 4, 4, Left, Left, 1, 1, DownRight, Left, freq, speed, true);

			testMove<true>(UpLeft, 4, 4, Up, Up, -1, -1, UpLeft, Up, freq, speed, false);
			testMove<true>(UpLeft, 4, 4, Right, Right, -1, -1, UpLeft, Left, freq, speed, false);
			testMove<true>(UpLeft, 4, 4, Down, Down, -1, -1, UpLeft, Up, freq, speed, false);
			testMove<true>(UpLeft, 4, 4, Left, Left, -1, -1, UpLeft, Left, freq, speed, false);

			testMove<true>(UpLeft, 4, 4, Up, Up, -1, -1, UpLeft, Up, freq, speed, true);
			testMove<true>(UpLeft, 4, 4, Right, Right, -1, -1, UpLeft, Right, freq, speed, true);
			testMove<true>(UpLeft, 4, 4, Down, Down, -1, -1, UpLeft, Down, freq, speed, true);
			testMove<true>(UpLeft, 4, 4, Left, Left, -1, -1, UpLeft, Left, freq, speed, true);

			testMove<true>(DownLeft, 4, 4, Up, Up, -1, 1, DownLeft, Down, freq, speed, false);
			testMove<true>(DownLeft, 4, 4, Right, Right, -1, 1, DownLeft, Left, freq, speed, false);
			testMove<true>(DownLeft, 4, 4, Down, Down, -1, 1, DownLeft, Down, freq, speed, false);
			testMove<true>(DownLeft, 4, 4, Left, Left, -1, 1, DownLeft, Left, freq, speed, false);

			testMove<true>(DownLeft, 4, 4, Up, Up, -1, 1, DownLeft, Up, freq, speed, true);
			testMove<true>(DownLeft, 4, 4, Right, Right, -1, 1, DownLeft, Right, freq, speed, true);
			testMove<true>(DownLeft, 4, 4, Down, Down, -1, 1, DownLeft, Down, freq, speed, true);
			testMove<true>(DownLeft, 4, 4, Left, Left, -1, 1, DownLeft, Left, freq, speed, true);
		}
	}
}

static void testJump(bool success, int x, int y, int dir, int face,
		int dx, int dy, int freq, int speed, bool facing_locked) {

	const MockGame mg(MockMap::ePassBlock20x15);
	auto& ch = *mg.GetEvent(1);

	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetFacing(face);
	ch.SetMoveSpeed(speed);
	ch.SetMoveFrequency(freq);
	const auto maxstop = ch.GetMaxStopCount();
	if (facing_locked) {
		ch.SetFacingLocked(true);
	}

	CAPTURE(success);
	CAPTURE(speed);
	CAPTURE(freq);

	const auto tx = x + dx;
	const auto ty = y + dy;

	// Expected direction after the jump finishes.
	auto tdir = Down;
	if (std::abs(dy) >= std::abs(dx)) {
		tdir = dy >= 0 ? Down : Up;
	} else {
		tdir = dx >= 0 ? Right : Left;
	}
	auto tface = facing_locked ? face : tdir;

	// Jump in place always succeeds
	if (x == tx && y == ty) {
		success = true;
	}

	INFO("BEFORE JUMP");

	REQUIRE_EQ(ch.Jump(tx, ty), success);

	if (!success) {
		INFO("FAIL JUMP");
		testChar(ch, tx, ty, tdir, tface, 0, false, 0, 0, freq, speed, 0, maxstop);
		return;
	}

	testChar(ch, tx, ty, tdir, tface, 256, true, x, y, freq, speed, 0, maxstop);

	// FIXME: Verify all speeds
	static const int jump_speed[] = {8, 12, 16, 24, 32, 64};
	const auto dt = jump_speed[speed - 1];

	INFO("ANIMATE JUMP");

	for(int i = 256 - dt; i > 0; i -= dt) {
		ForceUpdate(ch);
		testChar(ch, tx, ty, tdir, tface, i, true, x, y, freq, speed, 0, maxstop);
	}

	INFO("AFTER JUMP");

	ForceUpdate(ch);
	testChar(ch, tx, ty, tdir, tface, 0, false, x, y, freq, speed, 0, maxstop);
}

TEST_CASE("Jump") {
	for (int freq = 1; freq <= 8; ++freq) {
		for (int speed = 1; speed <= 6; ++speed) {
			for (int dx = -2; dx <= 2; ++dx) {
				for (int dy = -2; dy <= 2; ++dy) {
					// Normal jump
					testJump(true, 4, 4, Down, Down, dx, dy, freq, speed, false);
					testJump(false, 16, 16, Down, Down, 0, 0, freq, speed, false);

					// Facing locked jump
					testJump(true, 4, 4, Down, Down, dx, dy, freq, speed, true);
					testJump(false, 16, 16, Down, Down, 0, 0, freq, speed, true);
				}
			}
		}
	}
}

static void testStop(bool success, bool jump, int x, int y) {
	const MockGame mg(MockMap::ePassBlock20x15);

	auto& ch = *MockGame::GetEvent(1);
	ch.SetX(x);
	ch.SetY(y);
	ch.SetStopCount(99);
	ch.SetMaxStopCount(200);

	if (jump) {
		REQUIRE_EQ(ch.Jump(x + 1, y + 1), success);
	} else {
		REQUIRE_EQ(ch.Move(Up), success);
	}

	REQUIRE_EQ(ch.GetStopCount(), 99);
	REQUIRE_EQ(ch.GetMaxStopCount(), 200);
}

TEST_CASE("StopCountMove") { testStop(true, false, 4, 4); }
TEST_CASE("StopCountMoveFail") { testStop(false, false, 16, 16); }
TEST_CASE("StopCountJump") { testStop(true, true, 4, 4); }
TEST_CASE("StopCountJumpFail") { testStop(false, true, 16, 16); }

TEST_SUITE_END();
