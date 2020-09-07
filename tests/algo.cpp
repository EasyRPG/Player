#include "test_mock_actor.h"
#include "algo.h"
#include "doctest.h"

static Game_Actor MakeActor(int id) {
	return Game_Actor(id);
}

static Game_Enemy& MakeEnemy(int id) {
	auto& tp = lcf::Data::troops[0];
	tp.members.resize(8);
	tp.members[id - 1].enemy_id = id;
	Main_Data::game_enemyparty->ResetBattle(1);
	auto& enemy = (*Main_Data::game_enemyparty)[id - 1];
	return enemy;
}

TEST_SUITE_BEGIN("Algo");

TEST_CASE("RowAdj") {
	const MockActor m;

	auto actor = MakeActor(1);

	SUBCASE("front") {
		actor.SetBattleRow(lcf::rpg::SaveActor::RowType_front);

		SUBCASE("offsense") {
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_none, true));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_initiative, true));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_back, true));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_surround, true));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_pincers, true));
		}

		SUBCASE("defense") {
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_none, false));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_initiative, false));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_back, false));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_surround, false));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_pincers, false));
		}
	}

	SUBCASE("back") {
		actor.SetBattleRow(lcf::rpg::SaveActor::RowType_back);

		SUBCASE("offsense") {
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_none, true));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_initiative, true));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_back, true));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_surround, true));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_pincers, true));
		}

		SUBCASE("defense") {
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_none, false));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_initiative, false));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_back, false));
			REQUIRE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_surround, false));
			REQUIRE_FALSE(Algo::IsRowAdjusted(actor, lcf::rpg::System::BattleCondition_pincers, false));
		}
	}
}

