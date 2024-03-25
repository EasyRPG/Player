/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <algorithm>
#include <sstream>
#include <iterator>
#include "game_actor.h"
#include "game_battle.h"
#include "game_party.h"
#include "sprite_actor.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <lcf/rpg/skill.h>
#include "util_macro.h"
#include "utils.h"
#include "pending_message.h"
#include "attribute.h"
#include "rand.h"
#include "algo.h"

constexpr int max_level_2k = 50;
constexpr int max_level_2k3 = 99;

int Game_Actor::MaxHpValue() const {
	auto& val = lcf::Data::system.easyrpg_max_actor_hp;
	if (val == -1) {
		return Player::IsRPG2k() ? 999 : 9999;
	}
	return val;
}

int Game_Actor::MaxSpValue() const {
	auto& val = lcf::Data::system.easyrpg_max_actor_sp;
	if (val == -1) {
		return 999;
	}
	return val;
}

int Game_Actor::MaxStatBattleValue() const {
	auto& val = lcf::Data::system.easyrpg_max_stat_battle_value;
	if (val == -1) {
		return 9999;
	}
	return val;
}

int Game_Actor::MaxStatBaseValue() const {
	auto& val = lcf::Data::system.easyrpg_max_stat_base_value;
	if (val == -1) {
		return 999;
	}
	return val;
}

int Game_Actor::MaxExpValue() const {
	auto& val = lcf::Data::system.easyrpg_max_exp;
	if (val == -1) {
		return Player::IsRPG2k() ? 999999 : 9999999;
	}
	return val;
}

Game_Actor::Game_Actor(int actor_id) {
	data.ID = actor_id;
	if (actor_id == 0) {
		return;
	}
	ReloadDbActor();

	data.two_weapon = dbActor->two_weapon;
	data.lock_equipment = dbActor->lock_equipment;
	data.auto_battle = dbActor->auto_battle;
	data.super_guard = dbActor->super_guard;

	data.hp_mod = 0;
	data.sp_mod = 0;
	data.attack_mod = 0;
	data.defense_mod = 0;
	data.spirit_mod = 0;
	data.agility_mod = 0;

	MakeExpList();
	SetBattlePosition(GetOriginalPosition());

	data.level = 0;
	if (dbActor->initial_level > 0) {
		// For games like COLORS: Lost Memories which use level 0, don't change level because it'll clamp to 1.
		ChangeLevel(dbActor->initial_level, nullptr);
	}
	SetHp(GetMaxHp());
	SetSp(GetMaxSp());

	// Remove items that do not exist in the database anymore
	std::array<int, 5> ids = {{
		dbActor->initial_equipment.weapon_id,
		dbActor->initial_equipment.shield_id,
		dbActor->initial_equipment.armor_id,
		dbActor->initial_equipment.helmet_id,
		dbActor->initial_equipment.accessory_id }};
	std::replace_if(ids.begin(), ids.end(), [] (const int& item_id) {
		return lcf::ReaderUtil::GetElement(lcf::Data::items, item_id) == nullptr;
	}, 0);

	for (int i = 0; i <= 4; i++) {
		SetEquipment(i + 1, ids[i]);
	}

	data.status.resize(lcf::Data::states.size(), 0);

	Fixup();
}

void Game_Actor::SetSaveData(lcf::rpg::SaveActor save) {
	if (data.ID != save.ID) {
		Output::Debug("Game_Actor: Fixing actor ID mismatch {} != {}", save.ID, data.ID);
		save.ID = data.ID;
	}

	data = std::move(save);

	if (Player::IsRPG2k()) {
		data.two_weapon = dbActor->two_weapon;
		data.lock_equipment = dbActor->lock_equipment;
		data.auto_battle = dbActor->auto_battle;
		data.super_guard = dbActor->super_guard;
	}

	MakeExpList();
	Fixup();
}

void Game_Actor::ReloadDbActor() {
	dbActor = lcf::ReaderUtil::GetElement(lcf::Data::actors, GetId());
}

lcf::rpg::SaveActor Game_Actor::GetSaveData() const {
	auto save = data;
	if (Player::IsRPG2k()) {
		// RPG_RT doesn't save these chunks in rm2k as they are meaningless
		save.two_weapon = false;
		save.lock_equipment = false;
		save.auto_battle = false;
		save.super_guard = false;
	}
	return save;
}

void Game_Actor::Fixup() {
	RemoveInvalidData();
	ResetEquipmentStates(false);
}

bool Game_Actor::UseItem(int item_id, const Game_Battler* source) {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use invalid item {}", item_id);
		return false;
	}

	if (!IsDead()) {
		if (item->type == lcf::rpg::Item::Type_book) {
			return LearnSkill(item->skill_id, nullptr);
		}

		if (item->type == lcf::rpg::Item::Type_material) {
			SetBaseMaxHp(GetBaseMaxHp() + item->max_hp_points);
			SetBaseMaxSp(GetBaseMaxSp() + item->max_sp_points);
			SetBaseAtk(GetBaseAtk() + item->atk_points2);
			SetBaseDef(GetBaseDef() + item->def_points2);
			SetBaseAgi(GetBaseAgi() + item->agi_points2);
			SetBaseSpi(GetBaseSpi() + item->spi_points2);

			return true;
		}
	}

	return Game_Battler::UseItem(item_id, source);
}

bool Game_Actor::IsItemUsable(int item_id) const {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("IsItemUsable: Invalid item ID {}", item_id);
		return false;
	}

	int query_idx = GetId() - 1;
	auto* query_set = &item->actor_set;
	if (Player::IsRPG2k3() && lcf::Data::system.equipment_setting == lcf::rpg::System::EquipmentSetting_class) {
		auto* cls = GetClass();

		// Class index. If there's no class, in the "class_set" it's equal to 0. The first class is 1, not 0
		query_idx = cls ? cls->ID : 0;
		query_set = &item->class_set;
	}

	// If the actor or class ID is out of range this is an optimization in the ldb file
	// (all actors or classes missing can equip the item)
	if (query_set->size() <= (unsigned)(query_idx)) {
		return true;
	}
	return (*query_set)[query_idx];
}

