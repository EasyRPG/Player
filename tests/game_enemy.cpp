#include "test_mock_actor.h"
#include "doctest.h"

template <typename... Args>
static Game_Actor MakeEnemy(int id, Args... args) {
	MakeDBActor(id, args...);
	return Game_Actor(id);
}

TEST_SUITE_BEGIN("Game_Enemy");

TEST_CASE("Limits2k") {
	const MockActor m(Player::EngineRpg2k);

	auto enemy = Game_Enemy(nullptr);

	REQUIRE_EQ(enemy.MaxHpValue(), 9999);
	REQUIRE_EQ(enemy.MaxStatBaseValue(), 999);
	REQUIRE_EQ(enemy.MaxStatBattleValue(), 9999);
}

TEST_CASE("Limits2k3") {
	const MockActor m(Player::EngineRpg2k3);

	auto enemy = Game_Enemy(nullptr);

	REQUIRE_EQ(enemy.MaxHpValue(), 99999);
	REQUIRE_EQ(enemy.MaxStatBaseValue(), 999);
	REQUIRE_EQ(enemy.MaxStatBattleValue(), 9999);
}

TEST_SUITE_END();
