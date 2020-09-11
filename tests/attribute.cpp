#include "test_mock_actor.h"
#include "attribute.h"
#include "doctest.h"
#include "utils.h"

static Game_Actor MakeActor(int id) {
	return Game_Actor(id);
}

static lcf::DBBitArray MakeAttributeSet(std::initializer_list<bool> v = {}) {
	auto a = lcf::DBBitArray(lcf::Data::attributes.size());
	int idx = 0;
	for (auto b: v) {
		a[idx++] = b;
	}
	return a;
}

TEST_SUITE_BEGIN("Attribute");

TEST_CASE("Rate") {
	const MockActor m;

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, 1, -2, 3, -4, 5);
	MakeDBAttribute(2, lcf::rpg::Attribute::Type_magical, -6, 7, -8, 9, -10);

	SUBCASE("phys") {
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 0), 1);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 1), -2);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 2), 3);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 3), -4);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 4), 5);
	}

	SUBCASE("mag") {
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 0), -6);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 1), 7);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 2), -8);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 3), 9);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 4), -10);
	}

	SUBCASE("badrate") {
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, -1), 0);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(1, 5), 0);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, -1), 0);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(2, 5), 0);
	}

	SUBCASE("badid") {
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(-1, 0), 0);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(0, 0), 0);
		REQUIRE_EQ(Attribute::GetAttributeRateModifier(INT_MAX, 0), 0);
	}
}

template <typename... Args>
auto MakeAttributeSetArray(const Args*... args) {
	return Utils::MakeArray<const lcf::DBBitArray*>(args...);
}

TEST_CASE("ApplyMultiplerDefault") {
	const MockActor m;

	const auto& target = MakeActor(1);

	auto as0 = MakeAttributeSet();
	auto as1 = MakeAttributeSet();

	SUBCASE("0") {
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, {}), 100);
	}

	SUBCASE("1") {
		auto sets = MakeAttributeSetArray(&as0);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, MakeSpan(sets)), 100);
	}

	SUBCASE("2") {
		auto sets = MakeAttributeSetArray(&as0, &as1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, MakeSpan(sets)), 100);
	}
}

void testSingle(int type) {
	MakeDBAttribute(1, type, -100, 200, 100, 50, 0);
	auto as0 = MakeAttributeSet({ true });
	auto sets = MakeAttributeSetArray(&as0);

	SUBCASE("0") {
		SetDBActorAttribute(1, 1, 0);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), -100);
	}

	SUBCASE("1") {
		SetDBActorAttribute(1, 1, 1);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 200);
	}

	SUBCASE("2") {
		SetDBActorAttribute(1, 1, 2);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 100);
	}

	SUBCASE("3") {
		SetDBActorAttribute(1, 1, 3);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 50);
	}

	SUBCASE("4") {
		SetDBActorAttribute(1, 1, 4);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 0);
	}
}

TEST_CASE("ApplyMultiplerSingle") {
	const MockActor m;

	SUBCASE("phys") {
		testSingle(lcf::rpg::Attribute::Type_physical);
	}

	SUBCASE("mag") {
		testSingle(lcf::rpg::Attribute::Type_magical);
	}
}

void testMaxMulti(int type) {
	for (int i = 0; i < 5; ++i) {
		MakeDBAttribute(i + 1, type, -100, 200, 100, 50, 0);
	}


	SUBCASE("all") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 1);
		SetDBActorAttribute(1, 3, 2);
		SetDBActorAttribute(1, 4, 3);
		SetDBActorAttribute(1, 5, 4);

		auto as0 = MakeAttributeSet({ true, true, true, true, true });
		auto sets = MakeAttributeSetArray(&as0);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 200);
	}

	SUBCASE("0") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 4);

		auto as0 = MakeAttributeSet({ true, true });
		auto sets = MakeAttributeSetArray(&as0);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 0);
	}

	SUBCASE("neg") {
		SetDBActorAttribute(1, 3, 0);

		auto as0 = MakeAttributeSet({ false, false, true });
		auto sets = MakeAttributeSetArray(&as0);
		const auto& target = MakeActor(1);

		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), -100);
	}
}

TEST_CASE("ApplyMultiplerMaxMulti") {
	const MockActor m;

	SUBCASE("phys") {
		testMaxMulti(lcf::rpg::Attribute::Type_physical);
	}

	SUBCASE("mag") {
		testMaxMulti(lcf::rpg::Attribute::Type_magical);
	}
}

