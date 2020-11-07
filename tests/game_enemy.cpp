#include "test_mock_actor.h"
#include "doctest.h"

static void nullDBEnemy(lcf::rpg::Enemy&) {}

template <typename F = decltype(&nullDBEnemy)>
static Game_Enemy& MakeEnemy(int id, int hp, int sp, int atk, int def, int spi, int agi, const F& f = &nullDBEnemy) {
	auto& tp = lcf::Data::troops[0];
	tp.members.resize(1);
	tp.members[0].enemy_id = id;
	auto* dbe = MakeDBEnemy(id, hp, sp, atk, def, spi, agi);
	f(*dbe);
	Main_Data::game_enemyparty->ResetBattle(1);
	auto& enemy = (*Main_Data::game_enemyparty)[0];
	return enemy;
}

TEST_SUITE_BEGIN("Game_Enemy");

TEST_CASE("Default") {
	const MockActor m;

	auto& e = MakeEnemy(1, 100, 10, 11, 12, 13, 14);
	const auto& ce = e;

	REQUIRE_EQ(e.GetId(), 1);
	REQUIRE_EQ(e.GetType(), Game_Battler::Type_Enemy);

	REQUIRE(e.GetStates().empty());
	REQUIRE(ce.GetStates().empty());

	REQUIRE_EQ(e.GetOriginalPosition(), Point{});

	REQUIRE(e.GetName().empty());
	REQUIRE(e.GetSpriteName().empty());

	REQUIRE_EQ(e.GetBaseMaxHp(), 100);
	REQUIRE_EQ(e.GetBaseMaxSp(), 10);
	REQUIRE_EQ(e.GetHp(), 100);
	REQUIRE_EQ(e.GetSp(), 10);
	REQUIRE_EQ(e.GetBaseAtk(), 11);
	REQUIRE_EQ(e.GetBaseDef(), 12);
	REQUIRE_EQ(e.GetBaseSpi(), 13);
	REQUIRE_EQ(e.GetBaseAgi(), 14);

	REQUIRE_EQ(e.GetHue(), 0);


	REQUIRE_EQ(e.GetBattleAnimationId(), 0);
	REQUIRE_EQ(e.GetFlyingOffset(), 0);
	REQUIRE_EQ(e.IsTransparent(), 0);
	REQUIRE(e.IsInParty());
}

static void testLimits(int hp, int base, int battle) {
	auto& enemy = MakeEnemy(1, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(enemy.MaxHpValue(), hp);
	REQUIRE_EQ(enemy.MaxStatBaseValue(), base);
	REQUIRE_EQ(enemy.MaxStatBattleValue(), battle);

	SUBCASE("up") {
		enemy.SetAtkModifier(999999);
		enemy.SetDefModifier(999999);
		enemy.SetSpiModifier(999999);
		enemy.SetAgiModifier(999999);
		REQUIRE_EQ(enemy.GetAtk(), battle);
		REQUIRE_EQ(enemy.GetDef(), battle);
		REQUIRE_EQ(enemy.GetSpi(), battle);
		REQUIRE_EQ(enemy.GetAgi(), battle);
	}

	SUBCASE("down") {
		enemy.SetAtkModifier(-999999);
		enemy.SetDefModifier(-999999);
		enemy.SetSpiModifier(-999999);
		enemy.SetAgiModifier(-999999);
		REQUIRE_EQ(enemy.GetAtk(), 1);
		REQUIRE_EQ(enemy.GetDef(), 1);
		REQUIRE_EQ(enemy.GetSpi(), 1);
		REQUIRE_EQ(enemy.GetAgi(), 1);
	}
}

TEST_CASE("Limits") {
	SUBCASE("2k") {
		const MockActor m(Player::EngineRpg2k);

		testLimits(9999, 999, 9999);
	}
	SUBCASE("2k3") {
		const MockActor m(Player::EngineRpg2k3);

		testLimits(99999, 999, 9999);
	}
}

static decltype(auto) MakeEnemyHit(bool miss) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) { e.miss = miss; });
}

TEST_CASE("HitRate") {
	const MockActor m;

	SUBCASE("default") {
		auto& e = MakeEnemyHit(false);
		REQUIRE_EQ(e.GetHitChance(), 90);
	}

	SUBCASE("miss") {
		auto& e = MakeEnemyHit(true);
		REQUIRE_EQ(e.GetHitChance(), 70);
	}
}

static decltype(auto) MakeEnemyCrit(bool crit, int rate) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) { e.critical_hit = crit; e.critical_hit_chance = rate; });
}

