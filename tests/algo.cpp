#include "test_mock_actor.h"
#include "algo.h"
#include "rand.h"
#include "doctest.h"

TEST_SUITE_BEGIN("Algo");

static void testRowAdj(lcf::rpg::SaveActor::RowType row, bool offense, bool none, bool back, bool surround, bool pincers) {
	REQUIRE_EQ(none, Algo::IsRowAdjusted(row, lcf::rpg::System::BattleCondition_none, offense));
	REQUIRE_EQ(none, Algo::IsRowAdjusted(row, lcf::rpg::System::BattleCondition_initiative, offense));
	REQUIRE_EQ(back, Algo::IsRowAdjusted(row, lcf::rpg::System::BattleCondition_back, offense));
	REQUIRE_EQ(surround, Algo::IsRowAdjusted(row, lcf::rpg::System::BattleCondition_surround, offense));
	REQUIRE_EQ(pincers, Algo::IsRowAdjusted(row, lcf::rpg::System::BattleCondition_pincers, offense));
}


TEST_CASE("RowAdj") {
	const MockActor m;

	SUBCASE("front") {
		auto row = lcf::rpg::SaveActor::RowType_front;

		SUBCASE("offsense") {
			testRowAdj(row, true, true, false, true, false);
		}

		SUBCASE("defense") {
			testRowAdj(row, false, false, true, true, false);
		}
	}

	SUBCASE("back") {
		auto row = lcf::rpg::SaveActor::RowType_back;

		SUBCASE("offsense") {
			testRowAdj(row, true, false, true, true, false);
		}

		SUBCASE("defense") {
			testRowAdj(row, false, true, false, true, false);
		}
	}
}

static void testRowAdjBattler(const Game_Battler& battler, bool offense, bool bug,
		bool none, bool back, bool surround, bool pincers) {
	REQUIRE_EQ(none, Algo::IsRowAdjusted(battler, lcf::rpg::System::BattleCondition_none, offense, bug));
	REQUIRE_EQ(none, Algo::IsRowAdjusted(battler, lcf::rpg::System::BattleCondition_initiative, offense, bug));
	REQUIRE_EQ(back, Algo::IsRowAdjusted(battler, lcf::rpg::System::BattleCondition_back, offense, bug));
	REQUIRE_EQ(surround, Algo::IsRowAdjusted(battler, lcf::rpg::System::BattleCondition_surround, offense, bug));
	REQUIRE_EQ(pincers, Algo::IsRowAdjusted(battler, lcf::rpg::System::BattleCondition_pincers, offense, bug));
}

TEST_CASE("RowAdjBattler") {
	const MockBattle mb;

	SUBCASE("actor") {
		auto& actor = *Main_Data::game_party->GetActor(0);

		SUBCASE("front") {
			actor.SetBattleRow(lcf::rpg::SaveActor::RowType_front);

			SUBCASE("offsense") {
				testRowAdjBattler(actor, true, true, true, false, true, false);
				testRowAdjBattler(actor, true, false, true, false, true, false);
			}

			SUBCASE("defense") {
				testRowAdjBattler(actor, false, true, false, true, true, false);
				testRowAdjBattler(actor, false, false, false, true, true, false);
			}
		}

		SUBCASE("back") {
			actor.SetBattleRow(lcf::rpg::SaveActor::RowType_back);

			SUBCASE("offsense") {
				testRowAdjBattler(actor, true, true, false, true, true, false);
				testRowAdjBattler(actor, true, false, false, true, true, false);
			}

			SUBCASE("defense") {
				testRowAdjBattler(actor, false, true, true, false, true, false);
				testRowAdjBattler(actor, false, false, true, false, true, false);
			}
		}
	}

	SUBCASE("enemy") {
		auto& enemy = *Main_Data::game_enemyparty->GetEnemy(0);

		SUBCASE("offsense") {
			testRowAdjBattler(enemy, true, true, true, false, true, false);
			testRowAdjBattler(enemy, true, false, false, false, false, false);
		}

		SUBCASE("defense") {
			testRowAdjBattler(enemy, false, true, false, true, true, false);
			testRowAdjBattler(enemy, false, false, false, false, false, false);
		}
	}
}

