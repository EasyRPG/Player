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
#include "game_message.h"
#include "game_party.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <lcf/rpg/skill.h>
#include "util_macro.h"
#include "utils.h"
#include "pending_message.h"
#include "compiler.h"

constexpr int max_level_2k = 50;
constexpr int max_level_2k3 = 99;

static int max_exp_value() {
	return Player::IsRPG2k() ? 999999 : 9999999;
}

int Game_Actor::MaxHpValue() const {
	return Player::IsRPG2k() ? 999 : 9999;
}

int Game_Actor::MaxStatBattleValue() const {
	return Player::IsRPG2k() ? 999 : 9999;
}

int Game_Actor::MaxStatBaseValue() const {
	return 999;
}

Game_Actor::Game_Actor(int actor_id) :
	Game_Battler(),
	actor_id(actor_id) {
	GetData().Setup(actor_id);
	Setup();
}

void Game_Actor::Setup() {
	MakeExpList();
}

void Game_Actor::Init() {
	const std::vector<lcf::rpg::Learning>& skills = GetActor().skills;
	for (int i = 0; i < (int)skills.size(); i++) {
		if (skills[i].level <= GetLevel()) {
			LearnSkill(skills[i].skill_id, nullptr);
		}
	}

	RemoveInvalidData();

	if (GetLevel() > 0) {
		SetHp(GetMaxHp());
		SetSp(GetMaxSp());
		SetExp(exp_list[GetLevel() - 1]);
	}

	ResetEquipmentStates(false);
}

void Game_Actor::Fixup() {
	GetData().Fixup(actor_id);
	if (Player::IsRPG2k()) {
		auto& actor = GetActor();
		GetData().two_weapon = actor.two_weapon;
		GetData().lock_equipment = actor.lock_equipment;
		GetData().auto_battle = actor.auto_battle;
		GetData().super_guard = actor.super_guard;
	}

	RemoveInvalidData();

	ResetEquipmentStates(false);

	MakeExpList();
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

	int query_idx = actor_id - 1;
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
	return query_set->at(query_idx);
}

bool Game_Actor::IsSkillLearned(int skill_id) const {
	return std::find(GetData().skills.begin(), GetData().skills.end(), skill_id) != GetData().skills.end();
}

bool Game_Actor::IsSkillUsable(int skill_id) const {
	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("IsSkillUsable: Invalid skill ID {}", skill_id);
		return false;
	}

	// Actor must have all attributes of the skill equipped as weapons
	const lcf::rpg::Item* item = GetEquipment(lcf::rpg::Item::Type_weapon);
	const lcf::rpg::Item* item2 = HasTwoWeapons() ? GetEquipment(lcf::rpg::Item::Type_weapon + 1) : nullptr;

	for (size_t i = 0; i < skill->attribute_effects.size(); ++i) {
		bool required = skill->attribute_effects[i] && lcf::Data::attributes[i].type == lcf::rpg::Attribute::Type_physical;
		if (required) {
			if (item && i < item->attribute_set.size() && item->attribute_set[i]) {
				continue;
			}
			if (item2 && i < item2->attribute_set.size() && item2->attribute_set[i]) {
				continue;
			}
			return false;
		}
	}

	return Game_Battler::IsSkillUsable(skill_id);
}

int Game_Actor::CalculateSkillCost(int skill_id) const {
	int cost = Game_Battler::CalculateSkillCost(skill_id);
	if (HasHalfSpCost()) {
		cost = (cost + 1) / 2;
	}
	return cost;
}

int Game_Actor::CalculateWeaponSpCost() const {
	int cost = 0;
	auto* w1 = GetWeapon();
	if (w1) {
		cost += w1->sp_cost;
	}
	auto* w2 = Get2ndWeapon();
	if (w2) {
		cost += w2->sp_cost;
	}

	if (HasHalfSpCost()) {
		cost = (cost + 1) / 2;
	}

	return cost;
}

