#include "test_mock_actor.h"
#include "enemyai.h"
#include "rand.h"
#include "algo.h"
#include "game_battlealgorithm.h"
#include "game_switches.h"
#include "main_data.h"
#include "doctest.h"

#include <iostream>

static Game_Enemy MakeEnemy(int id) {
	MakeDBEnemy(id, 100, 100, 1, 1, 1, 1);
	auto& tp = lcf::Data::troops[0];
	tp.members.resize(8);
	tp.members[id - 1].enemy_id = id;
	Main_Data::game_enemyparty->ResetBattle(1);
	auto& enemy = (*Main_Data::game_enemyparty)[id - 1];
	return std::move(enemy);
}

decltype(auto) MakeActor(int id) {
	auto actor = Game_Actor(id);
	return actor;
}

lcf::rpg::Skill* MakeSkill(int id, int type, int scope) {
	auto* s = MakeDBSkill(id, 90, 0, 0, 0, 0);
	s->type = type;
	s->scope = scope;
	return s;
}

TEST_SUITE_BEGIN("EnemyAi");

static void testIrregularSkill(bool expect, int type, const Game_Battler& target, bool bugs) {
	REQUIRE_EQ(expect, EnemyAi::IsSkillEffectiveOn(*MakeSkill(1, type, lcf::rpg::Skill::Scope_enemy), target, bugs));
	REQUIRE_EQ(expect, EnemyAi::IsSkillEffectiveOn(*MakeSkill(1, type, lcf::rpg::Skill::Scope_enemies), target, bugs));
	REQUIRE_EQ(expect, EnemyAi::IsSkillEffectiveOn(*MakeSkill(1, type, lcf::rpg::Skill::Scope_self), target, bugs));
	REQUIRE_EQ(expect, EnemyAi::IsSkillEffectiveOn(*MakeSkill(1, type, lcf::rpg::Skill::Scope_ally), target, bugs));
	REQUIRE_EQ(expect, EnemyAi::IsSkillEffectiveOn(*MakeSkill(1, type, lcf::rpg::Skill::Scope_party), target, bugs));
}

static void testNormalSkill(bool expect_enemy, bool expect_ally, lcf::rpg::Skill& skill, const Game_Battler& target, bool bugs) {
	CAPTURE(bugs);
	CAPTURE(target.IsDead());
	CAPTURE(target.IsHidden());
	CAPTURE(target.HasState(2));
	CAPTURE(skill.scope);

	skill.scope = lcf::rpg::Skill::Scope_enemy;
	REQUIRE_EQ(expect_enemy, EnemyAi::IsSkillEffectiveOn(skill, target, bugs));

	skill.scope = lcf::rpg::Skill::Scope_enemies;
	REQUIRE_EQ(expect_enemy, EnemyAi::IsSkillEffectiveOn(skill, target, bugs));

	skill.scope = lcf::rpg::Skill::Scope_self;
	REQUIRE_EQ(expect_ally, EnemyAi::IsSkillEffectiveOn(skill, target, bugs));

	skill.scope = lcf::rpg::Skill::Scope_ally;
	REQUIRE_EQ(expect_ally, EnemyAi::IsSkillEffectiveOn(skill, target, bugs));

	skill.scope = lcf::rpg::Skill::Scope_party;
	REQUIRE_EQ(expect_ally, EnemyAi::IsSkillEffectiveOn(skill, target, bugs));
}

