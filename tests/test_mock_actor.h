#ifndef EP_TEST_MOCK_ACTOR
#define EP_TEST_MOCK_ACTOR

#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_switches.h"
#include "game_player.h"
#include "game_battle.h"
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
	initVec(lcf::Data::animations, 200);

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
		enemy.state_ranks.resize(lcf::Data::states.size(), 1);
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
		_engine = Player::game_config.engine;
		Player::game_config.engine = eng;

		InitEmptyDB();

		Main_Data::Cleanup();

		Main_Data::game_system = std::make_unique<Game_System>();
		Main_Data::game_actors = std::make_unique<Game_Actors>();
		Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
		Main_Data::game_party = std::make_unique<Game_Party>();
		Main_Data::game_switches = std::make_unique<Game_Switches>();
		Main_Data::game_variables = std::make_unique<Game_Variables>(Game_Variables::min_2k, Game_Variables::max_2k);
		Main_Data::game_player = std::make_unique<Game_Player>();

		Main_Data::game_party->SetupNewGame();
	}

	~MockActor() {
		Main_Data::Cleanup();

		lcf::Data::data = {};
		Player::game_config.engine = _engine;
	}
private:
	int _engine = {};
};

struct MockBattle : public MockActor {
	MockBattle(int party_size = 4, int troop_size = 4, int eng = Player::EngineRpg2k3 | Player::EngineEnglish) : MockActor(eng)
	{
		Main_Data::game_party->Clear();
		for (int i = 0; i < party_size; ++i) {
			Main_Data::game_party->AddActor(i + 1);
		}

		auto& tp = lcf::Data::troops[0];
		tp.members.resize(troop_size);
		for (int i = 0; i < troop_size; ++i) {
			tp.members[i].enemy_id = i + 1;
		}
		Main_Data::game_enemyparty->ResetBattle(1);
		Game_Battle::battle_running = true;
	}

	~MockBattle() {
		Game_Battle::battle_running = false;
	}
};

inline lcf::rpg::Actor* MakeDBActor(int id, int level = 1, int final_level = 50,
		int hp = 1, int sp = 0, int atk = 0, int def = 0, int spi = 0, int agi = 0,
		bool two_weapon = false, bool lock_equip = false, bool autobattle = false, bool super_guard = false)
{
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

inline lcf::rpg::Enemy* MakeDBEnemy(int id,
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

inline lcf::rpg::Item* MakeDBEquip(int id, int type,
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

inline lcf::rpg::Skill* MakeDBSkill(int id, int hit, int power, int phys, int mag, int var)
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

inline lcf::rpg::Attribute* MakeDBAttribute(int id, int type, int a, int b, int c, int d, int e)
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

inline void SetDBActorAttribute(int id, int attr_id, int rank) {
	auto& actor = lcf::Data::actors[id - 1];
	actor.attribute_ranks[attr_id - 1] = rank;
}

inline void SetDBEnemyAttribute(int id, int attr_id, int rank) {
	auto& enemy = lcf::Data::enemies[id - 1];
	enemy.attribute_ranks[attr_id - 1] = rank;
}

inline void SetDBItemAttribute(int id, int attr_id, bool value) {
	auto& item = lcf::Data::items[id - 1];
	item.attribute_set[attr_id - 1] = value;
}

inline void SetDBSkillAttribute(int id, int attr_id, bool value) {
	auto& skill = lcf::Data::skills[id - 1];
	skill.attribute_effects[attr_id - 1] = value;
}

inline void Setup(Game_Actor* actor, int hp, int sp, int atk, int def, int spi, int agi) {
	actor->SetBaseMaxHp(hp);
	actor->SetHp(hp);
	actor->SetBaseMaxSp(sp);
	actor->SetSp(sp);
	actor->SetBaseAtk(atk);
	actor->SetBaseDef(def);
	actor->SetBaseSpi(spi);
	actor->SetBaseAgi(agi);
}

inline void Setup(Game_Enemy* enemy, int hp, int sp, int atk, int def, int spi, int agi) {
	auto& db = lcf::Data::enemies[enemy->GetId() - 1];
	db.max_hp = hp;
	db.max_sp = sp;
	db.attack = atk;
	db.defense = def;
	db.spirit = spi;
	db.agility = agi;

	enemy->SetHp(hp);
	enemy->SetSp(sp);
}



#endif