TEST_CASE("Variance") {
	SUBCASE("0 var disabled") {
		REQUIRE_EQ(Algo::VarianceAdjustEffect(100, 0), 100);
	}

	SUBCASE(">0") {
		SUBCASE("max") {
			Rand::LockGuard lk(INT32_MAX);
			for (int var = 1; var <= 10; ++var) {
				REQUIRE_EQ(100 + 5 * var, Algo::VarianceAdjustEffect(100, var));
			}
		}
		SUBCASE("min") {
			Rand::LockGuard lk(INT32_MIN);
			for (int var = 1; var <= 10; ++var) {
				REQUIRE_EQ(100 - 5 * var, Algo::VarianceAdjustEffect(100, var));
			}
		}
	}

	SUBCASE("one") {
		SUBCASE("max") {
			Rand::LockGuard lk(INT32_MAX);
			REQUIRE_EQ(2, Algo::VarianceAdjustEffect(1, 10));
		}
		SUBCASE("min") {
			Rand::LockGuard lk(INT32_MIN);
			REQUIRE_EQ(1, Algo::VarianceAdjustEffect(1, 10));
		}
	}

	SUBCASE("zero") {

		SUBCASE("modern") {
			const MockActor m(Player::EngineEnglish);
			REQUIRE_FALSE(Player::IsLegacy());

			REQUIRE_EQ(Algo::VarianceAdjustEffect(0, 0), 0);
			REQUIRE_EQ(Algo::VarianceAdjustEffect(0, 1), 0);
		}
		SUBCASE("legacy") {
			const MockActor m(Player::EngineRpg2k);
			REQUIRE(Player::IsLegacy());

			SUBCASE("max") {
				Rand::LockGuard lk(INT32_MAX);
				REQUIRE_EQ(0, Algo::VarianceAdjustEffect(0, 0));
				REQUIRE_EQ(1, Algo::VarianceAdjustEffect(0, 1));
			}
			SUBCASE("min") {
				Rand::LockGuard lk(INT32_MIN);
				REQUIRE_EQ(0, Algo::VarianceAdjustEffect(0, 0));
				REQUIRE_EQ(0, Algo::VarianceAdjustEffect(0, 1));
			}
		}
	}

	SUBCASE("neg never varies") {
		REQUIRE_EQ(Algo::VarianceAdjustEffect(-100, 0), -100);
	}
}

void makeAgiSkills() {
	// Does physical evasion
	auto* s = MakeDBSkill(1, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_enemy;
	s->failure_message = 3;

	s = MakeDBSkill(2, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_enemies;
	s->failure_message = 3;

	// All ignore physical evasion
	s = MakeDBSkill(3, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_self;
	s->failure_message = 3;

	s = MakeDBSkill(4, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_ally;
	s->failure_message = 3;

	s = MakeDBSkill(5, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_party;
	s->failure_message = 3;

	s = MakeDBSkill(6, 90, 0, 0, 0, 0);
	s->scope = lcf::rpg::Skill::Scope_enemy;
	s->failure_message = 0;
}

auto* MakeAgiEnemy(int idx, int agi) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(idx);
	Setup(enemy, 1, 1, 1, 1, 1, agi);
	return enemy;
}

auto* MakeAgiActor(int idx, int agi) {
	auto* actor = Main_Data::game_party->GetActor(idx);
	Setup(actor, 1, 1, 1, 1, 1, agi);
	return actor;
}

static void testAgi(int src, int tgt, int res) {
	auto& source = *MakeAgiActor(0, src);
	auto& target = *MakeAgiEnemy(0, tgt);

	REQUIRE_EQ(res, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
	REQUIRE_EQ(res, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	REQUIRE_EQ(res, Algo::CalcSkillToHit(source, target, lcf::Data::skills[1], lcf::rpg::System::BattleCondition_none, false));
	for (int i = 2; i < 6; ++i) {
		CAPTURE(i);
		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[i], lcf::rpg::System::BattleCondition_none, false));

	}
}

TEST_CASE("HitRateAgi") {
	const MockBattle mb;
	makeAgiSkills();

	SUBCASE("100_100") { testAgi(100, 100, 90); }
	SUBCASE("100_50") { testAgi(100, 50, 92); }
	SUBCASE("50_100") { testAgi(50, 100, 85); }
	SUBCASE("10_1") { testAgi(10, 1, 94); }
	SUBCASE("10_0") { testAgi(10, 0, 94); }
	SUBCASE("0_1") { testAgi(0, 1, 90); }
}

static void testStates(Game_Battler& source, Game_Battler& target, int base) {
	SUBCASE("Cannot act") {
		target.AddState(2, true);
		REQUIRE_EQ(100, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
		REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	}

	SUBCASE("Avoid attacks") {
		target.AddState(3, true);
		REQUIRE_EQ(0, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
		// RPG_RT bug
		REQUIRE_EQ(base, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	}

	SUBCASE("Avoid attacks and cannot act") {
		target.AddState(2, true);
		target.AddState(3, true);
		REQUIRE_EQ(0, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
		// RPG_RT bug
		REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	}
}

TEST_CASE("HitRateStates") {
	const MockBattle mb;
	makeAgiSkills();
	lcf::Data::states[2 - 1].restriction = lcf::rpg::State::Restriction_do_nothing;
	lcf::Data::states[3 - 1].avoid_attacks = true;
	lcf::Data::states[4 - 1].reduce_hit_ratio = 50;

	auto& source = *MakeAgiActor(0, 100);
	auto& target = *MakeAgiEnemy(0, 100);

	testStates(source, target, 90);

	SUBCASE("source blind50") {
		source.AddState(4, true);
		REQUIRE_EQ(45, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
		REQUIRE_EQ(45, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));

		testStates(source, target, 45);
	}
}

static void testHitRateRow(Game_Battler& source, Game_Battler& target, int none, int back, int surround, int pincer, int back_no_bug, int surround_no_bug) {
	REQUIRE_EQ(none, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));
	REQUIRE_EQ(none, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative, true));
	REQUIRE_EQ(back, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back, true));
	REQUIRE_EQ(surround, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround, true));
	REQUIRE_EQ(pincer, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers, true));

	REQUIRE_EQ(none, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, false));
	REQUIRE_EQ(none, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative, false));
	REQUIRE_EQ(back_no_bug, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back, false));
	REQUIRE_EQ(surround_no_bug, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround, false));
	REQUIRE_EQ(pincer, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers, false));
}