TEST_CASE("Variance") {
	SUBCASE("0 var disabled") {
		REQUIRE_EQ(Algo::VarianceAdjustEffect(100, 0), 100);
	}

	const int num_iterations = 200;
	SUBCASE(">0") {
		for (int var = 1; var <= 10; ++var) {
			for (int i = 0; i < num_iterations; ++i) {
				auto adj = Algo::VarianceAdjustEffect(100, 1);
				REQUIRE_GE(adj, 100 - 5 * var);
				REQUIRE_LE(adj, 100 + 5 * var);
			}
		}
	}

	SUBCASE("one") {
		for (int i = 0; i < num_iterations; ++i) {
			auto adj = Algo::VarianceAdjustEffect(1, 10);
			REQUIRE_GE(adj, 1);
			REQUIRE_LE(adj, 2);
		}
	}

	SUBCASE("zero") {

		SUBCASE("modern") {
			const MockActor m(Player::EngineEnglish);
			REQUIRE_FALSE(Player::IsLegacy());

			for (int i = 0; i < num_iterations; ++i) {
				REQUIRE_EQ(Algo::VarianceAdjustEffect(0, 0), 0);
				REQUIRE_EQ(Algo::VarianceAdjustEffect(0, 1), 0);
			}
		}
		SUBCASE("legacy") {
			const MockActor m(Player::EngineRpg2k);
			REQUIRE(Player::IsLegacy());

			for (int i = 0; i < num_iterations; ++i) {
				REQUIRE_EQ(Algo::VarianceAdjustEffect(0, 0), 0);
				auto adj = Algo::VarianceAdjustEffect(0, 1);
				REQUIRE_GE(adj, 0);
				REQUIRE_LE(adj, 1);
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

decltype(auto) MakeAgiEnemy(int id, int agi) {
	MakeDBEnemy(id, 1, 1, 1, 1, 1, agi);
	return MakeEnemy(id);
}

decltype(auto) MakeAgiActor(int id, int agi) {
	auto actor = MakeActor(id);
	actor.SetBaseAgi(agi);
	return actor;
}

static void testAgi(int src, int tgt, int res) {
	auto source = MakeAgiActor(1, src);
	auto target = MakeAgiEnemy(1, tgt);

	REQUIRE_EQ(res, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
	REQUIRE_EQ(res, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	REQUIRE_EQ(res, Algo::CalcSkillToHit(source, target, lcf::Data::skills[1]));
	for (int i = 2; i < 6; ++i) {
		CAPTURE(i);
		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[i]));

	}
}

TEST_CASE("HitRateAgi") {
	const MockActor m;
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
		REQUIRE_EQ(100, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	}

	SUBCASE("Avoid attacks") {
		target.AddState(3, true);
		REQUIRE_EQ(0, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		// RPG_RT bug
		REQUIRE_EQ(base, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	}

	SUBCASE("Avoid attacks and cannot act") {
		target.AddState(2, true);
		target.AddState(3, true);
		REQUIRE_EQ(0, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		// RPG_RT bug
		REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	}
}

TEST_CASE("HitRateStates") {
	const MockActor m;
	makeAgiSkills();
	lcf::Data::states[2 - 1].restriction = lcf::rpg::State::Restriction_do_nothing;
	lcf::Data::states[3 - 1].avoid_attacks = true;
	lcf::Data::states[4 - 1].reduce_hit_ratio = 50;

	auto source = MakeAgiActor(1, 100);
	auto target = MakeAgiEnemy(1, 100);

	testStates(source, target, 90);

	SUBCASE("source blind50") {
		source.AddState(4, true);
		REQUIRE_EQ(45, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		REQUIRE_EQ(45, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));

		testStates(source, target, 45);
	}
}

TEST_CASE("HitRateArmorAndRow2k3") {
	const MockActor m(Player::EngineRpg2k3);
	makeAgiSkills();
	MakeDBEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 0, false, false, false, false, false, true, false, false);

	auto source = MakeAgiActor(1, 100);

	SUBCASE("actor target") {
		auto target = MakeAgiActor(2, 100);

		SUBCASE("Front") {
			target.SetBattleRow(lcf::rpg::SaveActor::RowType_front);

			SUBCASE("no armor") {
				REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
				REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

				REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
			}

			SUBCASE("phys eva up") {
				target.SetEquipment(3, 1);
				REQUIRE(target.HasPhysicalEvasionUp());

				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
				REQUIRE_EQ(40, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
				REQUIRE_EQ(40, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

				REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
			}
		}

		SUBCASE("Back") {
			target.SetBattleRow(lcf::rpg::SaveActor::RowType_back);

			SUBCASE("no armor") {
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
				REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
				REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

				REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
			}

			SUBCASE("phys eva up") {
				target.SetEquipment(3, 1);
				REQUIRE(target.HasPhysicalEvasionUp());
				REQUIRE_EQ(40, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(40, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
				REQUIRE_EQ(40, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
				REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

				REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
			}
		}
	}

	SUBCASE("enemy target") {
		auto target = MakeAgiEnemy(2, 100);

		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
		// RPG_RT bug we implement
		REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	}
}

TEST_CASE("HitRateArmorAndRow2k") {
	const MockActor m(Player::EngineRpg2k);
	makeAgiSkills();
	MakeDBEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 0, false, false, false, false, false, true, false, false);

	auto source = MakeAgiActor(1, 100);

	SUBCASE("actor target") {
		auto target = MakeAgiActor(2, 100);

		SUBCASE("no armor") {
			REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
			REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
			REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
			REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
			REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

			REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
		}

		SUBCASE("phys eva up") {
			target.SetEquipment(3, 1);
			REQUIRE(target.HasPhysicalEvasionUp());

			REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
			REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
			REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
			REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
			REQUIRE_EQ(65, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

			REQUIRE_EQ(65, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
		}
	}

	SUBCASE("enemy target") {
		auto target = MakeAgiEnemy(2, 100);

		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_initiative));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_back));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_surround));
		REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_pincers));

		REQUIRE_EQ(90, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
	}
}

TEST_CASE("HitRateWeapons") {
	const MockActor m(Player::EngineRpg2k3);
	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 90, 0, false, false, false, true, false, false, false, false);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 50, 90, 0, false, false, false, false, false, false, false, false);

	auto source = MakeAgiActor(1, 100);
	auto target = MakeAgiEnemy(1, 200);

	source.SetEquipment(1, 1);
	source.SetEquipment(2, 2);

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponAll), 150);
	REQUIRE(source.AttackIgnoresEvasion(Game_Battler::WeaponAll));
	REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponAll, lcf::rpg::System::BattleCondition_none));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponNone), 100);
	REQUIRE_FALSE(source.AttackIgnoresEvasion(Game_Battler::WeaponNone));
	REQUIRE_EQ(85, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponNone, lcf::rpg::System::BattleCondition_none));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponPrimary), 100);
	REQUIRE(source.AttackIgnoresEvasion(Game_Battler::WeaponPrimary));
	REQUIRE_EQ(90, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponPrimary, lcf::rpg::System::BattleCondition_none));

	REQUIRE_EQ(source.GetAgi(Game_Battler::WeaponSecondary), 150);
	REQUIRE_FALSE(source.AttackIgnoresEvasion(Game_Battler::WeaponSecondary));
	REQUIRE_EQ(88, Algo::CalcNormalAttackToHit(source, target, Game_Battler::WeaponSecondary, lcf::rpg::System::BattleCondition_none));

	REQUIRE_EQ(100, Algo::CalcSkillToHit(source, target, lcf::Data::skills[0]));
}

