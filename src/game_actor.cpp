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
#include "reader_util.h"
#include "rpg_skill.h"
#include "util_macro.h"
#include "utils.h"

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
	const std::vector<RPG::Learning>& skills = GetActor().skills;
	for (int i = 0; i < (int)skills.size(); i++) {
		if (skills[i].level <= GetLevel()) {
			LearnSkill(skills[i].skill_id);
		}
	}

	RemoveInvalidData();

	if (GetLevel() > 0) {
		SetHp(GetMaxHp());
		SetSp(GetMaxSp());
		SetExp(exp_list[GetLevel() - 1]);
	}
}

void Game_Actor::Fixup() {
	GetData().Fixup(actor_id);

	RemoveInvalidData();
}

int Game_Actor::GetId() const {
	return actor_id;
}

bool Game_Actor::UseItem(int item_id, const Game_Battler* source) {
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use invalid item %d", item_id);
		return false;
	}

	if (IsDead() && item->type != RPG::Item::Type_medicine) {
		return false;
	}

	if (item->type == RPG::Item::Type_book) {
		return LearnSkill(item->skill_id);
	}

	if (item->type == RPG::Item::Type_material) {
		SetBaseMaxHp(GetBaseMaxHp() + item->max_hp_points);
		SetBaseMaxSp(GetBaseMaxSp() + item->max_sp_points);
		SetBaseAtk(GetBaseAtk() + item->atk_points2);
		SetBaseDef(GetBaseDef() + item->def_points2);
		SetBaseAgi(GetBaseAgi() + item->agi_points2);
		SetBaseSpi(GetBaseSpi() + item->spi_points2);

		return true;
	}

	return Game_Battler::UseItem(item_id, source);
}

