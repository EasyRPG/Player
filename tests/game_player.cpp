#include "game_player.h"
#include "doctest.h"
#include "options.h"
#include "game_map.h"
#include "main_data.h"
#include <climits>

namespace {
struct MapGuard {
	MapGuard() {
		Game_Map::Init();
		Main_Data::game_player = std::make_unique<Game_Player>();
	}

	MapGuard(const MapGuard&) = delete;
	MapGuard& operator=(const MapGuard&) = delete;
	~MapGuard() {
		Main_Data::game_player = {};
		Game_Map::Quit();
	}
};
}

TEST_SUITE_BEGIN("Game_Player");

TEST_CASE("SaveCounts") {
	Game_Player ch;

	REQUIRE(!ch.IsMapCompatibleWithSave(-1));
	REQUIRE(ch.IsMapCompatibleWithSave(0));
	REQUIRE(!ch.IsMapCompatibleWithSave(1));

	REQUIRE(!ch.IsDatabaseCompatibleWithSave(-1));
	REQUIRE(ch.IsDatabaseCompatibleWithSave(0));
	REQUIRE(!ch.IsDatabaseCompatibleWithSave(1));

	ch.UpdateSaveCounts(55, 77);

	REQUIRE(!ch.IsMapCompatibleWithSave(76));
	REQUIRE(ch.IsMapCompatibleWithSave(77));
	REQUIRE(!ch.IsMapCompatibleWithSave(78));

	REQUIRE(!ch.IsDatabaseCompatibleWithSave(54));
	REQUIRE(ch.IsDatabaseCompatibleWithSave(55));
	REQUIRE(!ch.IsDatabaseCompatibleWithSave(56));
}

static void testPan(Game_Player& ch, bool active, bool locked, int dx, int dy, int dtx, int dty, int w) {
	REQUIRE_EQ(ch.IsPanActive(), active);
	REQUIRE_EQ(ch.IsPanLocked(), locked);
	REQUIRE_EQ(ch.GetPanX(), RPG::SavePartyLocation::kPanXDefault + dx * SCREEN_TILE_SIZE);
	REQUIRE_EQ(ch.GetPanY(), RPG::SavePartyLocation::kPanYDefault + dy * SCREEN_TILE_SIZE);
	REQUIRE_EQ(ch.GetTargetPanX(), RPG::SavePartyLocation::kPanXDefault + dtx * SCREEN_TILE_SIZE);
	REQUIRE_EQ(ch.GetTargetPanY(), RPG::SavePartyLocation::kPanYDefault + dty * SCREEN_TILE_SIZE);
	REQUIRE_EQ(ch.GetPanWait(), w);
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


TEST_CASE("AnimatePan") {
	MapGuard mg;

	auto& ch = *Main_Data::game_player;

	ch.StartPan(Game_Player::PanLeft, 1, 1);
	ch.StartPan(Game_Player::PanUp, 2, 1);

	testPan(ch, true, false, 0, 0, 1, 2, 128);

	// FIXME: Requires setting up a map file
	//ch.Update();
}

TEST_SUITE_END();
