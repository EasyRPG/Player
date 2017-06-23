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
#include "game_actor.h"
#include "game_battle.h"
#include "game_message.h"
#include "game_party.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "rpg_skill.h"
#include "util_macro.h"
#include "utils.h"

static int max_hp_value() {
	return Player::IsRPG2k() ? 999 : 9999;
}

static int max_other_stat_value() {
	return 999;
}

static int max_exp_value() {
	return Player::IsRPG2k() ? 999999 : 9999999;
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
	const std::vector<RPG::Learning>& skills = Data::actors[actor_id - 1].skills;
	for (int i = 0; i < (int)skills.size(); i++)
		if (skills[i].level <= GetLevel())
			LearnSkill(skills[i].skill_id);
	SetHp(GetMaxHp());
	SetSp(GetMaxSp());
	SetExp(exp_list[GetLevel() - 1]);

	RemoveInvalidEquipment();
}

void Game_Actor::Fixup() {
	GetData().Fixup(actor_id);

	RemoveInvalidEquipment();
}

int Game_Actor::GetId() const {
	return actor_id;
}

bool Game_Actor::UseItem(int item_id) {
	const RPG::Item& item = Data::items[item_id - 1];

	if (IsDead() && item.type != RPG::Item::Type_medicine) {
		return false;
	}

	if (item.type == RPG::Item::Type_book) {
		return LearnSkill(item.skill_id);
	}

	if (item.type == RPG::Item::Type_material) {
		SetBaseMaxHp(GetBaseMaxHp() + item.max_hp_points);
		SetBaseMaxSp(GetBaseMaxSp() + item.max_sp_points);
		SetBaseAtk(GetBaseAtk() + item.atk_points2);
		SetBaseDef(GetBaseDef() + item.def_points2);
		SetBaseAgi(GetBaseAgi() + item.agi_points2);
		SetBaseSpi(GetBaseSpi() + item.spi_points2);

		return true;
	}

	return Game_Battler::UseItem(item_id);
}

bool Game_Actor::IsItemUsable(int item_id) const {
	const RPG::Item& item = Data::items[item_id - 1];

	// If the actor ID is out of range this is an optimization in the ldb file
	// (all actors missing can equip the item)
	if (item.actor_set.size() <= (unsigned)(actor_id - 1)) {
		return true;
	} else {
		return item.actor_set.at(actor_id - 1);
	}
}

bool Game_Actor::IsSkillLearned(int skill_id) const {
	return std::find(GetData().skills.begin(), GetData().skills.end(), skill_id) != GetData().skills.end();
}

