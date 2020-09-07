#include "test_mock_actor.h"
#include "doctest.h"

template <typename... Args>
static Game_Actor MakeActor(int id, Args... args) {
	MakeDBActor(id, args...);
	return Game_Actor(id);
}

TEST_SUITE_BEGIN("Game_Actor");

TEST_CASE("Limits2k") {
	const MockActor m(Player::EngineRpg2k);
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(actor.MaxHpValue(), 999);
	REQUIRE_EQ(actor.MaxStatBaseValue(), 999);
	// FIXME: Verify Clamps
	REQUIRE_EQ(actor.MaxStatBattleValue(), 999);
}

TEST_CASE("Limits2k3") {
	const MockActor m(Player::EngineRpg2k3);

	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(actor.MaxHpValue(), 9999);
	REQUIRE_EQ(actor.MaxStatBaseValue(), 999);
	// FIXME: Verify Clamps
	REQUIRE_EQ(actor.MaxStatBattleValue(), 9999);
}


TEST_CASE("Default") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(actor.GetType(), Game_Battler::Type_Ally);
	REQUIRE_EQ(actor.GetId(), 1);

	REQUIRE_EQ(actor.GetLevel(), 1);
	REQUIRE_EQ(actor.GetMaxLevel(), 99);

	REQUIRE_EQ(actor.GetExp(), 0);
	REQUIRE_EQ(actor.GetBaseExp(), 0);
	REQUIRE_EQ(actor.GetNextExp(), 718);

	REQUIRE_EQ(actor.GetClass(), nullptr);

	REQUIRE_EQ(actor.GetBaseMaxHp(), 100);
	REQUIRE_EQ(actor.GetBaseMaxSp(), 10);
	REQUIRE_EQ(actor.GetMaxHp(), 100);
	REQUIRE_EQ(actor.GetMaxSp(), 10);

	REQUIRE_EQ(actor.GetHp(), 100);
	REQUIRE_EQ(actor.GetSp(), 10);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor.GetBaseAtk(w), 11);
		REQUIRE_EQ(actor.GetBaseDef(w), 12);
		REQUIRE_EQ(actor.GetBaseSpi(w), 13);
		REQUIRE_EQ(actor.GetBaseAgi(w), 14);

		REQUIRE_EQ(actor.GetAtk(w), 11);
		REQUIRE_EQ(actor.GetDef(w), 12);
		REQUIRE_EQ(actor.GetSpi(w), 13);
		REQUIRE_EQ(actor.GetAgi(w), 14);

		REQUIRE_FALSE(actor.HasPreemptiveAttack(w));
		REQUIRE_FALSE(actor.HasDualAttack(w));
		REQUIRE_FALSE(actor.HasAttackAll(w));
		REQUIRE_FALSE(actor.AttackIgnoresEvasion(w));

		REQUIRE_EQ(actor.GetHitChance(w), 90);
		REQUIRE(actor.GetCriticalHitChance(w) == doctest::Approx(0.03333f));
	}

	REQUIRE_FALSE(actor.PreventsTerrainDamage());
	REQUIRE_FALSE(actor.PreventsCritical());
	REQUIRE_FALSE(actor.HasPhysicalEvasionUp());
	REQUIRE_FALSE(actor.HasHalfSpCost());

	REQUIRE_EQ(actor.GetWeaponId(), 0);
	REQUIRE_EQ(actor.GetShieldId(), 0);
	REQUIRE_EQ(actor.GetArmorId(), 0);
	REQUIRE_EQ(actor.GetHelmetId(), 0);
	REQUIRE_EQ(actor.GetAccessoryId(), 0);

	REQUIRE_EQ(actor.GetWeapon(), nullptr);
	REQUIRE_EQ(actor.Get2ndWeapon(), nullptr);
	REQUIRE_EQ(actor.GetShield(), nullptr);
	REQUIRE_EQ(actor.GetArmor(), nullptr);
	REQUIRE_EQ(actor.GetHelmet(), nullptr);
	REQUIRE_EQ(actor.GetAccessory(), nullptr);

	REQUIRE_FALSE(actor.IsEquipmentFixed());
	REQUIRE_FALSE(actor.HasStrongDefense());
	REQUIRE_FALSE(actor.HasTwoWeapons());
	REQUIRE_FALSE(actor.GetAutoBattle());

	REQUIRE_EQ(actor.GetBattleRow(), Game_Actor::RowType::RowType_front);
}

