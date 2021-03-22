#include "test_mock_actor.h"
#include "rand.h"
#include "algo.h"
#include "game_battlealgorithm.h"
#include "main_data.h"
#include "doctest.h"

#include <iostream>

TEST_SUITE_BEGIN("BattleAlgorithm");

doctest::String toString(Game_BattleAlgorithm::StateEffect::Effect e) {
	return doctest::toString(int(e));
}

namespace {
using AlgoBase = Game_BattleAlgorithm::AlgorithmBase;
using AlgoType = Game_BattleAlgorithm::Type;

class TestAlgo : public AlgoBase {
public:
	TestAlgo(Game_Battler* source, Game_Battler* target)
		: AlgoBase(AlgoType::None, source, target) {}

	TestAlgo(Game_Battler* source, std::vector<Game_Battler*> targets)
		: AlgoBase(AlgoType::None, source, std::move(targets)) {}

	TestAlgo(Game_Battler* source, Game_Party_Base* target)
		: AlgoBase(AlgoType::None, source, target) {}

	TestAlgo(Game_Battler* source)
		: AlgoBase(AlgoType::None, source, source) {}

	bool IsReflected(const Game_Battler& target) const override {
		return &target == mock_reflect_target;
	}

public:
	const Game_Battler* mock_reflect_target = nullptr;
};

} // namespace


TEST_CASE("Default") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	TestAlgo algo(source);

	REQUIRE_EQ(source, algo.GetSource());
	REQUIRE_EQ(0, algo.GetCurrentRepeat());

	REQUIRE_EQ(0, algo.GetAffectedSwitch());
	REQUIRE_EQ(false, algo.IsAffectHp());
	REQUIRE_EQ(false, algo.IsAffectSp());
	REQUIRE_EQ(false, algo.IsAffectAtk());
	REQUIRE_EQ(false, algo.IsAffectDef());
	REQUIRE_EQ(false, algo.IsAffectSpi());
	REQUIRE_EQ(false, algo.IsAffectAgi());
	REQUIRE_EQ(0, algo.GetAffectedHp());
	REQUIRE_EQ(0, algo.GetAffectedSp());
	REQUIRE_EQ(0, algo.GetAffectedAtk());
	REQUIRE_EQ(0, algo.GetAffectedDef());
	REQUIRE_EQ(0, algo.GetAffectedSpi());
	REQUIRE_EQ(0, algo.GetAffectedAgi());
	REQUIRE_EQ(false, algo.IsAbsorbHp());
	REQUIRE_EQ(false, algo.IsAbsorbSp());
	REQUIRE_EQ(false, algo.IsAbsorbAtk());
	REQUIRE_EQ(false, algo.IsAbsorbDef());
	REQUIRE_EQ(false, algo.IsAbsorbSpi());
	REQUIRE_EQ(false, algo.IsAbsorbAgi());
	REQUIRE_EQ(0, algo.GetStateEffects().size());
	REQUIRE_EQ(0, algo.GetShiftedAttributes().size());

	REQUIRE_EQ(false, algo.IsSuccess());
	REQUIRE_EQ(false, algo.IsPositive());
	REQUIRE_EQ(false, algo.IsRevived());
	REQUIRE_EQ(false, algo.IsCriticalHit());
}

TEST_CASE("TargetSingle") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	TestAlgo algo(source, target);

	auto common = [&]() {
		REQUIRE_EQ(source, algo.GetSource());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());
		REQUIRE_EQ(nullptr, algo.GetOriginalPartyTarget());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
	};

	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());

	algo.Start();

	common();
	REQUIRE_EQ(true, algo.IsCurrentTargetValid());
	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(false, algo.TargetNext());

	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());
}

TEST_CASE("TargetMultiple") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	std::vector<Game_Battler*> targets = { Main_Data::game_enemyparty->GetEnemy(0), Main_Data::game_enemyparty->GetEnemy(1) };
	TestAlgo algo(source, targets);

	auto common = [&]() {
		REQUIRE_EQ(source, algo.GetSource());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());
		REQUIRE_EQ(nullptr, algo.GetOriginalPartyTarget());
		REQUIRE_EQ(2, algo.GetOriginalTargets().size());
		REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
	};

	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());

	algo.Start();
	common();
	REQUIRE_EQ(true, algo.IsCurrentTargetValid());
	REQUIRE_EQ(targets[0], algo.GetTarget());

	REQUIRE_EQ(true, algo.TargetNext());
	common();
	REQUIRE_EQ(true, algo.IsCurrentTargetValid());
	REQUIRE_EQ(targets[1], algo.GetTarget());

	REQUIRE_EQ(false, algo.TargetNext());
	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());
}

TEST_CASE("TargetParty") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);
	auto* targets = Main_Data::game_enemyparty.get();
	TestAlgo algo(source, targets);

	auto common = [&]() {
		REQUIRE_EQ(source, algo.GetSource());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());
		REQUIRE_EQ(targets, algo.GetOriginalPartyTarget());
		REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
	};

	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());
	REQUIRE_EQ(0, algo.GetOriginalTargets().size());

	algo.Start();
	for (int i = 0; i < 4; ++i) {
		common();
		REQUIRE_EQ(true, algo.IsCurrentTargetValid());
		REQUIRE_EQ(targets->GetEnemy(i), algo.GetTarget());
		REQUIRE_EQ(4, algo.GetOriginalTargets().size());
		REQUIRE_EQ(i < 3, algo.TargetNext());
	}

	common();
	REQUIRE_EQ(false, algo.IsCurrentTargetValid());
	REQUIRE_EQ(nullptr, algo.GetTarget());
	REQUIRE_EQ(4, algo.GetOriginalTargets().size());
}

TEST_CASE("TargetPartySize1") {
	const MockBattle mb(4, 1);
	auto* source = Main_Data::game_party->GetActor(0);
	auto* targets = Main_Data::game_enemyparty.get();
	TestAlgo algo(source, targets);

	REQUIRE_EQ(targets, algo.GetOriginalPartyTarget());
	REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
	REQUIRE_EQ(0, algo.GetOriginalTargets().size());

	algo.Start();
	REQUIRE_EQ(targets, algo.GetOriginalPartyTarget());
	REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
	REQUIRE_EQ(1, algo.GetOriginalTargets().size());
}

TEST_CASE("AddTarget") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(3);
	TestAlgo algo(source, target);

	algo.Start();

	REQUIRE_EQ(target->GetId(), algo.GetTarget()->GetId());
	REQUIRE_EQ(1, algo.GetOriginalTargets().size());
	REQUIRE_EQ(target, algo.GetOriginalSingleTarget());

	SUBCASE("AddOne") {
		auto* new_target = Main_Data::game_enemyparty->GetEnemy(1);

		CAPTURE(target->GetId());
		CAPTURE(new_target->GetId());

		SUBCASE("NoCurrent") {
			algo.AddTarget(new_target, false);

			REQUIRE_EQ(target->GetId(), algo.GetTarget()->GetId());
			REQUIRE_EQ(1, algo.GetOriginalTargets().size());
			REQUIRE_EQ(target, algo.GetOriginalSingleTarget());

			REQUIRE_EQ(true, algo.TargetNext());
		}

		SUBCASE("Current") {
			algo.AddTarget(new_target, true);
		}

		REQUIRE_EQ(new_target->GetId(), algo.GetTarget()->GetId());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
	}

	SUBCASE("AddParty") {
		auto* new_party = Main_Data::game_enemyparty.get();

		SUBCASE("NoCurrent") {
			algo.AddTargets(new_party, false);

			REQUIRE_EQ(target->GetId(), algo.GetTarget()->GetId());
			REQUIRE_EQ(1, algo.GetOriginalTargets().size());
			REQUIRE_EQ(target, algo.GetOriginalSingleTarget());

			REQUIRE_EQ(true, algo.TargetNext());
		}

		SUBCASE("Current") {
			algo.AddTargets(new_party, true);
		}

		for (int i = 0; i < 4; ++i) {
			REQUIRE_EQ(new_party->GetEnemy(i)->GetId(), algo.GetTarget()->GetId());
			REQUIRE_EQ(1, algo.GetOriginalTargets().size());
			REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
			REQUIRE_EQ(i < 3, algo.TargetNext());
		}
	}
}

TEST_CASE("ReflectTarget") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);

	SUBCASE("noreflect") {
		auto* target = Main_Data::game_enemyparty->GetEnemy(3);
		TestAlgo algo(source, target);

		algo.Start();

		REQUIRE_EQ(target, algo.GetTarget());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());

		algo.mock_reflect_target = nullptr;
		REQUIRE_EQ(false, algo.ReflectTargets());

		REQUIRE_EQ(target, algo.GetTarget());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());
	}

	SUBCASE("single") {
		auto* target = Main_Data::game_enemyparty->GetEnemy(3);
		TestAlgo algo(source, target);

		algo.Start();

		REQUIRE_EQ(target, algo.GetTarget());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());

		algo.mock_reflect_target = target;

		REQUIRE_EQ(true, algo.ReflectTargets());

		REQUIRE_EQ(source, algo.GetTarget());
		REQUIRE_EQ(1, algo.GetOriginalTargets().size());
		REQUIRE_EQ(target, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(target, algo.GetReflectTarget());

		REQUIRE_EQ(false, algo.TargetNext());
	}

	SUBCASE("multiple") {
		std::vector<Game_Battler*> targets = { Main_Data::game_enemyparty->GetEnemy(1), Main_Data::game_enemyparty->GetEnemy(2) };
		TestAlgo algo(source, targets);

		algo.Start();

		REQUIRE_EQ(targets.front(), algo.GetTarget());
		REQUIRE_EQ(2, algo.GetOriginalTargets().size());
		REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());

		SUBCASE("first") {
			algo.mock_reflect_target = targets[0];

			REQUIRE_EQ(true, algo.ReflectTargets());
			REQUIRE_EQ(targets[0], algo.GetReflectTarget());
		}

		SUBCASE("second") {
			algo.mock_reflect_target = targets[1];

			REQUIRE_EQ(true, algo.ReflectTargets());
			REQUIRE_EQ(targets[1], algo.GetReflectTarget());
		}

		REQUIRE_EQ(source, algo.GetTarget());
		REQUIRE_EQ(2, algo.GetOriginalTargets().size());
		REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());

		REQUIRE_EQ(false, algo.TargetNext());
	}

	SUBCASE("party") {
		auto* targets = Main_Data::game_enemyparty.get();
		TestAlgo algo(source, targets);

		algo.Start();

		REQUIRE_EQ(targets->GetEnemy(0), algo.GetTarget());
		REQUIRE_EQ(4, algo.GetOriginalTargets().size());
		REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());
		REQUIRE_EQ(nullptr, algo.GetReflectTarget());

		SUBCASE("ref0") {
			algo.mock_reflect_target = targets->GetEnemy(0);
		}
		SUBCASE("ref1") {
			algo.mock_reflect_target = targets->GetEnemy(1);
		}
		SUBCASE("ref2") {
			algo.mock_reflect_target = targets->GetEnemy(2);
		}
		SUBCASE("ref3") {
			algo.mock_reflect_target = targets->GetEnemy(3);
		}

		REQUIRE_EQ(true, algo.ReflectTargets());
		REQUIRE_EQ(algo.mock_reflect_target, algo.GetReflectTarget());

		auto* sources = Main_Data::game_party.get();

		for (int i = 0; i < 4; ++i) {
			REQUIRE_EQ(sources->GetActor(i), algo.GetTarget());
			REQUIRE_EQ(4, algo.GetOriginalTargets().size());
			REQUIRE_EQ(nullptr, algo.GetOriginalSingleTarget());

			REQUIRE_EQ(i < 3, algo.TargetNext());
		}
	}
}