bool Game_Actor::IsItemUsable(int item_id) const {
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("IsItemUsable: Invalid item ID %d", item_id);
		return false;
	}

	int query_idx = actor_id - 1;
	auto* query_set = &item->actor_set;
	if (Player::IsRPG2k3() && Data::system.equipment_setting == RPG::System::EquipmentSetting_class) {
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
	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
	if (!skill) {
		Output::Warning("IsSkillUsable: Invalid skill ID %d", skill_id);
		return false;
	}

	// Actor must have all attributes of the skill equipped as weapons
	const RPG::Item* item = GetEquipment(RPG::Item::Type_weapon);
	const RPG::Item* item2 = HasTwoWeapons() ? GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;

	for (size_t i = 0; i < skill->attribute_effects.size(); ++i) {
		bool required = skill->attribute_effects[i] && Data::attributes[i].type == RPG::Attribute::Type_physical;
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

bool Game_Actor::LearnSkill(int skill_id) {
	if (skill_id > 0 && !IsSkillLearned(skill_id)) {
		const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
		if (!skill) {
			Output::Warning("Actor %d: Can't learn invalid skill %d", GetId(), skill_id);
			return false;
		}

		GetData().skills.push_back((int16_t)skill_id);
		GetData().skills_size = GetData().skills.size();
		std::sort(GetData().skills.begin(), GetData().skills.end());
		return true;
	}
	return false;
}

bool Game_Actor::UnlearnSkill(int skill_id) {
	std::vector<int16_t>::iterator it = std::find(GetData().skills.begin(), GetData().skills.end(), skill_id);
	if (it != GetData().skills.end()) {
		GetData().skills.erase(it);
		GetData().skills_size = GetData().skills.size();
		return true;
	}
	return false;
}

void Game_Actor::UnlearnAllSkills() {
	GetData().skills.clear();
	GetData().skills_size = 0;
}

void Game_Actor::SetFace(const std::string& file_name, int index) {
	GetData().face_name.assign(file_name);
	GetData().face_id = index;
}

const RPG::Item* Game_Actor::GetEquipment(int equip_type) const {
	if (equip_type <= 0 || equip_type > (int)GetData().equipped.size())
		return nullptr;

	int item_id = GetData().equipped[equip_type - 1];
	return ReaderUtil::GetElement(Data::items, item_id);
}

int Game_Actor::SetEquipment(int equip_type, int new_item_id) {
	if (equip_type <= 0 || equip_type > (int) GetData().equipped.size())
		return -1;

	int old_item_id = GetData().equipped[equip_type - 1];

	const RPG::Item* new_item = ReaderUtil::GetElement(Data::items, new_item_id);
	if (new_item_id != 0 && !new_item) {
		Output::Warning("SetEquipment: Can't equip item with invalid ID %d", new_item_id);
		GetData().equipped[equip_type - 1] = 0;
		return old_item_id;
	}

	GetData().equipped[equip_type - 1] = (short)new_item_id;
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
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, GetWeaponId());
	const RPG::Item* item2 = ReaderUtil::GetElement(Data::items, GetShieldId());
	if (item && item2 && (item->two_handed || item2->two_handed)) {
		ChangeEquipment(equip_type == RPG::Item::Type_weapon ? equip_type + 1 : equip_type - 1, 0);
	}
}

const std::vector<int16_t>& Game_Actor::GetWholeEquipment() const {
	return GetData().equipped;
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

const std::vector<int16_t>& Game_Actor::GetStates() const {
	return GetData().status;
}

std::vector<int16_t>& Game_Actor::GetStates() {
	return GetData().status;
}

void Game_Actor::AddState(int state_id) {
	Game_Battler::AddState(state_id);
}

void Game_Actor::RemoveState(int state_id) {
	Game_Battler::RemoveState(state_id);
}

void Game_Actor::RemoveBattleStates() {
	Game_Battler::RemoveBattleStates();
}

void Game_Actor::RemoveAllStates() {
	Game_Battler::RemoveAllStates();
}

int Game_Actor::GetHp() const {
	return GetData().current_hp;
}

int Game_Actor::GetSp() const {
	return GetData().current_sp;
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
			? *ReaderUtil::GetElement(GetClass()->parameters.maxhp, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.maxhp, GetLevel());
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
			? *ReaderUtil::GetElement(GetClass()->parameters.maxsp, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.maxsp, GetLevel());
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
			? *ReaderUtil::GetElement(GetClass()->parameters.attack, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.attack, GetLevel());
	}

	if (mod) {
		n += GetData().attack_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				// Invalid equipment was removed
				n += ReaderUtil::GetElement(Data::items, *it)->atk_points1;
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
			? *ReaderUtil::GetElement(GetClass()->parameters.defense, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.defense, GetLevel());
	}

	if (mod) {
		n += GetData().defense_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				// Invalid equipment was removed
				n += ReaderUtil::GetElement(Data::items, *it)->def_points1;
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
			? *ReaderUtil::GetElement(GetClass()->parameters.spirit, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.spirit, GetLevel());
	}

	if (mod) {
		n += GetData().spirit_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				// Invalid equipment was removed
				n += ReaderUtil::GetElement(Data::items, *it)->spi_points1;
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
			? *ReaderUtil::GetElement(GetClass()->parameters.agility, GetLevel())
			: *ReaderUtil::GetElement(GetActor().parameters.agility, GetLevel());
	}

	if (mod) {
		n += GetData().agility_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				// Invalid equipment was removed
				n += ReaderUtil::GetElement(Data::items, *it)->agi_points1;
			}
		}
	}

	return Utils::Clamp(n, 1, MaxStatBaseValue());
}

int Game_Actor::GetBaseAgi() const {
	return GetBaseAgi(true, true);
}