bool Game_Actor::IsSkillLearned(int skill_id) const {
	return std::find(data.skills.begin(), data.skills.end(), skill_id) != data.skills.end();
}

bool Game_Actor::IsSkillUsable(int skill_id) const {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("IsSkillUsable: Invalid skill ID {}", skill_id);
		return false;
	}

	if (!skill->affect_attr_defence) {
		// Actor must have all attributes of the skill equipped as weapons
		const auto* w1 = GetWeapon();
		const auto* w2 = Get2ndWeapon();

		for (size_t i = 0; i < skill->attribute_effects.size(); ++i) {
			bool required = skill->attribute_effects[i] && lcf::Data::attributes[i].type == lcf::rpg::Attribute::Type_physical;
			if (required) {
				if (w1 && i < w1->attribute_set.size() && w1->attribute_set[i]) {
					continue;
				}
				if (w2 && i < w2->attribute_set.size() && w2->attribute_set[i]) {
					continue;
				}
				return false;
			}
		}
	}

	return Game_Battler::IsSkillUsable(skill_id);
}

int Game_Actor::CalculateSkillCost(int skill_id) const {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("CalculateSkillCost: Invalid skill ID {}", skill_id);
		return 0;
	}
	return Algo::CalcSkillCost(*skill, GetMaxSp(), HasHalfSpCost());
}

bool Game_Actor::LearnSkill(int skill_id, PendingMessage* pm) {
	if (skill_id > 0 && !IsSkillLearned(skill_id)) {
		const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
		if (!skill) {
			Output::Warning("Actor {}: Can't learn invalid skill {}", GetId(), skill_id);
			return false;
		}

		data.skills.push_back((int16_t)skill_id);
		std::sort(data.skills.begin(), data.skills.end());

		if (pm) {
			pm->PushLine(GetLearningMessage(*skill));
		}

		return true;
	}
	return false;
}

int Game_Actor::LearnLevelSkills(int min_level, int max_level, PendingMessage* pm) {
	auto& skills = data.class_id > 0 ? GetClass()->skills : dbActor->skills;

	int count = 0;

	// Learn new skills
	for (const lcf::rpg::Learning& learn : skills) {
		// Skill learning, up to current level
		if (learn.level >= min_level && learn.level <= max_level) {
			const auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, learn.skill_id);
			if (!skill) {
				Output::Debug("Actor {}: Level up (level={}). Ignoring invalid skill {}", GetId(), learn.level, learn.skill_id);
			} else {
				count += LearnSkill(learn.skill_id, pm);
			}
		}
	}
	return count;
}

bool Game_Actor::UnlearnSkill(int skill_id) {
	std::vector<int16_t>::iterator it = std::find(data.skills.begin(), data.skills.end(), skill_id);
	if (it != data.skills.end()) {
		data.skills.erase(it);
		return true;
	}
	return false;
}

void Game_Actor::UnlearnAllSkills() {
	data.skills.clear();
}

void Game_Actor::SetFace(const std::string& file_name, int index) {
	if (file_name == dbActor->face_name && index == dbActor->face_index) {
		data.face_name = "";
		data.face_id = 0;
	} else {
		data.face_name.assign(file_name);
		data.face_id = index;
	}
}

const lcf::rpg::Item* Game_Actor::GetEquipment(int equip_type) const {
	if (equip_type <= 0 || equip_type > (int)data.equipped.size())
		return nullptr;

	int item_id = data.equipped[equip_type - 1];
	return lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
}

int Game_Actor::SetEquipment(int equip_type, int new_item_id) {
	if (equip_type <= 0 || equip_type > (int) data.equipped.size())
		return -1;

	int old_item_id = data.equipped[equip_type - 1];
	const lcf::rpg::Item* old_item = lcf::ReaderUtil::GetElement(lcf::Data::items, old_item_id);

	const lcf::rpg::Item* new_item = lcf::ReaderUtil::GetElement(lcf::Data::items, new_item_id);
	if (new_item_id != 0 && !new_item) {
		Output::Warning("SetEquipment: Can't equip item with invalid ID {}", new_item_id);
		new_item_id = 0;
	}

	data.equipped[equip_type - 1] = (short)new_item_id;

	AdjustEquipmentStates(old_item, false, false);
	AdjustEquipmentStates(new_item, true, false);

	return old_item_id;
}

void Game_Actor::ChangeEquipment(int equip_type, int item_id) {
	if (item_id != 0 && !IsItemUsable(item_id)) {
		return;
	}

	int prev_item = SetEquipment(equip_type, item_id);

	if (prev_item != 0) {
		Main_Data::game_party->AddItem(prev_item, 1);
	}
	if (item_id != 0) {
		Main_Data::game_party->RemoveItem(item_id, 1);
	}

	// In case you have a two_handed weapon equipped, the other weapon is removed.
	const lcf::rpg::Item* item = GetWeapon();
	const lcf::rpg::Item* item2 = Get2ndWeapon();
	if (item2 == nullptr) {
		item2 = GetShield();
	}
	if (item && item2 && ((item->type == lcf::rpg::Item::Type_weapon && item->two_handed) || (item2->type == lcf::rpg::Item::Type_weapon && item2->two_handed))) {
		ChangeEquipment(equip_type == lcf::rpg::Item::Type_weapon ? equip_type + 1 : equip_type - 1, 0);
	}
}

bool Game_Actor::IsEquipped(int equip_id) const {
	for (auto equip : GetWholeEquipment()) {
		if (equip == equip_id) {
			return true;
		}
	}
	return false;
}

void Game_Actor::RemoveWholeEquipment() {
	for (int i = 1; i <= 5; ++i) {
		ChangeEquipment(i, 0);
	}
}

int Game_Actor::GetItemCount(int item_id) {
	int number = 0;

	// quirk: 0 is "no item in slot"
	// This can be used to count how many slots are empty
	if (item_id >= 0) {
		for (int16_t i : GetWholeEquipment()) {
			if (item_id == i) {
				++number;
			}
		}
	}

	return number;
}