TEST_CASE("Retarget") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);

	auto test = [&](auto* target, auto* next) {
		TestAlgo algo(source, target);

		target->Kill();
		algo.Start();

		REQUIRE_EQ(next, algo.GetTarget());
	};

	SUBCASE("enemy") {
		test(Main_Data::game_enemyparty->GetEnemy(0), Main_Data::game_enemyparty->GetEnemy(1));
	}

	SUBCASE("actor") {
		test(Main_Data::game_party->GetActor(0), Main_Data::game_party->GetActor(1));
	}
}

TEST_CASE("Repeat") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);
	std::vector<Game_Battler*> targets = { Main_Data::game_enemyparty->GetEnemy(1), Main_Data::game_enemyparty->GetEnemy(2) };
	TestAlgo algo(source, targets);

	algo.SetRepeat(3);
	REQUIRE_EQ(algo.GetCurrentRepeat(), 0);

	SUBCASE("RequireValidTarget") {
		REQUIRE_EQ(false, algo.IsCurrentTargetValid());
		REQUIRE_EQ(false, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);

		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);

		REQUIRE_EQ(true, algo.IsCurrentTargetValid());
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);
	}

	SUBCASE("NoRequireValidTarget") {
		REQUIRE_EQ(false, algo.IsCurrentTargetValid());
		REQUIRE_EQ(true, algo.RepeatNext(false));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);

		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);

		REQUIRE_EQ(true, algo.IsCurrentTargetValid());
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 2);
	}

	SUBCASE("2kStyle") {
		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 2);
		REQUIRE_EQ(false, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);

		REQUIRE_EQ(true, algo.TargetNext());
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);
		REQUIRE_EQ(true, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 2);
		REQUIRE_EQ(false, algo.RepeatNext(true));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
	}

	SUBCASE("2k3Style") {
		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
		REQUIRE_EQ(true, algo.TargetNext());
		REQUIRE_EQ(false, algo.TargetNext());
		REQUIRE_EQ(true, algo.RepeatNext(false));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);

		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 1);
		REQUIRE_EQ(true, algo.TargetNext());
		REQUIRE_EQ(false, algo.TargetNext());
		REQUIRE_EQ(true, algo.RepeatNext(false));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 2);

		algo.Start();
		REQUIRE_EQ(algo.GetCurrentRepeat(), 2);
		REQUIRE_EQ(true, algo.TargetNext());
		REQUIRE_EQ(false, algo.TargetNext());
		REQUIRE_EQ(false, algo.RepeatNext(false));
		REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
	}
}

TEST_CASE("Combo") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_party->GetActor(0);
	std::vector<Game_Battler*> targets = { Main_Data::game_enemyparty->GetEnemy(1), Main_Data::game_enemyparty->GetEnemy(2) };
	TestAlgo algo(source, targets);

	algo.SetRepeat(3);
	REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
	algo.ApplyComboHitsMultiplier(2);
	REQUIRE_EQ(algo.GetCurrentRepeat(), 0);

	algo.Start();

	for (int i = 0; i < 5; ++i) {
		REQUIRE_EQ(algo.GetCurrentRepeat(), i);
		REQUIRE_EQ(true, algo.RepeatNext(false));
	}
	REQUIRE_EQ(algo.GetCurrentRepeat(), 5);
	REQUIRE_EQ(false, algo.RepeatNext(false));
	REQUIRE_EQ(algo.GetCurrentRepeat(), 0);
}

TEST_CASE("Flags") {
	const MockBattle mb;
	TestAlgo algo(Main_Data::game_party->GetActor(0));

	SUBCASE("success") {
		REQUIRE_FALSE(algo.IsSuccess());

		REQUIRE(algo.SetIsSuccess());
		REQUIRE(algo.IsSuccess());

		REQUIRE_FALSE(algo.SetIsFailure());
		REQUIRE_FALSE(algo.IsSuccess());

		REQUIRE_FALSE(algo.SetIsSuccessIf(false));
		REQUIRE_FALSE(algo.IsSuccess());

		REQUIRE(algo.SetIsSuccessIf(true));
		REQUIRE(algo.IsSuccess());

		REQUIRE(algo.SetIsSuccessIf(true));
		REQUIRE(algo.IsSuccess());

		REQUIRE(algo.SetIsSuccessIf(false));
		REQUIRE(algo.IsSuccess());
	}

	SUBCASE("critical_hit") {
		REQUIRE_FALSE(algo.IsCriticalHit());

		REQUIRE(algo.SetIsCriticalHit(true));
		REQUIRE(algo.IsCriticalHit());

		REQUIRE_FALSE(algo.SetIsCriticalHit(false));
		REQUIRE_FALSE(algo.IsCriticalHit());
	}

	SUBCASE("positive") {
		REQUIRE_FALSE(algo.IsPositive());

		REQUIRE(algo.SetIsPositive(true));
		REQUIRE(algo.IsPositive());

		REQUIRE_FALSE(algo.SetIsPositive(false));
		REQUIRE_FALSE(algo.IsPositive());
	}
}

TEST_CASE("SwitchEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);

	TestAlgo algo(source);

	REQUIRE_EQ(0, algo.GetAffectedSwitch());
	REQUIRE_EQ(7, algo.SetAffectedSwitch(7));
	REQUIRE_EQ(7, algo.GetAffectedSwitch());

	REQUIRE_EQ(false, Main_Data::game_switches->Get(7));
	REQUIRE_EQ(7, algo.ApplySwitchEffect());
	REQUIRE_EQ(true, Main_Data::game_switches->Get(7));
}