TEST_CASE("ActorFlags") {
	const MockActor m;

	for (auto two_weapon: { true, false }) {
		for (auto lock_equip: { true, false }) {
			for (auto auto_battle: { true, false }) {
				for (auto super_guard: { true, false }) {
					auto actor = MakeActor(1, 1, 99, 1, 1, 1, 1, 1, 1, two_weapon, lock_equip, auto_battle, super_guard);

					REQUIRE_EQ(actor.HasTwoWeapons(), two_weapon);
					REQUIRE_EQ(actor.IsEquipmentFixed(), lock_equip);
					REQUIRE_EQ(actor.GetAutoBattle(), auto_battle);
					REQUIRE_EQ(actor.HasStrongDefense(), super_guard);
				}
			}
		}
	}
}

TEST_CASE("AdjParams") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);


	actor.SetBaseMaxHp(501);
	actor.SetBaseMaxSp(502);
	actor.SetBaseAtk(503);
	actor.SetBaseDef(504);
	actor.SetBaseSpi(505);
	actor.SetBaseAgi(506);

	REQUIRE_EQ(actor.GetBaseMaxHp(), 501);
	REQUIRE_EQ(actor.GetBaseMaxSp(), 502);
	REQUIRE_EQ(actor.GetMaxHp(), 501);
	REQUIRE_EQ(actor.GetMaxSp(), 502);

	REQUIRE_EQ(actor.GetHp(), 100);
	REQUIRE_EQ(actor.GetSp(), 10);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor.GetBaseAtk(w), 503);
		REQUIRE_EQ(actor.GetBaseDef(w), 504);
		REQUIRE_EQ(actor.GetBaseSpi(w), 505);
		REQUIRE_EQ(actor.GetBaseAgi(w), 506);

		REQUIRE_EQ(actor.GetAtk(w), 503);
		REQUIRE_EQ(actor.GetDef(w), 504);
		REQUIRE_EQ(actor.GetSpi(w), 505);
		REQUIRE_EQ(actor.GetAgi(w), 506);
	}

	actor.SetAtkModifier(10);
	actor.SetDefModifier(20);
	actor.SetSpiModifier(30);
	actor.SetAgiModifier(40);

	for (int w = -1; w < 2; ++w) {
		REQUIRE_EQ(actor.GetBaseAtk(w), 503);
		REQUIRE_EQ(actor.GetBaseDef(w), 504);
		REQUIRE_EQ(actor.GetBaseSpi(w), 505);
		REQUIRE_EQ(actor.GetBaseAgi(w), 506);

		REQUIRE_EQ(actor.GetAtk(w), 513);
		REQUIRE_EQ(actor.GetDef(w), 524);
		REQUIRE_EQ(actor.GetSpi(w), 535);
		REQUIRE_EQ(actor.GetAgi(w), 546);
	}
}