void Game_Actor::FullHeal() {
	RemoveAllStates();
	SetHp(GetMaxHp());
	SetSp(GetMaxSp());
	// Emulates RPG_RT behavior of resetting even battle equipment states on full heal.
	ResetEquipmentStates(true);
}

int Game_Actor::GetBaseMaxHp(bool mod) const {
	int n = 0;
	// Special handling for games that use a level of 0 -> Return 0 Hp
	// Same applies for other stats
	if (GetLevel() > 0) {
		// Looks like RPG_RT only applies Class changes (class_id > 0 - 20kdc)
		// when the class was changed by the ChangeClass event, otherwise it uses
		// the normal actor attributes.
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.maxhp, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.maxhp, GetLevel());
	}

	if (mod)
		n += data.hp_mod;

	return Utils::Clamp(n, 1, MaxHpValue());
}

int Game_Actor::GetBaseMaxHp() const {
	return GetBaseMaxHp(true);
}

int Game_Actor::GetBaseMaxSp(bool mod) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.maxsp, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.maxsp, GetLevel());
	}

	if (mod)
		n += data.sp_mod;

	return Utils::Clamp(n, 0, MaxSpValue());
}

int Game_Actor::GetBaseMaxSp() const {
	return GetBaseMaxSp(true);
}

static bool IsArmorType(const lcf::rpg::Item* item) {
	return item->type == lcf::rpg::Item::Type_shield
		|| item->type == lcf::rpg::Item::Type_armor
		|| item->type == lcf::rpg::Item::Type_helmet
		|| item->type == lcf::rpg::Item::Type_accessory;
}

template <bool allow_weapon, bool allow_armor, typename F>
void ForEachEquipment(Span<const short> equipped, F&& f, Game_Battler::Weapon weapon = Game_Battler::WeaponAll) {
	for (int slot = 0; slot < static_cast<int>(equipped.size()); ++slot) {
		const auto item_id = equipped[slot];
		if (item_id <= 0) {
			continue;
		}

		auto* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
		// Invalid equipment was removed
		assert(item != nullptr);

		if (item->type == lcf::rpg::Item::Type_weapon) {
			if (!allow_weapon || (weapon != Game_Battler::WeaponAll && weapon != slot + 1)) {
				continue;
			}
		} else if (IsArmorType(item)) {
			if (!allow_armor) {
				continue;
			}
		} else {
			assert(false && "Invalid item type equipped!");
			continue;
		}

		f(*item);
	}
}

int Game_Actor::GetBaseAtk(Weapon weapon, bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.attack, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.attack, GetLevel());
	}

	if (mod) {
		n += data.attack_mod;
	}

	if (equip) {
		ForEachEquipment<true,true>(GetWholeEquipment(), [&](auto& item) { n += item.atk_points1; }, weapon);
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseAtk(Weapon weapon) const {
	return GetBaseAtk(weapon, true, true);
}

int Game_Actor::GetBaseDef(Weapon weapon, bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.defense, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.defense, GetLevel());
	}

	if (mod) {
		n += data.defense_mod;
	}

	if (equip) {
		ForEachEquipment<true,true>(GetWholeEquipment(), [&](auto& item) { n += item.def_points1; }, weapon);
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseDef(Weapon weapon) const {
	return GetBaseDef(weapon, true, true);
}

int Game_Actor::GetBaseSpi(Weapon weapon, bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.spirit, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.spirit, GetLevel());
	}

	if (mod) {
		n += data.spirit_mod;
	}

	if (equip) {
		ForEachEquipment<true,true>(GetWholeEquipment(), [&](auto& item) { n += item.spi_points1; }, weapon);
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseSpi(Weapon weapon) const {
	return GetBaseSpi(weapon, true, true);
}

int Game_Actor::GetBaseAgi(Weapon weapon, bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = data.class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.agility, GetLevel())
			: *lcf::ReaderUtil::GetElement(dbActor->parameters.agility, GetLevel());
	}

	if (mod) {
		n += data.agility_mod;
	}

	if (equip) {
		ForEachEquipment<true,true>(GetWholeEquipment(), [&](auto& item) { n += item.agi_points1; }, weapon);
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseAgi(Weapon weapon) const {
	return GetBaseAgi(weapon, true, true);
}

int Game_Actor::CalculateExp(int level) const {
	const lcf::rpg::Class* klass = lcf::ReaderUtil::GetElement(lcf::Data::classes, data.class_id);

	int exp_curve = Player::IsRPG2k() ? 1 : 2;
	if (lcf::Data::system.easyrpg_alternative_exp > 0) {
		exp_curve = lcf::Data::system.easyrpg_alternative_exp;
	}

	double base, inflation, correction;
	if (klass) {
		base = klass->exp_base;
		inflation = klass->exp_inflation;
		correction = klass->exp_correction;
	}
	else {
		const lcf::rpg::Actor& actor = *lcf::ReaderUtil::GetElement(lcf::Data::actors, GetId());
		base = actor.exp_base;
		inflation = actor.exp_inflation;
		correction = actor.exp_correction;
	}

	int result = 0;
	if (exp_curve == 1) {
		inflation = 1.5 + (inflation * 0.01);

		for (int i = level; i >= 1; i--)
		{
			result = result + (int)(correction + base);
			base = base * inflation;
			inflation = ((level+1) * 0.002 + 0.8) * (inflation - 1) + 1;
		}
	} else /*Rpg2k3*/ {
		for (int i = 1; i <= level; i++)
		{
			result += (int)base;
			result += i * (int)inflation;
			result += (int)correction;
		}
	}
	return min(result, MaxExpValue());
}

void Game_Actor::MakeExpList() {
	exp_list.resize((size_t)GetMaxLevel());
	for (int i = 1; i < (int)exp_list.size(); ++i) {
		exp_list[i] = CalculateExp(i);
	}
}

std::string Game_Actor::GetExpString(bool status_scene) const {
	(void)status_scene;
	// RPG_RT displays dashes for max level. As a customization
	// we always display the amount of EXP.
	// if (GetNextExp() == -1) { return (MaxExpValue() >= 1000000 || status_scene) ? "-------" : "------"; }
	return std::to_string(GetExp());
}

std::string Game_Actor::GetNextExpString(bool status_scene) const {
	if (GetNextExp() == -1) {
		return (MaxExpValue() >= 1000000 || status_scene) ? "-------" : "------";
	}
	return std::to_string(GetNextExp());
}

int Game_Actor::GetBaseExp() const {
	return GetBaseExp(GetLevel());
}

int Game_Actor::GetBaseExp(int level) const {
	return GetNextExp(level - 1);
}

int Game_Actor::GetNextExp() const {
	return GetNextExp(GetLevel());
}

int Game_Actor::GetNextExp(int level) const {
	if (level >= GetMaxLevel() || level <= -1) {
		return -1;
	} else if (level == 0) {
		return 0;
	} else {
		return exp_list[level];
	}
}

int Game_Actor::GetStateProbability(int state_id) const {
	int rate = 2, mul = 100; // C - default

	const uint8_t* r = lcf::ReaderUtil::GetElement(dbActor->state_ranks, state_id);
	if (r) {
		rate = *r;
	}

	// This takes the armor of the character with the most resistance for that particular state
	for (const auto equipment : GetWholeEquipment()) {
		lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, equipment);
		if (item != nullptr
				&& !(Player::IsRPG2k3() && item->reverse_state_effect)
				&& (item->type == lcf::rpg::Item::Type_shield || item->type == lcf::rpg::Item::Type_armor
			|| item->type == lcf::rpg::Item::Type_helmet || item->type == lcf::rpg::Item::Type_accessory)
			&& state_id  <= static_cast<int>(item->state_set.size()) && item->state_set[state_id - 1]) {
			mul = std::min<int>(mul, 100 - item->state_chance);
		}
	}

	// GetStateRate verifies the state_id
	return GetStateRate(state_id, rate) * mul / 100;
}