TEST_CASE("CritRate") {
	const MockActor m;

	SUBCASE("disable_0") {
		auto& e = MakeEnemyCrit(false, 0);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("disable_1") {
		auto& e = MakeEnemyCrit(false, 1);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("disable_30") {
		auto& e = MakeEnemyCrit(false, 30);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("enable_0") {
		auto& e = MakeEnemyCrit(true, 0);
		REQUIRE_EQ(e.GetCriticalHitChance(), std::numeric_limits<float>::infinity());
	}

	SUBCASE("enable_1") {
		auto& e = MakeEnemyCrit(true, 1);
		REQUIRE_EQ(e.GetCriticalHitChance(), 1.0f);
	}

	SUBCASE("enable_30") {
		auto& e = MakeEnemyCrit(true, 30);
		REQUIRE_EQ(e.GetCriticalHitChance(), doctest::Approx(0.03333f));
	}
}

static decltype(auto) MakeEnemyReward(int exp, int gold, int drop_id, int drop_prob) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) {
			e.exp = exp;
			e.gold = gold;
			e.drop_id = drop_id;
			e.drop_prob = drop_prob;
			});
}

TEST_CASE("RewardExp") {
	const MockActor m;

	SUBCASE("0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetExp(), 0);
	}

	SUBCASE("55") {
		auto& e = MakeEnemyReward(55, 0, 0, 0);
		REQUIRE_EQ(e.GetExp(), 55);
	}
}

TEST_CASE("RewardGold") {
	const MockActor m;

	SUBCASE("0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetMoney(), 0);
	}

	SUBCASE("55") {
		auto& e = MakeEnemyReward(0, 55, 0, 0);
		REQUIRE_EQ(e.GetMoney(), 55);
	}
}

TEST_CASE("RewardItem") {
	const MockActor m;

	SUBCASE("0_0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetDropId(), 0);
		REQUIRE_EQ(e.GetDropProbability(), 0);
	}

	SUBCASE("0_55") {
		auto& e = MakeEnemyReward(0, 0, 0, 55);
		REQUIRE_EQ(e.GetDropId(), 0);
		REQUIRE_EQ(e.GetDropProbability(), 55);
	}

	SUBCASE("1_0") {
		auto& e = MakeEnemyReward(0, 0, 1, 0);
		REQUIRE_EQ(e.GetDropId(), 1);
		REQUIRE_EQ(e.GetDropProbability(), 0);
	}

	SUBCASE("1_55") {
		auto& e = MakeEnemyReward(0, 0, 1, 55);
		REQUIRE_EQ(e.GetDropId(), 1);
		REQUIRE_EQ(e.GetDropProbability(), 55);
	}
}

static Game_Enemy& MakeEnemyAttribute(int id, int attr_id, int rank) {
	return MakeEnemy(id, 1, 1, 1, 1, 1, 1, [&](auto&) { SetDBEnemyAttribute(id, attr_id, rank); } );
}

TEST_CASE("Attribute") {
	const MockActor m;

	SUBCASE("0") {
		const auto& enemy = MakeEnemyAttribute(1, 1, 0);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 0);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 0);
	}

	SUBCASE("1") {
		const auto& enemy = MakeEnemyAttribute(1, 1, 1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 1);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 1);
	}

	SUBCASE("2") {
		const auto& enemy = MakeEnemyAttribute(1, 1, 2);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 2);
	}
}

TEST_CASE("BadAttribute") {
	const MockActor m;
	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, -100, 200, 100, 50, 0);

	SUBCASE("0") {
		const auto& enemy = MakeEnemyAttribute(1, 1, 0);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(0), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(0), 2);
	}

	SUBCASE("INT_MAX") {
		const auto& enemy = MakeEnemyAttribute(1, 1, 0);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(INT_MAX), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(INT_MAX), 2);
	}
}

TEST_CASE("AttributeShift") {
	const MockActor m;

	SUBCASE("normal") {
		auto& enemy = MakeEnemyAttribute(1, 1, 2);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 0);
		REQUIRE(enemy.CanShiftAttributeRate(1, 1));
		REQUIRE_EQ(1, enemy.CanShiftAttributeRate(1, 2));
		REQUIRE(enemy.CanShiftAttributeRate(1, -1));
		REQUIRE_EQ(-1, enemy.CanShiftAttributeRate(1, -2));

		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 0);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 2);

		enemy.ShiftAttributeRate(1, 1);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 3);

		enemy.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 0);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 2);

		enemy.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 1);

		enemy.ShiftAttributeRate(1, 100);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 3);

		enemy.ShiftAttributeRate(1, -100);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 1);
	}

	SUBCASE("overflow") {
		auto& enemy = MakeEnemyAttribute(1, 1, 4);
		enemy.ShiftAttributeRate(1, 1);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 4);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 4);
	}

	SUBCASE("underflow") {
		auto& enemy = MakeEnemyAttribute(1, 1, 0);
		enemy.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(enemy.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(enemy.GetBaseAttributeRate(1), 0);
		REQUIRE_EQ(enemy.GetAttributeRate(1), 0);
	}
}