bool Game_Actor::LearnSkill(int skill_id, PendingMessage* pm) {
	if (skill_id > 0 && !IsSkillLearned(skill_id)) {
		const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
		if (!skill) {
			Output::Warning("Actor {}: Can't learn invalid skill {}", GetId(), skill_id);
			return false;
		}

		GetData().skills.push_back((int16_t)skill_id);
		std::sort(GetData().skills.begin(), GetData().skills.end());

		if (pm) {
			pm->PushLine(GetLearningMessage(*skill));
		}

		return true;
	}
	return false;
}

int Game_Actor::LearnLevelSkills(int min_level, int max_level, PendingMessage* pm) {
	auto& skills = GetData().class_id > 0 ? GetClass()->skills : GetActor().skills;

	int count = 0;

	// Learn new skills
	for (const lcf::rpg::Learning& learn : skills) {
		// Skill learning, up to current level
		if (learn.level >= min_level && learn.level <= max_level) {
			count += LearnSkill(learn.skill_id, pm);
		}
	}
	return count;
}

bool Game_Actor::UnlearnSkill(int skill_id) {
	std::vector<int16_t>::iterator it = std::find(GetData().skills.begin(), GetData().skills.end(), skill_id);
	if (it != GetData().skills.end()) {
		GetData().skills.erase(it);
		return true;
	}
	return false;
}

void Game_Actor::UnlearnAllSkills() {
	GetData().skills.clear();
}

void Game_Actor::SetFace(const std::string& file_name, int index) {
	GetData().face_name.assign(file_name);
	GetData().face_id = index;
}

const lcf::rpg::Item* Game_Actor::GetEquipment(int equip_type) const {
	if (equip_type <= 0 || equip_type > (int)GetData().equipped.size())
		return nullptr;

	int item_id = GetData().equipped[equip_type - 1];
	return lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
}

int Game_Actor::SetEquipment(int equip_type, int new_item_id) {
	if (equip_type <= 0 || equip_type > (int) GetData().equipped.size())
		return -1;

	int old_item_id = GetData().equipped[equip_type - 1];
	const lcf::rpg::Item* old_item = lcf::ReaderUtil::GetElement(lcf::Data::items, old_item_id);

	const lcf::rpg::Item* new_item = lcf::ReaderUtil::GetElement(lcf::Data::items, new_item_id);
	if (new_item_id != 0 && !new_item) {
		Output::Warning("SetEquipment: Can't equip item with invalid ID {}", new_item_id);
		new_item_id = 0;
	}

	GetData().equipped[equip_type - 1] = (short)new_item_id;

	AdjustEquipmentStates(old_item, false, false);
	AdjustEquipmentStates(new_item, true, false);

	return old_item_id;
}

void Game_Actor::ChangeEquipment(int equip_type, int item_id) {
	int prev_item = SetEquipment(equip_type, item_id);

	if (prev_item != 0) {
		Main_Data::game_party->AddItem(prev_item, 1);
	}
	if (item_id != 0) {
		Main_Data::game_party->RemoveItem(item_id, 1);
	}

	// In case you have a two_handed weapon equipped, the other weapon is removed.
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, GetWeaponId());
	const lcf::rpg::Item* item2 = lcf::ReaderUtil::GetElement(lcf::Data::items, GetShieldId());
	if (item && item2 && (item->two_handed || item2->two_handed)) {
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

	if (item_id > 0) {
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
	ChangeHp(GetMaxHp());
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
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.maxhp, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.maxhp, GetLevel());
	}

	if (mod)
		n += GetData().hp_mod;

	return Utils::Clamp(n, 1, MaxHpValue());
}

int Game_Actor::GetBaseMaxHp() const {
	return GetBaseMaxHp(true);
}

int Game_Actor::GetBaseMaxSp(bool mod) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.maxsp, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.maxsp, GetLevel());
	}

	if (mod)
		n += GetData().sp_mod;

	return Utils::Clamp(n, 0, MaxStatBaseValue());
}

int Game_Actor::GetBaseMaxSp() const {
	return GetBaseMaxSp(true);
}