TEST_CASE("HitRateArmorAndRow2k3") {
	const MockBattle mb(4, 4, Player::EngineRpg2k3);
	makeAgiSkills();
	MakeDBEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 0, false, false, false, false, false, true, false, false);

	auto& source = *MakeAgiActor(1, 100);

	SUBCASE("actor target") {
		auto& target = *MakeAgiActor(2, 100);

		SUBCASE("Front") {
			target.SetBattleRow(lcf::rpg::SaveActor::RowType_front);

			SUBCASE("no armor") {
				testHitRateRow(source, target, 90, 65, 65, 90, 65, 65);

				REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
			}

			SUBCASE("phys eva up") {
				target.SetEquipment(3, 1);
				REQUIRE(target.HasPhysicalEvasionUp());

				testHitRateRow(source, target, 65, 40, 40, 65, 40, 40);

				REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
			}
		}

		SUBCASE("Back") {
			target.SetBattleRow(lcf::rpg::SaveActor::RowType_back);

			SUBCASE("no armor") {
				testHitRateRow(source, target, 65, 90, 65, 90, 90, 65);

				REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
			}

			SUBCASE("phys eva up") {
				target.SetEquipment(3, 1);
				REQUIRE(target.HasPhysicalEvasionUp());

				testHitRateRow(source, target, 40, 65, 40, 65, 65, 40);

				REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
			}
		}
	}

	SUBCASE("enemy target") {
		auto& target = *MakeAgiEnemy(2, 100);

		testHitRateRow(source, target, 90, 65, 65, 90, 90, 90);

		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	}
}

static void testHitRateRow2k(Game_Battler& source, Game_Battler& target, int rate) {
	testHitRateRow(source, target, rate, rate, rate, rate, rate, rate);
}

TEST_CASE("HitRateArmorAndRow2k") {
	const MockBattle mb(4, 4, Player::EngineRpg2k);
	makeAgiSkills();
	MakeDBEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 0, false, false, false, false, false, true, false, false);

	auto& source = *MakeAgiActor(1, 100);

	SUBCASE("actor target") {
		auto& target = *MakeAgiActor(2, 100);

		SUBCASE("no armor") {
			testHitRateRow2k(source, target, 90);

			REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
		}

		SUBCASE("phys eva up") {
			target.SetEquipment(3, 1);
			REQUIRE(target.HasPhysicalEvasionUp());

			testHitRateRow2k(source, target, 65);

			REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
		}
	}

	SUBCASE("enemy target") {
		auto& target = *MakeAgiEnemy(2, 100);

		testHitRateRow2k(source, target, 90);

		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
	}
}

TEST_CASE("HitRateWeapons") {
	const MockBattle mb(4, 4, Player::EngineRpg2k3);
	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 90, 0, false, false, false, true, false, false, false, false);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 50, 90, 0, false, false, false, false, false, false, false, false);

	auto& source = *MakeAgiActor(1, 100);
	auto& target = *MakeAgiEnemy(1, 200);

	source.SetEquipment(1, 1);
	source.SetEquipment(2, 2);

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponAll), 150);
	REQUIRE(source.AttackIgnoresEvasion(Game_Battler::WeaponAll));
	REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none, true));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponNone), 100);
	REQUIRE_FALSE(source.AttackIgnoresEvasion(Game_Battler::WeaponNone));
	REQUIRE_EQ(85, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponNone, lcf::rpg::System::BattleCondition_none, true));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponPrimary), 100);
	REQUIRE(source.AttackIgnoresEvasion(Game_Battler::WeaponPrimary));
	REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponPrimary, lcf::rpg::System::BattleCondition_none, true));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponSecondary), 150);
	REQUIRE_FALSE(source.AttackIgnoresEvasion(Game_Battler::WeaponSecondary));
	REQUIRE_EQ(88, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponSecondary, lcf::rpg::System::BattleCondition_none, true));

	REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0], lcf::rpg::System::BattleCondition_none, false));
}