TEST_CASE("HpEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 200, 0, 1, 1, 1, 1);
	Setup(target, 200, 0, 1, 1, 1, 1);
	source->SetHp(100);
	target->SetHp(100);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedHp());
	REQUIRE_EQ(false, algo.IsAffectHp());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedHp(0));
		REQUIRE_EQ(0, algo.GetAffectedHp());
		REQUIRE_EQ(true, algo.IsAffectHp());

		SUBCASE("alive") {
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(100, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}

		SUBCASE("dead") {
			target->Kill();
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbHp(true);
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(100, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedHp(-50));
		REQUIRE_EQ(-50, algo.GetAffectedHp());
		REQUIRE_EQ(true, algo.IsAffectHp());

		SUBCASE("alive") {
			REQUIRE_EQ(-50, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(50, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}

		SUBCASE("dead") {
			target->Kill();
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbHp(true);
			REQUIRE_EQ(-50, algo.ApplyHpEffect());

			REQUIRE_EQ(150, source->GetHp());
			REQUIRE_EQ(50, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedHp(50));
		REQUIRE_EQ(50, algo.GetAffectedHp());
		REQUIRE_EQ(true, algo.IsAffectHp());

		SUBCASE("alive") {
			REQUIRE_EQ(50, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(150, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}

		SUBCASE("dead") {
			target->Kill();
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbHp(true);
			REQUIRE_EQ(50, algo.ApplyHpEffect());

			REQUIRE_EQ(50, source->GetHp());
			REQUIRE_EQ(150, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedHp(-999));
		REQUIRE_EQ(-999, algo.GetAffectedHp());
		REQUIRE_EQ(true, algo.IsAffectHp());

		SUBCASE("alive") {
			REQUIRE_EQ(-100, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("dead") {
			target->Kill();
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbHp(true);
			REQUIRE_EQ(-100, algo.ApplyHpEffect());

			REQUIRE_EQ(200, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedHp(999));
		REQUIRE_EQ(999, algo.GetAffectedHp());
		REQUIRE_EQ(true, algo.IsAffectHp());

		SUBCASE("alive") {
			REQUIRE_EQ(100, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(200, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
		}

		SUBCASE("dead") {
			target->Kill();
			REQUIRE_EQ(0, algo.ApplyHpEffect());

			REQUIRE_EQ(100, source->GetHp());
			REQUIRE_EQ(0, target->GetHp());
			REQUIRE_EQ(true, target->IsDead());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbHp(true);
			REQUIRE_EQ(100, algo.ApplyHpEffect());

			REQUIRE_EQ(0, source->GetHp());
			REQUIRE_EQ(200, target->GetHp());
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(true, source->IsDead());
		}
	}
}

TEST_CASE("SpEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 1, 200, 1, 1, 1, 1);
	Setup(target, 1, 200, 1, 1, 1, 1);
	source->SetSp(100);
	target->SetSp(100);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedSp());
	REQUIRE_EQ(false, algo.IsAffectSp());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedSp(0));
		REQUIRE_EQ(0, algo.GetAffectedSp());
		REQUIRE_EQ(true, algo.IsAffectSp());

		SUBCASE("normal") {
			REQUIRE_EQ(0, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(100, target->GetSp());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSp(true);
			REQUIRE_EQ(0, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(100, target->GetSp());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedSp(-50));
		REQUIRE_EQ(-50, algo.GetAffectedSp());
		REQUIRE_EQ(true, algo.IsAffectSp());

		SUBCASE("normal") {
			REQUIRE_EQ(-50, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(50, target->GetSp());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSp(true);
			REQUIRE_EQ(-50, algo.ApplySpEffect());

			REQUIRE_EQ(150, source->GetSp());
			REQUIRE_EQ(50, target->GetSp());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedSp(50));
		REQUIRE_EQ(50, algo.GetAffectedSp());
		REQUIRE_EQ(true, algo.IsAffectSp());

		SUBCASE("normal") {
			REQUIRE_EQ(50, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(150, target->GetSp());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSp(true);
			REQUIRE_EQ(50, algo.ApplySpEffect());

			REQUIRE_EQ(50, source->GetSp());
			REQUIRE_EQ(150, target->GetSp());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedSp(-999));
		REQUIRE_EQ(-999, algo.GetAffectedSp());
		REQUIRE_EQ(true, algo.IsAffectSp());

		SUBCASE("normal") {
			REQUIRE_EQ(-100, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(0, target->GetSp());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSp(true);
			REQUIRE_EQ(-100, algo.ApplySpEffect());

			REQUIRE_EQ(200, source->GetSp());
			REQUIRE_EQ(0, target->GetSp());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedSp(999));
		REQUIRE_EQ(999, algo.GetAffectedSp());
		REQUIRE_EQ(true, algo.IsAffectSp());

		SUBCASE("normal") {
			REQUIRE_EQ(100, algo.ApplySpEffect());

			REQUIRE_EQ(100, source->GetSp());
			REQUIRE_EQ(200, target->GetSp());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSp(true);
			REQUIRE_EQ(100, algo.ApplySpEffect());

			REQUIRE_EQ(0, source->GetSp());
			REQUIRE_EQ(200, target->GetSp());
		}
	}
}

TEST_CASE("AtkEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 1, 0, 200, 1, 1, 1);
	Setup(target, 1, 0, 200, 1, 1, 1);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedAtk());
	REQUIRE_EQ(false, algo.IsAffectAtk());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedAtk(0));
		REQUIRE_EQ(0, algo.GetAffectedAtk());
		REQUIRE_EQ(true, algo.IsAffectAtk());

		SUBCASE("normal") {
			REQUIRE_EQ(0, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(200, target->GetAtk());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAtk(true);
			REQUIRE_EQ(0, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(200, target->GetAtk());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedAtk(-50));
		REQUIRE_EQ(-50, algo.GetAffectedAtk());
		REQUIRE_EQ(true, algo.IsAffectAtk());

		SUBCASE("normal") {
			REQUIRE_EQ(-50, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(150, target->GetAtk());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAtk(true);
			REQUIRE_EQ(-50, algo.ApplyAtkEffect());

			REQUIRE_EQ(250, source->GetAtk());
			REQUIRE_EQ(150, target->GetAtk());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedAtk(50));
		REQUIRE_EQ(50, algo.GetAffectedAtk());
		REQUIRE_EQ(true, algo.IsAffectAtk());

		SUBCASE("normal") {
			REQUIRE_EQ(50, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(250, target->GetAtk());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAtk(true);
			REQUIRE_EQ(50, algo.ApplyAtkEffect());

			REQUIRE_EQ(150, source->GetAtk());
			REQUIRE_EQ(250, target->GetAtk());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedAtk(-999));
		REQUIRE_EQ(-999, algo.GetAffectedAtk());
		REQUIRE_EQ(true, algo.IsAffectAtk());

		SUBCASE("normal") {
			REQUIRE_EQ(-100, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(100, target->GetAtk());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAtk(true);
			REQUIRE_EQ(-100, algo.ApplyAtkEffect());

			REQUIRE_EQ(300, source->GetAtk());
			REQUIRE_EQ(100, target->GetAtk());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedAtk(999));
		REQUIRE_EQ(999, algo.GetAffectedAtk());
		REQUIRE_EQ(true, algo.IsAffectAtk());

		SUBCASE("normal") {
			REQUIRE_EQ(200, algo.ApplyAtkEffect());

			REQUIRE_EQ(200, source->GetAtk());
			REQUIRE_EQ(400, target->GetAtk());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAtk(true);
			REQUIRE_EQ(200, algo.ApplyAtkEffect());

			REQUIRE_EQ(100, source->GetAtk());
			REQUIRE_EQ(400, target->GetAtk());
		}
	}
}

TEST_CASE("DefEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 1, 0, 1, 200, 1, 1);
	Setup(target, 1, 0, 1, 200, 1, 1);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedDef());
	REQUIRE_EQ(false, algo.IsAffectDef());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedDef(0));
		REQUIRE_EQ(0, algo.GetAffectedDef());
		REQUIRE_EQ(true, algo.IsAffectDef());

		SUBCASE("normal") {
			REQUIRE_EQ(0, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(200, target->GetDef());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbDef(true);
			REQUIRE_EQ(0, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(200, target->GetDef());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedDef(-50));
		REQUIRE_EQ(-50, algo.GetAffectedDef());
		REQUIRE_EQ(true, algo.IsAffectDef());

		SUBCASE("normal") {
			REQUIRE_EQ(-50, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(150, target->GetDef());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbDef(true);
			REQUIRE_EQ(-50, algo.ApplyDefEffect());

			REQUIRE_EQ(250, source->GetDef());
			REQUIRE_EQ(150, target->GetDef());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedDef(50));
		REQUIRE_EQ(50, algo.GetAffectedDef());
		REQUIRE_EQ(true, algo.IsAffectDef());

		SUBCASE("normal") {
			REQUIRE_EQ(50, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(250, target->GetDef());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbDef(true);
			REQUIRE_EQ(50, algo.ApplyDefEffect());

			REQUIRE_EQ(150, source->GetDef());
			REQUIRE_EQ(250, target->GetDef());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedDef(-999));
		REQUIRE_EQ(-999, algo.GetAffectedDef());
		REQUIRE_EQ(true, algo.IsAffectDef());

		SUBCASE("normal") {
			REQUIRE_EQ(-100, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(100, target->GetDef());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbDef(true);
			REQUIRE_EQ(-100, algo.ApplyDefEffect());

			REQUIRE_EQ(300, source->GetDef());
			REQUIRE_EQ(100, target->GetDef());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedDef(999));
		REQUIRE_EQ(999, algo.GetAffectedDef());
		REQUIRE_EQ(true, algo.IsAffectDef());

		SUBCASE("normal") {
			REQUIRE_EQ(200, algo.ApplyDefEffect());

			REQUIRE_EQ(200, source->GetDef());
			REQUIRE_EQ(400, target->GetDef());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbDef(true);
			REQUIRE_EQ(200, algo.ApplyDefEffect());

			REQUIRE_EQ(100, source->GetDef());
			REQUIRE_EQ(400, target->GetDef());
		}
	}
}

TEST_CASE("SpiEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 1, 0, 1, 1, 200, 1);
	Setup(target, 1, 0, 1, 1, 200, 1);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedSpi());
	REQUIRE_EQ(false, algo.IsAffectSpi());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedSpi(0));
		REQUIRE_EQ(0, algo.GetAffectedSpi());
		REQUIRE_EQ(true, algo.IsAffectSpi());

		SUBCASE("normal") {
			REQUIRE_EQ(0, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(200, target->GetSpi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSpi(true);
			REQUIRE_EQ(0, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(200, target->GetSpi());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedSpi(-50));
		REQUIRE_EQ(-50, algo.GetAffectedSpi());
		REQUIRE_EQ(true, algo.IsAffectSpi());

		SUBCASE("normal") {
			REQUIRE_EQ(-50, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(150, target->GetSpi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSpi(true);
			REQUIRE_EQ(-50, algo.ApplySpiEffect());

			REQUIRE_EQ(250, source->GetSpi());
			REQUIRE_EQ(150, target->GetSpi());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedSpi(50));
		REQUIRE_EQ(50, algo.GetAffectedSpi());
		REQUIRE_EQ(true, algo.IsAffectSpi());

		SUBCASE("normal") {
			REQUIRE_EQ(50, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(250, target->GetSpi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSpi(true);
			REQUIRE_EQ(50, algo.ApplySpiEffect());

			REQUIRE_EQ(150, source->GetSpi());
			REQUIRE_EQ(250, target->GetSpi());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedSpi(-999));
		REQUIRE_EQ(-999, algo.GetAffectedSpi());
		REQUIRE_EQ(true, algo.IsAffectSpi());

		SUBCASE("normal") {
			REQUIRE_EQ(-100, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(100, target->GetSpi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSpi(true);
			REQUIRE_EQ(-100, algo.ApplySpiEffect());

			REQUIRE_EQ(300, source->GetSpi());
			REQUIRE_EQ(100, target->GetSpi());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedSpi(999));
		REQUIRE_EQ(999, algo.GetAffectedSpi());
		REQUIRE_EQ(true, algo.IsAffectSpi());

		SUBCASE("normal") {
			REQUIRE_EQ(200, algo.ApplySpiEffect());

			REQUIRE_EQ(200, source->GetSpi());
			REQUIRE_EQ(400, target->GetSpi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbSpi(true);
			REQUIRE_EQ(200, algo.ApplySpiEffect());

			REQUIRE_EQ(100, source->GetSpi());
			REQUIRE_EQ(400, target->GetSpi());
		}
	}
}


TEST_CASE("AgiEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	Setup(source, 1, 0, 1, 1, 1, 200);
	Setup(target, 1, 0, 1, 1, 1, 200);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(target, algo.GetTarget());

	REQUIRE_EQ(0, algo.GetAffectedAgi());
	REQUIRE_EQ(false, algo.IsAffectAgi());

	SUBCASE("0") {
		REQUIRE_EQ(0, algo.SetAffectedAgi(0));
		REQUIRE_EQ(0, algo.GetAffectedAgi());
		REQUIRE_EQ(true, algo.IsAffectAgi());

		SUBCASE("normal") {
			REQUIRE_EQ(0, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(200, target->GetAgi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAgi(true);
			REQUIRE_EQ(0, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(200, target->GetAgi());
		}
	}

	SUBCASE("-50") {
		REQUIRE_EQ(-50, algo.SetAffectedAgi(-50));
		REQUIRE_EQ(-50, algo.GetAffectedAgi());
		REQUIRE_EQ(true, algo.IsAffectAgi());

		SUBCASE("normal") {
			REQUIRE_EQ(-50, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(150, target->GetAgi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAgi(true);
			REQUIRE_EQ(-50, algo.ApplyAgiEffect());

			REQUIRE_EQ(250, source->GetAgi());
			REQUIRE_EQ(150, target->GetAgi());
		}
	}

	SUBCASE("+50") {
		REQUIRE_EQ(50, algo.SetAffectedAgi(50));
		REQUIRE_EQ(50, algo.GetAffectedAgi());
		REQUIRE_EQ(true, algo.IsAffectAgi());

		SUBCASE("normal") {
			REQUIRE_EQ(50, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(250, target->GetAgi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAgi(true);
			REQUIRE_EQ(50, algo.ApplyAgiEffect());

			REQUIRE_EQ(150, source->GetAgi());
			REQUIRE_EQ(250, target->GetAgi());
		}
	}

	SUBCASE("-999") {
		REQUIRE_EQ(-999, algo.SetAffectedAgi(-999));
		REQUIRE_EQ(-999, algo.GetAffectedAgi());
		REQUIRE_EQ(true, algo.IsAffectAgi());

		SUBCASE("normal") {
			REQUIRE_EQ(-100, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(100, target->GetAgi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAgi(true);
			REQUIRE_EQ(-100, algo.ApplyAgiEffect());

			REQUIRE_EQ(300, source->GetAgi());
			REQUIRE_EQ(100, target->GetAgi());
		}
	}

	SUBCASE("+999") {
		REQUIRE_EQ(999, algo.SetAffectedAgi(999));
		REQUIRE_EQ(999, algo.GetAffectedAgi());
		REQUIRE_EQ(true, algo.IsAffectAgi());

		SUBCASE("normal") {
			REQUIRE_EQ(200, algo.ApplyAgiEffect());

			REQUIRE_EQ(200, source->GetAgi());
			REQUIRE_EQ(400, target->GetAgi());
		}

		SUBCASE("absorb") {
			algo.SetIsAbsorbAgi(true);
			REQUIRE_EQ(200, algo.ApplyAgiEffect());

			REQUIRE_EQ(100, source->GetAgi());
			REQUIRE_EQ(400, target->GetAgi());
		}
	}
}

TEST_CASE("BasicStateEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	TestAlgo algo(source, target);
	algo.Start();

	const int id = 3;

	SUBCASE("inflict") {
		auto st = Game_BattleAlgorithm::StateEffect::Inflicted;
		algo.AddAffectedState({ id, st });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("have") {
			target->AddState(id, true);

			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}

			REQUIRE_EQ(true, target->HasState(id));
		}

		SUBCASE("havenot") {
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(true, target->HasState(id));
		}
	}

	SUBCASE("already") {
		auto st = Game_BattleAlgorithm::StateEffect::AlreadyInflicted;
		algo.AddAffectedState({ id, st });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("have") {
			target->AddState(id, true);
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(false, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(true, target->HasState(id));
		}

		SUBCASE("havenot") {
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(false, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(false, target->HasState(id));
		}
	}

	SUBCASE("healed") {
		auto st = Game_BattleAlgorithm::StateEffect::Healed;
		algo.AddAffectedState({ id, st });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("have") {
			target->AddState(id, true);
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(false, target->HasState(id));
		}

		SUBCASE("havenot") {
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(false, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(false, target->HasState(id));
		}
	}

	SUBCASE("healed_physical") {
		auto st = Game_BattleAlgorithm::StateEffect::HealedByAttack;
		algo.AddAffectedState({ id, st });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("have") {
			target->AddState(id, true);
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(false, target->HasState(id));
		}

		SUBCASE("havenot") {
			SUBCASE("all") {
				algo.ApplyStateEffects();
			}
			SUBCASE("one") {
				REQUIRE_EQ(false, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			}
			REQUIRE_EQ(false, target->HasState(id));
		}
	}

	SUBCASE("many") {
		algo.AddAffectedState({ 2, Game_BattleAlgorithm::StateEffect::Inflicted});
		algo.AddAffectedState({ 3, Game_BattleAlgorithm::StateEffect::Inflicted});
		algo.AddAffectedState({ 4, Game_BattleAlgorithm::StateEffect::Inflicted});
		algo.AddAffectedState({ 3, Game_BattleAlgorithm::StateEffect::Healed});
		REQUIRE_EQ(4, algo.GetStateEffects().size());

		SUBCASE("all") {
			algo.ApplyStateEffects();

			REQUIRE_EQ(true, target->HasState(2));
			REQUIRE_EQ(false, target->HasState(3));
			REQUIRE_EQ(true, target->HasState(4));
		}
		SUBCASE("each") {
			REQUIRE_EQ(false, target->HasState(2));
			REQUIRE_EQ(false, target->HasState(3));
			REQUIRE_EQ(false, target->HasState(4));

			REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[0]));
			REQUIRE_EQ(true, target->HasState(2));
			REQUIRE_EQ(false, target->HasState(3));
			REQUIRE_EQ(false, target->HasState(4));

			REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[1]));
			REQUIRE_EQ(true, target->HasState(2));
			REQUIRE_EQ(true, target->HasState(3));
			REQUIRE_EQ(false, target->HasState(4));

			REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[2]));
			REQUIRE_EQ(true, target->HasState(2));
			REQUIRE_EQ(true, target->HasState(3));
			REQUIRE_EQ(true, target->HasState(4));

			REQUIRE_EQ(true, algo.ApplyStateEffect(algo.GetStateEffects()[3]));
			REQUIRE_EQ(true, target->HasState(2));
			REQUIRE_EQ(false, target->HasState(3));
			REQUIRE_EQ(true, target->HasState(4));
		}
	}
}


TEST_CASE("DeathStateEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	Setup(target, 100, 0, 1, 1, 1, 1);

	TestAlgo algo(source, target);
	algo.Start();

	REQUIRE_EQ(false, target->IsDead());

	SUBCASE("kill") {
		algo.AddAffectedState({ 1, Game_BattleAlgorithm::StateEffect::Inflicted });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("dead") {
			target->Kill();
			algo.ApplyStateEffects();
			REQUIRE_EQ(true, target->IsDead());
			REQUIRE_EQ(0, target->GetHp());
		}

		SUBCASE("alive") {
			algo.ApplyStateEffects();
			REQUIRE_EQ(true, target->IsDead());
			REQUIRE_EQ(0, target->GetHp());
		}
	}

	SUBCASE("revive") {
		algo.AddAffectedState({ 1, Game_BattleAlgorithm::StateEffect::Healed });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("dead") {
			target->Kill();
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(1, target->GetHp());
		}

		SUBCASE("dead+50") {
			target->Kill();
			algo.SetAffectedHp(50);
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(50, target->GetHp());
		}

		SUBCASE("dead+999") {
			target->Kill();
			algo.SetAffectedHp(999);
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(100, target->GetHp());
		}

		SUBCASE("dead-50") {
			target->Kill();
			algo.SetAffectedHp(-50);
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(1, target->GetHp());
		}

		SUBCASE("alive") {
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(100, target->GetHp());
		}
	}

	SUBCASE("revive_physical") {
		algo.AddAffectedState({ 1, Game_BattleAlgorithm::StateEffect::HealedByAttack });
		REQUIRE_EQ(1, algo.GetStateEffects().size());

		SUBCASE("dead") {
			target->Kill();
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(1, target->GetHp());
		}

		SUBCASE("alive") {
			algo.ApplyStateEffects();
			REQUIRE_EQ(false, target->IsDead());
			REQUIRE_EQ(100, target->GetHp());
		}
	}
}


TEST_CASE("AttributeShiftEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);

	TestAlgo algo(source, target);
	algo.Start();

	const int id = 3;
	const int alt_id = 5;

	REQUIRE_EQ(target, algo.GetTarget());
	REQUIRE_EQ(0, target->GetAttributeRateShift(id));
	REQUIRE_EQ(0, target->GetAttributeRateShift(alt_id));

	SUBCASE("Up") {
		algo.AddAffectedAttribute({ id, 1 });
		REQUIRE_EQ(1, algo.GetShiftedAttributes().size());

		SUBCASE("all") {
			algo.ApplyAttributeShiftEffects();
			REQUIRE_EQ(1, target->GetAttributeRateShift(id));
		}

		SUBCASE("one") {
			REQUIRE_EQ(1, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[0]));
			REQUIRE_EQ(1, target->GetAttributeRateShift(id));

			REQUIRE_EQ(0, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[0]));
			REQUIRE_EQ(1, target->GetAttributeRateShift(id));
		}
	}

	SUBCASE("Down") {
		algo.AddAffectedAttribute({ id, -1 });
		REQUIRE_EQ(1, algo.GetShiftedAttributes().size());

		SUBCASE("all") {
			algo.ApplyAttributeShiftEffects();
			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));
		}

		SUBCASE("one") {
			REQUIRE_EQ(-1, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[0]));
			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));

			REQUIRE_EQ(0, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[0]));
			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));
		}
	}

	SUBCASE("Both") {
		algo.AddAffectedAttribute({ id, -1 });
		algo.AddAffectedAttribute({ alt_id, 1 });
		REQUIRE_EQ(2, algo.GetShiftedAttributes().size());

		SUBCASE("all") {
			algo.ApplyAttributeShiftEffects();

			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));
			REQUIRE_EQ(1, target->GetAttributeRateShift(alt_id));
		}

		SUBCASE("each") {
			REQUIRE_EQ(-1, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[0]));
			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));
			REQUIRE_EQ(0, target->GetAttributeRateShift(alt_id));

			REQUIRE_EQ(1, algo.ApplyAttributeShiftEffect(algo.GetShiftedAttributes()[1]));
			REQUIRE_EQ(-1, target->GetAttributeRateShift(id));
			REQUIRE_EQ(1, target->GetAttributeRateShift(alt_id));
		}
	}
}

TEST_CASE("PostActionSwitches") {
	const MockBattle mb;
	TestAlgo algo(Main_Data::game_party->GetActor(0));

	algo.SetSwitchEnable(1);
	algo.SetSwitchDisable(4);

	Main_Data::game_switches->Set(1, false);
	Main_Data::game_switches->Set(4, true);

	REQUIRE_FALSE(Main_Data::game_switches->Get(1));
	REQUIRE(Main_Data::game_switches->Get(4));

	algo.ProcessPostActionSwitches();

	REQUIRE(Main_Data::game_switches->Get(1));
	REQUIRE_FALSE(Main_Data::game_switches->Get(4));
}

TEST_CASE("Algo::None") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	Game_BattleAlgorithm::None algo(source);

	REQUIRE(algo.GetStartMessage(0).empty());
	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Defend") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	Game_BattleAlgorithm::Defend algo(source);
	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Defend, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	REQUIRE(source->IsDefending());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();

	REQUIRE(source->IsDefending());

	Game_BattleAlgorithm::None algo2(source);

	// Any other algo clears defend status
	REQUIRE_FALSE(source->IsDefending());
}

TEST_CASE("Algo::Observe") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	Game_BattleAlgorithm::Observe algo(source);

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Charge") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	Game_BattleAlgorithm::Charge algo(source);

	REQUIRE_EQ(true, algo.ActionIsPossible());
	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());
	REQUIRE_FALSE(source->IsCharged());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());

	REQUIRE(source->IsCharged());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::SelfDestruct") {
	const MockBattle mb(4, 4);
	auto* source = Main_Data::game_enemyparty->GetEnemy(0);
	auto* targets = Main_Data::game_party.get();
	Game_BattleAlgorithm::SelfDestruct algo(source, targets);

	auto& state = lcf::Data::states[1];
	state.release_by_damage = 100;

	REQUIRE_EQ(true, targets->GetActor(0)->AddState(2, true));

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	algo.Start();
	REQUIRE_EQ(false, source->IsHidden());
	REQUIRE_EQ(true, targets->GetActor(0)->HasState(2));
	for (int i = 0; i < 4; ++i) {
		REQUIRE_EQ(targets->GetActor(i), algo.GetTarget());

		algo.Execute();
		REQUIRE(algo.IsAffectHp());
		REQUIRE(algo.GetAffectedHp() < 0);
		REQUIRE(algo.IsSuccess());
		REQUIRE_EQ(i > 0, source->IsHidden());

		algo.ApplyAll();
		REQUIRE_EQ(true, source->IsHidden());
		REQUIRE_EQ(false, targets->GetActor(i)->HasState(2));
		REQUIRE_EQ(i < 3, algo.TargetNext());
	}
	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Escape") {
	const MockBattle mb;
	auto* source = Main_Data::game_enemyparty->GetEnemy(0);
	Game_BattleAlgorithm::Escape algo(source);

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_WalkRight, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());

	algo.ApplyAll();
	REQUIRE_EQ(true, source->IsHidden());

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Transform") {
	const MockBattle mb;
	auto* source = Main_Data::game_enemyparty->GetEnemy(0);
	Game_BattleAlgorithm::Transform algo(source, 10);

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());
	REQUIRE_EQ(1, source->GetId());

	algo.ApplyAll();
	REQUIRE_EQ(10, source->GetId());

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::DoNothing") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	Game_BattleAlgorithm::None algo(source);

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Idle, algo.GetSourcePose());
	REQUIRE_EQ(true, algo.ActionIsPossible());

	REQUIRE(algo.GetStartMessage(0).empty());

	algo.Start();
	REQUIRE_EQ(source, algo.GetTarget());

	algo.Execute();
	REQUIRE(algo.IsSuccess());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());

	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Item::PossibleAndValid") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_party->GetActor(1);
	auto& item = lcf::Data::items[0];

	Game_BattleAlgorithm::Item algo(source, target, item);

	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Item, algo.GetSourcePose());
	REQUIRE_EQ(false, algo.ActionIsPossible());

	Main_Data::game_party->AddItem(1, 1);

	REQUIRE_EQ(true, algo.ActionIsPossible());

	for (int i = 0; i < 20; ++i) {
		item.type = i;
		bool success = (i == lcf::rpg::Item::Type_medicine || i == lcf::rpg::Item::Type_switch);
		REQUIRE_EQ(success, algo.IsTargetValid(*source));
	}
}

TEST_CASE("Algo::Item::Switch") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_party->GetActor(1);
	auto& item = lcf::Data::items[0];
	item.type = lcf::rpg::Item::Type_switch;
	item.switch_id = 5;

	Game_BattleAlgorithm::Item algo(source, target, item);
	Main_Data::game_party->AddItem(1, 1);
	REQUIRE_EQ(1, Main_Data::game_party->GetItemCount(1));

	algo.Start();
	REQUIRE_EQ(target, algo.GetTarget());
	REQUIRE_EQ(0, Main_Data::game_party->GetItemCount(1));

	algo.Execute();
	REQUIRE(algo.IsSuccess());
	REQUIRE_EQ(5, algo.GetAffectedSwitch());
}

TEST_CASE("Algo::Item::Medicine") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_party->GetActor(1);
	Setup(target, 200, 200, 1, 1, 1, 1);
	target->SetHp(100);
	target->SetSp(100);

	auto& item = lcf::Data::items[0];
	item.type = lcf::rpg::Item::Type_medicine;

	Game_BattleAlgorithm::Item algo(source, target, item);

	Main_Data::game_party->AddItem(1, 1);
	REQUIRE_EQ(1, Main_Data::game_party->GetItemCount(1));

	algo.Start();
	REQUIRE_EQ(0, Main_Data::game_party->GetItemCount(1));
	REQUIRE_EQ(target, algo.GetTarget());

	SUBCASE("hp") {
		item.recover_hp = 20;
		item.recover_hp_rate = 100;

		SUBCASE("normal") {
			SUBCASE("alive") {
				algo.Execute();

				REQUIRE_EQ(220, algo.GetAffectedHp());
				REQUIRE_EQ(0, algo.GetAffectedSp());
			}

			SUBCASE("dead") {
				target->Kill();
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(0, algo.GetAffectedSp());
			}
		}

		SUBCASE("koonly") {
			item.ko_only = true;
			SUBCASE("alive") {
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(0, algo.GetAffectedSp());
			}

			SUBCASE("dead") {
				target->Kill();
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(0, algo.GetAffectedSp());
			}
		}
	}

	SUBCASE("sp") {
		item.recover_sp = 20;
		item.recover_sp_rate = 100;

		SUBCASE("normal") {
			SUBCASE("alive") {
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(220, algo.GetAffectedSp());
			}

			SUBCASE("dead") {
				target->Kill();
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(220, algo.GetAffectedSp());
			}
		}

		SUBCASE("koonly") {
			item.ko_only = true;
			SUBCASE("alive") {
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(0, algo.GetAffectedSp());
			}

			SUBCASE("dead") {
				target->Kill();
				algo.Execute();

				REQUIRE_EQ(0, algo.GetAffectedHp());
				REQUIRE_EQ(220, algo.GetAffectedSp());
			}
		}
	}

	SUBCASE("state") {
		item.state_set = { false, true };
		target->AddState(2, true);

		algo.Execute();

		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(2, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::Healed, algo.GetStateEffects()[0].effect);
	}

	SUBCASE("revive") {
		item.state_set = { true };
		target->Kill();

		SUBCASE("only revive") {
			algo.Execute();

			REQUIRE_EQ(0, algo.GetAffectedHp());
			REQUIRE_EQ(0, algo.GetAffectedSp());
		}

		SUBCASE("revive+hp") {
			item.recover_hp = 20;
			item.recover_hp_rate = 100;

			algo.Execute();

			REQUIRE_EQ(220, algo.GetAffectedHp());
			REQUIRE_EQ(0, algo.GetAffectedSp());
		}

		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(1, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::Healed, algo.GetStateEffects()[0].effect);
	}

	REQUIRE(algo.IsSuccess());
	REQUIRE(algo.IsPositive());

	algo.ApplyAll();

	REQUIRE_EQ(false, algo.TargetNext());
	algo.ProcessPostActionSwitches();
}

TEST_CASE("Algo::Skill::Possible") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_party->GetActor(1);
	auto& skill = lcf::Data::skills[0];
	auto& item = lcf::Data::items[0];
	Setup(source, 200, 200, 1, 1, 1, 1);

	skill.sp_cost = 20;

	SUBCASE("normal") {
		Game_BattleAlgorithm::Skill algo(source, target, skill, nullptr);
		REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Skill, algo.GetSourcePose());

		source->SetSp(0);
		REQUIRE_EQ(false, algo.ActionIsPossible());

		source->SetSp(200);
		REQUIRE_EQ(true, algo.ActionIsPossible());
	}

	SUBCASE("item") {
		Game_BattleAlgorithm::Skill algo(source, target, skill, &item);
		REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Skill, algo.GetSourcePose());

		REQUIRE_EQ(false, algo.ActionIsPossible());

		Main_Data::game_party->AddItem(1, 1);

		REQUIRE_EQ(true, algo.ActionIsPossible());
	}
}

TEST_CASE("Algo::Skill::TargetValid") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_party->GetActor(1);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 200, 1, 1, 1, 1);

	Game_BattleAlgorithm::Skill algo(source, target, skill, nullptr);
	REQUIRE_EQ(lcf::rpg::BattlerAnimation::Pose_Skill, algo.GetSourcePose());

	REQUIRE(algo.IsTargetValid(*target));

	SUBCASE("hidden") {
		target->SetHidden(true);
		REQUIRE_FALSE(algo.IsTargetValid(*target));
	}

	SUBCASE("dead") {
		target->Kill();
		REQUIRE_FALSE(algo.IsTargetValid(*target));
	}

	SUBCASE("dead+kill") {
		target->Kill();
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		skill.state_effects = { true };
		REQUIRE_FALSE(algo.IsTargetValid(*target));
		skill.reverse_state_effect = true;
		REQUIRE_FALSE(algo.IsTargetValid(*target));
	}

	SUBCASE("dead+revive") {
		target->Kill();
		skill.scope = lcf::rpg::Skill::Scope_ally;
		skill.state_effects = { true };
		REQUIRE(algo.IsTargetValid(*target));
		skill.reverse_state_effect = true;
		REQUIRE(algo.IsTargetValid(*target));
	}
}

