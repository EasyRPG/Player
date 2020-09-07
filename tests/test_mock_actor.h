#ifndef EP_TEST_MOCK_ACTOR
#define EP_TEST_MOCK_ACTOR

#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "main_data.h"
#include "player.h"
#include "output.h"
#include <lcf/data.h>

static void InitEmptyDB() {
	auto initVec = [](auto& v, int size) {
		v.resize(size);
		for (int i = 0; i < size; ++i) {
			v[i].ID = i + 1;
		}
	};
	lcf::Data::data = {};

	initVec(lcf::Data::actors, 20);
	initVec(lcf::Data::skills, 200);
	initVec(lcf::Data::items, 200);
	initVec(lcf::Data::enemies, 20);
	initVec(lcf::Data::troops, 20);
	initVec(lcf::Data::terrains, 20);
	initVec(lcf::Data::attributes, 20);
	initVec(lcf::Data::states, 20);
	initVec(lcf::Data::chipsets, 20);
	initVec(lcf::Data::commonevents, 20);
	initVec(lcf::Data::battlecommands.commands, 20);
	initVec(lcf::Data::classes, 100);
	initVec(lcf::Data::battleranimations, 200);
	initVec(lcf::Data::switches, 200);
	initVec(lcf::Data::variables, 200);

	for (auto& actor: lcf::Data::actors) {
		actor.initial_level = 1;
		actor.final_level = 99;
		actor.parameters.Setup(actor.final_level);

		actor.state_ranks.resize(lcf::Data::states.size(), 2);
		actor.attribute_ranks.resize(lcf::Data::attributes.size(), 2);
	}

	for (auto& item: lcf::Data::items) {
		item.attribute_set = lcf::DBBitArray(lcf::Data::attributes.size());
		item.state_set = lcf::DBBitArray(lcf::Data::states.size());
	}

	for (auto& skill: lcf::Data::skills) {
		skill.attribute_effects = lcf::DBBitArray(lcf::Data::attributes.size());
		skill.state_effects = lcf::DBBitArray(lcf::Data::states.size());
	}

	for (auto& enemy: lcf::Data::enemies) {
		enemy.state_ranks.resize(lcf::Data::states.size(), 2);
		enemy.attribute_ranks.resize(lcf::Data::attributes.size(), 2);
	}

	for (auto& tp: lcf::Data::troops) {
		initVec(tp.members, 8);
	}

	auto& death = lcf::Data::states[0];
	death.priority = 100;
	death.restriction = lcf::rpg::State::Restriction_do_nothing;

}

class MockActor {
public:
	MockActor(int eng = Player::EngineRpg2k3 | Player::EngineEnglish)
	{
		_engine = Player::engine;
		Player::engine = eng;

		_ll = Output::GetLogLevel();
		Output::SetLogLevel(LogLevel::Error);

		InitEmptyDB();

		Main_Data::Cleanup();
		Main_Data::game_data.Setup();

		Main_Data::game_actors = std::make_unique<Game_Actors>();
		Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
		Main_Data::game_party = std::make_unique<Game_Party>();

		Main_Data::game_party->SetupNewGame();
	}

	~MockActor() {
		Main_Data::Cleanup();

		lcf::Data::data = {};
		Player::engine = _engine;
		Output::SetLogLevel(_ll);
	}
private:
	int _engine = {};
	LogLevel _ll = {};
};

static lcf::rpg::Actor* MakeDBActor(int id, int level, int final_level,
		int hp, int sp, int atk, int def, int spi, int agi,
		bool two_weapon = false, bool lock_equip = false, bool autobattle = false, bool super_guard = false)
{
	if (lcf::Data::actors.size() <= id) {
		lcf::Data::actors.resize(id + 1);
	}

	auto& actor = lcf::Data::actors[id - 1];
	actor.initial_level = 1;
	actor.final_level = final_level;
	actor.parameters.Setup(actor.final_level);
	actor.parameters.maxhp[level - 1] = hp;
	actor.parameters.maxsp[level - 1] = sp;
	actor.parameters.attack[level - 1] = atk;
	actor.parameters.defense[level - 1] = def;
	actor.parameters.spirit[level - 1] = spi;
	actor.parameters.agility[level - 1] = agi;
	actor.two_weapon = two_weapon;
	actor.lock_equipment = lock_equip;
	actor.auto_battle = autobattle;
	actor.super_guard = super_guard;

	actor.class_id = 0;
	actor.exp_base = 1;
	actor.exp_inflation = 677;
	actor.exp_correction = 40;
	return &actor;
}

static lcf::rpg::Enemy* MakeDBEnemy(int id,
		int hp, int sp, int atk, int def, int spi, int agi)
{
	auto& enemy = lcf::Data::enemies[id - 1];
	enemy.max_hp = hp;
	enemy.max_sp = sp;
	enemy.attack = atk;
	enemy.defense = def;
	enemy.spirit = spi;
	enemy.agility = agi;
	return &enemy;
}

static lcf::rpg::Item* MakeDBEquip(int id, int type,
		int atk = 1, int def = 1, int spi = 1, int agi = 1,
		int hit=100, int crt=0,
		bool w1 = false, bool w2 = false, bool w3 = false, bool w4 = false,
		bool a1 = false, bool a2 = false, bool a3 = false, bool a4 = false)
{
	auto& item = lcf::Data::items[id - 1];
	item.type = type;
	item.atk_points1 = atk;
	item.def_points1 = def;
	item.spi_points1 = spi;
	item.agi_points1 = agi;
	item.hit = hit;
	item.critical_hit = crt;
	item.preemptive = w1;
	item.dual_attack = w2;
	item.attack_all = w3;
	item.ignore_evasion = w4;
	item.prevent_critical = a1;
	item.raise_evasion = a2;
	item.half_sp_cost = a3;
	item.no_terrain_damage = a4;
	return &item;
}

static lcf::rpg::Skill* MakeDBSkill(int id, int hit, int power, int phys, int mag, int var)
{
	auto& skill = lcf::Data::skills[id - 1];
	skill.type = lcf::rpg::Skill::Type_normal;
	skill.hit = hit;
	skill.power = power;
	skill.physical_rate = phys;
	skill.magical_rate = mag;
	skill.variance = var;
	return &skill;
}

static lcf::rpg::Attribute* MakeDBAttribute(int id, int type, int a, int b, int c, int d, int e)
{
	auto& attr = lcf::Data::attributes[id - 1];
	attr.type = type;
	attr.a_rate = a;
	attr.b_rate = b;
	attr.c_rate = c;
	attr.d_rate = d;
	attr.e_rate = e;
	return &attr;
}

static void SetDBActorAttribute(int id, int attr_id, int rank) {
	auto& actor = lcf::Data::actors[id - 1];
	actor.attribute_ranks[attr_id - 1] = rank;
}

static void SetDBEnemyAttribute(int id, int attr_id, int rank) {
	auto& enemy = lcf::Data::enemies[id - 1];
	enemy.attribute_ranks[attr_id - 1] = rank;
}

static void SetDBItemAttribute(int id, int attr_id, bool value) {
	auto& item = lcf::Data::items[id - 1];
	item.attribute_set[attr_id - 1] = value;
}

static void SetDBSkillAttribute(int id, int attr_id, bool value) {
	auto& skill = lcf::Data::skills[id - 1];
	skill.attribute_effects[attr_id - 1] = value;
}

#endif