TEST_CASE("CritRate") {
	const MockBattle mb(4, 4, Player::EngineRpg2k3);
	lcf::Data::enemies[0].critical_hit = true;
	lcf::Data::enemies[0].critical_hit_chance = 30;
	lcf::Data::enemies[1].critical_hit = true;
	lcf::Data::enemies[1].critical_hit_chance = 30;
	MakeDBEquip(1, lcf::rpg::Item::Type_weapon)->critical_hit = 50;
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon)->critical_hit = 80;
	MakeDBEquip(3, lcf::rpg::Item::Type_armor)->prevent_critical = true;

	SUBCASE("actor -> actor - always fails") {
		auto& source = *Main_Data::game_party->GetActor(0);
		auto& target = *Main_Data::game_party->GetActor(1);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
	}

	SUBCASE("enemy -> enemy - always fails") {
		auto& source = *Main_Data::game_enemyparty->GetEnemy(0);
		auto& target = *Main_Data::game_enemyparty->GetEnemy(1);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
	}

	SUBCASE("actor -> enemy") {
		auto& source = *Main_Data::game_party->GetActor(0);
		auto& target = *Main_Data::game_enemyparty->GetEnemy(0);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		SUBCASE("baseline") {
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary, -1));
		}

		SUBCASE("weapons") {
			source.SetEquipment(1, 1);
			source.SetEquipment(2, 2);

			REQUIRE_EQ(83, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone, -1));
			REQUIRE_EQ(53, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary, -1));
			REQUIRE_EQ(83, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary, -1));
		}
	}

	SUBCASE("enemy -> actor") {
		auto& source = *Main_Data::game_enemyparty->GetEnemy(0);
		auto& target = *Main_Data::game_party->GetActor(0);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		SUBCASE("baseline") {
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary, -1));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary, -1));
		}

		SUBCASE("armor prevents critical") {
			target.SetEquipment(3, 3);

			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll, -1));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone, -1));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary, -1));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary, -1));
		}
	}
}

auto* MakeStatEnemy(int idx, int atk, int def, int spi) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(idx);
	Setup(enemy, 1, 1, atk, def, spi, 1);
	return enemy;
}

auto* MakeStatActor(int idx, int atk, int def, int spi) {
	auto* actor = Main_Data::game_party->GetActor(idx);
	Setup(actor, 1, 1, atk, def, spi, 1);
	return actor;
}

TEST_CASE("DefendAdjustment") {
	const MockBattle m;
	auto target = Main_Data::game_party->GetActor(0);

	SUBCASE("no strong") {

		SUBCASE("baseline") {
			REQUIRE_EQ(100, Algo::AdjustDamageForDefend(100, *target));
		}

		SUBCASE("defend") {
			target->SetIsDefending(true);
			REQUIRE_EQ(50, Algo::AdjustDamageForDefend(100, *target));
		}
	}

	SUBCASE("strong") {
		target->SetStrongDefense(true);

		SUBCASE("baseline") {
			REQUIRE_EQ(100, Algo::AdjustDamageForDefend(100, *target));
		}

		SUBCASE("defend") {
			target->SetIsDefending(true);
			REQUIRE_EQ(25, Algo::AdjustDamageForDefend(100, *target));
		}
	}
}

TEST_CASE("SelfDestructEffect") {
	const MockBattle m;

	auto target = MakeStatActor(1, 0, 100, 0);

	SUBCASE("100_100") {
		auto source = MakeStatEnemy(1, 100, 0, 0);
		REQUIRE_EQ(50, Algo::CalcSelfDestructEffect(*source, *target, false));
	}

	SUBCASE("200_100") {
		auto source = MakeStatEnemy(1, 200, 0, 0);
		REQUIRE_EQ(150, Algo::CalcSelfDestructEffect(*source, *target, false));
	}

	SUBCASE("1_100") {
		auto source = MakeStatEnemy(1, 1, 0, 0);
		REQUIRE_EQ(0, Algo::CalcSelfDestructEffect(*source, *target, false));
	}

	SUBCASE("150_100") {
		auto source = MakeStatEnemy(1, 150, 0, 0);
		REQUIRE_EQ(100, Algo::CalcSelfDestructEffect(*source, *target, false));
	}
}

