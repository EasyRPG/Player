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
#include "game_message.h"
#include "game_party.h"
#include "main_data.h"
#include "player.h"
#include "rpg_skill.h"
#include "util_macro.h"

Game_Actor::Game_Actor(int actor_id)
    : Game_Battler(), data(Main_Data::game_data.actors[actor_id - 1]) {
	data.Setup(actor_id);

	Setup();
}

void Game_Actor::Setup() { MakeExpList(); }

void Game_Actor::Init() {
	const std::vector<RPG::Learning>& skills = Data::actors[data.ID - 1].skills;
	for (int i = 0; i < (int)skills.size(); i++)
		if (skills[i].level <= GetLevel()) LearnSkill(skills[i].skill_id);
	SetHp(GetMaxHp());
	SetSp(GetMaxSp());
	SetExp(exp_list[GetLevel() - 1]);
}

void Game_Actor::Fixup() { data.Fixup(); }

int Game_Actor::GetId() const { return data.ID; }

bool Game_Actor::UseItem(int item_id) {
	const RPG::Item& item = Data::items[item_id - 1];

	if (item.type == RPG::Item::Type_book) {
		return LearnSkill(item.skill_id);
	} else {
		return Game_Battler::UseItem(item_id);
	}
}

bool Game_Actor::UseSkill(int skill_id) {
	// const RPG::Skill& skill = Data::skills[skill_id - 1];

	return Game_Battler::UseSkill(skill_id);
}

bool Game_Actor::IsSkillLearned(int skill_id) const {
	return std::find(data.skills.begin(), data.skills.end(), skill_id) != data.skills.end();
}

bool Game_Actor::IsSkillUsable(int skill_id) const {
	if (!IsSkillLearned(skill_id)) {
		return false;
	} else {
		return Game_Battler::IsSkillUsable(skill_id);
	}
}

bool Game_Actor::LearnSkill(int skill_id) {
	if (skill_id > 0 && !IsSkillLearned(skill_id)) {
		data.skills.push_back((int16_t)skill_id);
		std::sort(data.skills.begin(), data.skills.end());
		return true;
	}
	return false;
}

bool Game_Actor::UnlearnSkill(int skill_id) {
	std::vector<int16_t>::iterator it = std::find(data.skills.begin(), data.skills.end(), skill_id);
	if (it != data.skills.end()) {
		data.skills.erase(it);
		return true;
	}
	return false;
}

void Game_Actor::SetFace(const std::string& file_name, int index) {
	data.face_name.assign(file_name);
	data.face_id = index;
}

int Game_Actor::GetEquipment(int equip_type) const {
	if (equip_type < 0 || equip_type >= (int)data.equipped.size()) return -1;
	return data.equipped[equip_type];
}