TEST_CASE("Algo::Skill::Consumption") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto& skill = lcf::Data::skills[0];
	skill.sp_cost = 20;
	Setup(source, 200, 200, 1, 1, 1, 1);

	SUBCASE("skill") {
		Game_BattleAlgorithm::Skill algo(source, skill, nullptr);

		REQUIRE_EQ(200, source->GetSp());
		algo.Start();
		REQUIRE_EQ(180, source->GetSp());
	}

	SUBCASE("skill item") {
		auto& item = lcf::Data::items[0];
		item.type = lcf::rpg::Item::Type_special;

		Game_BattleAlgorithm::Skill algo(source, skill, &item);
		Main_Data::game_party->AddItem(1, 1);
		REQUIRE_EQ(1, Main_Data::game_party->GetItemCount(1));
		algo.Start();
		REQUIRE_EQ(0, Main_Data::game_party->GetItemCount(1));
	}

	SUBCASE("skill equip") {
		auto& item = lcf::Data::items[0];
		item.type = lcf::rpg::Item::Type_weapon;

		Game_BattleAlgorithm::Skill algo(source, skill, &item);
		Main_Data::game_party->AddItem(1, 1);
		REQUIRE_EQ(1, Main_Data::game_party->GetItemCount(1));
		algo.Start();
		REQUIRE_EQ(1, Main_Data::game_party->GetItemCount(1));
	}
}

