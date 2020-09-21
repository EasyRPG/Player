#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "main_data.h"
#include <climits>

#include "mock_game.h"

TEST_SUITE_BEGIN("Game_Player");

static void testPanAbs(Game_Player& ch, bool active, bool locked, int dx, int dy, int dtx, int dty, int w) {
	CAPTURE(active);
	CAPTURE(locked);
	CAPTURE(dx);
	CAPTURE(dy);
	CAPTURE(dtx);
	CAPTURE(dty);
	CAPTURE(w);

	REQUIRE_EQ(ch.IsPanActive(), active);
	REQUIRE_EQ(ch.IsPanLocked(), locked);
	REQUIRE_EQ(ch.GetPanX(), lcf::rpg::SavePartyLocation::kPanXDefault + dx);
	REQUIRE_EQ(ch.GetPanY(), lcf::rpg::SavePartyLocation::kPanYDefault + dy);
	REQUIRE_EQ(ch.GetTargetPanX(), lcf::rpg::SavePartyLocation::kPanXDefault + dtx);
	REQUIRE_EQ(ch.GetTargetPanY(), lcf::rpg::SavePartyLocation::kPanYDefault + dty);
	REQUIRE_EQ(ch.GetPanWait(), w);
}

static void testPan(Game_Player& ch, bool active, bool locked, int dx, int dy, int dtx, int dty, int w) {
	testPanAbs(ch, active, locked, dx * SCREEN_TILE_SIZE, dy * SCREEN_TILE_SIZE, dtx * SCREEN_TILE_SIZE, dty * SCREEN_TILE_SIZE, w);
}

TEST_CASE("StartResetPan") {
	Game_Player ch;

	ch.StartPan(Game_Player::PanRight, 16, 1);
	testPan(ch, true, false, 0, 0, -16, 0, 1024);

	ch.StartPan(Game_Player::PanUp, 8, 1);
	testPan(ch, true, false, 0, 0, -16, 8, 1024);

	ch.StartPan(Game_Player::PanLeft, 4, 1);
	testPan(ch, true, false, 0, 0, -12, 8, 768);

	ch.StartPan(Game_Player::PanDown, 9, 1);
	testPan(ch, true, false, 0, 0, -12, -1, 768);

	ch.ResetPan(2);

	testPan(ch, false, false, 0, 0, 0, 0, 0);
}

static void testPanSpeed(int dist, int speed, int wait) {
	Game_Player ch;
	ch.StartPan(Game_Player::PanRight, dist, speed);
	REQUIRE_EQ(ch.GetPanWait(), wait);
}

TEST_CASE("PanSpeed") {
	testPanSpeed(1, 1, 64);
	testPanSpeed(1, 2, 32);
	testPanSpeed(1, 3, 16);
	testPanSpeed(1, 4, 8);
	testPanSpeed(1, 5, 4);
	testPanSpeed(1, 6, 2);

	testPanSpeed(2, 1, 128);
	testPanSpeed(2, 2, 64);
	testPanSpeed(2, 3, 32);
	testPanSpeed(2, 4, 16);
	testPanSpeed(2, 5, 8);
	testPanSpeed(2, 6, 4);
}

TEST_CASE("LockPan") {
	Game_Player ch;

	REQUIRE(!ch.IsPanLocked());
	ch.StartPan(Game_Character::Right, 1, 1);
	REQUIRE(!ch.IsPanLocked());
	ch.ResetPan(1);
	REQUIRE(!ch.IsPanLocked());

	ch.LockPan();
	REQUIRE(ch.IsPanLocked());
	ch.StartPan(Game_Character::Right, 1, 1);
	REQUIRE(ch.IsPanLocked());
	ch.ResetPan(1);
	REQUIRE(ch.IsPanLocked());

	ch.UnlockPan();
	REQUIRE(!ch.IsPanLocked());
}