TEST_CASE("CritRate") {
	const MockActor m(Player::EngineRpg2k3);
	lcf::Data::enemies[0].critical_hit = true;
	lcf::Data::enemies[0].critical_hit_chance = 30;
	lcf::Data::enemies[1].critical_hit = true;
	lcf::Data::enemies[1].critical_hit_chance = 30;
	MakeDBEquip(1, lcf::rpg::Item::Type_weapon)->critical_hit = 50;
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon)->critical_hit = 80;
	MakeDBEquip(3, lcf::rpg::Item::Type_armor)->prevent_critical = true;

	SUBCASE("actor -> actor - always fails") {
		auto source = MakeActor(1);
		auto target = MakeActor(2);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
	}

	SUBCASE("enemy -> enemy - always fails") {
		auto source = MakeEnemy(1);
		auto target = MakeEnemy(2);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
	}

	SUBCASE("actor -> enemy") {
		auto source = MakeActor(1);
		auto target = MakeEnemy(1);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		SUBCASE("baseline") {
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary));
		}

		SUBCASE("weapons") {
			source.SetEquipment(1, 1);
			source.SetEquipment(2, 2);

			REQUIRE_EQ(83, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone));
			REQUIRE_EQ(53, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary));
			REQUIRE_EQ(83, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary));
		}
	}

	SUBCASE("enemy -> actor") {
		auto source = MakeEnemy(1);
		auto target = MakeActor(1);

		REQUIRE_GT(source.GetCriticalHitChance(), 0.0f);

		SUBCASE("baseline") {
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary));
			REQUIRE_EQ(3, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary));
		}

		SUBCASE("armor prevents critical") {
			target.SetEquipment(3, 3);

			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponAll));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponNone));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponPrimary));
			REQUIRE_EQ(0, Algo::CalcCriticalHitChance(source, target, Game_Battler::WeaponSecondary));
		}
	}
}

decltype(auto) MakeStatEnemy(int id, int atk, int def, int spi) {
	MakeDBEnemy(id, 1, 1, atk, def, spi, 1);
	return MakeEnemy(id);
}

decltype(auto) MakeStatActor(int id, int atk, int def, int spi) {
	auto actor = MakeActor(id);
	actor.SetBaseAtk(atk);
	actor.SetBaseDef(def);
	actor.SetBaseSpi(spi);
	return actor;
}

TEST_CASE("DefendAdjustment") {
	const MockActor m;

	SUBCASE("no strong") {
		auto target = MakeActor(1);

		SUBCASE("baseline") {
			REQUIRE_EQ(100, Algo::AdjustDamageForDefend(100, target));
		}

		SUBCASE("defend") {
			target.SetIsDefending(true);
			REQUIRE_EQ(50, Algo::AdjustDamageForDefend(100, target));
		}
	}

	SUBCASE("strong") {
		lcf::Data::actors[0].super_guard = true;
		auto target = MakeActor(1);

		SUBCASE("baseline") {
			REQUIRE_EQ(100, Algo::AdjustDamageForDefend(100, target));
		}

		SUBCASE("defend") {
			target.SetIsDefending(true);
			REQUIRE_EQ(25, Algo::AdjustDamageForDefend(100, target));
		}
	}
}

