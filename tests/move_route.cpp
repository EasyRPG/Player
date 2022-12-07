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

TEST_SUITE_BEGIN("MoveRoute");

lcf::rpg::MoveRoute MakeRoute(std::initializer_list<lcf::rpg::MoveCommand> cmds, bool repeat = false, bool skip = false) {
	lcf::rpg::MoveRoute mr;
	mr.move_commands = cmds;
	mr.repeat = repeat;
	mr.skippable = skip;
	return mr;
}

static void testMoveRoute(
		const Game_Character& ch,
		bool paused, int move_frequency,
		int stop_count, int max_stop_count,
		int move_route_idx, bool overwritten, bool done,
		const lcf::rpg::MoveRoute& mr)
{
	CAPTURE(paused);
	CAPTURE(move_frequency);
	CAPTURE(stop_count);
	CAPTURE(max_stop_count);
	CAPTURE(move_route_idx);
	CAPTURE(overwritten);
	CAPTURE(done);
	CAPTURE(mr);

	REQUIRE_EQ(ch.IsPaused(), paused);
	REQUIRE_EQ(ch.GetMoveFrequency(), move_frequency);
	REQUIRE_EQ(ch.GetStopCount(), stop_count);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.GetMoveRouteIndex(), move_route_idx);
	REQUIRE_EQ(ch.IsMoveRouteOverwritten(), overwritten);
	REQUIRE_EQ(ch.IsMoveRouteFinished(), done);
	REQUIRE_EQ(ch.GetMoveRoute(), mr);
	REQUIRE_EQ(ch.IsStopCountActive(), stop_count < max_stop_count);
}

template <typename... Args>
static void testMoveRouteDir(const Game_Character& ch,
		int dir, int face,
		Args&&... args) {

	CAPTURE(dir);
	CAPTURE(face);

	REQUIRE_EQ(ch.GetDirection(), dir);
	REQUIRE_EQ(ch.GetFacing(), face);
	testMoveRoute(ch, std::forward<Args>(args)...);
}

template <typename... Args>
static void testMoveRouteMove(const Game_Character& ch,
		int x, int y, int remaining_step,
		Args&&... args) {

	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(remaining_step);

	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_GE(ch.GetRemainingStep(), 0);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);
	REQUIRE(!ch.IsJumping());
	if (remaining_step > 0) {
		REQUIRE(ch.IsMoving());
	} else {
		REQUIRE(ch.IsStopping());
		REQUIRE(!ch.IsMoving());
	}
	testMoveRouteDir(ch, std::forward<Args>(args)...);
}

template <typename... Args>
static void testMoveRouteJump(const Game_Character& ch,
		int x, int y, int remaining_step,
		bool is_jumping, int begin_jump_x, int begin_jump_y,
		Args&&... args) {

	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(remaining_step);
	CAPTURE(is_jumping);
	CAPTURE(begin_jump_x);
	CAPTURE(begin_jump_y);

	REQUIRE_EQ(ch.GetX(), x);
	REQUIRE_EQ(ch.GetY(), y);
	REQUIRE_GE(ch.GetRemainingStep(), 0);
	REQUIRE_EQ(ch.GetRemainingStep(), remaining_step);

	REQUIRE_EQ(ch.GetBeginJumpX(), begin_jump_x);
	REQUIRE_EQ(ch.GetBeginJumpY(), begin_jump_y);

	if (remaining_step > 0) {
		REQUIRE_EQ(ch.IsJumping(), is_jumping);
		REQUIRE_EQ(ch.IsMoving(), !is_jumping);
	} else {
		REQUIRE(ch.IsStopping());
		REQUIRE(!ch.IsJumping());
		REQUIRE(!ch.IsMoving());
	}
	testMoveRouteDir(ch, std::forward<Args>(args)...);
}

TEST_CASE("DefaultMoveRoute") {
	auto ch = MoveRouteVehicle();
	testMoveRoute(ch, false, 2, 0, 0, 0, false, false, lcf::rpg::MoveRoute());
}

TEST_CASE("ForceMoveRouteEmpty") {
	auto ch = MoveRouteVehicle();
	ch.ForceMoveRoute(lcf::rpg::MoveRoute(), 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, lcf::rpg::MoveRoute());
}