TEST_CASE("Algo::Skill::Switch") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto& skill = lcf::Data::skills[0];
	skill.type = lcf::rpg::Skill::Type_switch;
	skill.switch_id = 7;

	Game_BattleAlgorithm::Skill algo(source, skill);

	algo.Start();
	REQUIRE_EQ(0, algo.GetAffectedSwitch());

	algo.Execute();
	REQUIRE(algo.IsSuccess());
	REQUIRE_EQ(7, algo.GetAffectedSwitch());
}

TEST_CASE("Algo::Skill::EscapeTeleport") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto& skill = lcf::Data::skills[0];
	SUBCASE("escape") {
		skill.type = lcf::rpg::Skill::Type_escape;
	}
	SUBCASE("teleport") {
		skill.type = lcf::rpg::Skill::Type_teleport;
	}

	Game_BattleAlgorithm::Skill algo(source, skill);

	algo.Start();
	algo.Execute();
	REQUIRE(algo.IsSuccess());
}

TEST_CASE("Algo::Skill::EscapeTeleport") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto& skill = lcf::Data::skills[0];
	skill.type = lcf::rpg::Skill::Type_switch;
	skill.switch_id = 7;

	Game_BattleAlgorithm::Skill algo(source, skill);

	algo.Start();
	REQUIRE_EQ(0, algo.GetAffectedSwitch());

	algo.Execute();
	REQUIRE(algo.IsSuccess());
	REQUIRE_EQ(7, algo.GetAffectedSwitch());
}

TEST_CASE("Algo::Skill::PositiveFlag") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto& skill = lcf::Data::skills[0];

	for (int type = 0; type < 10; ++type) {
		for (int scope = 0; scope < 10; ++scope) {
			skill.type = type;
			skill.scope = scope;
			bool positive = Algo::IsNormalOrSubskill(skill) && Algo::SkillTargetsAllies(skill);

			Game_BattleAlgorithm::Skill algo(source, skill);
			algo.Start();
			algo.Execute();
			REQUIRE_EQ(positive, algo.IsPositive());
		}
	}
}