int Game_Actor::CalculateExp(int level) const {
	const RPG::Class* klass = ReaderUtil::GetElement(Data::classes, GetData().class_id);

	double base, inflation, correction;
	if (klass) {
		base = klass->exp_base;
		inflation = klass->exp_inflation;
		correction = klass->exp_correction;
	}
	else {
		const RPG::Actor& actor = *ReaderUtil::GetElement(Data::actors, actor_id);
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
	std::stringstream ss;
	ss << GetExp();
	return ss.str();
}

std::string Game_Actor::GetNextExpString() const {
	if (GetNextExp() == -1) {
		return "------";
	} else {
		std::stringstream ss;
		ss << GetNextExp();
		return ss.str();
	}
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

	const uint8_t* r = ReaderUtil::GetElement(GetActor().state_ranks, state_id);
	if (r) {
		rate = *r;
	}

	// This takes the armor of the character with the most resistance for that particular state
	for (const auto equipment : GetWholeEquipment()) {
		RPG::Item* item = ReaderUtil::GetElement(Data::items, equipment);
		if (item != nullptr && (item->type == RPG::Item::Type_shield || item->type == RPG::Item::Type_armor
			|| item->type == RPG::Item::Type_helmet || item->type == RPG::Item::Type_accessory)
			&& state_id  <= item->state_set.size() && item->state_set[state_id - 1]) {
			mul = std::min<int>(mul, 100 - item->state_chance);
		}
	}

	// GetStateRate verifies the state_id
	return GetStateRate(state_id, rate) * mul / 100;
}

int Game_Actor::GetAttributeModifier(int attribute_id) const {
	int rate = 2; // C - default

	const uint8_t* r = ReaderUtil::GetElement(GetActor().attribute_ranks, attribute_id);
	if (r) {
		rate = *r;
	}

	// GetAttributeRate will verify this but actors already need a check earlier
	// because of attribute_shift
	const int* shift = ReaderUtil::GetElement(attribute_shift, attribute_id);

	if (!shift) {
		Output::Warning("GetAttributeModifier: Invalid attribute ID %d", attribute_id);
		return 0;
	}

	rate += *shift;
	for (auto id_object : GetWholeEquipment()) {
		RPG::Item *object = ReaderUtil::GetElement(Data::items, id_object);
		if (object != nullptr && (object->type == RPG::Item::Type_shield || object->type == RPG::Item::Type_armor
			|| object->type == RPG::Item::Type_helmet || object->type == RPG::Item::Type_accessory)
			&& object->attribute_set.size() >= attribute_id && object->attribute_set[attribute_id - 1]) {
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

const std::string& Game_Actor::GetName() const {
	return GetData().name;
}

const std::string& Game_Actor::GetSpriteName() const {
	return GetData().sprite_name;
}

int Game_Actor::GetSpriteIndex() const {
	return GetData().sprite_id;
}

std::string Game_Actor::GetFaceName() const {
	return GetData().face_name;
}

int Game_Actor::GetFaceIndex() const {
	return GetData().face_id;
}

std::string Game_Actor::GetTitle() const {
	return GetData().title;
}

int Game_Actor::GetWeaponId() const {
	int item_id = GetData().equipped[0];
	return item_id <= (int)Data::items.size() ? item_id : 0;
}

int Game_Actor::GetShieldId() const {
	int item_id = GetData().equipped[1];
	return item_id <= (int)Data::items.size() ? item_id : 0;
}

int Game_Actor::GetArmorId() const {
	int item_id = GetData().equipped[2];
	return item_id <= (int)Data::items.size() ? item_id : 0;
}

int Game_Actor::GetHelmetId() const {
	int item_id = GetData().equipped[3];
	return item_id <= (int)Data::items.size() ? item_id : 0;
}

int Game_Actor::GetAccessoryId() const {
	int item_id = GetData().equipped[4];
	return item_id <= (int)Data::items.size() ? item_id : 0;
}

int Game_Actor::GetLevel() const {
	return GetData().level;
}

int Game_Actor::GetMaxLevel() const {
	return std::max<int32_t>(1, std::min<int32_t>(GetActor().final_level, Player::IsRPG2k() ? max_level_2k : max_level_2k3));
}

int Game_Actor::GetExp() const {
	return GetData().exp;
}

void Game_Actor::SetExp(int _exp) {
	GetData().exp = min(max(_exp, 0), max_exp_value());
}

void Game_Actor::ChangeExp(int exp, bool level_up_message) {
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
		ChangeLevel(new_level, level_up_message);
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
		ss << " " << Data::terms.level_up << " ";
		ss << " " << Data::terms.level << " " << new_level;
		return ss.str();
	} else if (Player::IsRPG2kE()) {
		ss << new_level;
		return Utils::ReplacePlaceholders(
			Data::terms.level_up,
			{'S', 'V', 'U'},
			{GetData().name, ss.str(), Data::terms.level}
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
		ss << particle << Data::terms.level << " ";
		ss << new_level << space << Data::terms.level_up;
		return ss.str();
	}
}

std::string Game_Actor::GetLearningMessage(const RPG::Learning& learn) const {
	std::stringstream ss;

	std::string skill_name = "??? BAD SKILL ???";
	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, learn.skill_id);
	if (skill) {
		skill_name = skill->name;
	}

	if (Player::IsRPG2kE()) {
		return Utils::ReplacePlaceholders(
			Data::terms.skill_learned,
			{'S', 'O'},
			{GetData().name, skill_name}
		);
	}
	else {
		ss << skill_name;
		ss << (Player::IsRPG2k3E() ? " " : "") << Data::terms.skill_learned;
		return ss.str();
	}
}

void Game_Actor::ChangeLevel(int new_level, bool level_up_message) {
	const std::vector<RPG::Learning>* skills;
	if (GetData().class_id > 0) {
		skills = &GetClass()->skills;
	} else {
		skills = &GetActor().skills;
	}

	bool level_up = false;

	int old_level = GetLevel();
	SetLevel(new_level);
	new_level = GetLevel(); // Level adjusted to max

	if (new_level > old_level) {
		if (level_up_message) {
			Game_Message::texts.push_back(GetLevelUpMessage(new_level));
			level_up = true;
		}

		// Learn new skills
		for (const RPG::Learning& learn : *skills) {
			// Skill learning, up to current level
			if (learn.level > old_level && learn.level <= new_level) {
				LearnSkill(learn.skill_id);
				if (level_up_message) {
					Game_Message::texts.push_back(GetLearningMessage(learn));
					level_up = true;
				}
			}
		}

		if (level_up) {
			Game_Message::texts.back().append("\f");
			Game_Message::message_waiting = true;
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
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("IsEquippable: Invalid item ID %d", item_id);
		return false;
	}

	if (HasTwoWeapons() &&
		item->type == RPG::Item::Type_shield) {
			return false;
	}

	return IsItemUsable(item_id);
}

bool Game_Actor::IsEquipmentFixed() const {
	if (GetData().lock_equipment) {
		return true;
	}

	for (auto state_id: GetInflictedStates()) {
		auto* state = ReaderUtil::GetElement(Data::states, state_id);
		if (state && state->cursed) {
			return true;
		}
	}
	return false;
}

bool Game_Actor::HasStrongDefense() const {
	return GetData().super_guard;
}

const std::vector<int16_t>& Game_Actor::GetSkills() const {
	return GetData().skills;
}

const RPG::Skill* Game_Actor::GetRandomSkill() const {
	const std::vector<int16_t>& skills = GetSkills();
	if (skills.empty()) {
		return nullptr;
	}

	// Skills are guaranteed to be valid
	return ReaderUtil::GetElement(Data::skills, skills[Utils::GetRandomNumber(0, skills.size() - 1)]);
}

bool Game_Actor::HasTwoWeapons() const {
	return GetData().two_weapon;
}

bool Game_Actor::GetAutoBattle() const {
	return GetData().auto_battle;
}

int Game_Actor::GetBattleX() const {
	float position = 0.0;

	if (GetActor().battle_x == 0 ||
		Data::battlecommands.placement == RPG::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float left = GetBattleRow() == RowType::RowType_back ? 25.0 : 50.0;
		float right = left;

		const RPG::Terrain* terrain = ReaderUtil::GetElement(Data::terrains, Game_Battle::GetTerrainId());
		if (terrain) {
			// No warning, already reported on battle start
			right = left + terrain->grid_c / 1103;
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

		switch (Game_Battle::GetBattleMode()) {
			case Game_Battle::BattleNormal:
			case Game_Battle::BattleInitiative:
				return SCREEN_TARGET_WIDTH - position;
			case Game_Battle::BattleBackAttack:
				return position;
			case Game_Battle::BattlePincer:
			case Game_Battle::BattleSurround:
				// ToDo: Correct position
				return SCREEN_TARGET_WIDTH - position;
		}
	}
	else {
		//Output::Debug("%d %d %d %d", Data::terrains[0].grid_a, Data::terrains[0].grid_b, Data::terrains[0].grid_c, Data::terrains[0].grid_location);

		position = GetActor().battle_x * SCREEN_TARGET_WIDTH / 320;
	}

	return position;
}

int Game_Actor::GetBattleY() const {
	float position = 0.0;

	if (GetActor().battle_y == 0 ||
		Data::battlecommands.placement == RPG::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float top = 0.0f;
		float bottom = 0.0f;
		const RPG::Terrain* terrain = ReaderUtil::GetElement(Data::terrains, Game_Battle::GetTerrainId());
		if (terrain) {
			// No warning, already reported on battle start
			top = terrain->grid_a;
			bottom = top + terrain->grid_b / 13;
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
	return a.rename_skill ? a.skill_name : Data::terms.command_skill;
}

void Game_Actor::SetName(const std::string &new_name) {
	GetData().name = new_name;
}

void Game_Actor::SetTitle(const std::string &new_title) {
	GetData().title = new_title;
}

void Game_Actor::SetSprite(const std::string &file, int index, bool transparent) {
	GetData().sprite_name = file;
	GetData().sprite_id = index;
	GetData().sprite_flags = transparent ? 3 : 0;
}

void Game_Actor::ChangeBattleCommands(bool add, int id) {
	auto& cmds = GetData().battle_commands;

	// If changing battle commands, that is when RPG_RT will replace the -1 list with a 'true' list.
	// Fetch original command array.
	if (!GetData().changed_battle_commands) {
		cmds = Data::actors[GetId() - 1].battle_commands;
		GetData().changed_battle_commands = true;
	}

	// The battle commands array always has a size of 7 padded with -1. The last element before the padding is 0 which
	// stands for the Row command
	if (add) {
		const RPG::BattleCommand* cmd = ReaderUtil::GetElement(Data::battlecommands.commands, id);
		if (!cmd) {
			Output::Warning("ChangeBattleCommands: Can't add invalid battle command %d", id);
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

const std::vector<const RPG::BattleCommand*> Game_Actor::GetBattleCommands() const {
	std::vector<const RPG::BattleCommand*> commands;
	std::vector<int32_t> obc = GetData().battle_commands;
	if (!GetData().changed_battle_commands) {
		// In this case, get it straight from the LDB.
		obc = Data::actors[actor_id - 1].battle_commands;
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

		const RPG::BattleCommand* cmd = ReaderUtil::GetElement(Data::battlecommands.commands, command_index);
		if (!cmd) {
			Output::Warning("GetBattleCommands: Invalid battle command ID %d", command_index);
			continue;
		}

		commands.push_back(cmd);
	}

	return commands;
}

const RPG::Class* Game_Actor::GetClass() const {
	int id = GetData().class_id;

	if (id < 0) {
		// This means class ID hasn't been changed yet.
		id = GetActor().class_id;
	}

	return ReaderUtil::GetElement(Data::classes, id);
}

void Game_Actor::SetClass(int _class_id) {
	if (_class_id != 0) {
		const RPG::Class* cls = ReaderUtil::GetElement(Data::classes, _class_id);

		if (!cls) {
			Output::Warning("Actor %d: Can't change to invalid class %d", GetId(), _class_id);
			return;
		}
	}

	GetData().class_id = _class_id;
	GetData().changed_battle_commands = true; // Any change counts as a battle commands change.

	// The class settings are not applied when the actor has a class on startup
	// but only when the "Change Class" event command is used.

	if (_class_id > 0) {
		GetData().super_guard = GetClass()->super_guard;
		GetData().lock_equipment = GetClass()->lock_equipment;
		GetData().two_weapon = GetClass()->two_weapon;
		GetData().auto_battle = GetClass()->auto_battle;

		GetData().battler_animation = GetClass()->battler_animation;

		GetData().battle_commands = GetClass()->battle_commands;
	} else {
		GetData().super_guard = GetActor().super_guard;
		GetData().lock_equipment = GetActor().lock_equipment;
		GetData().two_weapon = GetActor().two_weapon;
		GetData().auto_battle = GetActor().auto_battle;

		GetData().battler_animation = 0;

		GetData().battle_commands = GetActor().battle_commands;
	}
	MakeExpList();

	// Set EXP to at least minimum value
	if (GetExp() < GetBaseExp()) {
		SetExp(GetBaseExp());
	}
}

std::string Game_Actor::GetClassName() const {
    if (!GetClass()) {
        return "";
    }
    return GetClass()->name;
}

void Game_Actor::SetBaseMaxHp(int maxhp) {
	int new_hp_mod = GetData().hp_mod + (maxhp - GetBaseMaxHp());
	GetData().hp_mod = new_hp_mod;

	SetHp(GetData().current_hp);
}

void Game_Actor::SetBaseMaxSp(int maxsp) {
	int new_sp_mod = GetData().sp_mod + (maxsp - GetBaseMaxSp());
	GetData().sp_mod = new_sp_mod;

	SetSp(GetData().current_sp);
}

void Game_Actor::SetHp(int hp) {
	GetData().current_hp = min(max(hp, 0), GetMaxHp());
}

void Game_Actor::SetSp(int sp) {
	GetData().current_sp = min(max(sp, 0), GetMaxSp());
}

void Game_Actor::SetBaseAtk(int atk) {
	int new_attack_mod = GetData().attack_mod + (atk - GetBaseAtk());
	GetData().attack_mod = new_attack_mod;
}

void Game_Actor::SetBaseDef(int def) {
	int new_defense_mod = GetData().defense_mod + (def - GetBaseDef());
	GetData().defense_mod = new_defense_mod;
}

void Game_Actor::SetBaseSpi(int spi) {
	int new_spirit_mod = GetData().spirit_mod + (spi - GetBaseSpi());
	GetData().spirit_mod = new_spirit_mod;
}

void Game_Actor::SetBaseAgi(int agi) {
	int new_agility_mod = GetData().agility_mod + (agi - GetBaseAgi());
	GetData().agility_mod = new_agility_mod;
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
			const RPG::BattlerAnimation* anima = ReaderUtil::GetElement(Data::battleranimations, GetActor().battler_animation);
			if (!anima) {
				Output::Warning("Actor %d: Invalid battle animation ID %d", GetId(), GetActor().battler_animation);
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

	auto checkWeapon = [&](const RPG::Item* weapon) {
		if (weapon) {
			weapon_bonus = std::max(weapon_bonus, float(weapon->critical_hit));
		}
	};

	checkWeapon(GetWeapon());
	checkWeapon(Get2ndWeapon());

	return crit_chance + (weapon_bonus / 100.0f);
}

Game_Battler::BattlerType Game_Actor::GetType() const {
	return Game_Battler::Type_Ally;
}

int Game_Actor::IsControllable() const {
	return GetSignificantRestriction() == RPG::State::Restriction_normal && !GetAutoBattle();
}


const RPG::Actor& Game_Actor::GetActor() const {
	// Always valid
	return *ReaderUtil::GetElement(Data::actors, actor_id);
}

RPG::SaveActor& Game_Actor::GetData() const {
	// Always valid because the array is resized to match actor size
	return *ReaderUtil::GetElement(Main_Data::game_data.actors, actor_id);
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
	int eq_types[] = { RPG::Item::Type_weapon,
		HasTwoWeapons() ? RPG::Item::Type_weapon : RPG::Item::Type_shield,
		RPG::Item::Type_armor,
		RPG::Item::Type_helmet,
		RPG::Item::Type_accessory
	};

	for (int i = 1; i <= 5; ++i) {
		const RPG::Item* item = GetEquipment(i);
		if (item && item->type != eq_types[i - 1]) {
			Output::Debug("Actor %d: Removing invalid item %d (of type %d) from equipment slot %d (needs type %d)",
			GetId(), item->ID, item->type, i, eq_types[i - 1]);
			SetEquipment(i, 0);
		}
	}

	// Remove invalid class
	if (GetData().class_id > 0) {
		const RPG::Class* cls = ReaderUtil::GetElement(Data::classes, GetData().class_id);
		if (!cls) {
			Output::Warning("Actor %d: Removing invalid class %d", GetId(), GetData().class_id);
			SetClass(0);
		}
	}

	// Remove invalid skills
	for (int16_t skill_id : GetSkills()) {
		const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
		if (!skill) {
			Output::Warning("Actor %d: Removing invalid skill %d", GetId(), skill_id);
			UnlearnSkill(skill_id);
		}
	}

	// Remove invalid states
	if (GetStates().size() > Data::states.size()) {
		Output::Warning("Actor %d: State array contains invalid states (%d > %d)", GetId(), GetStates().size(), Data::states.size());
		GetStates().resize(Data::states.size());
	}

	// Remove invalid levels
	// Special handling for the game COLORS: Lost Memories which uses level 0
	// through database editing. Hopefully no game uses negative levels.
	if (GetLevel() == 0) {
		Output::Debug("Actor %d: Special handling for level 0", GetId());
	} else if (GetLevel() < 0) {
		Output::Warning("Actor %d: Invalid level %d, changed to 1", GetId(), GetLevel());
		SetLevel(1);
	} else if (GetLevel() > GetMaxLevel()) {
		Output::Warning("Actor %d: Invalid level %d, changed to %d", GetId(), GetLevel(), GetMaxLevel());
		SetLevel(GetMaxLevel());
	}
}

const RPG::Item* Game_Actor::GetWeapon() const {
	auto* weapon = GetEquipment(RPG::Item::Type_weapon);
	if (weapon && weapon->type == RPG::Item::Type_weapon) {
		return weapon;
	}
	return nullptr;
}

const RPG::Item* Game_Actor::Get2ndWeapon() const {
	// Checking of HasTwoWeapons() not neccessary. If true, the
	// item equipped in this slot will never be a weapon from
	// legitimate means.
	auto* weapon = GetEquipment(RPG::Item::Type_shield);
	if (weapon && weapon->type == RPG::Item::Type_weapon) {
		return weapon;
	}
	return nullptr;
}

const RPG::Item* Game_Actor::GetShield() const {
	auto* shield = GetEquipment(RPG::Item::Type_shield);
	if (shield && shield->type == RPG::Item::Type_shield) {
		return shield;
	}
	return nullptr;
}

const RPG::Item* Game_Actor::GetArmor() const {
	auto* armor = GetEquipment(RPG::Item::Type_armor);
	if (armor && armor->type == RPG::Item::Type_armor) {
		return armor;
	}
	return nullptr;
}

const RPG::Item* Game_Actor::GetHelmet() const {
	auto* helmet = GetEquipment(RPG::Item::Type_helmet);
	if (helmet && helmet->type == RPG::Item::Type_helmet) {
		return helmet;
	}
	return nullptr;
}

const RPG::Item* Game_Actor::GetAccessory() const {
	auto* accessory = GetEquipment(RPG::Item::Type_accessory);
	if (accessory && accessory->type == RPG::Item::Type_accessory) {
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
	auto checkEquip = [](const RPG::Item* item) {
		return item && item->prevent_critical;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::PreventsTerrainDamage() const {
	auto checkEquip = [](const RPG::Item* item) {
		return item && item->no_terrain_damage;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::HasPhysicalEvasionUp() const {
	auto checkEquip = [](const RPG::Item* item) {
		return item && item->raise_evasion;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}

bool Game_Actor::HasHalfSpCost() const {
	auto checkEquip = [](const RPG::Item* item) {
		return item && item->half_sp_cost;
	};
	return checkEquip(GetShield())
		|| checkEquip(GetArmor())
		|| checkEquip(GetHelmet())
		|| checkEquip(GetAccessory());
}