TEST_CASE("ForceMoveRouteSameFreq") {
	auto ch = MoveRouteVehicle();
	lcf::rpg::MoveRoute mr;
	mr.move_commands.push_back({});

	ch.ForceMoveRoute(mr, 2);
	// Note: Same freq means don't reset max stop count
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);

	ch.CancelMoveRoute();
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, mr);
}

TEST_CASE("ForceMoveRouteDiffFreq") {
	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{}});
	mr.move_commands.push_back({});

	ch.ForceMoveRoute(mr, 3);
	testMoveRoute(ch, false, 3, 0xFFFF, 64, 0, true, false, mr);

	// FIXME: Test original move frequency is robust to save games
	ch.CancelMoveRoute();
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, false, false, mr);
}

static void testInvalidCmd(bool repeat, bool skip) {
	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ -1 }}, repeat, skip);

	ch.ForceMoveRoute(mr, 3);
	testMoveRoute(ch, false, 3, 0xFFFF, 64, 0, true, false, mr);

	ForceUpdate(ch);
	if (repeat) {
		testMoveRoute(ch, false, 3, 0xFFFF + 1, 64, 0, true, true, mr);
	} else {
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	}
}

TEST_CASE("TestMoveRouteInvalidCmd") {
	const MapGuard mg;

	testInvalidCmd(false, false);
	testInvalidCmd(false, true);
	testInvalidCmd(true, false);
	testInvalidCmd(true, true);
}

template <bool success, bool repeat, bool skip>
static void testMove(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int tx, int ty, int tdir, int tface, int px = 0, int py = 0) {
	Main_Data::game_player->SetX(px);
	Main_Data::game_player->SetY(py);

	auto ch = MoveRouteVehicle();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetFacing(face);
	ch.SetAllowMovement(success);

	auto mr = MakeRoute({{ static_cast<int>(code) }}, repeat, skip);

	CAPTURE(code);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(dir);
	CAPTURE(face);
	CAPTURE(tx);
	CAPTURE(ty);
	CAPTURE(tdir);
	CAPTURE(tface);
	CAPTURE(px);
	CAPTURE(py);
	CAPTURE(success);
	CAPTURE(repeat);
	CAPTURE(skip);

	auto dx = tx - x;
	auto dy = ty - y;

	ch.ForceMoveRoute(mr, 3);
	testMoveRouteMove(ch, x, y, 0, dir, face, false, 3, 0xFFFF, 64, 0, true, false, mr);

	if (success) {
		bool repeated = false;
		for(int n = (repeat ? 3 : 1); n > 0; --n) {
			CAPTURE(n);
			for(int i = 224; i > 0; i -= 32) {
				ForceUpdate(ch);
				testMoveRouteMove(ch, tx, ty, i, tdir, tface, false, 3, 0, 64, 1, true, repeated, mr);
			}

			if (!repeat) {
				ForceUpdate(ch);
				testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 2, 0, 128, 0, false, false, mr);
				break;
			}

			repeated = true;
			for (int i = 0; i <= 64; ++i) {
				ForceUpdate(ch);
				testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 3, i, 64, 0, true, repeated, mr);
			}

			tx += dx;
			ty += dy;
		}
	} else {
		ForceUpdate(ch);
		if (skip) {
			if (repeat) {
				for (int i = 1; i <= 3; ++i) {
					testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 3, 0xFFFF + i, 64, 0, true, true, mr);
					ForceUpdate(ch);
				}
			} else {
				testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
			}
		} else {
			testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 3, 0xFFFF + 1, 64, 0, true, false, mr);
		}
	}
}

template <typename... Args>
static void testMoveSuccess(const Args&... args) {
	testMove<true, false, false>(args...);
	testMove<true, false, true>(args...);
	testMove<true, true, false>(args...);
	testMove<true, true, true>(args...);
}

template <typename... Args>
static void testMoveSuccessNoRepeat(const Args&... args) {
	testMove<true, false, false>(args...);
	testMove<true, false, true>(args...);
}

static void testMoveFail(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int tdir, int tface, int px = 0, int py = 0) {
	testMove<false, false, false>(code, x, y, dir, face, x, y, tdir, tface, px, py);
	testMove<false, true, false>(code, x, y, dir, face, x, y, tdir, tface, px, py);
}

static void testMoveSkip(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int px = 0, int py = 0) {
	testMove<false, false, true>(code, x, y, dir, face, x, y, dir, face, px, py);
	testMove<false, true, true>(code, x, y, dir, face, x, y, dir, face, px, py);
}