TEST_CASE("Algo::Skill::HitFail") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_hp = true;
	skill.affect_sp = true;
	skill.affect_attack = true;
	skill.affect_defense = true;
	skill.affect_spirit = true;
	skill.affect_agility = true;
	skill.state_effects = { false, true };
	skill.attribute_effects = { true };
	skill.affect_attr_defence = true;
	skill.power = 1;
	skill.hit = 0;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	auto test = [&]() {
		algo.Start();
		algo.Execute();
		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(false, algo.IsAffectHp());
		REQUIRE_EQ(false, algo.IsAffectSp());
		REQUIRE_EQ(false, algo.IsAffectAtk());
		REQUIRE_EQ(false, algo.IsAffectDef());
		REQUIRE_EQ(false, algo.IsAffectSpi());
		REQUIRE_EQ(false, algo.IsAffectAgi());
		REQUIRE_EQ(false, algo.IsAbsorbHp());
		REQUIRE_EQ(false, algo.IsAbsorbSp());
		REQUIRE_EQ(false, algo.IsAbsorbAtk());
		REQUIRE_EQ(false, algo.IsAbsorbDef());
		REQUIRE_EQ(false, algo.IsAbsorbSpi());
		REQUIRE_EQ(false, algo.IsAbsorbAgi());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
		REQUIRE_EQ(0, algo.GetShiftedAttributes().size());
	};

	SUBCASE("positive") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		SUBCASE("normal") {
			test();
		}
		SUBCASE("absorb") {
			skill.absorb_damage = true;
			test();
		}
	}

	SUBCASE("negative") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		SUBCASE("normal") {
			test();
		}
		SUBCASE("absorb") {
			skill.absorb_damage = true;
			test();
		}
	}
}


TEST_CASE("Algo::Skill::HpEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 0, 1, 1, 1, 1);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_hp = true;
	skill.power = 1;
	skill.hit = 100;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;

	auto& state = lcf::Data::states[1];
	state.release_by_damage = 100;

	target->AddState(2, true);
	REQUIRE_EQ(true, target->HasState(2));

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	auto test = [&](bool absorb, bool healing, bool recover_states) {
		CAPTURE(absorb);
		CAPTURE(healing);
		CAPTURE(recover_states);
		algo.Execute();

		REQUIRE_EQ(true, algo.IsAffectHp());
		REQUIRE_EQ(true, algo.IsSuccess());

		REQUIRE_EQ(absorb, algo.IsAbsorbHp());
		if (healing) {
			REQUIRE(algo.GetAffectedHp() > 0);
		} else {
			REQUIRE(algo.GetAffectedHp() < 0);
		}

		if (recover_states) {
			REQUIRE_EQ(1, algo.GetStateEffects().size());
			REQUIRE_EQ(2, algo.GetStateEffects()[0].state_id);
			REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::HealedByAttack, algo.GetStateEffects()[0].effect);
		} else {
			REQUIRE_EQ(0, algo.GetStateEffects().size());
		}
	};

	auto testZero = [&](bool success, bool absorb) {
		skill.power = 0;
		algo.Execute();
		REQUIRE_EQ(0, algo.GetAffectedHp());
		REQUIRE_EQ(success, algo.IsAffectHp());
		REQUIRE_EQ(success, algo.IsSuccess());
		REQUIRE_EQ(absorb, algo.IsAbsorbHp());
	};

	SUBCASE("positive") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		algo.Start();

		SUBCASE("normal") {
			SUBCASE("normal") {
				test(false, true, false);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				test(false, true, false);
			}

			SUBCASE("limit") {
				skill.power = 999999;
				algo.Execute();
				REQUIRE_EQ(9999, algo.GetAffectedHp());
			}
		}

		SUBCASE("attribute flip") {
			auto& attr = lcf::Data::attributes[0];
			attr.c_rate = -100;
			skill.attribute_effects = { true };

			SUBCASE("normal") {
				test(false, false, false);
			}
			SUBCASE("absorb") {
				test(false, false, false);
			}

			SUBCASE("limit") {
				// Inverting healing always non-lethal
				skill.power = 999999;
				algo.Execute();
				REQUIRE_EQ(-199, algo.GetAffectedHp());
			}
		}

		SUBCASE("zero") {
			SUBCASE("normal") {
				testZero(false, false);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				testZero(false, false);
			}
		}
	}

	SUBCASE("negative") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		algo.Start();

		SUBCASE("normal") {
			SUBCASE("normal") {
				SUBCASE("physical") {
					skill.physical_rate = 10;
					test(false, false, true);
				}
				SUBCASE("nophysical") {
					skill.physical_rate = 0;
					test(false, false, false);
				}
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				skill.physical_rate = 10;
				test(true, false, false);
			}

			SUBCASE("limit") {
				skill.power = 999999;
				SUBCASE("normal") {
					algo.Execute();
					REQUIRE_EQ(-9999, algo.GetAffectedHp());
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					algo.Execute();
					REQUIRE_EQ(-200, algo.GetAffectedHp());
				}
			}
		}

		SUBCASE("attribute flip") {
			auto& attr = lcf::Data::attributes[0];
			attr.c_rate = -100;
			skill.attribute_effects = { true };

			SUBCASE("normal") {
				SUBCASE("physical") {
					skill.physical_rate = 10;
					test(false, true, true);
				}
				SUBCASE("nophysical") {
					skill.physical_rate = 0;
					test(false, true, false);
				}
			}
			SUBCASE("absorb") {
				skill.physical_rate = 10;
				skill.absorb_damage = true;
				test(true, true, false);
			}

			SUBCASE("limit") {
				skill.power = 999999;
				SUBCASE("normal") {
					algo.Execute();
					REQUIRE_EQ(9999, algo.GetAffectedHp());
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					algo.Execute();
					REQUIRE_EQ(9999, algo.GetAffectedHp());
				}
			}
		}

		SUBCASE("zero") {
			SUBCASE("normal") {
				testZero(true, false);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				testZero(false, false);
			}
		}
	}
}

TEST_CASE("Algo::Skill::SpEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 200, 1, 1, 1, 1);
	target->SetSp(100);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_sp = true;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;
	skill.power = 1;
	skill.hit = 100;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	auto test = [&](bool absorb, bool healing) {
		CAPTURE(absorb);
		CAPTURE(healing);
		algo.Execute();

		REQUIRE_EQ(absorb, algo.IsAbsorbSp());
		if (healing) {
			REQUIRE(algo.GetAffectedSp() > 0);
		} else {
			REQUIRE(algo.GetAffectedSp() < 0);
		}

		REQUIRE_EQ(true, algo.IsAffectSp());
		REQUIRE_EQ(true, algo.IsSuccess());
	};

	auto testFail = [&]() {
		algo.Execute();
		REQUIRE_EQ(0, algo.GetAffectedSp());
		REQUIRE_EQ(false, algo.IsAffectSp());
		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(false, algo.IsAbsorbSp());
	};

	auto testZero = [&]() {
		skill.power = 0;
		testFail();
	};

	auto testLimit = [&]() {
		skill.power = 9999;
		testFail();
	};

	SUBCASE("positive") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		algo.Start();

		SUBCASE("normal") {
			SUBCASE("normal") {
				test(false, true);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				test(false, true);
			}
			SUBCASE("limit") {
				target->SetSp(200);
				testLimit();
			}
		}

		SUBCASE("attribute flip") {
			auto& attr = lcf::Data::attributes[0];
			attr.c_rate = -100;
			skill.attribute_effects = { true };

			SUBCASE("normal") {
				test(false, false);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				test(false, false);
			}
			SUBCASE("limit") {
				target->SetSp(0);
				testLimit();
			}
		}

		SUBCASE("zero") {
			testZero();
		}
	}

	SUBCASE("negative") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		algo.Start();

		SUBCASE("normal") {
			SUBCASE("normal") {
				test(false, false);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				test(true, false);
			}
			SUBCASE("limit") {
				SUBCASE("normal") {
					target->SetSp(0);
					testLimit();
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					target->SetSp(0);
					testLimit();
				}
			}
		}

		SUBCASE("attribute flip") {
			auto& attr = lcf::Data::attributes[0];
			attr.c_rate = -100;
			skill.attribute_effects = { true };

			SUBCASE("normal") {
				test(false, true);
			}
			SUBCASE("absorb") {
				skill.absorb_damage = true;
				test(true, true);
			}
			SUBCASE("limit") {
				SUBCASE("normal") {
					target->SetSp(200);
					testLimit();
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					target->SetSp(200);
					skill.power = 9999;
					test(true, true);
					REQUIRE_EQ(9999, algo.GetAffectedSp());
				}
			}
		}

		SUBCASE("zero") {
			testZero();
		}
	}
}