TEST_CASE("SelfDestructEffect") {
	const MockActor m;

	auto target = MakeStatActor(1, 0, 100, 0);

	SUBCASE("100_100") {
		auto source = MakeStatEnemy(1, 100, 0, 0);
		REQUIRE_EQ(50, Algo::CalcSelfDestructEffect(source, target, false));
	}

	SUBCASE("200_100") {
		auto source = MakeStatEnemy(1, 200, 0, 0);
		REQUIRE_EQ(150, Algo::CalcSelfDestructEffect(source, target, false));
	}

	SUBCASE("1_100") {
		auto source = MakeStatEnemy(1, 1, 0, 0);
		REQUIRE_EQ(0, Algo::CalcSelfDestructEffect(source, target, false));
	}

	SUBCASE("150_0") {
		auto source = MakeStatEnemy(1, 150, 0, 0);
		REQUIRE_EQ(100, Algo::CalcSelfDestructEffect(source, target, false));
	}

	SUBCASE("150_0_var") {
		auto source = MakeStatEnemy(1, 150, 0, 0);
		for (int i = 0; i < 200; ++i) {
			auto effect = Algo::CalcSelfDestructEffect(source, target, true);
			REQUIRE_GE(effect, 80);
			REQUIRE_LE(effect, 120);
		}
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

	SUBCASE("baseline") {
		REQUIRE_EQ(dmg, Algo::CalcSkillEffect(source, target, lcf::Data::skills[0], false));
		REQUIRE_EQ(dmg, Algo::CalcSkillEffect(source, target, lcf::Data::skills[1], false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[2], false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[3], false));
		REQUIRE_EQ(heal, Algo::CalcSkillEffect(source, target, lcf::Data::skills[4], false));
	}

	SUBCASE("attr2x") {
		REQUIRE_EQ(dmg * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[5], false));
		REQUIRE_EQ(dmg * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[6], false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[7], false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[8], false));
		REQUIRE_EQ(heal * 2, Algo::CalcSkillEffect(source, target, lcf::Data::skills[9], false));
	}
}

TEST_CASE("SkillEffect") {
	const MockActor m;
	MakeDBAttribute(1, lcf::rpg::Attribute::Type_magical, 200, 200, 200, 200, 200);

	SUBCASE("100/0/120 -> 0/100/90") {
		auto source = MakeStatActor(1, 100, 0, 120);
		auto target = MakeStatActor(1, 0, 100, 90);

		SUBCASE("10/10/10") {
			testSkillStats(10, 10, 10, source, target, 54, 90);
		}

		SUBCASE("0/10/10") {
			testSkillStats(0, 10, 10, source, target, 44, 80);
		}

		SUBCASE("10/0/10") {
			testSkillStats(10, 0, 10, source, target, 29, 40);
		}

		SUBCASE("10/10/0") {
			testSkillStats(10, 10, 0, source, target, 35, 60);
		}

		SUBCASE("0/0/0") {
			testSkillStats(0, 0, 0, source, target, 0, 0);
		}
	}

	SUBCASE("10/0/10 -> 0/100/100") {
		auto source = MakeStatActor(1, 10, 0, 10);
		auto target = MakeStatActor(1, 0, 100, 100);

		SUBCASE("0/10/10") {
			testSkillStats(0, 10, 10, source, target, 0, 7);
		}
	}
}

static void testEnemyAttackEnemy(Game_Battler& source, Game_Battler& target, int dmg, int crit) {
	for (int wid = -1; wid <= 2; ++wid) {
		for (int cid = 0; cid <= 4; ++cid) {
			CAPTURE(wid);
			CAPTURE(cid);

			SUBCASE("no crit") {
				REQUIRE_EQ(dmg, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), false, false, lcf::rpg::System::BattleCondition(cid)));
			}

			SUBCASE("crit") {
				REQUIRE_EQ(crit, Algo::CalcNormalAttackEffect(source, target, Game_Battler::Weapon(wid), true, false, lcf::rpg::System::BattleCondition(cid)));
			}
		}
	}
}

static void testActorAttackRow(Game_Battler& source, Game_Battler& target, int none, int init, int back, int surround, int pincers) {
	REQUIRE_EQ(none, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_none));
	REQUIRE_EQ(init, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_initiative));
	REQUIRE_EQ(back, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_back));
	REQUIRE_EQ(surround, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_surround));
	REQUIRE_EQ(pincers, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_pincers));
}