int Game_Actor::SetEquipment(int equip_type, int new_item_id) {
	if (equip_type < 0 || equip_type >= (int)data.equipped.size()) return -1;

	int old_item_id = data.equipped[equip_type];
	data.equipped[equip_type] = (short)new_item_id;
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

const std::vector<int16_t>& Game_Actor::GetStates() const { return data.status; }

std::vector<int16_t>& Game_Actor::GetStates() { return data.status; }

int Game_Actor::GetHp() const { return data.current_hp; }

int Game_Actor::GetSp() const { return data.current_sp; }

int Game_Actor::GetBaseMaxHp(bool mod) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.maxhp[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.maxhp[data.level - 1];

	if (mod) n += data.hp_mod;

	return min(max(n, 1), 999);
}

int Game_Actor::GetBaseMaxHp() const { return GetBaseMaxHp(true); }

int Game_Actor::GetBaseMaxSp(bool mod) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.maxsp[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.maxsp[data.level - 1];

	if (mod) n += data.sp_mod;

	return min(max(n, 0), 999);
}

int Game_Actor::GetBaseMaxSp() const { return GetBaseMaxSp(true); }

int Game_Actor::GetBaseAtk(bool mod, bool equip) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.attack[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.attack[data.level - 1];

	if (mod) n += data.attack_mod;

	if (equip)
		for (std::vector<int16_t>::const_iterator it = data.equipped.begin();
		     it != data.equipped.end(); ++it)
			if (*it > 0) n += Data::items[*it - 1].atk_points1;

	return min(max(n, 1), 999);
}

int Game_Actor::GetBaseAtk() const { return GetBaseAtk(true, true); }

int Game_Actor::GetBaseDef(bool mod, bool equip) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.defense[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.defense[data.level - 1];

	if (mod) n += data.defense_mod;

	if (equip)
		for (std::vector<int16_t>::const_iterator it = data.equipped.begin();
		     it != data.equipped.end(); ++it)
			if (*it > 0) n += Data::items[*it - 1].def_points1;

	return min(max(n, 1), 999);
}

int Game_Actor::GetBaseDef() const { return GetBaseDef(true, true); }

int Game_Actor::GetBaseSpi(bool mod, bool equip) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.spirit[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.spirit[data.level - 1];

	if (mod) n += data.spirit_mod;

	if (equip)
		for (std::vector<int16_t>::const_iterator it = data.equipped.begin();
		     it != data.equipped.end(); ++it)
			if (*it > 0) n += Data::items[*it - 1].spi_points1;

	return min(max(n, 1), 999);
}

int Game_Actor::GetBaseSpi() const { return GetBaseSpi(true, true); }

int Game_Actor::GetBaseAgi(bool mod, bool equip) const {
	int n = data.changed_class ? Data::classes[data.class_id - 1].parameters.agility[data.level - 1]
	                           : Data::actors[data.ID - 1].parameters.agility[data.level - 1];

	if (mod) n += data.agility_mod;

	if (equip)
		for (std::vector<int16_t>::const_iterator it = data.equipped.begin();
		     it != data.equipped.end(); ++it)
			if (*it > 0) n += Data::items[*it - 1].agi_points1;

	return min(max(n, 1), 999);
}

int Game_Actor::GetBaseAgi() const { return GetBaseAgi(true, true); }

int Game_Actor::CalculateExp(int level) const {
	double base, inflation, correction;
	if (data.changed_class) {
		const RPG::Class& klass = Data::classes[data.class_id - 1];
		base = klass.exp_base;
		inflation = klass.exp_inflation;
		correction = klass.exp_correction;
	} else {
		const RPG::Actor& actor = Data::actors[data.ID - 1];
		base = actor.exp_base;
		inflation = actor.exp_inflation;
		correction = actor.exp_correction;
	}

	int result = 0;
	if (Player::engine == Player::EngineRpg2k) /*Rpg2k*/ {
		inflation = 1.5 + (inflation * 0.01);

		for (int i = level; i >= 1; i--) {
			result = result + (int)(correction + base);
			base = base * inflation;
			inflation = ((level + 1) * 0.002 + 0.8) * (inflation - 1) + 1;
		}
	} else /*Rpg2k3*/ {
		for (int i = 1; i <= level; i++) {
			result += (int)base;
			result += i * (int)inflation;
			result += (int)correction;
		}
	}
	return min(result, Player::engine == Player::EngineRpg2k ? 1000000 : 10000000);
}

void Game_Actor::MakeExpList() {
	int final_level = Data::actors[data.ID - 1].final_level;
	exp_list.resize(final_level, 0);
	;
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

int Game_Actor::GetBaseExp() const { return GetBaseExp(GetLevel()); }

int Game_Actor::GetBaseExp(int level) const { return GetNextExp(level - 1); }

int Game_Actor::GetNextExp() const { return GetNextExp(GetLevel()); }

int Game_Actor::GetNextExp(int level) const {
	if (level >= GetMaxLevel() || level <= 0) {
		return -1;
	} else {
		return exp_list[level];
	}
}

const std::string& Game_Actor::GetName() const { return data.name; }

const std::string& Game_Actor::GetSpriteName() const { return data.sprite_name; }

int Game_Actor::GetSpriteIndex() const { return data.sprite_id; }

std::string Game_Actor::GetFaceName() const { return data.face_name; }

int Game_Actor::GetFaceIndex() const { return data.face_id; }

std::string Game_Actor::GetTitle() const { return data.title; }

int Game_Actor::GetWeaponId() const { return data.equipped[0]; }

int Game_Actor::GetShieldId() const { return data.equipped[1]; }

int Game_Actor::GetArmorId() const { return data.equipped[2]; }

int Game_Actor::GetHelmetId() const { return data.equipped[3]; }

int Game_Actor::GetAccessoryId() const { return data.equipped[4]; }

int Game_Actor::GetLevel() const { return data.level; }

int Game_Actor::GetMaxLevel() const { return Data::actors[data.ID - 1].final_level; }

int Game_Actor::GetExp() const { return data.exp; }

void Game_Actor::SetExp(int _exp) { data.exp = min(max(_exp, 0), 999999); }

void Game_Actor::ChangeExp(int exp, bool level_up_message) {
	int new_level = GetLevel();
	int new_exp = min(max(exp, 0), 999999);

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

	if (new_level != data.level) {
		ChangeLevel(new_level, level_up_message);
	}
}

void Game_Actor::SetLevel(int _level) { data.level = min(max(_level, 1), GetMaxLevel()); }

void Game_Actor::ChangeLevel(int new_level, bool level_up_message) {
	const std::vector<RPG::Learning>& skills = Data::actors[data.ID - 1].skills;
	bool level_up = false;

	int old_level = GetLevel();
	SetLevel(new_level);
	new_level = GetLevel(); // Level adjusted to max

	if (new_level > old_level) {
		if (level_up_message) {
			std::stringstream ss;
			ss << data.name << " ";
			ss << Data::terms.level << " " << new_level;
			ss << Data::terms.level_up;
			Game_Message::texts.push_back(ss.str());
			level_up = true;
		}

		// Learn new skills
		for (std::vector<RPG::Learning>::const_iterator it = skills.begin(); it != skills.end();
		     ++it) {
			// Skill learning, up to current level
			if (it->level > old_level && it->level <= new_level) {
				if (LearnSkill(it->skill_id) && level_up_message) {
					std::stringstream ss;
					ss << Data::skills[it->skill_id - 1].name;
					ss << Data::terms.skill_learned;
					Game_Message::texts.push_back(ss.str());
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
	if (data.two_weapon && Data::items[item_id - 1].type == RPG::Item::Type_shield) {
		return false;
	}

	// If the actor ID is out of range this is an optimization in the ldb file
	// (all actors missing can equip the item)
	if (Data::items[item_id - 1].actor_set.size() <= (unsigned)(data.ID - 1)) {
		return true;
	} else {
		return Data::items[item_id - 1].actor_set.at(data.ID - 1);
	}
}

const std::vector<int16_t>& Game_Actor::GetSkills() const { return data.skills; }

const RPG::Skill& Game_Actor::GetRandomSkill() const {
	const std::vector<int16_t>& skills = GetSkills();

	return Data::skills[skills[rand() % (skills.size() + 1)] - 1];
}

bool Game_Actor::GetTwoSwordsStyle() const { return data.two_weapon; }

bool Game_Actor::GetAutoBattle() const { return data.auto_battle; }

int Game_Actor::GetBattleX() const {
	return (Data::actors[data.ID - 1].battle_x * SCREEN_TARGET_WIDTH / 320);
}

int Game_Actor::GetBattleY() const {
	return (Data::actors[data.ID - 1].battle_y * SCREEN_TARGET_HEIGHT / 240);
}

const std::string& Game_Actor::GetSkillName() const { return Data::actors[data.ID - 1].skill_name; }

void Game_Actor::SetName(const std::string& new_name) { data.name = new_name; }

void Game_Actor::SetTitle(const std::string& new_title) { data.title = new_title; }

void Game_Actor::SetSprite(const std::string& file, int index, bool transparent) {
	data.sprite_name = file;
	data.sprite_id = index;
	data.sprite_flags = transparent ? 3 : 0;
}

void Game_Actor::ChangeBattleCommands(bool add, int id) {
	if (add) {
		if (std::find(data.battle_commands.begin(), data.battle_commands.end(), id) ==
		    data.battle_commands.end()) {
			data.battle_commands.push_back(id);
			std::sort(data.battle_commands.begin(), data.battle_commands.end());
		}
	} else if (id == 0) {
		data.battle_commands.clear();
	} else {
		std::vector<uint32_t>::iterator it;
		it = std::find(data.battle_commands.begin(), data.battle_commands.end(), id);
		if (it != data.battle_commands.end()) data.battle_commands.erase(it);
	}
}

const std::vector<uint32_t>& Game_Actor::GetBattleCommands() { return data.battle_commands; }

int Game_Actor::GetClass() const { return data.class_id; }

void Game_Actor::SetClass(int _class_id) {
	data.class_id = _class_id;
	MakeExpList();
}

std::string Game_Actor::GetClassName() const {
	if (GetClass() <= 0) {
		return "";
	}
	return Data::classes[GetClass() - 1].name;
}

void Game_Actor::SetBaseMaxHp(int maxhp) {
	data.hp_mod += maxhp - GetBaseMaxHp();
	SetHp(data.current_hp);
}

void Game_Actor::SetBaseMaxSp(int maxsp) {
	data.sp_mod += maxsp - GetBaseMaxSp();
	SetSp(data.current_sp);
}

void Game_Actor::SetHp(int hp) { data.current_hp = min(max(hp, 0), GetMaxHp()); }

void Game_Actor::ChangeHp(int hp) {
	SetHp(GetHp() + hp);

	if (data.current_hp == 0) {
		// Death
		RemoveAllStates();
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

void Game_Actor::SetSp(int sp) { data.current_sp = min(max(sp, 0), GetMaxSp()); }

void Game_Actor::SetBaseAtk(int atk) { data.attack_mod += atk - GetBaseAtk(); }

void Game_Actor::SetBaseDef(int def) { data.defense_mod += def - GetBaseDef(); }

void Game_Actor::SetBaseSpi(int spi) { data.spirit_mod += spi - GetBaseSpi(); }

void Game_Actor::SetBaseAgi(int agi) { data.agility_mod += agi - GetBaseAgi(); }

int Game_Actor::GetBattleRow() const { return data.row; }

void Game_Actor::SetBattleRow(int battle_row) { data.row = battle_row; }

int Game_Actor::GetBattleAnimationId() const {
	if (Player::engine == Player::EngineRpg2k) {
		return 0;
	}

	return Data::battleranimations[Data::actors[data.ID - 1].battler_animation - 1].ID;
}

Game_Battler::BattlerType Game_Actor::GetType() const { return Game_Battler::Type_Ally; }
