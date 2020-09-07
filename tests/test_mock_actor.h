#ifndef EP_TEST_MOCK_ACTOR
#define EP_TEST_MOCK_ACTOR

#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "main_data.h"
#include "player.h"
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
	}

	for (auto& tp: lcf::Data::troops) {
		initVec(tp.members, 8);
	}

}

class MockActor {
public:
	MockActor(int eng = Player::EngineRpg2k3 | Player::EngineEnglish)
	{
		_engine = Player::engine;
		Player::engine = eng;

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
	}
private:
	int _engine = {};
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
	item.critical_hit = 0;
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

#endif