TEST_CASE("TryEquip") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 1, 11, 21, 31);
	MakeDBEquip(2, lcf::rpg::Item::Type_shield, 1, 11, 21, 31);
	MakeDBEquip(3, lcf::rpg::Item::Type_armor, 1, 11, 21, 31);
	MakeDBEquip(4, lcf::rpg::Item::Type_helmet, 1, 11, 21, 31);
	MakeDBEquip(5, lcf::rpg::Item::Type_accessory, 1, 11, 21, 31);

	for (int i = 1; i <= 5; ++i) {
		actor.SetEquipment(i, i);
		REQUIRE_EQ(actor.GetWeaponId(), 1);
		if (i >= 2) REQUIRE_EQ(actor.GetShieldId(), 2);
		if (i >= 3) REQUIRE_EQ(actor.GetArmorId(), 3);
		if (i >= 4) REQUIRE_EQ(actor.GetHelmetId(), 4);
		if (i >= 5) REQUIRE_EQ(actor.GetAccessoryId(), 5);

		REQUIRE_EQ(actor.GetBaseAtk(), 11 + i * 1);
		REQUIRE_EQ(actor.GetBaseDef(), 12 + i * 11);
		REQUIRE_EQ(actor.GetBaseSpi(), 13 + i * 21);
		REQUIRE_EQ(actor.GetBaseAgi(), 14 + i * 31);

		REQUIRE_EQ(actor.GetAtk(), actor.GetBaseAtk());
		REQUIRE_EQ(actor.GetDef(), actor.GetBaseDef());
		REQUIRE_EQ(actor.GetSpi(), actor.GetBaseSpi());
		REQUIRE_EQ(actor.GetAgi(), actor.GetBaseAgi());
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
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14, false);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 1, 2, 3, 4);
	MakeDBEquip(2, lcf::rpg::Item::Type_shield, 11, 12, 13, 14);

	REQUIRE_FALSE(actor.HasTwoWeapons());

	testWeapon(&actor, 1, 2);
}

TEST_CASE("DualWeaponParams") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14, true);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 1, 2, 3, 4);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 5, 6, 7, 8);

	REQUIRE(actor.HasTwoWeapons());

	testWeapon(&actor, 1, 2);
}

TEST_CASE("DualWeaponHit") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14, true);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 20, 40);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 75, 20);

	REQUIRE(actor.HasTwoWeapons());

	testWeapon(&actor, 1, 2);
}

TEST_CASE("DualWeaponFlags") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14, true);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, true, false, false, false);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, true, false, false);
	MakeDBEquip(3, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, false, true, false);
	MakeDBEquip(4, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 0, false, false, false, true);

	REQUIRE(actor.HasTwoWeapons());

	for (int i = 1; i <= 4; ++i) {
		for (int j = 1; j <= 4; ++j) {
			testWeapon(&actor, i, j);
		}
	}
}

TEST_CASE("ArmorFlags") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	MakeDBEquip(1, lcf::rpg::Item::Type_shield, 0, 0, 0, 0, 0, 0, false, false, false, false, true, false, false, false);
	MakeDBEquip(2, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 0, 0, false, false, false, false, false, true, false, false);
	MakeDBEquip(3, lcf::rpg::Item::Type_helmet, 0, 0, 0, 0, 0, 0, false, false, false, false, false, false, true, false);
	MakeDBEquip(4, lcf::rpg::Item::Type_accessory, 0, 0, 0, 0, 0, 0, false, false, false, false, false, false, false, true);


	testWeapon(&actor, 0, 1, 0, 0, 0);
	testWeapon(&actor, 0, 1, 2, 0, 0);
	testWeapon(&actor, 0, 1, 2, 3, 0);
	testWeapon(&actor, 0, 1, 2, 3, 4);
}

TEST_CASE("WeaponWithArmorFlags") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon, 0, 0, 0, 0, 100, 100, true, true, true, true, true, true, true, true);


	actor.SetEquipment(1, 1);
	REQUIRE_EQ(actor.GetWeaponId(), 1);

	REQUIRE_FALSE(actor.PreventsCritical());
	REQUIRE_FALSE(actor.HasPhysicalEvasionUp());
	REQUIRE_FALSE(actor.HasHalfSpCost());
	REQUIRE_FALSE(actor.PreventsTerrainDamage());
}