bool Game_Actor::IsSkillUsable(int skill_id) const {
	if (skill_id <= 0 || skill_id > (int)Data::skills.size()) {
		return false;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	// Actor must have all attributes of the skill equipped as weapons
	const RPG::Item* item = GetEquipment(RPG::Item::Type_weapon);
	const RPG::Item* item2 = HasTwoWeapons() ? GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;

	for (size_t i = 0; i < skill.attribute_effects.size(); ++i) {
		bool required = skill.attribute_effects[i] && Data::attributes[i].type == RPG::Attribute::Type_physical;
		if (required) {
			if (item && i < item->attribute_set.size()) {
				if (!item->attribute_set[i]) {
					return false;
				}
			} else if (item2 && i < item2->attribute_set.size()) {
				if (!item2->attribute_set[i]) {
					return false;
				}
			} else {
				return false;
			}
		}
	}

	return Game_Battler::IsSkillUsable(skill_id);
}

int Game_Actor::GetSpCostModifier() const {
	// Only non-weapons have this modifier
	int start = HasTwoWeapons() ? RPG::Item::Type_armor : RPG::Item::Type_shield;
	int sp_mod = 1;

	for (int i = start; i <= 5; ++i) {
		const RPG::Item* item = GetEquipment(i);
		if (item && item->half_sp_cost) {
			sp_mod = 2;
			break;
		}
	}

	return sp_mod;
}

int Game_Actor::CalculateSkillCost(int skill_id) const {
	return Game_Battler::CalculateSkillCost(skill_id) / GetSpCostModifier();
}

bool Game_Actor::LearnSkill(int skill_id) {
	if (skill_id > 0 && !IsSkillLearned(skill_id)) {
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

	if (item_id <= 0 || item_id >(int)Data::items.size()) {
		return nullptr;
	}

	return &Data::items[item_id - 1];
}

int Game_Actor::SetEquipment(int equip_type, int new_item_id) {
	if (equip_type <= 0 || equip_type > (int) GetData().equipped.size())
		return -1;

	int old_item_id = GetData().equipped[equip_type - 1];
	if (old_item_id > (int)Data::items.size())
		old_item_id = 0;

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
}

const std::vector<int16_t>& Game_Actor::GetWholeEquipment() const {
	return GetData().equipped;
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
	GetData().status_size = GetData().status.size();
}

void Game_Actor::RemoveState(int state_id) {
	Game_Battler::RemoveState(state_id);
	GetData().status_size = GetData().status.size();
}

void Game_Actor::RemoveBattleStates() {
	Game_Battler::RemoveBattleStates();
	GetData().status_size = GetData().status.size();
}

void Game_Actor::RemoveAllStates() {
	Game_Battler::RemoveAllStates();
	GetData().status_size = GetData().status.size();
}

int Game_Actor::GetHp() const {
	return GetData().current_hp;
}

int Game_Actor::GetSp() const {
	return GetData().current_sp;
}

int Game_Actor::GetBaseMaxHp(bool mod) const {
	// Looks like RPG_RT only applies Class changes (changed_class == true)
	// when the class was changed by the ChangeClass event, otherwise it uses
	// the normal actor attributes.
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.maxhp[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.maxhp[GetData().level - 1];

	if (mod)
		n += GetData().hp_mod;

	return min(max(n, 1), max_hp_value());
}

int Game_Actor::GetBaseMaxHp() const {
	return GetBaseMaxHp(true);
}

int Game_Actor::GetBaseMaxSp(bool mod) const {
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.maxsp[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.maxsp[GetData().level - 1];

	if (mod)
		n += GetData().sp_mod;

	return min(max(n, 0), max_other_stat_value());
}

int Game_Actor::GetBaseMaxSp() const {
	return GetBaseMaxSp(true);
}

int Game_Actor::GetBaseAtk(bool mod, bool equip) const {
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.attack[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.attack[GetData().level - 1];

	if (mod) {
		n += GetData().attack_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				n += Data::items[*it - 1].atk_points1;
			}
		}
	}

	return min(max(n, 1), max_other_stat_value());
}

int Game_Actor::GetBaseAtk() const {
	return GetBaseAtk(true, true);
}

int Game_Actor::GetBaseDef(bool mod, bool equip) const {
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.defense[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.defense[GetData().level - 1];

	if (mod) {
		n += GetData().defense_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				n += Data::items[*it - 1].def_points1;
			}
		}
	}

	return min(max(n, 1), max_other_stat_value());
}

int Game_Actor::GetBaseDef() const {
	return GetBaseDef(true, true);
}

int Game_Actor::GetBaseSpi(bool mod, bool equip) const {
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.spirit[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.spirit[GetData().level - 1];

	if (mod) {
		n += GetData().spirit_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				n += Data::items[*it - 1].spi_points1;
			}
		}
	}

	return min(max(n, 1), max_other_stat_value());
}

int Game_Actor::GetBaseSpi() const {
	return GetBaseSpi(true, true);
}

int Game_Actor::GetBaseAgi(bool mod, bool equip) const {
	int n = GetData().changed_class && GetData().class_id > 0
		? Data::classes[GetData().class_id - 1].parameters.agility[GetData().level - 1]
		: Data::actors[actor_id - 1].parameters.agility[GetData().level - 1];

	if (mod) {
		n += GetData().agility_mod;
	}

	if (equip) {
		for (std::vector<int16_t>::const_iterator it = GetData().equipped.begin(); it != GetData().equipped.end(); ++it) {
			if (*it > 0 && *it <= (int)Data::items.size()) {
				n += Data::items[*it - 1].agi_points1;
			}
		}
	}

	return min(max(n, 1), max_other_stat_value());
}

int Game_Actor::GetBaseAgi() const {
	return GetBaseAgi(true, true);
}

int Game_Actor::CalculateExp(int level) const
{
	double base, inflation, correction;
	if (GetData().changed_class && GetData().class_id > 0) {
		const RPG::Class& klass = Data::classes[GetData().class_id - 1];
		base = klass.exp_base;
		inflation = klass.exp_inflation;
		correction = klass.exp_correction;
	}
	else {
		const RPG::Actor& actor = Data::actors[actor_id - 1];
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
	int final_level = Data::actors[actor_id - 1].final_level;
	exp_list.resize(final_level, 0);;
	for (int i = 1; i < final_level; ++i) {
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
	if (level >= GetMaxLevel() || level <= 0) {
		return -1;
	} else {
		return exp_list[level];
	}
}

int Game_Actor::GetStateProbability(int state_id) const {
	int rate = 2; // C - default

	if (state_id <= (int)Data::actors[actor_id - 1].state_ranks.size()) {
		rate = Data::actors[actor_id - 1].state_ranks[state_id - 1];
	}

	return GetStateRate(state_id, rate);
}

int Game_Actor::GetAttributeModifier(int attribute_id) const {
	int rate = 2; // C - default

	if (attribute_id <= (int)Data::actors[actor_id - 1].attribute_ranks.size()) {
		rate = Data::actors[actor_id - 1].attribute_ranks[attribute_id - 1];
	}

	rate += attribute_shift[attribute_id - 1];
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
	return Data::actors[actor_id - 1].final_level;
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

	if (new_level != GetData().level) {
		ChangeLevel(new_level, level_up_message);
	}
}

void Game_Actor::SetLevel(int _level) {
	GetData().level = min(max(_level, 1), GetMaxLevel());
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
	std::string& skill_name = Data::skills[learn.skill_id - 1].name;
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
	if (GetData().changed_class && GetData().class_id > 0) {
		skills = &Data::classes[GetData().class_id - 1].skills;
	} else {
		skills = &Data::actors[actor_id - 1].skills;
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
		// Set HP and SP to maximum possible value
		SetHp(GetHp());
		SetSp(GetSp());

		// Experience adjustment:
		// Level minimum if higher then Level maximum
		if (GetExp() >= GetNextExp()) {
			SetExp(GetBaseExp());
		}
	}
}

bool Game_Actor::IsEquippable(int item_id) const {
	if (HasTwoWeapons() &&
		Data::items[item_id - 1].type == RPG::Item::Type_shield) {
			return false;
	}

	return IsItemUsable(item_id);
}

bool Game_Actor::IsEquipmentFixed() const {
	return GetData().lock_equipment;
}

bool Game_Actor::HasStrongDefense() const {
	return GetData().super_guard;
}

bool Game_Actor::HasPreemptiveAttack() const {
	const RPG::Item* item = GetEquipment(RPG::Item::Type_weapon);
	if (item && item->preemptive) {
		return true;
	}
	if (HasTwoWeapons()) {
		item = GetEquipment(RPG::Item::Type_weapon + 1);
		if (item && item->preemptive) {
			return true;
		}
	}
	return false;
}

const std::vector<int16_t>& Game_Actor::GetSkills() const {
	return GetData().skills;
}

const RPG::Skill& Game_Actor::GetRandomSkill() const {
	const std::vector<int16_t>& skills = GetSkills();

	return Data::skills[skills[Utils::GetRandomNumber(0, skills.size() - 1)] - 1];
}

bool Game_Actor::HasTwoWeapons() const {
	return GetData().two_weapon;
}

bool Game_Actor::GetAutoBattle() const {
	return GetData().auto_battle;
}

int Game_Actor::GetBattleX() const {
	float position = 0.0;

	if (Data::actors[actor_id - 1].battle_x == 0 ||
		Data::battlecommands.placement == RPG::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float left = GetBattleRow() == 1 ? 25.0 : 50.0;
		float right = left + Data::terrains[Game_Battle::GetTerrainId() - 1].grid_c / 1103;

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

		position = (Data::actors[actor_id - 1].battle_x*SCREEN_TARGET_WIDTH / 320);
	}

	return position;
}

int Game_Actor::GetBattleY() const {
	float position = 0.0;

	if (Data::actors[actor_id - 1].battle_y == 0 ||
		Data::battlecommands.placement == RPG::BattleCommands::Placement_automatic) {
		int party_pos = Main_Data::game_party->GetActorPositionInParty(actor_id);
		int party_size = Main_Data::game_party->GetBattlerCount();

		float top = Data::terrains[Game_Battle::GetTerrainId() - 1].grid_a;
		float bottom = top + Data::terrains[Game_Battle::GetTerrainId() - 1].grid_b / 13;

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
		case 4:
			switch (party_pos) {
			case 0:
				position = top;
				break;
			case 1:
				position = top + ((bottom - top) * 1.0/3);
				break;
			case 2:
				position = top + ((bottom - top) * 2.0/3);
				break;
			case 3:
				position = bottom;
				break;
			}
		}

		position -= 24;
	}
	else {
		position = (Data::actors[actor_id - 1].battle_y*SCREEN_TARGET_HEIGHT / 240);
	}

	return (int)position;
}

const std::string& Game_Actor::GetSkillName() const {
	return Data::actors[actor_id - 1].skill_name;
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
	if (add) {
		if (std::find(GetData().battle_commands.begin(), GetData().battle_commands.end(), id)
			== GetData().battle_commands.end()) {
			GetData().battle_commands.push_back(id);
			std::sort(GetData().battle_commands.begin(), GetData().battle_commands.end());
		}
	}
	else if (id == 0) {
		GetData().battle_commands.clear();
	}
	else {
		std::vector<uint32_t>::iterator it;
		it = std::find(GetData().battle_commands.begin(), GetData().battle_commands.end(), id);
		if (it != GetData().battle_commands.end())
			GetData().battle_commands.erase(it);
	}
}

const std::vector<const RPG::BattleCommand*> Game_Actor::GetBattleCommands() const {
	std::vector<const RPG::BattleCommand*> commands;

	for (size_t i = 0; i < GetData().battle_commands.size(); ++i) {
		int command_index = GetData().battle_commands[i];
		if (command_index == 0) {
			// Row command -> not impl
			continue;
		}

		if (command_index == -1) {
			// Fetch original command
			const RPG::Actor& actor = Data::actors[GetId() - 1];
			if (i + 1 <= actor.battle_commands.size()) {
				int bcmd_idx = Data::actors[GetId() - 1].battle_commands[i];

				if (bcmd_idx == -1) {
					// End of list
					continue;
				}

				if (bcmd_idx == 0) {
					// Row command
					continue;
				}

				commands.push_back(&Data::battlecommands.commands[bcmd_idx - 1]);
			}
		} else {
			commands.push_back(&Data::battlecommands.commands[command_index - 1]);
		}
	}

	return commands;
}

const RPG::Class* Game_Actor::GetClass() const {
	if (GetData().class_id <= 0) {
		return nullptr;
	}

	return &Data::classes[GetData().class_id - 1];
}

void Game_Actor::SetClass(int _class_id) {
	GetData().class_id = _class_id;
	GetData().changed_class = _class_id > 0;

	// The class settings are not applied when the actor has a class on startup
	// but only when the "Change Class" event command is used.

	if (GetData().changed_class) {
		GetData().battler_animation = GetClass()->battler_animation;
		GetData().super_guard = GetClass()->super_guard;
		GetData().lock_equipment = GetClass()->lock_equipment;
		GetData().two_weapon = GetClass()->two_weapon;
		GetData().auto_battle = GetClass()->auto_battle;
	} else {
		const RPG::Actor& actor = Data::actors[actor_id - 1];

		GetData().super_guard = actor.super_guard;
		GetData().lock_equipment = actor.lock_equipment;
		GetData().two_weapon = actor.two_weapon;
		GetData().auto_battle = actor.auto_battle;

		GetData().battler_animation = 0;
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
	GetData().hp_mod += maxhp - GetBaseMaxHp();
	SetHp(GetData().current_hp);
}

void Game_Actor::SetBaseMaxSp(int maxsp) {
	GetData().sp_mod += maxsp - GetBaseMaxSp();
	SetSp(GetData().current_sp);
}

void Game_Actor::SetHp(int hp) {
	GetData().current_hp = min(max(hp, 0), GetMaxHp());
}

void Game_Actor::ChangeHp(int hp) {
	Game_Battler::ChangeHp(hp);

	if (GetData().current_hp == 0) {
		// Death
		SetGauge(0);
		RemoveAllStates();
		SetDefending(false);
		SetCharged(false);
		AddState(1);
	} else {
		// Back to life
		RemoveState(1);
		if (GetHp() <= 0) {
			// Reviving gives at least 1 Hp
			SetHp(1);
		}
	}
}

void Game_Actor::SetSp(int sp) {
	GetData().current_sp = min(max(sp, 0), GetMaxSp());
}

void Game_Actor::SetBaseAtk(int atk) {
	GetData().attack_mod += atk - GetBaseAtk();
}

void Game_Actor::SetBaseDef(int def) {
	GetData().defense_mod += def - GetBaseDef();
}

void Game_Actor::SetBaseSpi(int spi) {
	GetData().spirit_mod += spi - GetBaseSpi();
}

void Game_Actor::SetBaseAgi(int agi) {
	GetData().agility_mod += agi - GetBaseAgi();
}

int Game_Actor::GetBattleRow() const {
	return GetData().row;
}

void Game_Actor::SetBattleRow(int battle_row) {
	GetData().row = battle_row;
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
		if (GetData().changed_class && GetClass()) {
			anim = GetClass()->battler_animation;
		} else {
			anim = Data::battleranimations[Data::actors[actor_id - 1].battler_animation - 1].ID;
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
	return 90;
}

float Game_Actor::GetCriticalHitChance() const {
	return Data::actors[actor_id - 1].critical_hit ? (1.0f / Data::actors[actor_id - 1].critical_hit_chance) : 0.0f;
}

Game_Battler::BattlerType Game_Actor::GetType() const {
	return Game_Battler::Type_Ally;
}

RPG::SaveActor & Game_Actor::GetData() const {
	return Main_Data::game_data.actors[actor_id - 1];
}

void Game_Actor::RemoveInvalidEquipment() {
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
}

