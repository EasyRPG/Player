#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "main_data.h"
#include "player.h"
#include <lcf/data.h>

#include "doctest.h"


lcf::rpg::Item* MakeEquip(int id, int type,
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

class MockActor {
public:
	MockActor(int eng = Player::EngineRpg2k3 | Player::EngineEnglish)
	{
		_engine = Player::engine;
		Player::engine = eng;

		lcf::Data::actors.resize(20);
		for (size_t i = 0; i < lcf::Data::actors.size(); ++i) {
			auto& actor = lcf::Data::actors[i];
			actor.ID = i + 1;
			actor.parameters.Setup(99);
			for (int i = 0; i < 99; ++i) {
				const auto lvl = i + 1;
				actor.parameters.maxhp[i] = lvl * 100;
				actor.parameters.maxsp[i]= lvl * 10;
				actor.parameters.attack[i] = lvl * 10 + 1;
				actor.parameters.defense[i] = lvl * 10 + 2;
				actor.parameters.spirit[i] = lvl * 10 + 3;
				actor.parameters.agility[i] = lvl * 10 + 4;
			}
			actor.initial_level = 1;
			actor.final_level = 99;
			actor.class_id = 0;
			actor.exp_base = 1;
			actor.exp_inflation = 677;
			actor.exp_correction = 40;

			actor.two_weapon = (i % 5) == 1;
			actor.lock_equipment = (i % 5) == 2;
			actor.auto_battle = (i % 5) == 3;
			actor.super_guard = (i % 5) == 4;
		}

		lcf::Data::items.resize(200);
		for (size_t i = 0; i < lcf::Data::items.size(); ++i) {
			lcf::Data::items[i].ID = i + 1;
		}

		Main_Data::Cleanup();
		Main_Data::game_data.Setup();

		Main_Data::game_actors = std::make_unique<Game_Actors>();
		Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
		Main_Data::game_party = std::make_unique<Game_Party>();

		Main_Data::game_party->SetupNewGame();
	}

	~MockActor() {
		Main_Data::Cleanup();

		lcf::Data::actors = {};
		lcf::Data::items = {};
		Player::engine = _engine;
	}
private:
	int _engine = {};
};

TEST_SUITE_BEGIN("Game_Actor");

TEST_CASE("Limits2k") {
	const MockActor m(Player::EngineRpg2k);

	auto* actor = Main_Data::game_actors->GetActor(1);

	REQUIRE(actor != nullptr);
	REQUIRE_EQ(actor->MaxHpValue(), 999);
	REQUIRE_EQ(actor->MaxStatBaseValue(), 999);
	// FIXME: Verify Clamps
	REQUIRE_EQ(actor->MaxStatBattleValue(), 999);
}

TEST_CASE("Limits2k3") {
	const MockActor m(Player::EngineRpg2k3);

	auto* actor = Main_Data::game_actors->GetActor(1);

	REQUIRE(actor != nullptr);
	REQUIRE_EQ(actor->MaxHpValue(), 9999);
	REQUIRE_EQ(actor->MaxStatBaseValue(), 999);
	// FIXME: Verify Clamps
	REQUIRE_EQ(actor->MaxStatBattleValue(), 9999);
}


TEST_CASE("Base") {
	const MockActor m;

	auto* actor = Main_Data::game_actors->GetActor(1);

	REQUIRE(actor != nullptr);
	REQUIRE_EQ(actor->GetType(), Game_Battler::Type_Ally);
	REQUIRE_EQ(actor->GetId(), 1);

	REQUIRE_EQ(actor->GetLevel(), 1);
	REQUIRE_EQ(actor->GetMaxLevel(), 99);

	REQUIRE_EQ(actor->GetExp(), 0);
	REQUIRE_EQ(actor->GetBaseExp(), 0);
	REQUIRE_EQ(actor->GetNextExp(), 718);

	REQUIRE_EQ(actor->GetClass(), nullptr);

	REQUIRE_EQ(actor->GetBaseMaxHp(), 100);
	REQUIRE_EQ(actor->GetBaseMaxSp(), 10);
	REQUIRE_EQ(actor->GetMaxHp(), 100);
	REQUIRE_EQ(actor->GetMaxSp(), 10);

	REQUIRE_EQ(actor->GetHp(), 100);
	REQUIRE_EQ(actor->GetSp(), 10);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor->GetBaseAtk(w), 11);
		REQUIRE_EQ(actor->GetBaseDef(w), 12);
		REQUIRE_EQ(actor->GetBaseSpi(w), 13);
		REQUIRE_EQ(actor->GetBaseAgi(w), 14);

		REQUIRE_EQ(actor->GetAtk(w), 11);
		REQUIRE_EQ(actor->GetDef(w), 12);
		REQUIRE_EQ(actor->GetSpi(w), 13);
		REQUIRE_EQ(actor->GetAgi(w), 14);

		REQUIRE_FALSE(actor->HasPreemptiveAttack(w));
		REQUIRE_FALSE(actor->HasDualAttack(w));
		REQUIRE_FALSE(actor->HasAttackAll(w));
		REQUIRE_FALSE(actor->AttackIgnoresEvasion(w));

		REQUIRE_EQ(actor->GetHitChance(w), 90);
		REQUIRE(actor->GetCriticalHitChance(w) == doctest::Approx(0.03333f));
	}

	REQUIRE_FALSE(actor->PreventsTerrainDamage());
	REQUIRE_FALSE(actor->PreventsCritical());
	REQUIRE_FALSE(actor->HasPhysicalEvasionUp());
	REQUIRE_FALSE(actor->HasHalfSpCost());

	REQUIRE_EQ(actor->GetWeaponId(), 0);
	REQUIRE_EQ(actor->GetShieldId(), 0);
	REQUIRE_EQ(actor->GetArmorId(), 0);
	REQUIRE_EQ(actor->GetHelmetId(), 0);
	REQUIRE_EQ(actor->GetAccessoryId(), 0);

	REQUIRE_EQ(actor->GetWeapon(), nullptr);
	REQUIRE_EQ(actor->Get2ndWeapon(), nullptr);
	REQUIRE_EQ(actor->GetShield(), nullptr);
	REQUIRE_EQ(actor->GetArmor(), nullptr);
	REQUIRE_EQ(actor->GetHelmet(), nullptr);
	REQUIRE_EQ(actor->GetAccessory(), nullptr);

	REQUIRE_FALSE(actor->IsEquipmentFixed());
	REQUIRE_FALSE(actor->HasStrongDefense());
	REQUIRE_FALSE(actor->HasTwoWeapons());
	REQUIRE_FALSE(actor->GetAutoBattle());

	REQUIRE_EQ(actor->GetBattleRow(), Game_Actor::RowType::RowType_front);
}