int Game_Actor::GetBaseAttributeRate(int attribute_id) const {
	int rate = 2; // C - default

	const auto* r = lcf::ReaderUtil::GetElement(dbActor->attribute_ranks, attribute_id);
	if (r) {
		rate = *r;
	}

	bool boost = false;
	ForEachEquipment<false,true>(GetWholeEquipment(), [&](auto& item) {
			boost |= attribute_id >= 1 && attribute_id <= static_cast<int>(item.attribute_set.size()) && item.attribute_set[attribute_id - 1];
			});
	rate += boost;

	return Utils::Clamp(rate, 0, 4);
}

bool Game_Actor::IsImmuneToAttributeDownshifts() const {
	return dbActor->easyrpg_immune_to_attribute_downshifts;
}

int Game_Actor::GetWeaponId() const {
	int item_id = GetWholeEquipment()[0];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetShieldId() const {
	int item_id = GetWholeEquipment()[1];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetArmorId() const {
	int item_id = GetWholeEquipment()[2];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetHelmetId() const {
	int item_id = GetWholeEquipment()[3];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetAccessoryId() const {
	int item_id = GetWholeEquipment()[4];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetMaxLevel() const {
	int max_level = Player::IsRPG2k() ? max_level_2k : max_level_2k3;
	if (lcf::Data::system.easyrpg_max_level > -1) {
		max_level = lcf::Data::system.easyrpg_max_level;
	}
	return Utils::Clamp<int32_t>(max_level, 1, dbActor->final_level);
}

void Game_Actor::SetExp(int _exp) {
	data.exp = Utils::Clamp<int32_t>(_exp, 0, MaxExpValue());
}

void Game_Actor::ChangeExp(int exp, PendingMessage* pm) {
	int new_level = GetLevel();
	int new_exp = Utils::Clamp<int>(exp, 0, MaxExpValue());

	if (new_exp > GetExp()) {
		for (int i = GetLevel() + 1; i <= GetMaxLevel(); ++i) {
			if (GetNextExp(new_level) != -1 && GetNextExp(new_level) > new_exp) {
				break;
			}
			new_level++;
		}
	} else if (new_exp < GetExp()) {
		for (int i = GetLevel(); i > 1; --i) {
			if (new_exp >= GetNextExp(i - 1)) {
				break;
			}
			new_level--;
		}
	}

	SetExp(new_exp);

	if (new_level != GetLevel()) {
		ChangeLevel(new_level, pm);
	}
}

void Game_Actor::SetLevel(int _level) {
	data.level = Utils::Clamp(_level, 1, GetMaxLevel());
	// Ensure current HP/SP remain clamped if new Max HP/SP is less.
	SetHp(GetHp());
	SetSp(GetSp());

}

std::string Game_Actor::GetLevelUpMessage(int new_level) const {
	std::stringstream ss;
	if (Player::IsRPG2k3E()) {
		ss << GetName();
		ss << " " << lcf::Data::terms.level_up << " ";
		ss << " " << lcf::Data::terms.level << " " << new_level;
		return ss.str();
	} else if (Player::IsRPG2kE()) {
		ss << new_level;
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.level_up,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(GetName(), ss.str(), lcf::Data::terms.level)
		);
	} else {
		std::string particle, space = "";
		if (Player::IsCP932()) {
			particle = "は";
			space += " ";
		}
		else {
			particle = " ";
		}
		ss << GetName();
		ss << particle << lcf::Data::terms.level << " ";
		ss << new_level << space << lcf::Data::terms.level_up;
		return ss.str();
	}
}

std::string Game_Actor::GetLearningMessage(const lcf::rpg::Skill& skill) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.skill_learned,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(GetName(), skill.name)
		);
	}

	return ToString(skill.name) + (Player::IsRPG2k3E() ? " " : "") + ToString(lcf::Data::terms.skill_learned);
}

void Game_Actor::ChangeLevel(int new_level, PendingMessage* pm) {
	int old_level = GetLevel();
	SetLevel(new_level);
	new_level = GetLevel(); // Level adjusted to max

	if (new_level > old_level) {
		if (pm) {
			pm->PushLine(GetLevelUpMessage(new_level));
		}

		// Learn new skills
		LearnLevelSkills(old_level + 1, new_level, pm);

		if (pm) {
			pm->PushPageEnd();
		}

		// Experience adjustment:
		// At least level minimum
		SetExp(max(GetBaseExp(), GetExp()));
	} else if (new_level < old_level) {
		// Experience adjustment:
		// Level minimum if higher then Level maximum
		if (GetExp() >= GetNextExp()) {
			SetExp(GetBaseExp());
		}
	}
}

bool Game_Actor::IsEquippable(int item_id) const {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("IsEquippable: Invalid item ID {}", item_id);
		return false;
	}

	if (HasTwoWeapons() &&
		item->type == lcf::rpg::Item::Type_shield) {
			return false;
	}

	return IsItemUsable(item_id);
}