TEST_CASE("AttributeShiftInvalid") {
	const MockActor m;
	auto& enemy = MakeEnemyAttribute(1, 1, 100);

	REQUIRE_EQ(enemy.GetAttributeRateShift(0), 0);
	REQUIRE_EQ(enemy.GetAttributeRateShift(INT_MAX), 0);
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(0, 1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(0, -1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(INT_MAX, 1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(INT_MAX, -1));

	enemy.ShiftAttributeRate(0, 1);
	enemy.ShiftAttributeRate(INT_MAX, 1);

	REQUIRE_EQ(enemy.GetAttributeRateShift(0), 0);
	REQUIRE_EQ(enemy.GetAttributeRateShift(INT_MAX), 0);
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(0, 1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(0, -1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(INT_MAX, 1));
	REQUIRE_FALSE(enemy.CanShiftAttributeRate(INT_MAX, -1));
}

TEST_CASE("ChangeHp") {
	const MockActor m;

	auto& enemy = MakeEnemy(1, 500, 500, 500, 500, 500, 500);

	REQUIRE_EQ(enemy.GetHp(), 500);

	SUBCASE("dmg") {
		REQUIRE_EQ(enemy.ChangeHp(-9999, true), -500);
		REQUIRE_EQ(enemy.GetHp(), 0);
		REQUIRE(enemy.IsDead());

		REQUIRE_EQ(enemy.ChangeHp(9999, true), 0);
	}

	SUBCASE("kill") {
		enemy.Kill();
		REQUIRE_EQ(enemy.GetHp(), 0);
		REQUIRE(enemy.IsDead());

		REQUIRE_EQ(enemy.ChangeHp(9999, true), 0);
	}

	SUBCASE("dmg_nokill") {
		REQUIRE_EQ(enemy.ChangeHp(-9999, false), -499);
		REQUIRE_EQ(enemy.GetHp(), 1);
		REQUIRE_FALSE(enemy.IsDead());

		REQUIRE_EQ(enemy.ChangeHp(9999, true), 499);
		REQUIRE_EQ(enemy.GetHp(), 500);
		REQUIRE_FALSE(enemy.IsDead());
	}
}

TEST_CASE("ChangeSp") {
	const MockActor m;

	auto& enemy = MakeEnemy(1, 500, 500, 500, 500, 500, 500);

	REQUIRE_EQ(enemy.GetSp(), 500);

	REQUIRE_EQ(enemy.ChangeSp(-9999), -500);
	REQUIRE_EQ(enemy.GetSp(), 0);

	REQUIRE_EQ(enemy.ChangeSp(9999), 500);
}

TEST_CASE("ChangeParam") {
	const MockActor m;

	auto& enemy = MakeEnemy(1, 500, 500, 200, 300, 400, 500);

	REQUIRE_EQ(enemy.GetAtk(), 200);
	REQUIRE_EQ(enemy.GetDef(), 300);
	REQUIRE_EQ(enemy.GetSpi(), 400);
	REQUIRE_EQ(enemy.GetAgi(), 500);

	SUBCASE("atk") {
		REQUIRE_EQ(enemy.ChangeAtkModifier(-9999), -100);
		REQUIRE_EQ(enemy.GetAtk(), 100);

		REQUIRE_EQ(enemy.ChangeAtkModifier(9999), 300);
		REQUIRE_EQ(enemy.GetAtk(), 400);
	}

	SUBCASE("def") {
		REQUIRE_EQ(enemy.ChangeDefModifier(-9999), -150);
		REQUIRE_EQ(enemy.GetDef(), 150);

		REQUIRE_EQ(enemy.ChangeDefModifier(9999), 450);
		REQUIRE_EQ(enemy.GetDef(), 600);
	}

	SUBCASE("spi") {
		REQUIRE_EQ(enemy.ChangeSpiModifier(-9999), -200);
		REQUIRE_EQ(enemy.GetSpi(), 200);

		REQUIRE_EQ(enemy.ChangeSpiModifier(9999), 600);
		REQUIRE_EQ(enemy.GetSpi(), 800);
	}

	SUBCASE("agi") {
		REQUIRE_EQ(enemy.ChangeAgiModifier(-9999), -250);
		REQUIRE_EQ(enemy.GetAgi(), 250);

		REQUIRE_EQ(enemy.ChangeAgiModifier(9999), 750);
		REQUIRE_EQ(enemy.GetAgi(), 1000);
	}
}

TEST_SUITE_END();
