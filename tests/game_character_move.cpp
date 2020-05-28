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

	ch.Move(move_dir);
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

	for (int freq = 1; freq <= 16; ++freq) {
		for (int speed = 1; speed <= 16; ++speed) {
			testMove<true>(Up, 8, 8, Down, Down, 8, 7, Up, Up, freq, speed);
			testMove<true>(Right, 8, 8, Down, Down, 9, 8, Right, Right, freq, speed);
			testMove<true>(Down, 8, 8, Down, Down, 8, 9, Down, Down, freq, speed);
			testMove<true>(Left, 8, 8, Down, Down, 7, 8, Left, Left, freq, speed);
		}
	}
}

TEST_CASE("MoveFail") {
	const MapGuard mg;

	for (int freq = 1; freq <= 16; ++freq) {
		for (int speed = 1; speed <= 16; ++speed) {
			testMove<false>(Up, 8, 8, Down, Down, 8, 8, Up, Up, freq, speed);
			testMove<false>(Right, 8, 8, Down, Down, 8, 8, Right, Right, freq, speed);
			testMove<false>(Down, 8, 8, Down, Down, 8, 8, Down, Down, freq, speed);
			testMove<false>(Left, 8, 8, Down, Down, 8, 8, Left, Left, freq, speed);
		}
	}
}

TEST_CASE("MoveDiagonal") {
	const MapGuard mg;

	for (int freq = 1; freq <= 16; ++freq) {
		for (int speed = 1; speed <= 16; ++speed) {
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

#if 0
TEST_CASE("CommandMoveFail") {
	const MapGuard mg;

	testMoveFail(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down, Up, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down, Right, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down, Down, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down, Left, Left);
}

TEST_CASE("CommandMoveDiagonal") {
	const MapGuard mg;

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up, 9, 7, UpRight, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right, 9, 7, UpRight, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down, 9, 7, UpRight, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left, 9, 7, UpRight, Right);

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up, 9, 9, DownRight, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right, 9, 9, DownRight, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down, 9, 9, DownRight, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left, 9, 9, DownRight, Right);

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up, 7, 9, DownLeft, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right, 7, 9, DownLeft, Left);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down, 7, 9, DownLeft, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left, 7, 9, DownLeft, Left);

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up, 7, 7, UpLeft, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right, 7, 7, UpLeft, Left);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down, 7, 7, UpLeft, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left, 7, 7, UpLeft, Left);
}

TEST_CASE("CommandMoveDiagonalFail") {
	const MapGuard mg;

	testMoveFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up, UpRight, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right, UpRight, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down, UpRight, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left, UpRight, Right);

	testMoveFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up, DownRight, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right, DownRight, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down, DownRight, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left, DownRight, Right);

	testMoveFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up, DownLeft, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right, DownLeft, Left);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down, DownLeft, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left, DownLeft, Left);

	testMoveFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up, UpLeft, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right, UpLeft, Left);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down, UpLeft, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left, UpLeft, Left);
}
#endif

TEST_SUITE_END();