static void testSkillStats(int power, int phys, int mag, Game_Battler& source, Game_Battler& target, int dmg, int heal) {
	MakeDBSkill(1, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemy;
	MakeDBSkill(2, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemies;
	MakeDBSkill(3, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_self;
	MakeDBSkill(4, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_ally;
	MakeDBSkill(5, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_party;

	MakeDBSkill(6, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemy;
	SetDBSkillAttribute(6, 1, true);
	MakeDBSkill(7, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemies;
	SetDBSkillAttribute(7, 1, true);
	MakeDBSkill(8, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_self;
	SetDBSkillAttribute(8, 1, true);
	MakeDBSkill(9, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_ally;
	SetDBSkillAttribute(9, 1, true);
	MakeDBSkill(10, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_party;
	SetDBSkillAttribute(10, 1, true);

	MakeDBSkill(11, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemy;
	lcf::Data::skills[10].ignore_defense = true;
	MakeDBSkill(12, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemies;
	lcf::Data::skills[11].ignore_defense = true;
	MakeDBSkill(13, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_self;
	lcf::Data::skills[12].ignore_defense = true;
	MakeDBSkill(14, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_ally;
	lcf::Data::skills[13].ignore_defense = true;
	MakeDBSkill(15, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_party;
	lcf::Data::skills[14].ignore_defense = true;

	MakeDBSkill(16, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemy;
	SetDBSkillAttribute(16, 1, true);
	lcf::Data::skills[15].ignore_defense = true;
	MakeDBSkill(17, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_enemies;
	SetDBSkillAttribute(17, 1, true);
	lcf::Data::skills[16].ignore_defense = true;
	MakeDBSkill(18, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_self;
	SetDBSkillAttribute(18, 1, true);
	lcf::Data::skills[17].ignore_defense = true;
	MakeDBSkill(19, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_ally;
	SetDBSkillAttribute(19, 1, true);
	lcf::Data::skills[18].ignore_defense = true;
	MakeDBSkill(20, 100, power, phys, mag, 0)->scope = lcf::rpg::Skill::Scope_party;
	SetDBSkillAttribute(20, 1, true);
	lcf::Data::skills[19].ignore_defense = true;

	SUBCASE("baseline") {
		REQUIRE_EQ(dmg, Algo::CalcSkillEffect(source, target, lcf::Data::skills[0], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(dmg, Algo::CalcSkillEffect(source, target, lcf::Data::skills[1], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[2], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[3], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[4], false, false, lcf::rpg::System::BattleCondition_none, false));
	}

	SUBCASE("attr2x") {
		REQUIRE_EQ(dmg * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[5], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(dmg * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[6], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[7], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[8], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[9], false, false, lcf::rpg::System::BattleCondition_none, false));
	}

	SUBCASE("ignore_defense") {
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[10], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[11], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[12], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[13], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[14], false, false, lcf::rpg::System::BattleCondition_none, false));
	}

	SUBCASE("ignore_defense+attr2x") {
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[15], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[16], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[17], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[18], false, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[19], false, false, lcf::rpg::System::BattleCondition_none, false));
	}
}

TEST_CASE("SkillEffect") {
	const MockBattle m;
	MakeDBAttribute(1, lcf::rpg::Attribute::Type_magical, 200, 200, 200, 200, 200);

	SUBCASE("100/0/120 -> 0/100/90") {
		auto source = MakeStatActor(0, 100, 0, 120);
		auto target = MakeStatActor(1, 0, 100, 90);

		SUBCASE("10/10/10") {
			testSkillStats(10, 10, 10, *source, *target, 54, 90);
		}

		SUBCASE("0/10/10") {
			testSkillStats(0, 10, 10, *source, *target, 44, 80);
		}

		SUBCASE("10/0/10") {
			testSkillStats(10, 0, 10, *source, *target, 29, 40);
		}

		SUBCASE("10/10/0") {
			testSkillStats(10, 10, 0, *source, *target, 35, 60);
		}

		SUBCASE("0/0/0") {
			testSkillStats(0, 0, 0, *source, *target, 0, 0);
		}
	}

	SUBCASE("10/0/10 -> 0/100/100") {
		auto source = MakeStatActor(0, 10, 0, 10);
		auto target = MakeStatActor(1, 0, 100, 100);

		SUBCASE("0/10/10") {
			testSkillStats(0, 10, 10, *source, *target, 0, 7);
		}
	}
}

static void testEnemyAttackEnemy(Game_Battler& source, Game_Battler& target, int dmg, int crit, int charged) {
	for (int wid = -1; wid <= 2; ++wid) {
		for (int cid = 0; cid <= 4; ++cid) {
			CAPTURE(wid);
			CAPTURE(cid);

			SUBCASE("no crit") {
				REQUIRE_EQ(dmg, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), false, false, false, lcf::rpg::System::BattleCondition(cid), true));
				REQUIRE_EQ(dmg, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), false, false, false, lcf::rpg::System::BattleCondition(cid), false));
			}

			SUBCASE("crit") {
				REQUIRE_EQ(crit, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), true, false, false, lcf::rpg::System::BattleCondition(cid), true));
				REQUIRE_EQ(crit, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), true, false, false, lcf::rpg::System::BattleCondition(cid), false));
			}

			SUBCASE("charged") {
				REQUIRE_EQ(charged, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), false, true, false, lcf::rpg::System::BattleCondition(cid), true));
				REQUIRE_EQ(charged, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), false, true, false, lcf::rpg::System::BattleCondition(cid), false));
			}

			SUBCASE("crit+charged") {
				REQUIRE_EQ(crit, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), true, true, false, lcf::rpg::System::BattleCondition(cid), true));
				REQUIRE_EQ(crit, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), true, true, false, lcf::rpg::System::BattleCondition(cid), false));
			}
		}
	}
}

static void testActorAttackRow(Game_Battler& source, Game_Battler& target, int none, int back, int surround, int pincers, int back_no_bug, int surround_no_bug) {
	REQUIRE_EQ(none, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, true));
	REQUIRE_EQ(none, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_initiative, true));
	REQUIRE_EQ(back, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_back, true));
	REQUIRE_EQ(surround, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_surround, true));
	REQUIRE_EQ(pincers, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_pincers, true));

	REQUIRE_EQ(none, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, false));
	REQUIRE_EQ(none, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_initiative, false));
	REQUIRE_EQ(back_no_bug, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_back, false));
	REQUIRE_EQ(surround_no_bug, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_surround, false));
	REQUIRE_EQ(pincers, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_pincers, false));
}