static void TestNormalAttack(int engine) {
	const MockActor m(engine);
	lcf::Data::enemies[1].attribute_ranks[0] = 2;
	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, 200, 200, 200, 200, 200);

	const bool is2k3 = Player::IsRPG2k3();

	SUBCASE("enemy 120/0 -> enemy 0/90") {
		auto source = MakeStatEnemy(1, 120, 0, 0);
		auto target = MakeStatEnemy(2, 0, 90, 0);

		REQUIRE_EQ(source.GetAtk(), 120);
		REQUIRE_EQ(target.GetDef(), 90);

		SUBCASE("baseline") {
			testEnemyAttackEnemy(source, target, 38, 114);
		}

		SUBCASE("charged") {
			source.SetCharged(true);
			testEnemyAttackEnemy(source, target, 76, 114);
		}
	}

	SUBCASE("enemy 0/0 -> enemy 0/100") {
		auto source = MakeStatEnemy(1, 0, 0, 0);
		auto target = MakeStatEnemy(2, 0, 100, 0);

		REQUIRE_EQ(0, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_none));
	}

	SUBCASE("enemy 9999/0 -> enemy 0/0") {
		auto source = MakeStatEnemy(1, 9999, 0, 0);
		auto target = MakeStatEnemy(2, 0, 0, 0);

		REQUIRE_EQ(4999, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_none));
	}

	SUBCASE("actor 120/0 -> enemy 0/90") {
		auto source = MakeStatActor(1, 120, 0, 0);
		auto target = MakeStatEnemy(1, 0, 90, 0);

		REQUIRE_EQ(source.GetAtk(), 120);
		REQUIRE_EQ(target.GetDef(), 90);

		SUBCASE("front row") {
			source.SetBattleRow(lcf::rpg::SaveActor::RowType_front);
			if (is2k3) {
				testActorAttackRow(source, target, 47, 47, /* RPG_RT bug */ 28, 47, 38);
			} else {
				testActorAttackRow(source, target, 38, 38, 38, 38, 38);
			}
		}

		SUBCASE("back row") {
			source.SetBattleRow(lcf::rpg::SaveActor::RowType_back);
			if (is2k3) {
				testActorAttackRow(source, target, 38, 38, /* RPG_RT bug */ 35, 47, 38);
			} else {
				testActorAttackRow(source, target, 38, 38, 38, 38, 38);
			}
		}

		SUBCASE("weapons and attributes") {
			MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 20, 0, 0, 0, 0);
			MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 0);
			SetDBItemAttribute(2, 1, true);

			REQUIRE(lcf::Data::items[1].attribute_set[0]);

			source.SetEquipment(1, 1);
			source.SetEquipment(2, 2);

			REQUIRE_EQ(source.GetWeaponId(), 1);
			REQUIRE_EQ(source.GetShieldId(), 2);

			if (is2k3) {
				REQUIRE_EQ(140, source.GetAtk(Game_Battler::WeaponAll));
				REQUIRE_EQ(120, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(47, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponNone, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(60, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponPrimary, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(94, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponSecondary, false, false, lcf::rpg::System::BattleCondition_none));
			} else {
				REQUIRE_EQ(140, source.GetAtk(Game_Battler::WeaponAll));
				REQUIRE_EQ(96, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponAll, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(38, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponNone, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(48, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponPrimary, false, false, lcf::rpg::System::BattleCondition_none));
				REQUIRE_EQ(76, Algo::CalcNormalAttackEffect(source, target, Game_Battler::WeaponSecondary, false, false, lcf::rpg::System::BattleCondition_none));
			}
		}
	}

	SUBCASE("actor 120/0 -> actor 0/90") {
		auto source = MakeStatActor(1, 120, 0, 0);
		auto target = MakeStatActor(2, 0, 90, 0);

		REQUIRE_EQ(source.GetAtk(), 120);
		REQUIRE_EQ(target.GetDef(), 90);

		SUBCASE("source front") {
			source.SetBattleRow(lcf::rpg::SaveActor::RowType_front);
			SUBCASE("target front") {
				target.SetBattleRow(lcf::rpg::SaveActor::RowType_front);
				if (is2k3) {
					testActorAttackRow(source, target, 47, 47, 28, 35, 38);
				} else {
					testActorAttackRow(source, target, 38, 38, 38, 38, 38);
				}
			}
			SUBCASE("target back") {
				target.SetBattleRow(lcf::rpg::SaveActor::RowType_back);
				if (is2k3) {
					testActorAttackRow(source, target, 35, 35, 38, 35, 38);
				} else {
					testActorAttackRow(source, target, 38, 38, 38, 38, 38);
				}
			}
		}

		SUBCASE("source back") {
			source.SetBattleRow(lcf::rpg::SaveActor::RowType_back);
			SUBCASE("target front") {
				target.SetBattleRow(lcf::rpg::SaveActor::RowType_front);
				if (is2k3) {
					testActorAttackRow(source, target, 38, 38, 35, 35, 38);
				} else {
					testActorAttackRow(source, target, 38, 38, 38, 38, 38);
				}
			}
			SUBCASE("target back") {
				target.SetBattleRow(lcf::rpg::SaveActor::RowType_back);
				if (is2k3) {
					testActorAttackRow(source, target, 28, 28, 47, 35, 38);
				} else {
					testActorAttackRow(source, target, 38, 38, 38, 38, 38);
				}
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


TEST_SUITE_END();