int Game_Actor::GetBaseAtk(bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.attack, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.attack, GetLevel());
	}

	if (mod) {
		n += GetData().attack_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)lcf::Data::items.size()) {
				// Invalid equipment was removed
				n += lcf::ReaderUtil::GetElement(lcf::Data::items, *it)->atk_points1;
			}
		}
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseAtk() const {
	return GetBaseAtk(true, true);
}

int Game_Actor::GetBaseDef(bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.defense, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.defense, GetLevel());
	}

	if (mod) {
		n += GetData().defense_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)lcf::Data::items.size()) {
				// Invalid equipment was removed
				n += lcf::ReaderUtil::GetElement(lcf::Data::items, *it)->def_points1;
			}
		}
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseDef() const {
	return GetBaseDef(true, true);
}

int Game_Actor::GetBaseSpi(bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.spirit, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.spirit, GetLevel());
	}

	if (mod) {
		n += GetData().spirit_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)lcf::Data::items.size()) {
				// Invalid equipment was removed
				n += lcf::ReaderUtil::GetElement(lcf::Data::items, *it)->spi_points1;
			}
		}
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseSpi() const {
	return GetBaseSpi(true, true);
}

int Game_Actor::GetBaseAgi(bool mod, bool equip) const {
	int n = 0;
	if (GetLevel() > 0) {
		n = GetData().class_id > 0
			? *lcf::ReaderUtil::GetElement(GetClass()->parameters.agility, GetLevel())
			: *lcf::ReaderUtil::GetElement(GetActor().parameters.agility, GetLevel());
	}

	if (mod) {
		n += GetData().agility_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)lcf::Data::items.size()) {
				// Invalid equipment was removed
				n += lcf::ReaderUtil::GetElement(lcf::Data::items, *it)->agi_points1;
			}
		}
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseAgi() const {
	return GetBaseAgi(true, true);
}