bool Game_Actor::IsEquipmentFixed(bool check_states) const {
	if (data.lock_equipment) {
		return true;
	}

	if (check_states) {
		for (auto state_id: GetInflictedStates()) {
			auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);
			if (state && state->cursed) {
				return true;
			}
		}
	}
	return false;
}

const lcf::rpg::Skill* Game_Actor::GetRandomSkill() const {
	const std::vector<int16_t>& skills = GetSkills();
	if (skills.empty()) {
		return nullptr;
	}

	// Skills are guaranteed to be valid
	return lcf::ReaderUtil::GetElement(lcf::Data::skills, skills[Rand::GetRandomNumber(0, skills.size() - 1)]);
}

Point Game_Actor::GetOriginalPosition() const {
	return { dbActor->battle_x, dbActor->battle_y };
}

StringView Game_Actor::GetSkillName() const {
	return dbActor->rename_skill ? StringView(dbActor->skill_name) : StringView(lcf::Data::terms.command_skill);
}

void Game_Actor::SetSprite(const std::string &file, int index, bool transparent) {
	if (file == dbActor->character_name
			&& index == dbActor->character_index
			&& transparent == dbActor->transparent) {
		data.sprite_name = "";
		data.sprite_id = 0;
		data.transparency = 0;
	} else {
		data.sprite_name = file;
		data.sprite_id = index;
		data.transparency = transparent ? 3 : 0;
	}
}

void Game_Actor::ChangeBattleCommands(bool add, int id) {
	auto& cmds = data.battle_commands;

	// If changing battle commands, that is when RPG_RT will replace the -1 list with a 'true' list.
	// Fetch original command array.
	if (!data.changed_battle_commands) {
		cmds = lcf::Data::actors[GetId() - 1].battle_commands;
		data.changed_battle_commands = true;
	}

	// The battle commands array always has a size of 7 padded with -1. The last element before the padding is 0 which
	// stands for the Row command
	if (add) {
		const lcf::rpg::BattleCommand* cmd = lcf::ReaderUtil::GetElement(lcf::Data::battlecommands.commands, id);
		if (!cmd) {
			Output::Warning("ChangeBattleCommands: Can't add invalid battle command {}", id);
			return;
		}

		if (std::find(cmds.begin(), cmds.end(), id)	== cmds.end()) {
			std::vector<int32_t> new_cmds;
			std::copy_if(cmds.begin(), cmds.end(),
						 std::back_inserter(new_cmds), [](int32_t i) { return i != 0 && i != -1; });
			// Needs space for at least 2 more commands (new command and row)
			if (new_cmds.size() >= 6) {
				return;
			}
			new_cmds.push_back(id);
			new_cmds.push_back(0);
			cmds = new_cmds;
		}
	} else if (id == 0) {
		cmds.clear();
		cmds.push_back(0);
	} else {
		std::vector<int32_t>::iterator it;
		it = std::find(cmds.begin(), cmds.end(), id);
		if (it != cmds.end())
			cmds.erase(it);
	}

	cmds.resize(7, -1);
}

const lcf::rpg::BattleCommand* Game_Actor::GetBattleCommand(int idx) const {
	Span<const int32_t> commands;
	if (data.changed_battle_commands) {
		commands = data.battle_commands;
	} else if (dbActor) {
		commands = dbActor->battle_commands;
	}
	int cmd_id = 0;
	if (idx >= 0 && idx < static_cast<int>(commands.size())) {
		cmd_id = commands[idx];
	}
	return lcf::ReaderUtil::GetElement(lcf::Data::battlecommands.commands, cmd_id);
}

const std::vector<const lcf::rpg::BattleCommand*> Game_Actor::GetBattleCommands() const {
	std::vector<const lcf::rpg::BattleCommand*> commands;
	std::vector<int32_t> obc = data.battle_commands;
	if (!data.changed_battle_commands) {
		// In this case, get it straight from the LDB.
		obc = lcf::Data::actors[GetId() - 1].battle_commands;
	}

	for (int command_index : obc) {
		if (command_index == 0) {
			// Row command -> not impl
			continue;
		}

		if (command_index == -1) {
			// Empty slot
			continue;
		}

		const lcf::rpg::BattleCommand* cmd = lcf::ReaderUtil::GetElement(lcf::Data::battlecommands.commands, command_index);
		if (!cmd) {
			Output::Warning("GetBattleCommands: Invalid battle command ID {}", command_index);
			continue;
		}

		commands.push_back(cmd);
	}

	return commands;
}

const lcf::rpg::Class* Game_Actor::GetClass() const {
	int id = data.class_id;

	if (id < 0) {
		// This means class ID hasn't been changed yet.
		id = dbActor->class_id;
	}

	return lcf::ReaderUtil::GetElement(lcf::Data::classes, id);
}