TEST_CASE("CommandMove") {
	const MapGuard mg;

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down, 8, 7, Up, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down, 9, 8, Right, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down, 8, 9, Down, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down, 7, 8, Left, Left);
}

TEST_CASE("CommandMoveFail") {
	const MapGuard mg;

	testMoveFail(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down, Up, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down, Right, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down, Down, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down, Left, Left);
}

TEST_CASE("CommandMoveSkip") {
	const MapGuard mg;

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down);
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

TEST_CASE("CommandMoveDiagonalSkip") {
	const MapGuard mg;

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left);

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left);

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left);

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left);
}

TEST_CASE("CommandMoveForward") {
	const MapGuard mg;

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up, 8, 7, Up, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right, 9, 8, Right, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down, 8, 9, Down, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left, 7, 8, Left, Left);

	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up, 9, 7, UpRight, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right, 9, 7, UpRight, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down, 9, 9, DownRight, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right, 9, 9, DownRight, Right);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up, 7, 7, UpLeft, Up);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left, 7, 7, UpLeft, Left);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down, 7, 9, DownLeft, Down);
	testMoveSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left, 7, 9, DownLeft, Left);
}

TEST_CASE("CommandMoveForwardFail") {
	const MapGuard mg;

	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up, Up, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right, Right, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down, Down, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left, Left, Left);

	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up, UpRight, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right, UpRight, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down, DownRight, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right, DownRight, Right);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up, UpLeft, Up);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left, UpLeft, Left);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down, DownLeft, Down);
	testMoveFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left, DownLeft, Left);
}

TEST_CASE("CommandMoveForwardSkip") {
	const MapGuard mg;

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left);

	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down);
	testMoveSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left);
}

TEST_CASE("CommandMoveRandom") {
	// FIXME: TBD
}


static void testTurn(lcf::rpg::MoveCommand::Code code, int orig_dir, int dir, int face, int x = 0, int y = 0, int px = 0, int py = 0) {
	Main_Data::game_player->SetX(px);
	Main_Data::game_player->SetY(py);

	auto ch = MoveRouteVehicle();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(orig_dir);
	ch.SetFacing(orig_dir);
	auto mr = MakeRoute({{ static_cast<int>(code) }});

	CAPTURE(code);
	CAPTURE(orig_dir);
	CAPTURE(dir);
	CAPTURE(face);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(px);
	CAPTURE(py);

	ch.ForceMoveRoute(mr, 2);
	testMoveRouteDir(ch, orig_dir, orig_dir, false, 2, 0xFFFF, 0, 0, true, false, mr);

	for(int i = 1; i <= 64; ++i) {
		ForceUpdate(ch);
		testMoveRouteDir(ch, dir, face, false, 2, i, 64, 1, true, false, mr);
	}

	ForceUpdate(ch);
	testMoveRouteDir(ch, dir, face, false, 2, 65, 128, 1, false, false, mr);
}

TEST_CASE("CommandTurn") {
	const MapGuard mg;

	testTurn(lcf::rpg::MoveCommand::Code::face_up, Down, Up, Up);
	testTurn(lcf::rpg::MoveCommand::Code::face_left, Down, Left, Left);
	testTurn(lcf::rpg::MoveCommand::Code::face_right, Down, Right, Right);
	testTurn(lcf::rpg::MoveCommand::Code::face_down, Down, Down, Down);

	testTurn(lcf::rpg::MoveCommand::Code::turn_180_degree, Down, Up, Up);
	testTurn(lcf::rpg::MoveCommand::Code::turn_180_degree, Up, Down, Down);
	testTurn(lcf::rpg::MoveCommand::Code::turn_180_degree, Left, Right, Right);
	testTurn(lcf::rpg::MoveCommand::Code::turn_180_degree, Right, Left, Left);

	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_right, Down, Left, Left);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_right, Left, Up, Up);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_right, Up, Right, Right);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_right, Right, Down, Down);

	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_left, Down, Right, Right);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_left, Right, Up, Up);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_left, Up, Left, Left);
	testTurn(lcf::rpg::MoveCommand::Code::turn_90_degree_left, Left, Down, Down);
}

TEST_CASE("CommandTurnRandom") {
	const MapGuard mg;

	for (int i = 0; i < 10; ++i) {
		auto ch = MoveRouteVehicle();
		auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::face_random_direction) }});

		ch.ForceMoveRoute(mr, 3);
		testMoveRouteDir(ch, Down, Down, false, 3, 0xFFFF, 64, 0, true, false, mr);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 3, 1, 32, 1, true, false, mr);

		REQUIRE_GE(ch.GetDirection(), Up);
		REQUIRE_LE(ch.GetDirection(), Left);
	}
}