int Game_Actor::CalculateExp(int level) const {
	const lcf::rpg::Class* klass = lcf::ReaderUtil::GetElement(lcf::Data::classes, GetData().class_id);

	double base, inflation, correction;
	if (klass) {
		base = klass->exp_base;
		inflation = klass->exp_inflation;
		correction = klass->exp_correction;
	}
	else {
		const lcf::rpg::Actor& actor = *lcf::ReaderUtil::GetElement(lcf::Data::actors, actor_id);
		base = actor.exp_base;
		inflation = actor.exp_inflation;
		correction = actor.exp_correction;
	}

	int result = 0;
	if (Player::IsRPG2k()) {
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
	return min(result, max_exp_value());
}

void Game_Actor::MakeExpList() {
	exp_list.resize((size_t)GetMaxLevel());
	for (int i = 1; i < (int)exp_list.size(); ++i) {
		exp_list[i] = CalculateExp(i);
	}
}

std::string Game_Actor::GetExpString() const {
	// RPG_RT displays dashes for max level. As a customization
	// we always display the amount of EXP.
	// if (GetNextExp() == -1) { return Player::IsRPG2k3() ? "-------" : "------"; }
	return std::to_string(GetExp());
}

std::string Game_Actor::GetNextExpString() const {
	if (GetNextExp() == -1) {
		return Player::IsRPG2k3() ? "-------" : "------";
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

	const uint8_t* r = lcf::ReaderUtil::GetElement(GetActor().state_ranks, state_id);
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

int Game_Actor::GetAttributeModifier(int attribute_id) const {
	int rate = 2; // C - default

	const uint8_t* r = lcf::ReaderUtil::GetElement(GetActor().attribute_ranks, attribute_id);
	if (r) {
		rate = *r;
	}

	// GetAttributeRate will verify this but actors already need a check earlier
	// because of attribute_shift
	const int* shift = lcf::ReaderUtil::GetElement(attribute_shift, attribute_id);

	if (!shift) {
		Output::Warning("GetAttributeModifier: Invalid attribute ID {}", attribute_id);
		return 0;
	}

	rate += *shift;
	for (auto id_object : GetWholeEquipment()) {
		lcf::rpg::Item *object = lcf::ReaderUtil::GetElement(lcf::Data::items, id_object);
		if (object != nullptr && (object->type == lcf::rpg::Item::Type_shield || object->type == lcf::rpg::Item::Type_armor
			|| object->type == lcf::rpg::Item::Type_helmet || object->type == lcf::rpg::Item::Type_accessory)
			&& static_cast<int>(object->attribute_set.size()) >= attribute_id && object->attribute_set[attribute_id - 1]) {
			rate++;
			break;
		}
	}

	if (rate < 0) {
		rate = 0;
	} else if (rate > 4) {
		rate = 4;
	}

	return GetAttributeRate(attribute_id, rate);
}

int Game_Actor::GetWeaponId() const {
	int item_id = GetData().equipped[0];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetShieldId() const {
	int item_id = GetData().equipped[1];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetArmorId() const {
	int item_id = GetData().equipped[2];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetHelmetId() const {
	int item_id = GetData().equipped[3];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetAccessoryId() const {
	int item_id = GetData().equipped[4];
	return item_id <= (int)lcf::Data::items.size() ? item_id : 0;
}

int Game_Actor::GetMaxLevel() const {
	return std::max<int32_t>(1, std::min<int32_t>(GetActor().final_level, Player::IsRPG2k() ? max_level_2k : max_level_2k3));
}

void Game_Actor::SetExp(int _exp) {
	GetData().exp = min(max(_exp, 0), max_exp_value());
}

void Game_Actor::ChangeExp(int exp, PendingMessage* pm) {
	int new_level = GetLevel();
	int new_exp = min(max(exp, 0), max_exp_value());

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
	GetData().level = min(max(_level, 1), GetMaxLevel());
	// Ensure current HP/SP remain clamped if new Max HP/SP is less.
	SetHp(GetHp());
	SetSp(GetSp());

}

std::string Game_Actor::GetLevelUpMessage(int new_level) const {
	std::stringstream ss;
	if (Player::IsRPG2k3E()) {
		ss << GetData().name;
		ss << " " << lcf::Data::terms.level_up << " ";
		ss << " " << lcf::Data::terms.level << " " << new_level;
		return ss.str();
	} else if (Player::IsRPG2kE()) {
		ss << new_level;
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.level_up,
			{'S', 'V', 'U'},
			{GetData().name, ss.str(), lcf::Data::terms.level}
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
		ss << GetData().name;
		ss << particle << lcf::Data::terms.level << " ";
		ss << new_level << space << lcf::Data::terms.level_up;
		return ss.str();
	}
}

std::string Game_Actor::GetLearningMessage(const lcf::rpg::Skill& skill) const {
	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.skill_learned,
			{'S', 'O'},
			{GetData().name, skill.name}
		);
	}

	return skill.name + (Player::IsRPG2k3E() ? " " : "") + lcf::Data::terms.skill_learned;
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

bool Game_Actor::IsEquipmentFixed() const {
	if (GetData().lock_equipment) {
		return true;
	}

	for (auto state_id: GetInflictedStates()) {
		auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);
		if (state && state->cursed) {
			return true;
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
	return lcf::ReaderUtil::GetElement(lcf::Data::skills, skills[Utils::GetRandomNumber(0, skills.size() - 1)]);
}

int Game_Actor::GetBattleX() const {
	float position = 0.0;

	if (GetActor().battle_x == 0 ||
		lcf::Data::battlecommands.placement == lcf::rpg::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float left = GetBattleRow() == RowType::RowType_back ? 25.0 : 50.0;
		float right = left;

		const lcf::rpg::Terrain* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, Game_Battle::GetTerrainId());
		if (terrain) {
			// No warning, already reported on battle start
			right = left + terrain->grid_inclination / 1103;
		}

		switch (party_size) {
		case 1:
			position = left + ((right - left) / 2);
			break;
		case 2:
			switch (party_pos) {
			case 0:
				position = right;
				break;
			case 1:
				position = left;
				break;
			}
			break;
		case 3:
			switch (party_pos) {
			case 0:
				position = right;
				break;
			case 1:
				position = left + ((right - left) / 2);
				break;
			case 2:
				position = left;
				break;
			}
			break;
		case 4:
			switch (party_pos) {
			case 0:
				position = right;
				break;
			case 1:
				position = left + ((right - left) * 2.0/3);
				break;
			case 2:
				position = left + ((right - left) * 1.0/3);
				break;
			case 3:
				position = left;
				break;
			}
			break;
		}

		switch (Game_Battle::GetBattleCondition()) {
			case lcf::rpg::System::BattleCondition_none:
			case lcf::rpg::System::BattleCondition_initiative:
				return SCREEN_TARGET_WIDTH - position;
			case lcf::rpg::System::BattleCondition_back:
				return position;
			case lcf::rpg::System::BattleCondition_surround:
			case lcf::rpg::System::BattleCondition_pincers:
				// ToDo: Correct position
				return SCREEN_TARGET_WIDTH - position;
		}
	}
	else {
		// Output::Debug("{} {} {} {}", lcf::Data::terrains[0].grid_top_y, lcf::Data::terrains[0].grid_elongation, lcf::Data::terrains[0].grid_inclination, lcf::Data::terrains[0].grid_location);

		position = GetActor().battle_x * SCREEN_TARGET_WIDTH / 320;
	}

	return position;
}

int Game_Actor::GetBattleY() const {
	float position = 0.0;

	if (GetActor().battle_y == 0 ||
		lcf::Data::battlecommands.placement == lcf::rpg::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float top = 0.0f;
		float bottom = 0.0f;
		const lcf::rpg::Terrain* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, Game_Battle::GetTerrainId());
		if (terrain) {
			// No warning, already reported on battle start
			top = terrain->grid_top_y;
			bottom = top + terrain->grid_elongation / 13;
		}

		switch (party_size) {
		case 1:
			position = top + ((bottom - top) / 2);
			break;
		case 2:
			switch (party_pos) {
			case 0:
				position = top;
				break;
			case 1:
				position = bottom;
				break;
			}
			break;
		case 3:
			switch (party_pos) {
			case 0:
				position = top;
				break;
			case 1:
				position = top + ((bottom - top) / 2);
				break;
			case 2:
				position = bottom;
				break;
			}
			break;
		case 4:
			switch (party_pos) {
			case 0:
				position = top;
				break;
			case 1:
				position = top + ((bottom - top) * 1.0 / 3);
				break;
			case 2:
				position = top + ((bottom - top) * 2.0 / 3);
				break;
			case 3:
				position = bottom;
				break;
			}
			break;
		}

		position -= 24;
	}
	else {
		position = GetActor().battle_y * SCREEN_TARGET_HEIGHT / 240;
	}

	return (int)position;
}

const std::string& Game_Actor::GetSkillName() const {
	auto& a = GetActor();
	return a.rename_skill ? a.skill_name : lcf::Data::terms.command_skill;
}

void Game_Actor::SetSprite(const std::string &file, int index, bool transparent) {
	GetData().sprite_name = file;
	GetData().sprite_id = index;
	GetData().transparency = transparent ? 3 : 0;
}

void Game_Actor::ChangeBattleCommands(bool add, int id) {
	auto& cmds = GetData().battle_commands;

	// If changing battle commands, that is when RPG_RT will replace the -1 list with a 'true' list.
	// Fetch original command array.
	if (!GetData().changed_battle_commands) {
		cmds = lcf::Data::actors[GetId() - 1].battle_commands;
		GetData().changed_battle_commands = true;
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
			std::sort(new_cmds.begin(), new_cmds.end());
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

const std::vector<const lcf::rpg::BattleCommand*> Game_Actor::GetBattleCommands() const {
	std::vector<const lcf::rpg::BattleCommand*> commands;
	std::vector<int32_t> obc = GetData().battle_commands;
	if (!GetData().changed_battle_commands) {
		// In this case, get it straight from the LDB.
		obc = lcf::Data::actors[actor_id - 1].battle_commands;
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
	int id = GetData().class_id;

	if (id < 0) {
		// This means class ID hasn't been changed yet.
		id = GetActor().class_id;
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
	GetData().hp_mod = 0;
	GetData().sp_mod = 0;
	GetData().attack_mod = 0;
	GetData().defense_mod = 0;
	GetData().spirit_mod = 0;
	GetData().agility_mod = 0;

	GetData().class_id = new_class_id;
	GetData().changed_battle_commands = true; // Any change counts as a battle commands change.

	// The class settings are not applied when the actor has a class on startup
	// but only when the "Change Class" event command is used.

	if (cls) {
		GetData().super_guard = cls->super_guard;
		GetData().lock_equipment = cls->lock_equipment;
		GetData().two_weapon = cls->two_weapon;
		GetData().auto_battle = cls->auto_battle;

		GetData().battler_animation = cls->battler_animation;

		GetData().battle_commands = cls->battle_commands;
	} else {
		GetData().super_guard = GetActor().super_guard;
		GetData().lock_equipment = GetActor().lock_equipment;
		GetData().two_weapon = GetActor().two_weapon;
		GetData().auto_battle = GetActor().auto_battle;

		GetData().battler_animation = 0;

		GetData().battle_commands = GetActor().battle_commands;
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

std::string Game_Actor::GetClassName() const {
    if (!GetClass()) {
        return "";
    }
    return GetClass()->name;
}

static int ClampMaxHpMod(int hp, const Game_Actor* actor) {
	auto limit = actor->MaxHpValue();
	return Utils::Clamp(hp, -limit, limit);
}

static int ClampStatMod(int value, const Game_Actor* actor) {
	auto limit = actor->MaxStatBaseValue();
	return Utils::Clamp(value, -limit, limit);
}

void Game_Actor::SetBaseMaxHp(int maxhp) {
	int new_hp_mod = GetData().hp_mod + (maxhp - GetBaseMaxHp());
	GetData().hp_mod = ClampMaxHpMod(new_hp_mod, this);

	SetHp(GetData().current_hp);
}

void Game_Actor::SetBaseMaxSp(int maxsp) {
	int new_sp_mod = GetData().sp_mod + (maxsp - GetBaseMaxSp());
	GetData().sp_mod = ClampStatMod(new_sp_mod, this);

	SetSp(GetData().current_sp);
}

void Game_Actor::SetHp(int hp) {
	GetData().current_hp = Utils::Clamp(hp, 0, GetMaxHp());
}

void Game_Actor::SetSp(int sp) {
	GetData().current_sp = Utils::Clamp(sp, 0, GetMaxSp());
}

void Game_Actor::SetBaseAtk(int atk) {
	int new_attack_mod = GetData().attack_mod + (atk - GetBaseAtk());
	GetData().attack_mod = ClampStatMod(new_attack_mod, this);
}

void Game_Actor::SetBaseDef(int def) {
	int new_defense_mod = GetData().defense_mod + (def - GetBaseDef());
	GetData().defense_mod = ClampStatMod(new_defense_mod, this);
}

void Game_Actor::SetBaseSpi(int spi) {
	int new_spirit_mod = GetData().spirit_mod + (spi - GetBaseSpi());
	GetData().spirit_mod = ClampStatMod(new_spirit_mod, this);
}

void Game_Actor::SetBaseAgi(int agi) {
	int new_agility_mod = GetData().agility_mod + (agi - GetBaseAgi());
	GetData().agility_mod = ClampStatMod(new_agility_mod, this);
}

Game_Actor::RowType Game_Actor::GetBattleRow() const {
	return RowType(GetData().row);
}

void Game_Actor::SetBattleRow(RowType battle_row) {
	GetData().row = int(battle_row);
}

int Game_Actor::GetBattleAnimationId() const {
	if (Player::IsRPG2k()) {
		return 0;
	}

	int anim = 0;

	if (GetData().battler_animation <= 0) {
		// Earlier versions of EasyRPG didn't save this value correctly

		// The battle animation of the class only matters when the class was
		// changed by event "Change Class"
		if ((GetData().class_id > 0) && GetClass()) {
			anim = GetClass()->battler_animation;
		} else {
			const lcf::rpg::BattlerAnimation* anima = lcf::ReaderUtil::GetElement(lcf::Data::battleranimations, GetActor().battler_animation);
			if (!anima) {
				Output::Warning("Actor {}: Invalid battle animation ID {}", GetId(), GetActor().battler_animation);
				return 0;
			}

			anim = anima->ID;
		}
	} else {
		anim = GetData().battler_animation;
	}

	if (anim == 0) {
		// Chunk was missing, set to proper default
		return 1;
	}

	return anim;
}

int Game_Actor::GetHitChance() const {
	auto* weapon1 = GetWeapon();
	auto* weapon2 = Get2ndWeapon();
	if (weapon1 && weapon2) {
		return std::max(weapon1->hit, weapon2->hit);
	} else if(weapon1) {
		return weapon1->hit;
	} else if(weapon2) {
		return weapon2->hit;
	}
	return 90;
}

float Game_Actor::GetCriticalHitChance() const {
	auto& actor = GetActor();
	float crit_chance = actor.critical_hit ? 1.0f / actor.critical_hit_chance : 0.0f;

	float weapon_bonus = 0;

	auto checkWeapon = [&](const lcf::rpg::Item* weapon) {
		if (weapon) {
			weapon_bonus = std::max(weapon_bonus, float(weapon->critical_hit));
		}
	};

	checkWeapon(GetWeapon());
	checkWeapon(Get2ndWeapon());

	return crit_chance + (weapon_bonus / 100.0f);
}

int Game_Actor::IsControllable() const {
	return GetSignificantRestriction() == lcf::rpg::State::Restriction_normal && !GetAutoBattle();
}


const lcf::rpg::Actor& Game_Actor::GetActor() const {
	// Always valid
	return *lcf::ReaderUtil::GetElement(lcf::Data::actors, actor_id);
}

lcf::rpg::SaveActor& Game_Actor::GetData() const {
	// Always valid because the array is resized to match actor size
	return *lcf::ReaderUtil::GetElement(Main_Data::game_data.actors, actor_id);
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
	if (GetData().class_id > 0) {
		const lcf::rpg::Class* cls = lcf::ReaderUtil::GetElement(lcf::Data::classes, GetData().class_id);
		if (!cls) {
			Output::Warning("Actor {}: Removing invalid class {}", GetId(), GetData().class_id);
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

bool Game_Actor::HasPreemptiveAttack() const {
	auto* w1 = GetWeapon();
	auto* w2 = Get2ndWeapon();
	return (w1 && w1->preemptive) || (w2 && w2->preemptive);
}

bool Game_Actor::HasDualAttack() const {
	auto* w1 = GetWeapon();
	auto* w2 = Get2ndWeapon();
	return (w1 && w1->dual_attack) || (w2 && w2->dual_attack);
}

bool Game_Actor::HasAttackAll() const {
	auto* w1 = GetWeapon();
	auto* w2 = Get2ndWeapon();
	return (w1 && w1->attack_all) || (w2 && w2->attack_all);
}


bool Game_Actor::AttackIgnoresEvasion() const {
	auto* w1 = GetWeapon();
	auto* w2 = Get2ndWeapon();
	return (w1 && w1->ignore_evasion) || (w2 && w2->ignore_evasion);
}

bool Game_Actor::PreventsCritical() const {
	auto checkEquip = [](const lcf::rpg::Item* item) {
		return item && item->prevent_critical;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::PreventsTerrainDamage() const {
	auto checkEquip = [](const lcf::rpg::Item* item) {
		return item && item->no_terrain_damage;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::HasPhysicalEvasionUp() const {
	auto checkEquip = [](const lcf::rpg::Item* item) {
		return item && item->raise_evasion;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::HasHalfSpCost() const {
	auto checkEquip = [](const lcf::rpg::Item* item) {
		return item && item->half_sp_cost;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}


static bool IsArmorType(const lcf::rpg::Item* item) {
	return item->type == lcf::rpg::Item::Type_shield
		|| item->type == lcf::rpg::Item::Type_armor
		|| item->type == lcf::rpg::Item::Type_helmet
		|| item->type == lcf::rpg::Item::Type_accessory;
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
		int num_states = std::min(states.size(), lcf::Data::states.size());
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