TEST_CASE("Algo::Skill::ParamEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 200, 100, 100, 100, 100);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.power = 1;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;
	skill.hit = 100;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	auto test = [&](bool increase) {
		CAPTURE(increase);
		algo.Execute();

		REQUIRE_EQ(false, algo.IsAbsorbAtk());
		REQUIRE_EQ(false, algo.IsAbsorbDef());
		REQUIRE_EQ(false, algo.IsAbsorbSpi());
		REQUIRE_EQ(false, algo.IsAbsorbAgi());

		if (skill.affect_attack) {
			if (increase) {
				REQUIRE(algo.GetAffectedAtk() > 0);
			} else {
				REQUIRE(algo.GetAffectedAtk() < 0);
			}
			REQUIRE_EQ(true, algo.IsAffectAtk());
		}
		if (skill.affect_defense) {
			if (increase) {
				REQUIRE(algo.GetAffectedDef() > 0);
			} else {
				REQUIRE(algo.GetAffectedDef() < 0);
			}
			REQUIRE_EQ(true, algo.IsAffectDef());
		}
		if (skill.affect_spirit) {
			if (increase) {
				REQUIRE(algo.GetAffectedSpi() > 0);
			} else {
				REQUIRE(algo.GetAffectedSpi() < 0);
			}
			REQUIRE_EQ(true, algo.IsAffectSpi());
		}
		if (skill.affect_agility) {
			if (increase) {
				REQUIRE(algo.GetAffectedAgi() > 0);
			} else {
				REQUIRE(algo.GetAffectedAgi() < 0);
			}
			REQUIRE_EQ(true, algo.IsAffectAgi());
		}
		REQUIRE_EQ(true, algo.IsSuccess());
	};

	auto testZero = [&]() {
		skill.power = 0;
		algo.Execute();
		REQUIRE_EQ(0, algo.GetAffectedAtk());
		REQUIRE_EQ(0, algo.GetAffectedDef());
		REQUIRE_EQ(0, algo.GetAffectedSpi());
		REQUIRE_EQ(0, algo.GetAffectedAgi());
		REQUIRE_EQ(false, algo.IsAffectAtk());
		REQUIRE_EQ(false, algo.IsAffectDef());
		REQUIRE_EQ(false, algo.IsAffectSpi());
		REQUIRE_EQ(false, algo.IsAffectAgi());
		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(false, algo.IsAbsorbAtk());
		REQUIRE_EQ(false, algo.IsAbsorbDef());
		REQUIRE_EQ(false, algo.IsAbsorbSpi());
		REQUIRE_EQ(false, algo.IsAbsorbAgi());
	};

	auto testLimit = [&](bool increase) {
		skill.power = 999;
		const auto adj = increase ? 9999 : -9999;
		if (skill.affect_attack) {
			target->ChangeAtkModifier(adj);
		}
		if (skill.affect_defense) {
			target->ChangeDefModifier(adj);
		}
		if (skill.affect_spirit) {
			target->ChangeSpiModifier(adj);
		}
		if (skill.affect_agility) {
			target->ChangeAgiModifier(adj);
		}

		testZero();
	};

	auto doParamTest = [&]() {
		SUBCASE("positive") {
			skill.scope = lcf::rpg::Skill::Scope_ally;
			algo.Start();

			SUBCASE("normal") {
				SUBCASE("normal") {
					test(true);
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					test(true);
				}

				SUBCASE("limit") {
					testLimit(true);
				}
			}

			SUBCASE("attribute flip") {
				auto& attr = lcf::Data::attributes[0];
				attr.c_rate = -100;
				skill.attribute_effects = { true };

				SUBCASE("normal") {
					test(false);
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					test(false);
				}
				SUBCASE("limit") {
					testLimit(false);
				}
			}

			SUBCASE("zero") {
				testZero();
			}
		}

		SUBCASE("negative") {
			skill.scope = lcf::rpg::Skill::Scope_enemy;
			algo.Start();

			SUBCASE("normal") {
				SUBCASE("normal") {
					test(false);
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					test(false);
				}

				SUBCASE("limit") {
					testLimit(false);
				}
			}

			SUBCASE("attribute flip") {
				auto& attr = lcf::Data::attributes[0];
				attr.c_rate = -100;
				skill.attribute_effects = { true };

				SUBCASE("normal") {
					test(true);
				}
				SUBCASE("absorb") {
					skill.absorb_damage = true;
					test(true);
				}

				SUBCASE("limit") {
					testLimit(true);
				}
			}

			SUBCASE("zero") {
				testZero();
			}
		}
	};

	SUBCASE("atk") {
		skill.affect_attack = true;
		doParamTest();
	}
	SUBCASE("def") {
		skill.affect_defense = true;
		doParamTest();
	}
	SUBCASE("spi") {
		skill.affect_spirit = true;
		doParamTest();
	}
	SUBCASE("agi") {
		skill.affect_agility = true;
		doParamTest();
	}
}

TEST_CASE("Algo::Skill::State") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	auto& state = lcf::Data::states[1];
	state.b_rate = 100;

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.hit = 100;
	skill.state_effects = { false, true };

	Game_BattleAlgorithm::Skill algo(source, target, skill);
	algo.Start();

	auto good = [&](auto effect) {
		CAPTURE(effect);
		algo.Execute();

		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(2, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(effect, algo.GetStateEffects()[0].effect);
	};

	auto fail = [&]() {
		algo.Execute();

		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
	};

	SUBCASE("positive") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		SUBCASE("normal") {
			SUBCASE("have") {
				target->AddState(2, true);
				good(Game_BattleAlgorithm::StateEffect::Healed);
			}
			SUBCASE("havenot") {
				fail();
			}
		}

		SUBCASE("reverse") {
			skill.reverse_state_effect = true;
			SUBCASE("have") {
				target->AddState(2, true);
				good(Game_BattleAlgorithm::StateEffect::AlreadyInflicted);
			}
			SUBCASE("havenot") {
				SUBCASE("100%") {
					state.b_rate = 100;
					good(Game_BattleAlgorithm::StateEffect::Inflicted);
				}
				SUBCASE("0%") {
					state.b_rate = 0;
					fail();
				}
			}
		}
	}

	SUBCASE("negative") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		SUBCASE("normal") {
			SUBCASE("have") {
				target->AddState(2, true);
				good(Game_BattleAlgorithm::StateEffect::AlreadyInflicted);
			}
			SUBCASE("havenot") {
				SUBCASE("100%") {
					state.b_rate = 100;
					good(Game_BattleAlgorithm::StateEffect::Inflicted);
				}
				SUBCASE("0%") {
					state.b_rate = 0;
					fail();
				}
			}
		}

		SUBCASE("reverse") {
			skill.reverse_state_effect = true;
			SUBCASE("have") {
				target->AddState(2, true);
				good(Game_BattleAlgorithm::StateEffect::Healed);
			}
			SUBCASE("havenot") {
				fail();
			}
		}
	}
}

TEST_CASE("Algo::Skill::Revive") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	auto& state = lcf::Data::states[1];
	state.b_rate = 100;
	Setup(target, 200, 0, 1 ,1, 1, 1);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.hit = 100;
	skill.state_effects = { true };
	skill.physical_rate = skill.magical_rate = skill.variance = 0;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	auto good = [&](int hp) {
		algo.Execute();

		REQUIRE_EQ(true, algo.IsRevived());
		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(1, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::Healed, algo.GetStateEffects()[0].effect);

		if (hp > 0) {
			REQUIRE_EQ(true, algo.IsAffectHp());
			REQUIRE_EQ(hp, algo.GetAffectedHp());
		} else {
			REQUIRE_EQ(false, algo.IsAffectHp());
		}
	};

	target->Kill();
	skill.scope = lcf::rpg::Skill::Scope_ally;
	algo.Start();
	REQUIRE_EQ(target, algo.GetTarget());

	SUBCASE("0") {
		good(0);
	}

	SUBCASE("50%") {
		skill.power = 50;
		good(100);
	}

	SUBCASE("100%") {
		skill.power = 100;
		good(200);
	}

	SUBCASE("75") {
		skill.affect_hp = true;
		skill.power = 75;
		good(75);
	}

}

TEST_CASE("Algo::Skill::AttributeShift") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.hit = 100;
	skill.attribute_effects = { true };
	skill.affect_attr_defence = true;

	Game_BattleAlgorithm::Skill algo(source, target, skill);
	algo.Start();

	auto good = [&](auto shift) {
		CAPTURE(shift);
		algo.Execute();

		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(1, algo.GetShiftedAttributes().size());
		REQUIRE_EQ(1, algo.GetShiftedAttributes()[0].attr_id);
		REQUIRE_EQ(shift, algo.GetShiftedAttributes()[0].shift);
	};

	auto fail = [&]() {
		algo.Execute();

		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
	};

	SUBCASE("positive") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		SUBCASE("normal") {
			SUBCASE("ok") {
				good(1);
			}
			SUBCASE("fail") {
				target->ShiftAttributeRate(1, 1);
				fail();
			}
		}
	}

	SUBCASE("negative") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		SUBCASE("normal") {
			SUBCASE("ok") {
				good(-1);
			}
			SUBCASE("fail") {
				target->ShiftAttributeRate(1, -1);
				fail();
			}
		}
	}
}

TEST_CASE("Algo::Skill::KilledByDamage") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 200, 1, 1, 1, 1);
	target->SetHp(1);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_hp = true;
	skill.affect_sp = true;
	skill.affect_attack = true;
	skill.affect_defense = true;
	skill.affect_spirit = true;
	skill.affect_agility = true;
	skill.state_effects = { false, true };
	skill.attribute_effects = { true };
	skill.affect_attr_defence = true;
	skill.power = 999;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;
	skill.hit = 100;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	skill.scope = lcf::rpg::Skill::Scope_enemy;

	algo.Start();

	auto test = [&](bool absorb) {
		algo.Execute();
		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(true, algo.IsAffectHp());
		REQUIRE_GE(-1, algo.GetAffectedHp());
		REQUIRE_EQ(absorb, algo.IsAbsorbHp());

		// When killed, all other effects skipped.
		REQUIRE_EQ(false, algo.IsAffectSp());
		REQUIRE_EQ(false, algo.IsAffectAtk());
		REQUIRE_EQ(false, algo.IsAffectDef());
		REQUIRE_EQ(false, algo.IsAffectSpi());
		REQUIRE_EQ(false, algo.IsAffectAgi());
		REQUIRE_EQ(false, algo.IsAbsorbSp());
		REQUIRE_EQ(false, algo.IsAbsorbAtk());
		REQUIRE_EQ(false, algo.IsAbsorbDef());
		REQUIRE_EQ(false, algo.IsAbsorbSpi());
		REQUIRE_EQ(false, algo.IsAbsorbAgi());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
		REQUIRE_EQ(0, algo.GetShiftedAttributes().size());
	};

	SUBCASE("normal") {
		test(false);
	}
	SUBCASE("absorb") {
		skill.absorb_damage = true;
		test(true);
	}
}

TEST_CASE("Algo::Skill::KilledByState") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	auto& state = lcf::Data::states[0];
	state.b_rate = 100;
	Setup(target, 200, 200, 50, 50, 50, 50);
	target->SetHp(100);
	target->SetSp(100);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_hp = true;
	skill.affect_sp = true;
	skill.affect_attack = true;
	skill.affect_defense = true;
	skill.affect_spirit = true;
	skill.affect_agility = true;
	skill.state_effects = { true };
	skill.attribute_effects = { true };
	skill.affect_attr_defence = true;
	skill.power = 1;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;
	skill.hit = 100;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	skill.scope = lcf::rpg::Skill::Scope_enemy;

	algo.Start();

	auto test = [&]() {
		algo.Execute();
		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(true, algo.IsAffectHp());
		REQUIRE_EQ(true, algo.IsAffectSp());
		REQUIRE_EQ(true, algo.IsAffectAtk());
		REQUIRE_EQ(true, algo.IsAffectDef());
		REQUIRE_EQ(true, algo.IsAffectSpi());
		REQUIRE_EQ(true, algo.IsAffectAgi());
		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(1, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::Inflicted, algo.GetStateEffects()[0].effect);

		// When killed by state, all other effects skipped.
		REQUIRE_EQ(0, algo.GetShiftedAttributes().size());
	};

	SUBCASE("normal") {
		skill.scope = lcf::rpg::Skill::Scope_enemy;
		test();
	}

	SUBCASE("reverse") {
		skill.scope = lcf::rpg::Skill::Scope_ally;
		skill.reverse_state_effect = true;
		test();
	}
}