TEST_CASE("AdjParams") {
	const MockActor m;

	auto* actor = Main_Data::game_actors->GetActor(1);

	REQUIRE(actor != nullptr);

	actor->SetBaseMaxHp(501);
	actor->SetBaseMaxSp(502);
	actor->SetBaseAtk(503);
	actor->SetBaseDef(504);
	actor->SetBaseSpi(505);
	actor->SetBaseAgi(506);

	REQUIRE_EQ(actor->GetBaseMaxHp(), 501);
	REQUIRE_EQ(actor->GetBaseMaxSp(), 502);
	REQUIRE_EQ(actor->GetMaxHp(), 501);
	REQUIRE_EQ(actor->GetMaxSp(), 502);

	REQUIRE_EQ(actor->GetHp(), 100);
	REQUIRE_EQ(actor->GetSp(), 10);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor->GetBaseAtk(w), 503);
		REQUIRE_EQ(actor->GetBaseDef(w), 504);
		REQUIRE_EQ(actor->GetBaseSpi(w), 505);
		REQUIRE_EQ(actor->GetBaseAgi(w), 506);

		REQUIRE_EQ(actor->GetAtk(w), 503);
		REQUIRE_EQ(actor->GetDef(w), 504);
		REQUIRE_EQ(actor->GetSpi(w), 505);
		REQUIRE_EQ(actor->GetAgi(w), 506);
	}

	actor->SetAtkModifier(10);
	actor->SetDefModifier(20);
	actor->SetSpiModifier(30);
	actor->SetAgiModifier(40);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor->GetBaseAtk(w), 503);
		REQUIRE_EQ(actor->GetBaseDef(w), 504);
		REQUIRE_EQ(actor->GetBaseSpi(w), 505);
		REQUIRE_EQ(actor->GetBaseAgi(w), 506);

		REQUIRE_EQ(actor->GetAtk(w), 513);
		REQUIRE_EQ(actor->GetDef(w), 524);
		REQUIRE_EQ(actor->GetSpi(w), 535);
		REQUIRE_EQ(actor->GetAgi(w), 546);
	}
}