TEST_CASE("CommandWait") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::wait) }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRouteDir(ch, Down, Down, false, 2, 0xFFFF, 0, 0, true, false, mr);

	for(int i = 1; i <= 84; ++i) {
		ForceUpdate(ch);
		testMoveRouteDir(ch, Down, Down, false, 2, i, 84, 1, true, false, mr);
	}

	ForceUpdate(ch);
	testMoveRouteDir(ch, Down, Down, false, 2, 85, 128, 1, false, false, mr);
}

template <bool success, bool repeat, bool skip, bool end>
static void testJump(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int tx, int ty, int tdir, int tface, int px = 0, int py = 0) {
	Main_Data::game_player->SetX(px);
	Main_Data::game_player->SetY(py);

	auto ch = MoveRouteVehicle();
	ch.SetX(x);
	ch.SetY(y);
	ch.SetDirection(dir);
	ch.SetFacing(face);
	ch.SetAllowMovement(success);

	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::begin_jump) }}, repeat, skip);
	if (code != lcf::rpg::MoveCommand::Code::end_jump) {
		mr.move_commands.push_back({  static_cast<int>(code) });
	}
	if (end) {
		mr.move_commands.push_back({ static_cast<int>(lcf::rpg::MoveCommand::Code::end_jump) });
	}
	auto num_cmds = static_cast<int>(mr.move_commands.size());

	CAPTURE(code);
	CAPTURE(x);
	CAPTURE(y);
	CAPTURE(dir);
	CAPTURE(face);
	CAPTURE(tx);
	CAPTURE(ty);
	CAPTURE(tdir);
	CAPTURE(tface);
	CAPTURE(px);
	CAPTURE(py);
	CAPTURE(success);
	CAPTURE(repeat);
	CAPTURE(skip);

	auto dx = tx - x;
	auto dy = ty - y;

	ch.ForceMoveRoute(mr, 3);
	testMoveRouteJump(ch, x, y, 0, false, 0, 0, dir, face, false, 3, 0xFFFF, 64, 0, true, false, mr);

	if (!end) {
		ForceUpdate(ch);
		if (repeat) {
			testMoveRouteJump(ch, x, y, 0, false, 0, 0, dir, face, false, 3, 0xFFFF + 1, 64, 0, true, true, mr);
		} else {
			testMoveRouteJump(ch, x, y, 0, false, 0, 0, dir, face, false, 2, 0xFFFF + 1, 128, num_cmds, false, false, mr);
		}

		return;
	}

	if (success) {
		bool repeated = false;
		for(int n = (repeat ? 3 : 1); n > 0; --n) {
			CAPTURE(n);
			for(int i = 232; i > 0; i -= 24) {
				ForceUpdate(ch);
				testMoveRouteJump(ch, tx, ty, i, true, x, y, tdir, tface, false, 3, 0, 64, num_cmds, true, repeated, mr);
			}

			if (!repeat) {
				ForceUpdate(ch);
				testMoveRouteJump(ch, tx, ty, 0, false, x, y, tdir, tface, false, 2, 0, 128, 0, false, false, mr);
				break;
			}

			repeated = true;
			for (int i = 0; i <= 64; ++i) {
				ForceUpdate(ch);
				testMoveRouteMove(ch, tx, ty, 0, tdir, tface, false, 3, i, 64, 0, true, repeated, mr);
			}

			x = tx;
			y = ty;
			tx += dx;
			ty += dy;
		}
	} else {
		ForceUpdate(ch);
		if (skip) {
			if (repeat) {
				for (int i = 1; i <= 3; ++i) {
					testMoveRouteJump(ch, tx, ty, 0, false, 0, 0, tdir, tface, false, 3, 0xFFFF + i, 64, 0, true, true, mr);
					ForceUpdate(ch);
				}
			} else {
				testMoveRouteJump(ch, tx, ty, 0, false, 0, 0, tdir, tface, false, 2, 0xFFFF + 1, 128, num_cmds, false, false, mr);
			}
		} else {
			testMoveRouteJump(ch, tx, ty, 0, false, 0, 0, tdir, tface, false, 3, 0xFFFF + 1, 64, 0, true, false, mr);
		}
	}
}

