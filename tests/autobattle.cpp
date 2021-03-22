#include "test_mock_actor.h"
#include "autobattle.h"
#include "rand.h"
#include "doctest.h"

static Game_Enemy MakeEnemy(int id, int hp, int sp, int atk, int def, int spi, int agi) {
	MakeDBEnemy(id, hp, sp, atk, def, spi, agi);
	auto& tp = lcf::Data::troops[0];
	tp.members.resize(8);
	tp.members[id - 1].enemy_id = id;
	Main_Data::game_enemyparty->ResetBattle(1);
	auto& enemy = (*Main_Data::game_enemyparty)[id - 1];
	return std::move(enemy);
}

decltype(auto) MakeActor(int id, int hp, int sp, int atk, int def, int spi, int agi) {
	auto actor = Game_Actor(id);
	actor.SetBaseMaxHp(hp);
	actor.SetHp(actor.GetMaxHp());
	actor.SetBaseMaxSp(sp);
	actor.SetSp(actor.GetMaxSp());
	actor.SetBaseAtk(atk);
	actor.SetBaseDef(def);
	actor.SetBaseSpi(spi);
	actor.SetBaseAgi(agi);
	return actor;
}

TEST_SUITE_BEGIN("Autobattle");

static void testNormalAttack(const Game_Actor& source, const Game_Battler& target, double v0, double v1, double v2, double v3) {
	REQUIRE(doctest::Approx(v0) == AutoBattle::CalcNormalAttackAutoBattleTargetRank(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, false, true));
	REQUIRE(doctest::Approx(v1) == AutoBattle::CalcNormalAttackAutoBattleTargetRank(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, false, false));
	REQUIRE(doctest::Approx(v2) == AutoBattle::CalcNormalAttackAutoBattleTargetRank(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true, true));
	REQUIRE(doctest::Approx(v3) == AutoBattle::CalcNormalAttackAutoBattleTargetRank(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true, false));
}

TEST_CASE("NormalAttackTargetRank") {
	const MockActor m;

	SUBCASE("actor 120/0 -> enemy 0/90") {
		auto source = MakeActor(1, 500, 500, 120, 0, 0, 0);
		auto target = MakeEnemy(2, 500, 500, 0, 90, 0, 0);

		REQUIRE_EQ(source.GetAtk(), 120);
		REQUIRE_EQ(target.GetDef(), 90);

		SUBCASE("max") {
			Rand::LockGuard lk(INT32_MAX);
			testNormalAttack(source, target, 1.131, 1.131, 1.158, 1.158);
		}
		SUBCASE("min") {
			Rand::LockGuard lk(INT32_MIN);
			testNormalAttack(source, target, 0.141, 0.141, 0.114, 0.114);
		}
	}
}



TEST_SUITE_END();