TEST_CASE("TryEquip") {
	const MockActor m;

	auto* actor = Main_Data::game_actors->GetActor(1);

	REQUIRE(actor != nullptr);

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 1, 11, 21, 31);
	MakeEquip(2, lcf::rpg::Item::Type_shield, 1, 11, 21, 31);
	MakeEquip(3, lcf::rpg::Item::Type_armor, 1, 11, 21, 31);
	MakeEquip(4, lcf::rpg::Item::Type_helmet, 1, 11, 21, 31);
	MakeEquip(5, lcf::rpg::Item::Type_accessory, 1, 11, 21, 31);

	for (int i = 1; i <= 5; ++i) {
		actor->SetEquipment(i, i);
		REQUIRE_EQ(actor->GetWeaponId(), 1);
		if (i >= 2) REQUIRE_EQ(actor->GetShieldId(), 2);
		if (i >= 3) REQUIRE_EQ(actor->GetArmorId(), 3);
		if (i >= 4) REQUIRE_EQ(actor->GetHelmetId(), 4);
		if (i >= 5) REQUIRE_EQ(actor->GetAccessoryId(), 5);

		REQUIRE_EQ(actor->GetBaseAtk(), 11 + i * 1);
		REQUIRE_EQ(actor->GetBaseDef(), 12 + i * 11);
		REQUIRE_EQ(actor->GetBaseSpi(), 13 + i * 21);
		REQUIRE_EQ(actor->GetBaseAgi(), 14 + i * 31);

		REQUIRE_EQ(actor->GetAtk(), actor->GetBaseAtk());
		REQUIRE_EQ(actor->GetDef(), actor->GetBaseDef());
		REQUIRE_EQ(actor->GetSpi(), actor->GetBaseSpi());
		REQUIRE_EQ(actor->GetAgi(), actor->GetBaseAgi());
	}
}

static void testWeapon3(const Game_Actor* a,
		const lcf::rpg::Item* i1,
		lcf::rpg::Item* i2,
		lcf::rpg::Item* i3,
		lcf::rpg::Item* i4,
		lcf::rpg::Item* i5,
		int wid)
{
	const auto isw1 = i1 && i1->type == lcf::rpg::Item::Type_weapon;
	const auto atk1 = i1 ? i1->atk_points1 : 0;
	const auto def1 = i1 ? i1->def_points1 : 0;
	const auto spi1 = i1 ? i1->spi_points1 : 0;
	const auto agi1 = i1 ? i1->agi_points1 : 0;

	const auto pre1 = i1 && isw1 ? i1->preemptive : false;
	const auto dul1 = i1 && isw1 ? i1->dual_attack : false;
	const auto all1 = i1 && isw1 ? i1->attack_all : false;
	const auto eva1 = i1 && isw1 ? i1->ignore_evasion : false;

	const auto hit1 = i1 && isw1 ? i1->hit : 0;
	const auto crt1 = i1 && isw1 ? i1->critical_hit : 0;

	const auto isw2 = i2 && i2->type == lcf::rpg::Item::Type_weapon;
	const auto atk2 = i2 ? i2->atk_points1 : 0;
	const auto def2 = i2 ? i2->def_points1 : 0;
	const auto spi2 = i2 ? i2->spi_points1 : 0;
	const auto agi2 = i2 ? i2->agi_points1 : 0;

	const auto pre2 = i2 && isw2 ? i2->preemptive : false;
	const auto dul2 = i2 && isw2 ? i2->dual_attack : false;
	const auto all2 = i2 && isw2 ? i2->attack_all : false;
	const auto eva2 = i2 && isw2 ? i2->ignore_evasion : false;

	const auto hit2 = i2 && isw2 && i2->type == lcf::rpg::Item::Type_weapon ? i2->hit : 0;
	const auto crt2 = i2 && isw2 && i2->type == lcf::rpg::Item::Type_weapon ? i2->critical_hit : 0;

	const auto atkA = (i3 ? i3->atk_points1 : 0) + (i4 ? i4->atk_points1 : 0) + (i5 ? i5->atk_points1 : 0);
	const auto defA = (i3 ? i3->def_points1 : 0) + (i4 ? i4->def_points1 : 0) + (i5 ? i5->def_points1 : 0);
	const auto spiA = (i3 ? i3->spi_points1 : 0) + (i4 ? i4->spi_points1 : 0) + (i5 ? i5->spi_points1 : 0);
	const auto agiA = (i3 ? i3->agi_points1 : 0) + (i4 ? i4->agi_points1 : 0) + (i5 ? i5->agi_points1 : 0);

	const auto pctA = (i2 && !isw2 && i2->prevent_critical) || (i3 && i3->prevent_critical) || (i4 && i4->prevent_critical) || (i5 && i5->prevent_critical);
	const auto revA = (i2 && !isw2 && i2->raise_evasion) || (i3 && i3->raise_evasion) || (i4 && i4->raise_evasion) || (i5 && i5->raise_evasion);
	const auto hspA = (i2 && !isw2 && i2->half_sp_cost) || (i3 && i3->half_sp_cost) || (i4 && i4->half_sp_cost) || (i5 && i5->half_sp_cost);
	const auto notA = (i2 && !isw2 && i2->no_terrain_damage) || (i3 && i3->no_terrain_damage) || (i4 && i4->no_terrain_damage) || (i5 && i5->no_terrain_damage);

	CAPTURE(wid);
	CAPTURE(i1);
	CAPTURE(i2);
	CAPTURE(isw1);
	CAPTURE(isw2);
	CAPTURE(atk1);
	CAPTURE(atk2);
	CAPTURE(hit1);
	CAPTURE(hit2);

	REQUIRE_EQ(a->GetBaseAtk(wid), 11 + atk1 + atk2 + atkA);
	REQUIRE_EQ(a->GetBaseDef(wid), 12 + def1 + def2 + defA);
	REQUIRE_EQ(a->GetBaseSpi(wid), 13 + spi1 + spi2 + spiA);
	REQUIRE_EQ(a->GetBaseAgi(wid), 14 + agi1 + agi2 + agiA);

	REQUIRE_EQ(a->HasPreemptiveAttack(wid), pre1 | pre2);
	REQUIRE_EQ(a->HasDualAttack(wid), dul1 | dul2);
	REQUIRE_EQ(a->HasAttackAll(wid), all1 | all2);
	REQUIRE_EQ(a->AttackIgnoresEvasion(wid), eva1 | eva2);

	const auto mhit = (isw1 || isw2) ? std::max(hit1, hit2) : 90;
	REQUIRE_EQ(a->GetHitChance(wid), mhit);
	const auto mcrt = 0.03333f + static_cast<float>(std::max(crt1, crt2)) / 100.0f;
	REQUIRE_EQ(a->GetCriticalHitChance(wid), doctest::Approx(mcrt));

	REQUIRE_EQ(a->PreventsCritical(), pctA);
	REQUIRE_EQ(a->HasPhysicalEvasionUp(), revA);
	REQUIRE_EQ(a->HasHalfSpCost(), hspA);
	REQUIRE_EQ(a->PreventsTerrainDamage(), notA);

	REQUIRE_EQ(a->GetBaseAtk(wid), a->GetAtk(wid));
	REQUIRE_EQ(a->GetBaseDef(wid), a->GetDef(wid));
	REQUIRE_EQ(a->GetBaseSpi(wid), a->GetSpi(wid));
	REQUIRE_EQ(a->GetBaseAgi(wid), a->GetAgi(wid));
}