template <typename... Args>
static void testJumpSuccess(const Args&... args) {
	testJump<true, false, false, true>(args...);
	testJump<true, false, true, true>(args...);
	testJump<true, true, false, true>(args...);
	testJump<true, true, true, true>(args...);
}

template <typename... Args>
static void testJumpSuccessNoRepeat(const Args&... args) {
	testJump<true, false, false, true>(args...);
	testJump<true, false, true, true>(args...);
}

static void testJumpFail(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int tdir, int tface, int px = 0, int py = 0) {
	testJump<false, false, false, true>(code, x, y, dir, face, x, y, tdir, tface, px, py);
	testJump<false, true, false, true>(code, x, y, dir, face, x, y, tdir, tface, px, py);
}

static void testJumpSkip(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int px = 0, int py = 0) {
	testJump<false, false, true, true>(code, x, y, dir, face, x, y, dir, face, px, py);
	testJump<false, true, true, true>(code, x, y, dir, face, x, y, dir, face, px, py);
}

static void testJumpNoEnd(lcf::rpg::MoveCommand::Code code, int x, int y, int dir, int face, int px = 0, int py = 0) {
	testJump<false, false, true, false>(code, x, y, dir, face, x, y, dir, face, px, py);
	testJump<false, true, true, false>(code, x, y, dir, face, x, y, dir, face, px, py);
}

TEST_CASE("CommandJumpInPlace") {
	const MapGuard mg;

	// FIXME: Verify this behavior
	testJumpSuccess(lcf::rpg::MoveCommand::Code::end_jump, 8, 8, Up, Up, 8, 8, Down, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::end_jump, 8, 8, Right, Right, 8, 8, Down, Right);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::end_jump, 8, 8, Down, Down, 8, 8, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::end_jump, 8, 8, Left, Left, 8, 8, Down, Left);

	// FIXME: Test no collision
}

TEST_CASE("CommandJump") {
	const MapGuard mg;

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down, 8, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down, 9, 8, Right, Right);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down, 8, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down, 7, 8, Left, Left);
}

TEST_CASE("CommandJumpFail") {
	const MapGuard mg;

	testJumpFail(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down, Right, Right);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down, Left, Left);
}

TEST_CASE("CommandJumpSkip") {
	const MapGuard mg;

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_up, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_right, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_down, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_left, 8, 8, Down, Down);
}

TEST_CASE("CommandJumpDiagonal") {
	const MapGuard mg;

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up, 9, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right, 9, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down, 9, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left, 9, 7, Up, Up);

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up, 9, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right, 9, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down, 9, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left, 9, 9, Down, Down);

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up, 7, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right, 7, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down, 7, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left, 7, 9, Down, Down);

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up, 7, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right, 7, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down, 7, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left, 7, 7, Up, Up);
}

TEST_CASE("CommandJumpDiagonalFail") {
	const MapGuard mg;

	testJumpFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left, Up, Up);

	testJumpFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left, Down, Down);

	testJumpFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left, Down, Down);

	testJumpFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left, Up, Up);
}

TEST_CASE("CommandJumpDiagonalSkip") {
	const MapGuard mg;

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Up, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Right, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upright, 8, 8, Left, Left);

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Up, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Right, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downright, 8, 8, Left, Left);

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Up, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Right, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_downleft, 8, 8, Left, Left);

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Up, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Right, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_upleft, 8, 8, Left, Left);
}

TEST_CASE("CommandJumpForward") {
	const MapGuard mg;

	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up, 8, 7, Up, Up);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right, 9, 8, Right, Right);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down, 8, 9, Down, Down);
	testJumpSuccess(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left, 7, 8, Left, Left);

	// FIXME: For repeat, these will move diag and then up or down afterwards.
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up, 9, 7, Up, Up);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right, 9, 7, Up, Up);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down, 9, 9, Down, Down);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right, 9, 9, Down, Down);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up, 7, 7, Up, Up);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left, 7, 7, Up, Up);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down, 7, 9, Down, Down);
	testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left, 7, 9, Down, Down);
}

TEST_CASE("CommandJumpForwardFail") {
	const MapGuard mg;

	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right, Right, Right);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left, Left, Left);

	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left, Up, Up);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down, Down, Down);
	testJumpFail(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left, Down, Down);
}