TEST_CASE("ApplyComboPos") {
	const MockActor m;

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, -100, 200, 100, 50, 0);
	MakeDBAttribute(2, lcf::rpg::Attribute::Type_magical, -100, 200, 100, 50, 0);

	auto as0 = MakeAttributeSet({ true, true });
	auto sets = MakeAttributeSetArray(&as0);

	SUBCASE("100_100") {
		SetDBActorAttribute(1, 1, 2);
		SetDBActorAttribute(1, 2, 2);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 100);
	}

	SUBCASE("200_100") {
		SetDBActorAttribute(1, 1, 1);
		SetDBActorAttribute(1, 2, 2);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 200);
	}

	SUBCASE("50_100") {
		SetDBActorAttribute(1, 1, 3);
		SetDBActorAttribute(1, 2, 2);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 50);
	}

	SUBCASE("50_0") {
		SetDBActorAttribute(1, 1, 3);
		SetDBActorAttribute(1, 2, 4);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 0);
	}

	SUBCASE("200_50") {
		SetDBActorAttribute(1, 1, 1);
		SetDBActorAttribute(1, 2, 3);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 100);
	}
}

TEST_CASE("ApplyComboNeg2k3") {
	const MockActor m(Player::EngineRpg2k3);

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, -100, -50, 100, 50, 0);
	MakeDBAttribute(2, lcf::rpg::Attribute::Type_magical, -100, -50, 100, 50, 0);

	auto as0 = MakeAttributeSet({ true, true });
	auto sets = MakeAttributeSetArray(&as0);

	SUBCASE("-100_-50") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 1);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), -50);
	}

	SUBCASE("-100_50") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 3);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 50);
	}

	SUBCASE("-100_0") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 4);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 0);
	}
}

TEST_CASE("ApplyComboNeg2k") {
	const MockActor m(Player::EngineRpg2k);

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, -100, -50, 100, 50, 0);
	MakeDBAttribute(2, lcf::rpg::Attribute::Type_magical, -100, -50, 100, 50, 0);

	auto as0 = MakeAttributeSet({ true, true });
	auto sets = MakeAttributeSetArray(&as0);

	SUBCASE("-100_-50") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 1);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 100);
	}

	SUBCASE("-100_50") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 3);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 50);
	}

	SUBCASE("-100_0") {
		SetDBActorAttribute(1, 1, 0);
		SetDBActorAttribute(1, 2, 4);

		const auto& target = MakeActor(1);
		REQUIRE_EQ(Attribute::ApplyAttributeMultiplier(100, target, sets), 0);
	}
}

TEST_CASE("Skill") {
	const MockActor m(Player::EngineRpg2k);
	auto* s0 = MakeDBSkill(1, 100, 1, 0, 0, 0);
	SetDBSkillAttribute(1, 1, true);
	auto* s1 = MakeDBSkill(2, 100, 1, 0, 0, 0);

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, 300, 300, 300, 300, 300);
	const auto& target = MakeActor(1);

	SUBCASE("affect") {
		REQUIRE_EQ(Attribute::ApplyAttributeSkillMultiplier(100, target, *s0), 300);
	}

	SUBCASE("none") {
		REQUIRE_EQ(Attribute::ApplyAttributeSkillMultiplier(100, target, *s1), 100);
	}
}

TEST_CASE("NormalAttack") {
	const MockActor m(Player::EngineRpg2k);

	MakeDBAttribute(1, lcf::rpg::Attribute::Type_physical, 300, 300, 300, 300, 300);
	MakeDBAttribute(2, lcf::rpg::Attribute::Type_physical, 200, 200, 200, 200, 200);

	MakeDBEquip(1, lcf::rpg::Item::Type_weapon);
	SetDBItemAttribute(1, 1, true);
	MakeDBEquip(2, lcf::rpg::Item::Type_weapon);
	SetDBItemAttribute(2, 2, true);
	MakeDBEquip(3, lcf::rpg::Item::Type_weapon);
	MakeDBEquip(4, lcf::rpg::Item::Type_shield);
	SetDBItemAttribute(1, 4, true);

	lcf::Data::actors[1].two_weapon = true;
	auto source = MakeActor(2);
	const auto& target = MakeActor(1);

	SUBCASE("none") {
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponAll), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponNone), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponPrimary), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponSecondary), 100);
	}

	SUBCASE("primary") {
		source.SetEquipment(1, 1);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponAll), 300);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponNone), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponPrimary), 300);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponSecondary), 100);
	}

	SUBCASE("secondary") {
		source.SetEquipment(2, 2);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponAll), 200);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponNone), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponPrimary), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponSecondary), 200);
	}

	SUBCASE("dual") {
		source.SetEquipment(1, 1);
		source.SetEquipment(2, 2);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponAll), 300);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponNone), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponPrimary), 300);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponSecondary), 200);
	}

	SUBCASE("shield") {
		source.SetEquipment(2, 4);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponAll), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponNone), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponPrimary), 100);
		REQUIRE_EQ(Attribute::ApplyAttributeNormalAttackMultiplier(100, source, target, Game_Battler::WeaponSecondary), 100);
	}
}

TEST_SUITE_END();