static void testAnimPan(Game_Player::PanDirection dir_x, Game_Player::PanDirection dir_y, int speed) {
	static constexpr auto map_id = MockMap::ePass40x30;

	const MockGame mg(map_id);
	auto& ch = *mg.GetPlayer();

	ch.SetMapId(static_cast<int>(map_id));
	ch.MoveTo(static_cast<int>(map_id), 20, 15);

	ch.StartPan(dir_x, 1, speed);
	ch.StartPan(dir_y, 2, speed);

	auto wait = 2 * SCREEN_TILE_SIZE / (2 << speed);

	int step_x = 2 << speed;
	int step_y = 2 << speed;
	int dx = 0;
	int dy = 0;
	int fx = 1 * SCREEN_TILE_SIZE;
	int fy = 2 * SCREEN_TILE_SIZE;

	if (dir_x == Game_Player::PanRight) {
		fx = -1 * SCREEN_TILE_SIZE;
	}
	if (dir_y == Game_Player::PanDown) {
		fy = -2 * SCREEN_TILE_SIZE;
	}

	CAPTURE(speed);
	CAPTURE(dir_x);
	CAPTURE(dir_y);
	CAPTURE(step_x);
	CAPTURE(step_y);
	CAPTURE(wait);
	CAPTURE(fx);
	CAPTURE(fy);

	testPanAbs(ch, true, false, 0, 0, fx, fy, wait);

	for (int i = 1; i < wait; ++i) {
		ForceUpdate(ch);
		if (dir_x == Game_Player::PanRight) {
			dx = std::max(dx - step_x, fx);
		} else {
			dx = std::min(dx + step_x, fx);
		}
		if (dir_y == Game_Player::PanDown) {
			dy = std::max(dy - step_y, fy);
		} else {
			dy = std::min(dy + step_y, fy);
		}
		testPanAbs(ch, true, false, dx, dy, fx, fy, wait - i);
	}

	ForceUpdate(ch);
	testPanAbs(ch, false, false, fx, fy, fx, fy, 0);

	ch.ResetPan(speed);

	testPanAbs(ch, true, false, fx, fy, 0, 0, wait);

	dx = fx;
	dy = fy;
	for (int i = 1; i < wait; ++i) {
		ForceUpdate(ch);
		if (dir_x == Game_Player::PanRight) {
			dx = std::min(dx + step_x, 0);
		} else {
			dx = std::max(dx - step_x, 0);
		}
		if (dir_y == Game_Player::PanDown) {
			dy = std::min(dy + step_y, 0);
		} else {
			dy = std::max(dy - step_y, 0);
		}
		testPanAbs(ch, true, false, dx, dy, 0, 0, wait - i);
	}

	ForceUpdate(ch);
	testPanAbs(ch, false, false, 0, 0, 0, 0, 0);
}

TEST_CASE("AnimatePan") {
	for (int speed = 1; speed <= 6; ++speed) {
		testAnimPan(Game_Player::PanLeft, Game_Player::PanUp, speed);
		testAnimPan(Game_Player::PanRight, Game_Player::PanDown, speed);
	}
}

static void testAnimBlocked(Game_Player::PanDirection dir_x, Game_Player::PanDirection dir_y, int speed) {
	static constexpr auto map_id = MockMap::ePassBlock20x15;

	const MockGame mg(map_id);
	auto& ch = *mg.GetPlayer();

	ch.SetMapId(static_cast<int>(map_id));
	ch.MoveTo(static_cast<int>(map_id), 0, 0);

	const auto wait = 2 * SCREEN_TILE_SIZE / (2 << speed);
	int fx = 1 * SCREEN_TILE_SIZE;
	int fy = 2 * SCREEN_TILE_SIZE;

	if (dir_x == Game_Player::PanRight) {
		fx = -1 * SCREEN_TILE_SIZE;
	}
	if (dir_y == Game_Player::PanDown) {
		fy = -2 * SCREEN_TILE_SIZE;
	}

	ch.StartPan(dir_x, 1, speed);
	ch.StartPan(dir_y, 2, speed);

	testPanAbs(ch, true, false, 0, 0, fx, fy, wait);

	for (int i = 1; i < 10; ++i) {
		ForceUpdate(ch);
		testPanAbs(ch, true, false, 0, 0, fx, fy, wait);
	}

	ch.ResetPan(speed);
	testPan(ch, false, false, 0, 0, 0, 0, 0);
}

TEST_CASE("BlockedPan") {
	for (int speed = 1; speed <= 6; ++speed) {
		testAnimBlocked(Game_Player::PanLeft, Game_Player::PanUp, speed);
		testAnimBlocked(Game_Player::PanRight, Game_Player::PanDown, speed);
	}
}


TEST_SUITE_END();