TEST_CASE("CommandJumpForwardSkip") {
	const MapGuard mg;

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Up, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Right, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Down, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, Left, Left);

	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpRight, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownRight, Right);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Up);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, UpLeft, Left);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Down);
	testJumpSkip(lcf::rpg::MoveCommand::Code::move_forward, 8, 8, DownLeft, Left);
}

TEST_CASE("CommandJumpRandom") {
	// FIXME: TBD
}

TEST_CASE("CommandJumpNoEnd") {
	const MapGuard mg;

	// This makes a move route with just "Jump"
	testJumpNoEnd(lcf::rpg::MoveCommand::Code::end_jump, 8, 8, Down, Down);
}


TEST_CASE("CommandMoveTurnJumpHero") {
	const MapGuard mg;

	int x = 8;
	int y = 8;
	for (int dx = -2; dx <= 2; ++dx) {
		for (int dy = -2; dy <= 2; ++dy) {
			auto step_x = 0;
			auto step_y = 0;
			auto dir = Down;

			if (std::abs(dx) > std::abs(dy)) {
				dir = (dx > 0 ? Right : Left);
				step_x = (dx > 0 ? 1 : -1);
			} else {
				dir = (dy >= 0 ? Down : Up);
				step_y = (dy >= 0 ? 1 : -1);
			}
			auto rdir = Game_Character::GetDirection180Degree(dir);

			auto px = x + dx;
			auto py = y + dy;

			testTurn(lcf::rpg::MoveCommand::Code::face_hero, Left, dir, dir, x, y, px, py);
			testTurn(lcf::rpg::MoveCommand::Code::face_away_from_hero, Left, rdir, rdir, x, y, px, py);

			testMoveSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, x + step_x, y + step_y, dir, dir, px, py);
			testMoveSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, x - step_x, y - step_y, rdir, rdir, px, py);

			testMoveFail(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, dir, dir, px, py);
			testMoveFail(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, rdir, rdir, px, py);

			testMoveSkip(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, px, py);
			testMoveSkip(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, px, py);

			testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, x + step_x, y + step_y, dir, dir, px, py);
			testJumpSuccessNoRepeat(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, x - step_x, y - step_y, rdir, rdir, px, py);

			testJumpFail(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, dir, dir, px, py);
			testJumpFail(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, rdir, rdir, px, py);

			testJumpSkip(lcf::rpg::MoveCommand::Code::move_towards_hero, x, y, Left, Left, px, py);
			testJumpSkip(lcf::rpg::MoveCommand::Code::move_away_from_hero, x, y, Left, Left, px, py);
		}
	}
}

void testLockFacing(lcf::rpg::EventPage::AnimType at) {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	ch.SetAnimationType(at);
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::lock_facing) }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE_EQ(ch.IsFacingLocked(), Game_Character::IsDirectionFixedAnimationType(at));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(ch.IsFacingLocked());

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::unlock_facing) }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.IsFacingLocked());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE_EQ(ch.IsFacingLocked(), Game_Character::IsDirectionFixedAnimationType(at));
}

TEST_CASE("CommandLockFacing") {
	for (int i = 0; i <= static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++i) {
		testLockFacing(static_cast<lcf::rpg::EventPage::AnimType>(i));
	}
}

TEST_CASE("CommandSpeedChange") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::increase_movement_speed) }});
	const int n = 10;

	ch.SetMoveSpeed(1);
	int prev = ch.GetMoveSpeed();
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, (i == 0 ? 0 : 128), 0, true, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), std::min(prev + 1, 6));

		prev= ch.GetMoveSpeed();
	}

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::decrease_movement_speed) }});
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetMoveSpeed(), std::max(prev - 1, 1));

		prev= ch.GetMoveSpeed();
	}
}

TEST_CASE("CommandFreqChange") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::increase_movement_frequence) }});
	const int n = 10;

	for (int i = 1; i < 10; ++i) {
		const int freq = Utils::Clamp(i, 1, 8);

		ch.ForceMoveRoute(mr, freq);
		testMoveRoute(ch, false, freq, 0xFFFF, (i == 0 && freq == 2 ? 0 : Game_Character::GetMaxStopCountForStep(freq)), 0, true, false, mr);

		const int next_freq = Utils::Clamp(freq + 1, 1, 8);
		ForceUpdate(ch);
		// FIXME: Need another command for the frequency to not get reset when move route is done.
		//testMoveRoute(ch, false, next_freq, 0xFFFF + 1, Game_Character::GetMaxStopCountForStep(next_freq), 1, false, false, mr);

		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);

	}

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::decrease_movement_frequence) }});

	for (int i = 1; i < 10; ++i) {
		const int freq = Utils::Clamp(i, 1, 8);

		ch.ForceMoveRoute(mr, freq);
		testMoveRoute(ch, false, freq, 0xFFFF, Game_Character::GetMaxStopCountForStep(freq), 0, true, false, mr);

		const int next_freq = Utils::Clamp(freq - 1, 1, 8);
		ForceUpdate(ch);
		// FIXME: Need another command for the frequency to not get reset when move route is done.
		//testMoveRoute(ch, false, next_freq, 0xFFFF + 1, Game_Character::GetMaxStopCountForStep(next_freq), 1, false, false, mr);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	}
}