static void TestNormalAttack(int engine) {
	const MockBattle m(4, 4, engine);
	lcf::Data::enemies[1].attribute_ranks[0] = 2;
	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, 200, 200, 200, 200, 200);

	const bool is2k3 = Player::IsRPG2k3();

	SUBCASE("enemy 120/0 -> enemy 0/90") {
		auto source = MakeStatEnemy(0, 120, 0, 0);
		auto target = MakeStatEnemy(1, 0, 90, 0);

		REQUIRE_EQ(source->GetAtk(), 120);
		REQUIRE_EQ(target->GetDef(), 90);

		testEnemyAttackEnemy(*source, *target, 38, 114, 76);
	}

	SUBCASE("enemy 0/0 -> enemy 0/100") {
		auto source = MakeStatEnemy(0, 0, 0, 0);
		auto target = MakeStatEnemy(1, 0, 100, 0);

		REQUIRE_EQ(0, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, true));
	}

	SUBCASE("enemy 9999/0 -> enemy 0/0") {
		auto source = MakeStatEnemy(0, 9999, 0, 0);
		auto target = MakeStatEnemy(1, 0, 0, 0);

		REQUIRE_EQ(4999, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, true));
	}

	SUBCASE("actor 120/0 -> enemy 0/90") {
		auto source = MakeStatActor(0, 120, 0, 0);
		auto target = MakeStatEnemy(0, 0, 90, 0);

		REQUIRE_EQ(source->GetAtk(), 120);
		REQUIRE_EQ(target->GetDef(), 90);

		SUBCASE("front row") {
			source->SetBattleRow(lcf::rpg::SaveActor::RowType_front);
			if (is2k3) {
				testActorAttackRow(*source, *target, 47, 28, 35, 38, 38, 47);
			} else {
				testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
			}
		}

		SUBCASE("back row") {
			source->SetBattleRow(lcf::rpg::SaveActor::RowType_back);
			if (is2k3) {
				testActorAttackRow(*source, *target, 38, 35, 35, 38, 47, 47);
			} else {
				testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
			}
		}

		SUBCASE("weapons and attributes") {
			MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 20, 0, 0, 0, 0);
			MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 0);
			SetDBItemAttribute(2, 1, true);

			REQUIRE(lcf::Data::items[1].attribute_set[0]);

			source->SetEquipment(1, 1);
			source->SetEquipment(2, 2);

			REQUIRE_EQ(source->GetWeaponId(), 1);
			REQUIRE_EQ(source->GetShieldId(), 2);

			if (is2k3) {
				REQUIRE_EQ(140, source->GetAtk(Game_Battler::WeaponAll));
				REQUIRE_EQ(120, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(47, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponNone, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(60, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponPrimary, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(94, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponSecondary, false, false, false, lcf::rpg::System::BattleCondition_none, true));
			} else {
				REQUIRE_EQ(140, source->GetAtk(Game_Battler::WeaponAll));
				REQUIRE_EQ(96, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(38, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponNone, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(48, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponPrimary, false, false, false, lcf::rpg::System::BattleCondition_none, true));
				REQUIRE_EQ(76, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponSecondary, false, false, false, lcf::rpg::System::BattleCondition_none, true));
			}
		}
	}

	SUBCASE("actor 120/0 -> actor 0/90") {
		auto source = MakeStatActor(0, 120, 0, 0);
		auto target = MakeStatActor(1, 0, 90, 0);

		REQUIRE_EQ(source->GetAtk(), 120);
		REQUIRE_EQ(target->GetDef(), 90);

		SUBCASE("source front") {
			source->SetBattleRow(lcf::rpg::SaveActor::RowType_front);
			SUBCASE("target front") {
				target->SetBattleRow(lcf::rpg::SaveActor::RowType_front);
				if (is2k3) {
					testActorAttackRow(*source, *target, 47, 28, 35, 38, 28, 35);
				} else {
					testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
				}
			}
			SUBCASE("target back") {
				target->SetBattleRow(lcf::rpg::SaveActor::RowType_back);
				if (is2k3) {
					testActorAttackRow(*source, *target, 35, 38, 35, 38, 38, 35);
				} else {
					testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
				}
			}
		}

		SUBCASE("source back") {
			source->SetBattleRow(lcf::rpg::SaveActor::RowType_back);
			SUBCASE("target front") {
				target->SetBattleRow(lcf::rpg::SaveActor::RowType_front);
				if (is2k3) {
					testActorAttackRow(*source, *target, 38, 35, 35, 38, 35, 35);
				} else {
					testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
				}
			}
			SUBCASE("target back") {
				target->SetBattleRow(lcf::rpg::SaveActor::RowType_back);
				if (is2k3) {
					testActorAttackRow(*source, *target, 28, 47, 35, 38, 47, 35);
				} else {
					testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
				}
			}
		}
	}

	SUBCASE("enemy 120/0 -> actor 0/90") {
		auto source = MakeStatEnemy(0, 120, 0, 0);
		auto target = MakeStatActor(0, 0, 90, 0);

		REQUIRE_EQ(source->GetAtk(), 120);
		REQUIRE_EQ(target->GetDef(), 90);

		SUBCASE("target front") {
			target->SetBattleRow(lcf::rpg::SaveActor::RowType_front);
			if (is2k3) {
				testActorAttackRow(*source, *target, 38, 28, 28, 38, 28, 28);
			} else {
				testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
			}
		}
		SUBCASE("target back") {
			target->SetBattleRow(lcf::rpg::SaveActor::RowType_back);
			if (is2k3) {
				testActorAttackRow(*source, *target, 28, 38, 28, 38, 38, 28);
			} else {
				testActorAttackRow(*source, *target, 38, 38, 38, 38, 38, 38);
			}
		}
	}
}