void Game_Actor::ChangeClass(int new_class_id,
		int new_level,
		ClassChangeSkillMode new_skill,
		ClassChangeParamMode new_param,
		PendingMessage* pm
		)
{
	const auto* cls = lcf::ReaderUtil::GetElement(lcf::Data::classes, new_class_id);
	if (new_class_id != 0 && cls == nullptr) {
		Output::Warning("Actor {}: Can't change to invalid class {}", GetId(), new_class_id);
		return;
	}

	// RPG_RT always removes all equipment on level change.
	RemoveWholeEquipment();

	const auto prev_level = GetLevel();
	const auto hp = GetHp();
	const auto sp = GetSp();

	auto max_hp = GetBaseMaxHp();
	auto max_sp = GetBaseMaxSp();
	auto atk = GetBaseAtk();
	auto def = GetBaseDef();
	auto spi = GetBaseSpi();
	auto agi = GetBaseAgi();

	SetLevel(1);
	data.hp_mod = 0;
	data.sp_mod = 0;
	data.attack_mod = 0;
	data.defense_mod = 0;
	data.spirit_mod = 0;
	data.agility_mod = 0;

	data.class_id = new_class_id;
	data.changed_battle_commands = true; // Any change counts as a battle commands change.

	// The class settings are not applied when the actor has a class on startup
	// but only when the "Change Class" event command is used.

	if (cls) {
		data.super_guard = cls->super_guard;
		data.lock_equipment = cls->lock_equipment;
		data.two_weapon = cls->two_weapon;
		data.auto_battle = cls->auto_battle;

		data.battler_animation = cls->battler_animation;

		data.battle_commands = cls->battle_commands;
	} else {
		data.super_guard = dbActor->super_guard;
		data.lock_equipment = dbActor->lock_equipment;
		data.two_weapon = dbActor->two_weapon;
		data.auto_battle = dbActor->auto_battle;

		data.battler_animation = 0;

		data.battle_commands = dbActor->battle_commands;
	}

	MakeExpList();

	switch (new_param) {
		case eParamNoChange:
			break;
		case eParamHalf:
			max_hp /= 2;
			max_sp /= 2;
			atk /= 2;
			def /= 2;
			spi /= 2;
			agi /= 2;
			break;
		case eParamResetLevel1:
			max_hp = GetBaseMaxHp();
			max_sp = GetBaseMaxSp();
			atk = GetBaseAtk();
			def = GetBaseDef();
			spi = GetBaseSpi();
			agi = GetBaseAgi();
			break;
		case eParamReset:
			break;
	}

	SetLevel(new_level);
	if (pm && new_level > 1 && (new_level > prev_level || new_skill != eSkillNoChange)) {
		pm->PushLine(GetLevelUpMessage(new_level));
	}

	// RPG_RT always resets EXP when class is changed, even if level unchanged.
	SetExp(GetBaseExp());

	if (new_param != eParamReset) {
		SetBaseMaxHp(max_hp);
		SetBaseMaxSp(max_sp);
		SetBaseAtk(atk);
		SetBaseDef(def);
		SetBaseSpi(spi);
		SetBaseAgi(agi);
	}

	SetHp(hp);
	SetSp(sp);

	switch (new_skill) {
		case eSkillNoChange:
			break;
		case eSkillReset:
			// RPG_RT has a bug where if (new_level == 1 && new_class_id == prev_class_id) no skills are removed.
			UnlearnAllSkills();
			// fall through
		case eSkillAdd:
			// RPG_RT has a bug where if (new_class_id == prev_class_id) level 1 skills are not learned.
			LearnLevelSkills(1, new_level, pm);
			break;
	}
}

StringView Game_Actor::GetClassName() const {
	if (!GetClass()) {
		return {};
	}
	return GetClass()->name;
}

static int ClampMaxHpMod(int hp, const Game_Actor* actor) {
	auto limit = actor->MaxHpValue();
	return Utils::Clamp(hp, -limit, limit);
}

static int ClampMaxSpMod(int sp, const Game_Actor* actor) {
	auto limit = actor->MaxSpValue();
	return Utils::Clamp(sp, -limit, limit);
}

static int ClampStatMod(int value, const Game_Actor* actor) {
	auto limit = actor->MaxStatBaseValue();
	return Utils::Clamp(value, -limit, limit);
}

void Game_Actor::SetBaseMaxHp(int maxhp) {
	int new_hp_mod = data.hp_mod + (maxhp - GetBaseMaxHp());
	data.hp_mod = ClampMaxHpMod(new_hp_mod, this);

	SetHp(data.current_hp);
}

void Game_Actor::SetBaseMaxSp(int maxsp) {
	int new_sp_mod = data.sp_mod + (maxsp - GetBaseMaxSp());
	data.sp_mod = ClampMaxSpMod(new_sp_mod, this);

	SetSp(data.current_sp);
}

int Game_Actor::SetHp(int hp) {
	data.current_hp = Utils::Clamp(hp, 0, GetMaxHp());
	return data.current_hp;
}

int Game_Actor::SetSp(int sp) {
	data.current_sp = Utils::Clamp(sp, 0, GetMaxSp());
	return data.current_sp;
}

void Game_Actor::SetBaseAtk(int atk) {
	int new_attack_mod = data.attack_mod + (atk - GetBaseAtk());
	data.attack_mod = ClampStatMod(new_attack_mod, this);
}

void Game_Actor::SetBaseDef(int def) {
	int new_defense_mod = data.defense_mod + (def - GetBaseDef());
	data.defense_mod = ClampStatMod(new_defense_mod, this);
}

void Game_Actor::SetBaseSpi(int spi) {
	int new_spirit_mod = data.spirit_mod + (spi - GetBaseSpi());
	data.spirit_mod = ClampStatMod(new_spirit_mod, this);
}

void Game_Actor::SetBaseAgi(int agi) {
	int new_agility_mod = data.agility_mod + (agi - GetBaseAgi());
	data.agility_mod = ClampStatMod(new_agility_mod, this);
}

Game_Actor::RowType Game_Actor::GetBattleRow() const {
	return RowType(data.row);
}

void Game_Actor::SetBattleRow(RowType battle_row) {
	data.row = int(battle_row);
}