TEST_CASE("CommandTranspChange") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::increase_transp) }});
	const int n = 10;

	ch.SetTransparency(0);
	int prev = ch.GetTransparency();
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, (i == 0 ? 0 : 128), 0, true, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), std::min(prev + 1, 7));

		prev = ch.GetTransparency();
	}

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::decrease_transp) }});
	for (int i = 0; i < 10; ++i) {
		ch.ForceMoveRoute(mr, 2);
		testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), prev);

		ForceUpdate(ch);
		testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
		REQUIRE_EQ(ch.GetTransparency(), std::max(prev - 1, 0));

		prev = ch.GetTransparency();
	}
}

TEST_CASE("CommandThrough") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::walk_everywhere_on) }});

	auto testoff = [&]() {
		REQUIRE(!ch.GetThrough());
		ch.SetThrough(true);
		REQUIRE(ch.GetThrough());
		ch.ResetThrough();
		REQUIRE(!ch.GetThrough());
	};

	auto teston = [&]() {
		REQUIRE(ch.GetThrough());
		ch.SetThrough(false);
		REQUIRE(!ch.GetThrough());
		ch.ResetThrough();
		REQUIRE(ch.GetThrough());
	};

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	testoff();

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	teston();

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::walk_everywhere_off) }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	teston();

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	testoff();

	ch.SetThrough(true);

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::walk_everywhere_off) }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.GetThrough());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	testoff();

	ch.SetThrough(true);
	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::walk_everywhere_on) }});
	ch.ForceMoveRoute(mr, 2);
	REQUIRE(ch.GetThrough());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	teston();
}

TEST_CASE("CommandStopAnimation") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::stop_animation) }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE(!ch.IsAnimPaused());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(ch.IsAnimPaused());

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::start_animation) }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(ch.IsAnimPaused());

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(!ch.IsAnimPaused());
}

TEST_CASE("CommandSwitchToggle") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::switch_on), "", 3 }});

	REQUIRE_NE(Main_Data::game_switches, nullptr);

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE(!Main_Data::game_switches->Get(3));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(Main_Data::game_switches->Get(3));

	mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::switch_off), "", 3 }});
	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 128, 0, true, false, mr);
	REQUIRE(Main_Data::game_switches->Get(3));

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE(!Main_Data::game_switches->Get(3));
}

TEST_CASE("CommandChangeGraphic") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::change_graphic), "x", 3 }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);
	REQUIRE_EQ(ch.GetSpriteName(), "");
	REQUIRE_EQ(ch.GetSpriteIndex(), 0);

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);
	REQUIRE_EQ(ch.GetSpriteName(), "x");
	REQUIRE_EQ(ch.GetSpriteIndex(), 3);
}

TEST_CASE("CommandPlaySound") {
	const MapGuard mg;

	auto ch = MoveRouteVehicle();
	auto mr = MakeRoute({{ static_cast<int>(lcf::rpg::MoveCommand::Code::play_sound_effect), "", 100, 100, 100 }});

	ch.ForceMoveRoute(mr, 2);
	testMoveRoute(ch, false, 2, 0xFFFF, 0, 0, true, false, mr);

	ForceUpdate(ch);
	testMoveRoute(ch, false, 2, 0xFFFF + 1, 128, 1, false, false, mr);

	// FIXME: Check mocked audio subsystem?
}

TEST_CASE("ClearPause") {
	auto ch = MoveRouteVehicle();

	ch.SetPaused(true);
	REQUIRE(ch.IsPaused());

	auto mr = MakeRoute({{}});
	ch.ForceMoveRoute(mr, 3);

	REQUIRE(!ch.IsPaused());
}

TEST_SUITE_END();