TEST_CASE("ArmorWithWeaponFlags") {
	const MockActor m;
	auto actor = MakeActor(1, 1, 99, 100, 10, 11, 12, 13, 14);

	MakeDBEquip(1, lcf::rpg::Item::Type_armor, 0, 0, 0, 0, 100, 100, true, true, true, true, true, true, true, true);

	actor.SetEquipment(3, 1);
	REQUIRE_EQ(actor.GetArmorId(), 1);

	for (int wid = -1; wid <= 5; ++wid ) {
		CAPTURE(wid);

		REQUIRE_EQ(actor.GetHitChance(wid), 90);
		REQUIRE_EQ(actor.GetCriticalHitChance(wid), doctest::Approx(0.03333f));
		REQUIRE_FALSE(actor.HasPreemptiveAttack(wid));
		REQUIRE_FALSE(actor.HasDualAttack(wid));
		REQUIRE_FALSE(actor.HasAttackAll(wid));
		REQUIRE_FALSE(actor.AttackIgnoresEvasion(wid));
	}
}

static Game_Actor MakeActorAttribute(int id, int attr_id, int rank) {
	SetDBActorAttribute(id, attr_id, rank);
	return MakeActor(id, 1, 99, 1, 1, 1, 1, 1, 1);
}

TEST_CASE("Attribute") {
	const MockActor m;

	SUBCASE("0") {
		const auto& actor = MakeActorAttribute(1, 1, 0);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 0);
		REQUIRE_EQ(actor.GetAttributeRate(1), 0);
	}

	SUBCASE("1") {
		const auto& actor = MakeActorAttribute(1, 1, 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 1);
		REQUIRE_EQ(actor.GetAttributeRate(1), 1);
	}

	SUBCASE("2") {
		const auto& actor = MakeActorAttribute(1, 1, 2);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 2);
	}
}

TEST_CASE("BadAttribute") {
	const MockActor m;
	const auto& actor = MakeActorAttribute(1, 1, 2);

	REQUIRE_EQ(actor.GetBaseAttributeRate(0), 2);
	REQUIRE_EQ(actor.GetAttributeRate(0), 2);
	REQUIRE_EQ(actor.GetBaseAttributeRate(INT_MAX), 2);
	REQUIRE_EQ(actor.GetAttributeRate(INT_MAX), 2);
}

TEST_CASE("AttributeEquip") {
	const MockActor m;
	MakeDBEquip(1, lcf::rpg::Item::Type_weapon);
	MakeDBEquip(2, lcf::rpg::Item::Type_shield);
	MakeDBEquip(3, lcf::rpg::Item::Type_armor);
	MakeDBEquip(4, lcf::rpg::Item::Type_helmet);
	MakeDBEquip(5, lcf::rpg::Item::Type_accessory);

	for (int i = 1; i <= 5; ++i) {
		SetDBItemAttribute(i, 1, true);

	}

	SUBCASE("normal") {
		auto actor = MakeActorAttribute(1, 1, 2);

		SUBCASE("weapon") {
			actor.SetEquipment(1, 1);
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
			REQUIRE_EQ(actor.GetAttributeRate(1), 2);
		}

		SUBCASE("shield") {
			actor.SetEquipment(2, 2);
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 3);
			REQUIRE_EQ(actor.GetAttributeRate(1), 3);
		}

		SUBCASE("armor") {
			actor.SetEquipment(3, 3);
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 3);
			REQUIRE_EQ(actor.GetAttributeRate(1), 3);
		}

		SUBCASE("helmet") {
			actor.SetEquipment(4, 4);
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 3);
			REQUIRE_EQ(actor.GetAttributeRate(1), 3);
		}

		SUBCASE("accessory") {
			actor.SetEquipment(5, 5);
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 3);
			REQUIRE_EQ(actor.GetAttributeRate(1), 3);
		}

		SUBCASE("all") {
			for (int i = 1; i <= 5; ++i) {
				actor.SetEquipment(i, i);
			}
			REQUIRE_EQ(actor.GetBaseAttributeRate(1), 3);
			REQUIRE_EQ(actor.GetAttributeRate(1), 3);
		}
	}

	SUBCASE("overflow") {
		auto actor = MakeActorAttribute(1, 1, 4);
		for (int i = 1; i <= 5; ++i) {
			actor.SetEquipment(i, i);
		}
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 4);
		REQUIRE_EQ(actor.GetAttributeRate(1), 4);
	}
}