int Game_Actor::GetBattleAnimationId() const {
	if (Player::IsRPG2k()) {
		return 0;
	}

	int anim = 0;

	if (data.battler_animation <= 0) {
		// Earlier versions of EasyRPG didn't save this value correctly

		// The battle animation of the class only matters when the class was
		// changed by event "Change Class"
		if ((data.class_id > 0) && GetClass()) {
			anim = GetClass()->battler_animation;
		} else {
			const lcf::rpg::BattlerAnimation* anima = lcf::ReaderUtil::GetElement(lcf::Data::battleranimations, dbActor->battler_animation);
			if (!anima) {
				Output::Warning("Actor {}: Invalid battle animation ID {}", GetId(), dbActor->battler_animation);
				return 0;
			}

			anim = anima->ID;
		}
	} else {
		anim = data.battler_animation;
	}

	if (anim == 0) {
		// Chunk was missing, set to proper default
		return 1;
	}

	return anim;
}

int Game_Actor::GetHitChance(Weapon weapon) const {
	int hit = INT_MIN;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { hit = std::max(hit, static_cast<int>(item.hit)); }, weapon);

	if (hit != INT_MIN) {
		return hit;
	} else {
		if (dbActor->easyrpg_unarmed_hit != -1) {
			return dbActor->easyrpg_unarmed_hit;
		} else {
			return 90;
		}
	}
}

float Game_Actor::GetCriticalHitChance(Weapon weapon) const {
	float crit_chance = dbActor->critical_hit ? 1.0f / dbActor->critical_hit_chance : 0.0f;

	float bonus = 0;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { bonus = std::max(bonus, static_cast<float>(item.critical_hit)); }, weapon);
	return crit_chance + (bonus / 100.0f);
}

int Game_Actor::IsControllable() const {
	return GetSignificantRestriction() == lcf::rpg::State::Restriction_normal && !GetAutoBattle();
}


void Game_Actor::RemoveInvalidData() {
	/*
	 The following actor data is cleaned up:
	 - Invalid equipment is removed
	 - An invalid class is removed
	 - Invalid states are removed
	 - Level is between 0 and 99, and does not exceed MaxLevel

	 For "external data" (not from LCF Actor or LSD SaveActor) the data is
	 verified in the corresponding functions.
	*/

	// Filter out invalid equipment
	int eq_types[] = { lcf::rpg::Item::Type_weapon,
		HasTwoWeapons() ? lcf::rpg::Item::Type_weapon : lcf::rpg::Item::Type_shield,
		lcf::rpg::Item::Type_armor,
		lcf::rpg::Item::Type_helmet,
		lcf::rpg::Item::Type_accessory
	};

	auto& equipment = GetWholeEquipment();
	for (size_t i = 0; i < equipment.size(); ++i) {
		int eq_id = equipment[i];
		lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, eq_id);

		if (!item && eq_id != 0) {
			Output::Debug("Actor {}: Removing invalid item {} from equipment slot {}",
			GetId(), eq_id, eq_types[i]);
			SetEquipment(i + 1, 0);
		} else if (item && item->type != eq_types[i]) {
			Output::Debug("Actor {}: Removing item {} (of type {}) from equipment slot {} (needs type {})",
			GetId(), item->ID, item->type, i + 1, eq_types[i]);
			SetEquipment(i + 1, 0);
		} else if (item && !IsItemUsable(item->ID)) {
			Output::Debug("Actor {}: Removing item {} from equipment slot {} (Not equippable by this actor)",
			GetId(), item->ID, i + 1);
			SetEquipment(i + 1, 0);
		}
	}

	// Remove invalid class
	if (data.class_id > 0) {
		const lcf::rpg::Class* cls = lcf::ReaderUtil::GetElement(lcf::Data::classes, data.class_id);
		if (!cls) {
			Output::Warning("Actor {}: Removing invalid class {}", GetId(), data.class_id);
			ChangeClass(0, GetLevel(), eSkillNoChange, eParamNoChange, nullptr);
		}
	}

	// Remove invalid skills
	for (int16_t skill_id : GetSkills()) {
		const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
		if (!skill) {
			Output::Warning("Actor {}: Removing invalid skill {}", GetId(), skill_id);
			UnlearnSkill(skill_id);
		}
	}

	// Remove invalid states
	if (GetStates().size() > lcf::Data::states.size()) {
		Output::Warning("Actor {}: State array contains invalid states ({} > {})", GetId(), GetStates().size(), lcf::Data::states.size());
		GetStates().resize(lcf::Data::states.size());
	}

	// Remove invalid levels
	// Special handling for the game COLORS: Lost Memories which uses level 0
	// through database editing. Hopefully no game uses negative levels.
	if (GetLevel() == 0) {
		Output::Debug("Actor {}: Special handling for level 0", GetId());
	} else if (GetLevel() < 0) {
		Output::Warning("Actor {}: Invalid level {}, changed to 1", GetId(), GetLevel());
		SetLevel(1);
	} else if (GetLevel() > GetMaxLevel()) {
		Output::Warning("Actor {}: Invalid level {}, changed to {}", GetId(), GetLevel(), GetMaxLevel());
		SetLevel(GetMaxLevel());
	}
}

const lcf::rpg::Item* Game_Actor::GetWeapon() const {
	auto* weapon = GetEquipment(lcf::rpg::Item::Type_weapon);
	if (weapon && weapon->type == lcf::rpg::Item::Type_weapon) {
		return weapon;
	}
	return nullptr;
}

const lcf::rpg::Item* Game_Actor::Get2ndWeapon() const {
	// Checking of HasTwoWeapons() not neccessary. If true, the
	// item equipped in this slot will never be a weapon from
	// legitimate means.
	auto* weapon = GetEquipment(lcf::rpg::Item::Type_shield);
	if (weapon && weapon->type == lcf::rpg::Item::Type_weapon) {
		return weapon;
	}
	return nullptr;
}

const lcf::rpg::Item* Game_Actor::GetShield() const {
	auto* shield = GetEquipment(lcf::rpg::Item::Type_shield);
	if (shield && shield->type == lcf::rpg::Item::Type_shield) {
		return shield;
	}
	return nullptr;
}

const lcf::rpg::Item* Game_Actor::GetArmor() const {
	auto* armor = GetEquipment(lcf::rpg::Item::Type_armor);
	if (armor && armor->type == lcf::rpg::Item::Type_armor) {
		return armor;
	}
	return nullptr;
}