static void testWeapon2(Game_Actor* a, int id1, int id2, int id3, int id4, int id5) {
	CAPTURE(a->GetId());
	CAPTURE(a->HasTwoWeapons());
	CAPTURE(id1);
	CAPTURE(id2);
	CAPTURE(id3);
	CAPTURE(id4);
	CAPTURE(id5);

	auto* i1 = id1 ? &lcf::Data::items[id1 - 1] : nullptr;
	auto* i2 = id2 ? &lcf::Data::items[id2 - 1] : nullptr;
	auto* i3 = id3 ? &lcf::Data::items[id3 - 1] : nullptr;
	auto* i4 = id4 ? &lcf::Data::items[id4 - 1] : nullptr;
	auto* i5 = id5 ? &lcf::Data::items[id5 - 1] : nullptr;

	a->SetEquipment(1, id1);
	a->SetEquipment(2, id2);
	a->SetEquipment(3, id3);
	a->SetEquipment(4, id4);
	a->SetEquipment(5, id5);

	REQUIRE_EQ(a->GetWeaponId(), id1);
	REQUIRE_EQ(a->GetShieldId(), id2);
	REQUIRE_EQ(a->GetArmorId(), id3);
	REQUIRE_EQ(a->GetHelmetId(), id4);
	REQUIRE_EQ(a->GetAccessoryId(), id5);

	testWeapon3(a, i1, i2, i3, i4, i5, Game_Battler::kWeaponAll);
	testWeapon3(a, i1, ((i2 && i2->type == lcf::rpg::Item::Type_weapon) ? nullptr : i2), i3, i4, i5, 0);
	testWeapon3(a, ((i1 && i1->type == lcf::rpg::Item::Type_weapon) ? nullptr : i1), i2, i3, i4, i5, 1);
}