TEST_CASE("AttributeShift") {
	const MockActor m;

	SUBCASE("normal") {
		auto actor = MakeActorAttribute(1, 1, 2);

		REQUIRE(actor.CanShiftAttributeRate(1, 1));
		REQUIRE_FALSE(actor.CanShiftAttributeRate(1, 2));
		REQUIRE(actor.CanShiftAttributeRate(1, -1));
		REQUIRE_FALSE(actor.CanShiftAttributeRate(1, -2));

		REQUIRE_EQ(actor.GetAttributeRateShift(1), 0);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 2);

		actor.ShiftAttributeRate(1, 1);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 3);

		actor.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), 0);
		REQUIRE_EQ(actor.GetAttributeRate(1), 2);

		actor.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 1);

		actor.ShiftAttributeRate(1, 100);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 3);

		actor.ShiftAttributeRate(1, -100);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 1);
	}

	SUBCASE("overflow") {
		auto actor = MakeActorAttribute(1, 1, 4);
		actor.ShiftAttributeRate(1, 1);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 4);
		REQUIRE_EQ(actor.GetAttributeRate(1), 4);
	}

	SUBCASE("underflow") {
		auto actor = MakeActorAttribute(1, 1, 0);
		actor.ShiftAttributeRate(1, -1);
		REQUIRE_EQ(actor.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 0);
		REQUIRE_EQ(actor.GetAttributeRate(1), 0);
	}
}

TEST_CASE("AttributeShiftInvalid") {
	const MockActor m;
	auto actor = MakeActorAttribute(1, 1, 100);

	REQUIRE_EQ(actor.GetAttributeRateShift(0), 0);
	REQUIRE_EQ(actor.GetAttributeRateShift(INT_MAX), 0);
	REQUIRE_FALSE(actor.CanShiftAttributeRate(0, 1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(0, -1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(INT_MAX, 1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(INT_MAX, -1));

	actor.ShiftAttributeRate(0, 1);
	actor.ShiftAttributeRate(INT_MAX, 1);

	REQUIRE_EQ(actor.GetAttributeRateShift(0), 0);
	REQUIRE_EQ(actor.GetAttributeRateShift(INT_MAX), 0);
	REQUIRE_FALSE(actor.CanShiftAttributeRate(0, 1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(0, -1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(INT_MAX, 1));
	REQUIRE_FALSE(actor.CanShiftAttributeRate(INT_MAX, -1));
}

TEST_CASE("AttributeShiftEquip") {
	const MockActor m;
	MakeDBEquip(1, lcf::rpg::Item::Type_shield);
	SetDBItemAttribute(1, 1, true);

	SUBCASE("normal") {
		auto actor = MakeActorAttribute(1, 1, 1);
		actor.SetEquipment(2, 1);
		actor.ShiftAttributeRate(1, 1);

		REQUIRE_EQ(actor.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 3);

		actor.ShiftAttributeRate(1, -2);

		REQUIRE_EQ(actor.GetAttributeRateShift(1), -1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 2);
		REQUIRE_EQ(actor.GetAttributeRate(1), 1);
	}

	SUBCASE("overflow") {
		auto actor = MakeActorAttribute(1, 1, 4);
		actor.SetEquipment(2, 1);
		actor.ShiftAttributeRate(1, 1);

		REQUIRE_EQ(actor.GetAttributeRateShift(1), 1);
		REQUIRE_EQ(actor.GetBaseAttributeRate(1), 4);
		REQUIRE_EQ(actor.GetAttributeRate(1), 4);
	}
}


TEST_SUITE_END();