const lcf::rpg::Item* Game_Actor::GetHelmet() const {
	auto* helmet = GetEquipment(lcf::rpg::Item::Type_helmet);
	if (helmet && helmet->type == lcf::rpg::Item::Type_helmet) {
		return helmet;
	}
	return nullptr;
}

const lcf::rpg::Item* Game_Actor::GetAccessory() const {
	auto* accessory = GetEquipment(lcf::rpg::Item::Type_accessory);
	if (accessory && accessory->type == lcf::rpg::Item::Type_accessory) {
		return accessory;
	}
	return nullptr;
}

bool Game_Actor::HasPreemptiveAttack(Weapon weapon) const {
	bool rc = false;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { rc |= item.preemptive; }, weapon);
	return rc;
}

int Game_Actor::GetNumberOfAttacks(Weapon weapon) const {
	if (GetWeapon() == nullptr && Get2ndWeapon() == nullptr && dbActor->easyrpg_dual_attack) {
		return 2;
	}
	int hits = 1;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { hits = std::max(hits, Algo::GetNumberOfAttacks(GetId(), item)); }, weapon);
	return hits;
}

bool Game_Actor::HasAttackAll(Weapon weapon) const {
	if (GetWeapon() == nullptr && Get2ndWeapon() == nullptr) {
		return dbActor->easyrpg_attack_all;
	}
	bool rc = false;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { rc |= item.attack_all; }, weapon);
	return rc;
}

bool Game_Actor::AttackIgnoresEvasion(Weapon weapon) const {
	if (GetWeapon() == nullptr && Get2ndWeapon() == nullptr) {
		return dbActor->easyrpg_ignore_evasion;
	}
	bool rc = false;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { rc |= item.ignore_evasion; }, weapon);
	return rc;
}

bool Game_Actor::PreventsCritical() const {
	if (dbActor->easyrpg_prevent_critical) {
		return true;
	}
	bool rc = false;
	ForEachEquipment<false, true>(GetWholeEquipment(), [&](auto& item) { rc |= item.prevent_critical; });
	return rc;
}

bool Game_Actor::PreventsTerrainDamage() const {
	bool rc = false;
	ForEachEquipment<false, true>(GetWholeEquipment(), [&](auto& item) { rc |= item.no_terrain_damage; });
	return rc;
}

bool Game_Actor::HasPhysicalEvasionUp() const {
	if (dbActor->easyrpg_raise_evasion) {
		return true;
	}
	bool rc = false;
	ForEachEquipment<false, true>(GetWholeEquipment(), [&](auto& item) { rc |= item.raise_evasion; });
	return rc;
}

bool Game_Actor::HasHalfSpCost() const {
	bool rc = false;
	ForEachEquipment<false, true>(GetWholeEquipment(), [&](auto& item) { rc |= item.half_sp_cost; });
	return rc;
}

int Game_Actor::CalculateWeaponSpCost(Weapon weapon) const {
	int cost = 0;
	ForEachEquipment<true, false>(GetWholeEquipment(), [&](auto& item) { cost += item.sp_cost; }, weapon);
	if (HasHalfSpCost()) {
		cost = (cost + 1) / 2;
	}

	return cost;
}

void Game_Actor::AdjustEquipmentStates(const lcf::rpg::Item* item, bool add, bool allow_battle_states) {
	// All states inflicted by new armor get inflicted.
	if (Player::IsRPG2k3()
			&& item
			&& IsArmorType(item)
			&& item->reverse_state_effect)
	{
		auto& states = item->state_set;
		for (int i = 0; i < (int)states.size(); ++i) {
			if (states[i]) {
				if (add) {
					AddState(i + 1, allow_battle_states);
				} else {
					RemoveState(i + 1, false);
				}
			}
		}
	}
}


void Game_Actor::ResetEquipmentStates(bool allow_battle_states) {
	AdjustEquipmentStates(GetShield(), true, allow_battle_states);
	AdjustEquipmentStates(GetArmor(), true, allow_battle_states);
	AdjustEquipmentStates(GetHelmet(), true, allow_battle_states);
	AdjustEquipmentStates(GetAccessory(), true, allow_battle_states);
}

PermanentStates Game_Actor::GetPermanentStates() const {
	PermanentStates ps;
	if (!Player::IsRPG2k3()) {
		return ps;
	}

	auto addEquip = [&](const lcf::rpg::Item* item) {
		if (!item || !IsArmorType(item) || !item->reverse_state_effect) {
			return;
		}
		auto& states = item->state_set;
		// Invalid states in armor already reported earlier in
		// calls to AdjustEquipmentStates.
		int num_states = std::min<int>(states.size(), lcf::Data::states.size());
		for (int i = 0; i < num_states; ++i) {
			if (states[i]) {
				ps.Add(i + 1);
			}
		}
	};

	addEquip(GetShield());
	addEquip(GetArmor());
	addEquip(GetHelmet());
	addEquip(GetAccessory());

	return ps;
}

bool Game_Actor::IsInParty() const {
	return Main_Data::game_party->IsActorInParty(GetId());
}

std::array<const lcf::rpg::Item*, 2> Game_Actor::GetWeapons(Game_Battler::Weapon weapon) const {
	std::array<const lcf::rpg::Item*, 2> w = {{}};
	int i = 0;
	if (weapon == Game_Battler::WeaponPrimary || weapon == Game_Battler::WeaponAll) {
		w[i] = GetWeapon();
		if (w[i]) { ++i; }
	}
	if (weapon == Game_Battler::WeaponSecondary || weapon == Game_Battler::WeaponAll) {
		w[i] = Get2ndWeapon();
	}
	return w;
}


void Game_Actor::UpdateBattle() {
	Game_Battler::UpdateBattle();
	auto* sprite = GetActorBattleSprite();
	if (sprite) {
		sprite->Update();
	}
	auto* weapon = Game_Battler::GetWeaponSprite();
	if (weapon) {
		weapon->Update();
	}
}