TEST_CASE("IsSkillEffectiveOn") {
	const MockActor m;

	std::vector<Game_Enemy> targets;
	targets.push_back(MakeEnemy(1));
	targets.push_back(MakeEnemy(2));
	targets.back().Kill();
	targets.push_back(MakeEnemy(3));
	targets.back().SetHidden(true);
	targets.push_back(MakeEnemy(4));
	targets.back().Kill();
	targets.back().SetHidden(true);
	targets.push_back(MakeEnemy(5));
	targets.back().AddState(2, true);

	SUBCASE("Switch") {
		for (auto& t: targets) {
			testIrregularSkill(true, lcf::rpg::Skill::Type_switch, t, true);
			testIrregularSkill(true, lcf::rpg::Skill::Type_switch, t, false);
		}
	}

	SUBCASE("Escape") {
		for (auto& t: targets) {
			testIrregularSkill(false, lcf::rpg::Skill::Type_escape, t, true);
			testIrregularSkill(false, lcf::rpg::Skill::Type_escape, t, false);
		}
	}

	SUBCASE("Teleport") {
		for (auto& t: targets) {
			testIrregularSkill(false, lcf::rpg::Skill::Type_teleport, t, true);
			testIrregularSkill(false, lcf::rpg::Skill::Type_teleport, t, false);
		}
	}

	SUBCASE("Normal") {
		auto* skill = MakeSkill(1, lcf::rpg::Skill::Type_normal, lcf::rpg::Skill::Scope_self);

		SUBCASE("None") {
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), false, *skill, t, true);
				testNormalSkill(t.Exists(), false, *skill, t, false);
			}
		}

		SUBCASE("Hp") {
			skill->affect_hp = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Sp") {
			skill->affect_sp = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Atk") {
			skill->affect_attack = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Def") {
			skill->affect_defense = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Spi") {
			skill->affect_spirit = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Agi") {
			skill->affect_agility = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Death") {
			skill->state_effects = { true };
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), !t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.IsDead(), *skill, t, false);
			}
		}

		SUBCASE("ReverseDeath") {
			skill->state_effects = { true };
			skill->reverse_state_effect = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), !t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), !t.IsDead() && t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("State") {
			skill->state_effects = { false, true };
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists() && t.HasState(2), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists() && t.HasState(2), *skill, t, false);
			}
		}

		SUBCASE("ReverseState") {
			skill->state_effects = { false, true };
			skill->reverse_state_effect = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists() && t.HasState(2), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}

		SUBCASE("Attribute") {
			skill->attribute_effects = { true };
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), false, *skill, t, true);
				testNormalSkill(t.Exists(), false, *skill, t, false);
			}
		}

		SUBCASE("AttributeEffect") {
			skill->attribute_effects = { true };
			skill->affect_attr_defence = true;
			for (auto& t: targets) {
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, true);
				testNormalSkill(t.Exists(), t.Exists(), *skill, t, false);
			}
		}
	}
}

TEST_CASE("NoActions") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	EnemyAi::SelectEnemyAiActionRpgRtCompat(enemy, true);
	REQUIRE_EQ(enemy.GetBattleAlgorithm(), nullptr);
}

namespace {
struct ActionParams {
	int rating;
	int kind;
	int basic;
};
}

static std::vector<lcf::rpg::EnemyAction> MakeActions(std::initializer_list<ActionParams> ilist) {
	std::vector<lcf::rpg::EnemyAction> actions;
	for (auto& il: ilist) {
		actions.push_back({});
		actions.back().rating = il.rating;
		actions.back().kind = il.kind;
		actions.back().basic = il.basic;
	}
	return actions;
}

static void testActionType(int start, int end, Game_BattleAlgorithm::Type type_compat, Game_BattleAlgorithm::Type type_improved, Game_Enemy& source) {
	for (int rng = start; rng < end; ++rng) {
		CAPTURE(rng);
		bool bugs = true;
		CAPTURE(bugs);

		Rand::LockGuard lk(rng);
		bugs = true;
		source.SetBattleAlgorithm(nullptr);
		EnemyAi::SelectEnemyAiActionRpgRtCompat(source, bugs);
		REQUIRE(source.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(type_compat), static_cast<int>(source.GetBattleAlgorithm()->GetType()));

		bugs = false;
		source.SetBattleAlgorithm(nullptr);
		EnemyAi::SelectEnemyAiActionRpgRtCompat(source, bugs);
		REQUIRE(source.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(type_improved), static_cast<int>(source.GetBattleAlgorithm()->GetType()));
	}
}

static void testActionNullptr(int start, int end, Game_Enemy& source) {
	for (int rng = start; rng < end; ++rng) {
		CAPTURE(rng);
		bool bugs = true;
		CAPTURE(bugs);

		Rand::LockGuard lk(rng);
		bugs = true;
		source.SetBattleAlgorithm(nullptr);
		EnemyAi::SelectEnemyAiActionRpgRtCompat(source, bugs);
		REQUIRE_FALSE(source.GetBattleAlgorithm());

		bugs = false;
		source.SetBattleAlgorithm(nullptr);
		EnemyAi::SelectEnemyAiActionRpgRtCompat(source, bugs);
		REQUIRE_FALSE(source.GetBattleAlgorithm());
	}
}