static void testWeapon(Game_Actor* a, int id1, int id2, int armor = 0, int helmet = 0, int acc = 0) {
	testWeapon2(a, id1, id2, armor, helmet, acc);
	testWeapon2(a, 0, id2, armor, helmet, acc);
	testWeapon2(a, id1, 0, armor, helmet, acc);
	testWeapon2(a, 0, 0, armor, helmet, acc);
}

TEST_CASE("SingleWeapon") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 1, 2, 3, 4);
	MakeEquip(2, lcf::rpg::Item::Type_shield, 11, 12, 13, 14);

	auto* a = Main_Data::game_actors->GetActor(1);
	REQUIRE(a != nullptr);
	REQUIRE_FALSE(a->HasTwoWeapons());

	testWeapon(a, 1, 2);
}



TEST_CASE("DualWeaponParams") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 1, 2, 3, 4);
	MakeEquip(2, lcf::rpg::Item::Type_weapon, 5, 6, 7, 8);

	auto* a = Main_Data::game_actors->GetActor(2);
	REQUIRE(a != nullptr);
	REQUIRE(a->HasTwoWeapons());

	testWeapon(a, 1, 2);
}

TEST_CASE("DualWeaponHit") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 20, 40);
	MakeEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 75, 20);

	auto* a = Main_Data::game_actors->GetActor(2);
	REQUIRE(a != nullptr);
	REQUIRE(a->HasTwoWeapons());

	testWeapon(a, 1, 2);
}

TEST_CASE("DualWeaponFlags") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, true, false, false, false);
	MakeEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, true, false, false);
	MakeEquip(3, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, false, true, false);
	MakeEquip(4, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, false, false, true);

	auto* a = Main_Data::game_actors->GetActor(2);
	REQUIRE(a != nullptr);
	REQUIRE(a->HasTwoWeapons());

	for (int i = 1; i <= 4; ++i) {
		for (int j = 1; j <= 4; ++j) {
			testWeapon(a, i, j);
		}
	}
}

TEST_CASE("ArmorFlags") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_shield, 0, 0, 0, 0, 0, 0, false, false, false, false, true, false, false, false);
	MakeEquip(2, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 0, 0, false, false, false, false, false, true, false, false);
	MakeEquip(3, lcf::rpg::Item::Type_helmet, 0, 0, 0, 0, 0, 0, false, false, false, false, false, false, true, false);
	MakeEquip(4, lcf::rpg::Item::Type_accessory, 0, 0, 0, 0, 0, 0, false, false, false, false, false, false, false, true);

	auto* a = Main_Data::game_actors->GetActor(1);
	REQUIRE(a != nullptr);

	testWeapon(a, 0, 1, 0, 0, 0);
	testWeapon(a, 0, 1, 2, 0, 0);
	testWeapon(a, 0, 1, 2, 3, 0);
	testWeapon(a, 0, 1, 2, 3, 4);
}

TEST_CASE("WeaponWithArmorFlags") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 100, true, true, true, true, true, true, true, true);

	auto* a = Main_Data::game_actors->GetActor(1);
	REQUIRE(a != nullptr);

	a->SetEquipment(1, 1);
	REQUIRE_EQ(a->GetWeaponId(), 1);

	REQUIRE_FALSE(a->PreventsCritical());
	REQUIRE_FALSE(a->HasPhysicalEvasionUp());
	REQUIRE_FALSE(a->HasHalfSpCost());
	REQUIRE_FALSE(a->PreventsTerrainDamage());
}

TEST_CASE("ArmorWithWeaponFlags") {
	const MockActor m;

	MakeEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 100, true, true, true, true, true, true, true, true);

	auto* a = Main_Data::game_actors->GetActor(1);
	REQUIRE(a != nullptr);

	a->SetEquipment(3, 1);
	REQUIRE_EQ(a->GetArmorId(), 1);

	for (int wid = -1; wid <= 5; ++wid ) {
		CAPTURE(wid);

		REQUIRE_EQ(a->GetHitChance(wid), 90);
		REQUIRE_EQ(a->GetCriticalHitChance(wid), doctest::Approx(0.03333f));
		REQUIRE_FALSE(a->HasPreemptiveAttack(wid));
		REQUIRE_FALSE(a->HasDualAttack(wid));
		REQUIRE_FALSE(a->HasAttackAll(wid));
		REQUIRE_FALSE(a->AttackIgnoresEvasion(wid));
	}
}

TEST_SUITE_END();