TEST_CASE("Algo::Skill::SkipIfHpSpFail") {
	// Skills should fail if hp and sp effect was active and failed, regardless of what additional effects might have succeeded
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& skill = lcf::Data::skills[0];
	Setup(target, 200, 200, 1, 1, 1, 1);

	skill.type = lcf::rpg::Skill::Type_normal;
	skill.affect_attack = true;
	skill.affect_defense = true;
	skill.affect_spirit = true;
	skill.affect_agility = true;
	skill.state_effects = { false, true };
	skill.attribute_effects = { true };
	skill.affect_attr_defence = true;
	skill.power = 999;
	skill.physical_rate = skill.magical_rate = skill.variance = 0;
	skill.hit = 100;

	Game_BattleAlgorithm::Skill algo(source, target, skill);

	skill.scope = lcf::rpg::Skill::Scope_enemy;

	algo.Start();

	auto test = [&]() {
		algo.Execute();
		REQUIRE_EQ(false, algo.IsSuccess());
		REQUIRE_EQ(false, algo.IsAffectHp());
		REQUIRE_EQ(false, algo.IsAffectSp());
		REQUIRE_EQ(false, algo.IsAffectAtk());
		REQUIRE_EQ(false, algo.IsAffectDef());
		REQUIRE_EQ(false, algo.IsAffectSpi());
		REQUIRE_EQ(false, algo.IsAffectAgi());
		REQUIRE_EQ(false, algo.IsAbsorbSp());
		REQUIRE_EQ(false, algo.IsAbsorbAtk());
		REQUIRE_EQ(false, algo.IsAbsorbDef());
		REQUIRE_EQ(false, algo.IsAbsorbSpi());
		REQUIRE_EQ(false, algo.IsAbsorbAgi());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
		REQUIRE_EQ(0, algo.GetShiftedAttributes().size());
	};

	SUBCASE("hp fail") {
		skill.affect_hp = true;
		// Not normally allowed to run on dead target, but a 0 hp absorb will trigger failure
		target->Kill();
		skill.power = 0;
		skill.absorb_damage = true;
		test();
	}

	SUBCASE("sp fail") {
		skill.affect_sp = true;
		// Absorb 0 sp will fail.
		target->SetSp(0);
		skill.absorb_damage = true;
		test();
	}
}

TEST_CASE("Algo::Normal::WeaponHits") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& w1 = lcf::Data::items[0];
	auto& w2 = lcf::Data::items[1];
	w1.type = lcf::rpg::Item::Type_weapon;
	w2.type = lcf::rpg::Item::Type_weapon;

	source->SetEquipment(1, 1);
	source->SetEquipment(2, 2);

	REQUIRE_EQ(&w1, source->GetWeapon());
	REQUIRE_EQ(&w2, source->Get2ndWeapon());

	auto test = [&](int total_hits, int hits_mult, int combo, auto style) {
		CAPTURE(total_hits);
		CAPTURE(hits_mult);
		CAPTURE(combo);
		CAPTURE(style);
		CAPTURE(w1.dual_attack);
		CAPTURE(w2.dual_attack);

		Game_BattleAlgorithm::Normal algo(source, target, hits_mult, style);
		if (combo > 1) {
			algo.ApplyComboHitsMultiplier(combo);
		}
		REQUIRE_EQ(total_hits, algo.GetTotalRepetitions());

		algo.Start();

		if (style == Game_BattleAlgorithm::Normal::Style_Combined) {
			auto expect = source->GetNumberOfAttacks(Game_Battler::WeaponAll) * hits_mult * combo;
			REQUIRE_EQ(expect, total_hits);
			for (int i = 0; i < total_hits; ++i) {
				REQUIRE_EQ(Game_Battler::WeaponAll, algo.GetWeapon());
				algo.RepeatNext(false);
			}
		}

		if (style == Game_BattleAlgorithm::Normal::Style_MultiHit) {
			auto expect_pri = source->GetNumberOfAttacks(Game_Battler::WeaponPrimary) * hits_mult * combo;
			auto expect_sec = source->GetNumberOfAttacks(Game_Battler::WeaponSecondary) * hits_mult * combo;
			REQUIRE_EQ(expect_pri + expect_sec, total_hits);

			for (int i = 0; i < expect_pri; ++i) {
				REQUIRE_EQ(Game_Battler::WeaponPrimary, algo.GetWeapon());
				algo.RepeatNext(false);
			}
			for (int i = 0; i < expect_sec; ++i) {
				REQUIRE_EQ(Game_Battler::WeaponSecondary, algo.GetWeapon());
				algo.RepeatNext(false);
			}
		}
	};

	SUBCASE("combined style") {
		for (int c = 1; c < 4; ++c) {
			for (int m = 1; m < 4; ++m) {
				const auto mc = m * c;
				w1.dual_attack = false;
				w2.dual_attack = false;
				test(1 * mc, m, c, Game_BattleAlgorithm::Normal::Style_Combined);
				w1.dual_attack = true;
				w2.dual_attack = false;
				test(2 * mc, m, c, Game_BattleAlgorithm::Normal::Style_Combined);
				w1.dual_attack = false;
				w2.dual_attack = true;
				test(2 * mc, m, c, Game_BattleAlgorithm::Normal::Style_Combined);
				w1.dual_attack = true;
				w2.dual_attack = true;
				test(2 * mc, m, c, Game_BattleAlgorithm::Normal::Style_Combined);
			}
		}
	}

	SUBCASE("multi style") {
		for (int c = 1; c < 4; ++c) {
			for (int m = 1; m < 4; ++m) {
				const auto mc = m * c;
				w1.dual_attack = false;
				w2.dual_attack = false;
				test(2 * mc, m, c, Game_BattleAlgorithm::Normal::Style_MultiHit);
				w1.dual_attack = true;
				w2.dual_attack = false;
				test(3 * mc, m, c, Game_BattleAlgorithm::Normal::Style_MultiHit);
				w1.dual_attack = false;
				w2.dual_attack = true;
				test(3 * mc, m, c, Game_BattleAlgorithm::Normal::Style_MultiHit);
				w1.dual_attack = true;
				w2.dual_attack = true;
				test(4 * mc, m, c, Game_BattleAlgorithm::Normal::Style_MultiHit);
			}
		}
	}
}

TEST_CASE("Algo::Normal::HitFail") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	auto& w1 = lcf::Data::items[0];
	w1.type = lcf::rpg::Item::Type_weapon;
	w1.hit = 0;

	source->SetEquipment(1, 1);

	REQUIRE_EQ(&w1, source->GetWeapon());

	Game_BattleAlgorithm::Normal algo(source, target);

	algo.Start();
	algo.Execute();

	REQUIRE_EQ(false, algo.IsSuccess());
	REQUIRE_EQ(false, algo.IsAffectHp());
	REQUIRE_EQ(false, algo.IsAbsorbHp());
}

TEST_CASE("Algo::Normal::HpEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	Setup(target, 200, 0, 1, 1, 1, 1);
	auto& w1 = lcf::Data::items[0];
	w1.type = lcf::rpg::Item::Type_weapon;
	w1.hit = 100;

	source->SetEquipment(1, 1);
	REQUIRE_EQ(&w1, source->GetWeapon());

	auto& state = lcf::Data::states[1];
	state.release_by_damage = 100;

	target->AddState(2, true);
	REQUIRE_EQ(true, target->HasState(2));

	Game_BattleAlgorithm::Normal algo(source, target);

	algo.Start();

	auto test = [&](bool increase, bool recover_states) {
		algo.Execute();

		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(true, algo.IsAffectHp());
		if (increase) {
			REQUIRE_LE(0, algo.GetAffectedHp());
		} else {
			REQUIRE_GE(0, algo.GetAffectedHp());
		}
		if (recover_states) {
			REQUIRE_EQ(1, algo.GetStateEffects().size());
			REQUIRE_EQ(2, algo.GetStateEffects()[0].state_id);
			REQUIRE_EQ(Game_BattleAlgorithm::StateEffect::HealedByAttack, algo.GetStateEffects()[0].effect);
		} else {
			REQUIRE_EQ(0, algo.GetStateEffects().size());
		}

		algo.ApplyAll();
	};

	SUBCASE("normal") {
		test(false, true);
	}

	SUBCASE("attribute flip") {
		auto& attr = lcf::Data::attributes[0];
		attr.c_rate = -100;
		w1.attribute_set = { true };
		test(true, true);
	}

	SUBCASE("kill") {
		w1.atk_points1 = 99999;
		test(false, false);
		REQUIRE_EQ(true, target->IsDead());
	}
}

TEST_CASE("Algo::Normal::StateEffect") {
	const MockBattle mb;
	auto* source = Main_Data::game_party->GetActor(0);
	auto* target = Main_Data::game_enemyparty->GetEnemy(0);
	Setup(target, 200, 0, 1, 1, 1, 1);
	auto& w1 = lcf::Data::items[0];
	w1.type = lcf::rpg::Item::Type_weapon;
	w1.hit = 100;
	w1.state_set = { false, true };
	w1.state_chance = 100;

	auto& state = lcf::Data::states[1];
	state.b_rate = 100;

	source->SetEquipment(1, 1);
	REQUIRE_EQ(&w1, source->GetWeapon());

	Game_BattleAlgorithm::Normal algo(source, target);

	algo.Start();

	auto good = [&](auto effect) {
		algo.Execute();

		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(1, algo.GetStateEffects().size());
		REQUIRE_EQ(2, algo.GetStateEffects()[0].state_id);
		REQUIRE_EQ(effect, algo.GetStateEffects()[0].effect);
	};

	auto none = [&]() {
		algo.Execute();

		REQUIRE_EQ(true, algo.IsSuccess());
		REQUIRE_EQ(0, algo.GetStateEffects().size());
	};

	SUBCASE("inflict") {
		SUBCASE("have") {
			target->AddState(2, true);
			none();
		}
		SUBCASE("havenot") {
			good(Game_BattleAlgorithm::StateEffect::Inflicted);
		}
	}

	SUBCASE("reverse") {
		w1.reverse_state_effect = true;
		SUBCASE("have") {
			target->AddState(2, true);
			good(Game_BattleAlgorithm::StateEffect::Healed);
		}
		SUBCASE("havenot") {
			none();
		}
	}
}

TEST_CASE("Algo::Normal::Charged") {
	const MockBattle mb;
	auto* source = Main_Data::game_enemyparty->GetEnemy(0);
	auto* target = Main_Data::game_party->GetActor(0);

	source->SetCharged(true);

	Game_BattleAlgorithm::Normal algo(source, target);

	REQUIRE_EQ(true, source->IsCharged());
	algo.Start();

	REQUIRE_EQ(false, source->IsCharged());
	REQUIRE_EQ(true, algo.IsChargedAttack());
}

TEST_CASE("Algo::None::Charged") {
	const MockBattle mb;
	auto* source = Main_Data::game_enemyparty->GetEnemy(0);
	source->SetCharged(true);

	Game_BattleAlgorithm::None algo(source);

	REQUIRE_EQ(true, source->IsCharged());
	algo.Start();
	REQUIRE_EQ(false, source->IsCharged());
}


TEST_SUITE_END();