TEST_CASE("Ratings") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	SUBCASE("40/50") {
		actions = MakeActions({{40, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("50/40") {
		actions = MakeActions({{50, 0, lcf::rpg::EnemyAction::Basic_attack }, { 40, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("41/50") {
		actions = MakeActions({{41, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 1, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
		testActionType(1, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("45/50") {
		actions = MakeActions({{45, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 5, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
		testActionType(5, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("49/50") {
		actions = MakeActions({{49, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 9, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
		testActionType(9, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("50/50") {
		actions = MakeActions({{50, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
		testActionType(0, 10, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
		testActionType(10, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("50/30/50") {
		actions = MakeActions({{50, 0, lcf::rpg::EnemyAction::Basic_attack }, { 30, 0, lcf::rpg::EnemyAction::Basic_defense }, { 50, 0, lcf::rpg::EnemyAction::Basic_observe }});
		testActionType(0, 10, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
		testActionType(10, 100, Game_BattleAlgorithm::Type::Observe, Game_BattleAlgorithm::Type::Observe, enemy);
	}
}

TEST_CASE("SkillEffectiveButNotUsable") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;
	auto* skill = MakeSkill(1, lcf::rpg::Skill::Type_normal, lcf::rpg::Skill::Scope_self);
	skill->affect_hp = true;
	skill->sp_cost = 10;

	actions = MakeActions({{50, lcf::rpg::EnemyAction::Kind_skill, 0}, { 50, 0, lcf::rpg::EnemyAction::Basic_attack }});
	actions.front().skill_id = 1;

	REQUIRE(EnemyAi::IsSkillEffectiveOn(*skill, enemy, true));

	SUBCASE("HasSp") {
		enemy.SetSp(100);
		REQUIRE_EQ(100, enemy.GetSp());
		REQUIRE(enemy.IsSkillUsable(1));
		testActionType(0, 10, Game_BattleAlgorithm::Type::Skill, Game_BattleAlgorithm::Type::Skill, enemy);
		testActionType(10, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("NoSp") {
		enemy.SetSp(0);
		REQUIRE_FALSE(enemy.IsSkillUsable(1));
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}
}

TEST_CASE("SkillUsableButNotEffective") {
	const MockActor m;

	auto enemy = MakeEnemy(1);

	auto& actions = lcf::Data::enemies[0].actions;
	auto* skill = MakeSkill(1, lcf::rpg::Skill::Type_normal, lcf::rpg::Skill::Scope_self);
	skill->sp_cost = 0;
	skill->state_effects = { false, true };
	lcf::Data::states[1].type = lcf::rpg::State::Persistence_persists;

	REQUIRE(enemy.IsSkillUsable(1));
	REQUIRE_FALSE(EnemyAi::IsSkillEffectiveOn(*skill, enemy, true));

	SUBCASE("50/50") {
		actions = MakeActions({{50, lcf::rpg::EnemyAction::Kind_skill, 0}, { 50, 0, lcf::rpg::EnemyAction::Basic_attack }});
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("70/50") {
		// If the only valid action is a not effective skill, it's still chosen, but then rejected.
		actions = MakeActions({{70, lcf::rpg::EnemyAction::Kind_skill, 0}, { 50, 0, lcf::rpg::EnemyAction::Basic_attack }});
		testActionNullptr(0, 100, enemy);
	}
}

TEST_CASE("ConditionSwitch") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_switch;
	actions.front().switch_id = 1;

	SUBCASE("SwOff") {
		Main_Data::game_switches->Set(1, false);
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("SwOn") {
		Main_Data::game_switches->Set(1, true);
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}
}

TEST_CASE("ConditionTurns") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_turn;

	Main_Data::game_party->ResetTurns();

	SUBCASE("0/0") {
		actions.front().condition_param2 = 0;
		actions.front().condition_param1 = 0;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("1/1") {
		actions.front().condition_param2 = 1;
		actions.front().condition_param1 = 1;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("MonsterParty") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_actors;

	std::vector<Game_Battler*> battlers;
	Main_Data::game_enemyparty->GetActiveBattlers(battlers);
	REQUIRE_EQ(8, battlers.size());

	SUBCASE("1/1") {
		actions.front().condition_param1 = 1;
		actions.front().condition_param2 = 1;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("1/8") {
		actions.front().condition_param1 = 1;
		actions.front().condition_param2 = 8;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("1/7") {
		actions.front().condition_param1 = 1;
		actions.front().condition_param2 = 7;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("9/10") {
		actions.front().condition_param1 = 9;
		actions.front().condition_param2 = 10;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("ConditionHp") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_hp;

	REQUIRE_EQ(100, enemy.GetMaxHp());
	enemy.SetHp(50);
	REQUIRE_EQ(50, enemy.GetHp());

	SUBCASE("0/100") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("50/50") {
		actions.front().condition_param1 = 50;
		actions.front().condition_param2 = 50;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("51/100") {
		actions.front().condition_param1 = 51;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("0/49") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 49;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("ConditionSp") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_sp;

	REQUIRE_EQ(100, enemy.GetMaxSp());
	enemy.SetSp(50);
	REQUIRE_EQ(50, enemy.GetSp());

	SUBCASE("0/100") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("50/50") {
		actions.front().condition_param1 = 50;
		actions.front().condition_param2 = 50;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("51/100") {
		actions.front().condition_param1 = 51;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("0/49") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 49;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("ConditionLevel") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_party_lvl;

	Main_Data::game_party->AddActor(1);
	Main_Data::game_actors->GetActor(1)->SetLevel(20);
	Main_Data::game_party->AddActor(2);
	Main_Data::game_actors->GetActor(2)->SetLevel(10);

	REQUIRE_EQ(15, Main_Data::game_party->GetAverageLevel());

	SUBCASE("0/50") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 50;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("15/15") {
		actions.front().condition_param1 = 15;
		actions.front().condition_param2 = 15;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("16/50") {
		actions.front().condition_param1 = 16;
		actions.front().condition_param2 = 50;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("0/14") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 14;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("ConditionFatigue") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	auto& actions = lcf::Data::enemies[0].actions;

	actions = MakeActions({{90, 0, lcf::rpg::EnemyAction::Basic_attack }, { 50, 0, lcf::rpg::EnemyAction::Basic_defense }});
	actions.front().condition_type = lcf::rpg::EnemyAction::ConditionType_party_fatigue;

	Main_Data::game_party->AddActor(1);
	Main_Data::game_actors->GetActor(1)->SetBaseMaxHp(100);
	Main_Data::game_actors->GetActor(1)->SetHp(50);
	Main_Data::game_actors->GetActor(1)->SetBaseMaxSp(0);
	Main_Data::game_actors->GetActor(1)->SetSp(0);
	Main_Data::game_party->AddActor(2);
	Main_Data::game_actors->GetActor(2)->SetBaseMaxHp(100);
	Main_Data::game_actors->GetActor(2)->SetHp(80);
	Main_Data::game_actors->GetActor(2)->SetBaseMaxSp(0);
	Main_Data::game_actors->GetActor(2)->SetSp(0);

	REQUIRE_EQ(57, Main_Data::game_party->GetFatigue());

	SUBCASE("0/100") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("57/57") {
		actions.front().condition_param1 = 57;
		actions.front().condition_param2 = 57;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Normal, Game_BattleAlgorithm::Type::Normal, enemy);
	}

	SUBCASE("58/100") {
		actions.front().condition_param1 = 58;
		actions.front().condition_param2 = 100;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}

	SUBCASE("0/56") {
		actions.front().condition_param1 = 0;
		actions.front().condition_param2 = 56;
		testActionType(0, 100, Game_BattleAlgorithm::Type::Defend, Game_BattleAlgorithm::Type::Defend, enemy);
	}
}

TEST_CASE("SetStateRestrictedAction") {
	const MockActor m;

	auto enemy = MakeEnemy(1);
	Main_Data::game_party->AddActor(1);
	lcf::Data::states[1].restriction = lcf::rpg::State::Restriction_do_nothing;
	lcf::Data::states[2].restriction = lcf::rpg::State::Restriction_attack_ally;
	lcf::Data::states[3].restriction = lcf::rpg::State::Restriction_attack_enemy;

	REQUIRE_EQ(nullptr, enemy.GetBattleAlgorithm());

	SUBCASE("None") {
		REQUIRE_FALSE(EnemyAi::SetStateRestrictedAction(enemy));
		REQUIRE_FALSE(enemy.GetBattleAlgorithm());
	}

	SUBCASE("Charged") {
		enemy.SetCharged(true);
		REQUIRE(EnemyAi::SetStateRestrictedAction(enemy));
		REQUIRE(enemy.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(Game_BattleAlgorithm::Type::Normal), static_cast<int>(enemy.GetBattleAlgorithm()->GetType()));
	}

	SUBCASE("NoAct") {
		enemy.AddState(2, true);
		REQUIRE(EnemyAi::SetStateRestrictedAction(enemy));
		REQUIRE(enemy.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(Game_BattleAlgorithm::Type::None), static_cast<int>(enemy.GetBattleAlgorithm()->GetType()));
	}

	SUBCASE("Confuse") {
		enemy.AddState(3, true);
		REQUIRE(EnemyAi::SetStateRestrictedAction(enemy));
		REQUIRE(enemy.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(Game_BattleAlgorithm::Type::Normal), static_cast<int>(enemy.GetBattleAlgorithm()->GetType()));
		enemy.GetBattleAlgorithm()->Start();
		REQUIRE_EQ(static_cast<int>(Game_Battler::Type_Enemy), static_cast<int>(enemy.GetBattleAlgorithm()->GetTarget()->GetType()));
	}

	SUBCASE("Provoke") {
		enemy.AddState(4, true);
		REQUIRE(EnemyAi::SetStateRestrictedAction(enemy));
		REQUIRE(enemy.GetBattleAlgorithm());
		REQUIRE_EQ(static_cast<int>(Game_BattleAlgorithm::Type::Normal), static_cast<int>(enemy.GetBattleAlgorithm()->GetType()));
		enemy.GetBattleAlgorithm()->Start();
		REQUIRE_EQ(static_cast<int>(Game_Battler::Type_Ally), static_cast<int>(enemy.GetBattleAlgorithm()->GetTarget()->GetType()));
	}
}

TEST_SUITE_END();