TEST_CASE("NormalAttackEffect") {
	SUBCASE("2k") {
		TestNormalAttack(Player::EngineRpg2k | Player::EngineEnglish);
	}

	SUBCASE("2k3") {
		TestNormalAttack(Player::EngineRpg2k3 | Player::EngineEnglish);
	}
}

TEST_CASE("NormalAttackVariance") {
	const MockBattle m;

	SUBCASE("enemy 120/0 -> enemy 0/90") {
		auto source = MakeStatEnemy(0, 120, 0, 0);
		auto target = MakeStatEnemy(1, 0, 90, 0);

		REQUIRE_EQ(source->GetAtk(), 120);
		REQUIRE_EQ(target->GetDef(), 90);

		SUBCASE("max") {
			Rand::LockGuard lk(INT32_MAX);
			REQUIRE_EQ(46, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, true, lcf::rpg::System::BattleCondition_none, false));
		}
		SUBCASE("min") {
			Rand::LockGuard lk(INT32_MIN);
			REQUIRE_EQ(31, Algo::CalcNormalAttackEffect(*source, *target, Game_Battler::WeaponAll, false, false, true, lcf::rpg::System::BattleCondition_none, false));
		}
	}
}

static void testSkillVar(Game_Battler& source, Game_Battler& target, int var, int dmg_low, int dmg_high, int heal_low, int heal_high) {
	CAPTURE(var);
	auto* skill1 = MakeDBSkill(1, 100, 10, 10, 10, var);
	skill1->scope = lcf::rpg::Skill::Scope_enemy;

	auto* skill2 = MakeDBSkill(2, 100, 10, 10, 10, var);
	skill2->scope = lcf::rpg::Skill::Scope_ally;

	SUBCASE("max") {
		Rand::LockGuard lk(INT32_MAX);
		REQUIRE_EQ(dmg_high, Algo::CalcSkillEffect(source, target, *skill1, true, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal_high, Algo::CalcSkillEffect(source, target, *skill2, true, false, lcf::rpg::System::BattleCondition_none, false));
	}
	SUBCASE("min") {
		Rand::LockGuard lk(INT32_MIN);
		REQUIRE_EQ(dmg_low, Algo::CalcSkillEffect(source, target, *skill1, true, false, lcf::rpg::System::BattleCondition_none, false));
		REQUIRE_EQ(heal_low, Algo::CalcSkillEffect(source, target, *skill2, true, false, lcf::rpg::System::BattleCondition_none, false));
	}
}

TEST_CASE("SkillEffectVariance") {
	const MockBattle m;
	auto source = MakeStatActor(0, 100, 0, 120);
	auto target = MakeStatActor(1, 0, 100, 90);

	SUBCASE("0") {
		testSkillVar(*source, *target, 0, 54, 54, 90, 90);
	}
	SUBCASE("1") {
		testSkillVar(*source, *target, 1, 52, 57, 86, 95);
	}
	SUBCASE("2") {
		testSkillVar(*source, *target, 2, 49, 59, 81, 99);
	}
	SUBCASE("10") {
		testSkillVar(*source, *target, 10, 27, 81, 45, 135);
	}
}

TEST_CASE("SelfDestructVariance") {
	const MockBattle m;

	auto target = MakeStatActor(0, 0, 100, 0);
	auto source = MakeStatEnemy(0, 150, 0, 0);

	SUBCASE("max") {
		Rand::LockGuard lk(INT32_MAX);
		REQUIRE_EQ(120, Algo::CalcSelfDestructEffect(*source, *target, true));
	}
	SUBCASE("min") {
		Rand::LockGuard lk(INT32_MIN);
		REQUIRE_EQ(80, Algo::CalcSelfDestructEffect(*source, *target, true));
	}
}


TEST_CASE("SkillCost") {
	const MockBattle m;

	auto* skill = MakeDBSkill(1, 90, 0, 0, 0, 0);

	SUBCASE("9") {
		skill->sp_cost = 9;
		skill->sp_type = lcf::rpg::Skill::SpType_cost;
		REQUIRE_EQ(9, Algo::CalcSkillCost(*skill, 100, false));
		REQUIRE_EQ(5, Algo::CalcSkillCost(*skill, 100, true));
	}

	SUBCASE("10") {
		skill->sp_cost = 10;
		skill->sp_type = lcf::rpg::Skill::SpType_cost;
		REQUIRE_EQ(10, Algo::CalcSkillCost(*skill, 100, false));
		REQUIRE_EQ(5, Algo::CalcSkillCost(*skill, 100, true));
	}

	SUBCASE("49%") {
		skill->sp_percent = 49;
		skill->sp_type = lcf::rpg::Skill::SpType_percent;
		REQUIRE_EQ(49, Algo::CalcSkillCost(*skill, 100, false));
		REQUIRE_EQ(24, Algo::CalcSkillCost(*skill, 100, true));
	}

	SUBCASE("50%") {
		skill->sp_percent = 50;
		skill->sp_type = lcf::rpg::Skill::SpType_percent;
		REQUIRE_EQ(50, Algo::CalcSkillCost(*skill, 100, false));
		REQUIRE_EQ(25, Algo::CalcSkillCost(*skill, 100, true));
	}
}

TEST_CASE("SkillTargets") {
	const MockBattle m;

	auto* skill = MakeDBSkill(1, 90, 0, 0, 0, 0);

	skill->scope = lcf::rpg::Skill::Scope_enemy;
	REQUIRE(Algo::SkillTargetsEnemies(*skill));
	REQUIRE_FALSE(Algo::SkillTargetsAllies(*skill));

	skill->scope = lcf::rpg::Skill::Scope_enemies;
	REQUIRE(Algo::SkillTargetsEnemies(*skill));
	REQUIRE_FALSE(Algo::SkillTargetsAllies(*skill));

	skill->scope = lcf::rpg::Skill::Scope_self;
	REQUIRE_FALSE(Algo::SkillTargetsEnemies(*skill));
	REQUIRE(Algo::SkillTargetsAllies(*skill));

	skill->scope = lcf::rpg::Skill::Scope_ally;
	REQUIRE_FALSE(Algo::SkillTargetsEnemies(*skill));
	REQUIRE(Algo::SkillTargetsAllies(*skill));

	skill->scope = lcf::rpg::Skill::Scope_party;
	REQUIRE_FALSE(Algo::SkillTargetsEnemies(*skill));
	REQUIRE(Algo::SkillTargetsAllies(*skill));
}

TEST_CASE("SkillTypes") {
	const MockBattle m;

	auto* skill = MakeDBSkill(1, 90, 0, 0, 0, 0);

	skill->type = lcf::rpg::Skill::Type_normal;
	REQUIRE(Algo::IsNormalOrSubskill(*skill));

	skill->type = lcf::rpg::Skill::Type_teleport;
	REQUIRE_FALSE(Algo::IsNormalOrSubskill(*skill));

	skill->type = lcf::rpg::Skill::Type_escape;
	REQUIRE_FALSE(Algo::IsNormalOrSubskill(*skill));

	skill->type = lcf::rpg::Skill::Type_switch;
	REQUIRE_FALSE(Algo::IsNormalOrSubskill(*skill));

	skill->type = lcf::rpg::Skill::Type_subskill;
	REQUIRE(Algo::IsNormalOrSubskill(*skill));

	skill->type = lcf::rpg::Skill::Type_subskill + 1;
	REQUIRE(Algo::IsNormalOrSubskill(*skill));

}

TEST_SUITE_END();
