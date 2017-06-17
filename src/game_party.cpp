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
#include <cassert>
#include <cmath>
#include "system.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_player.h"
#include "game_battle.h"
#include "game_targets.h"
#include "game_temp.h"
#include "game_system.h"
#include "output.h"

static RPG::SaveInventory& data = Main_Data::game_data.inventory;

Game_Party::Game_Party() {
	data.Setup();

	// Remove non existing actors
	std::vector<int16_t> temp_party;
	std::swap(temp_party, data.party);
	std::vector<int16_t>::iterator it;
	for (it = temp_party.begin(); it != temp_party.end(); ++it)
		if (Game_Actors::ActorExists(*it))
			data.party.push_back(*it);
	data.party_size = data.party.size();
}

Game_Actor& Game_Party::operator[] (const int index) {
	std::vector<Game_Actor*> actors = GetActors();

	if (index < 0 || (size_t)index >= actors.size()) {
		assert(false && "Subscript out of range");
	}

	return *actors[index];
}

int Game_Party::GetBattlerCount() const {
	return (int)GetActors().size();
}

void Game_Party::SetupBattleTestMembers() {
	Clear();

	std::vector<RPG::TestBattler>::const_iterator it;
	for (it = Data::system.battletest_data.begin();
		it != Data::system.battletest_data.end(); ++it) {
		AddActor(it->actor_id);
		Game_Actor* actor = Game_Actors::GetActor(it->actor_id);
		actor->SetEquipment(RPG::Item::Type_weapon, it->weapon_id);
		actor->SetEquipment(RPG::Item::Type_shield, it->shield_id);
		actor->SetEquipment(RPG::Item::Type_armor, it->armor_id);
		actor->SetEquipment(RPG::Item::Type_helmet, it->helmet_id);
		actor->SetEquipment(RPG::Item::Type_accessory, it->accessory_id);
		actor->ChangeLevel(it->level, false);
		actor->SetHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
	}
	data.party_size = data.party.size();

	Main_Data::game_player->Refresh();
}

void Game_Party::GetItems(std::vector<int>& item_list) {
	item_list.clear();

	std::vector<int16_t>::iterator it;
	for (it = data.item_ids.begin(); it != data.item_ids.end(); ++it)
		item_list.push_back(*it);
}

int Game_Party::GetItemCount(int item_id, bool get_equipped) {
	if (get_equipped && item_id > 0) {
		int number = 0;
		for (int i = 0; i < (int) data.party.size(); i++) {
			Game_Actor* actor = Game_Actors::GetActor(data.party[i]);
			number += actor->GetItemCount(item_id);
		}
		return number;
	} else {
		for (int i = 0; i < (int) data.item_ids.size(); i++)
			if (data.item_ids[i] == item_id)
				return data.item_counts[i];
	}

	return 0;
}

void Game_Party::GainGold(int n) {
	data.gold += n;
	data.gold = std::min(std::max(data.gold, 0), 999999);
}

void Game_Party::LoseGold(int n) {
	data.gold -= n;
	data.gold = std::min(std::max(data.gold, 0), 999999);
}

void Game_Party::AddItem(int item_id, int amount) {
	if (item_id < 1 || item_id > (int) Data::items.size()) {
		Output::Debug("Can't add item to party. %04d is not a valid item ID.",
						item_id);
		return;
	}

	for (int i = 0; i < (int) data.item_ids.size(); i++) {
		if (data.item_ids[i] != item_id)
			continue;

		int total_items = data.item_counts[i] + amount;

		if (total_items <= 0) {
			data.item_ids.erase(data.item_ids.begin() + i);
			data.items_size = data.item_ids.size();
			data.item_counts.erase(data.item_counts.begin() + i);
			data.item_usage.erase(data.item_usage.begin() + i);
			return;
		}

		data.item_counts[i] = (uint8_t)std::min(total_items, 99);
		// If the item was removed, the number of uses resets.
		// (Adding an item never changes the number of uses, even when
		// you already have x99 of them.)
		if (amount < 0) {
			data.item_usage[i] = 0;
		}

		return;
	}

	// Item isn't in the inventory yet

	if (amount <= 0) {
		return;
	}

	data.item_ids.push_back((int16_t)item_id);
	data.items_size = data.item_ids.size();
	data.item_counts.push_back((uint8_t)std::min(amount, 99));
	data.item_usage.push_back(0);
}

void Game_Party::RemoveItem(int item_id, int amount) {
	AddItem(item_id, -amount);
}

void Game_Party::ConsumeItemUse(int item_id) {
	switch (Data::items[item_id - 1].type) {
		case RPG::Item::Type_normal:
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			return;
	}

	if (item_id < 1 || item_id > (int) Data::items.size()) {
		Output::Warning("Can't use up item. %04d is not a valid item ID.",
						item_id);
		return;
	}

	for (int i = 0; i < (int) data.item_ids.size(); i++) {
		if (data.item_ids[i] != item_id)
			continue;

		if (Data::items[item_id - 1].uses == 0) {
			// Unlimited uses
			return;
		}

		data.item_usage[i]++;

		if (data.item_usage[i] >= Data::items[item_id - 1].uses) {
			if (data.item_counts[i] == 1) {
				// We just used up the last one
				data.item_ids.erase(data.item_ids.begin() + i);
				data.items_size = data.item_ids.size();
				data.item_counts.erase(data.item_counts.begin() + i);
				data.item_usage.erase(data.item_usage.begin() + i);
			} else {
				data.item_counts[i]--;
				data.item_usage[i] = 0;
			}
		}
		return;
	}
}

bool Game_Party::IsItemUsable(int item_id, const Game_Actor* target) const {
	if (item_id <= 0 || item_id > (int)Data::items.size()) {
		return false;
	}

	if (target && !target->IsItemUsable(item_id)) {
		return false;
	}

	RPG::Item& item = Data::items[item_id - 1];

	if (item_id > 0 && item_id <= (int)Data::items.size() && data.party.size() > 0) {
		switch (item.type) {
			case RPG::Item::Type_weapon:
			case RPG::Item::Type_shield:
			case RPG::Item::Type_armor:
			case RPG::Item::Type_helmet:
			case RPG::Item::Type_accessory:
				return item.use_skill && IsSkillUsable(item.skill_id, nullptr, true);
			case RPG::Item::Type_special:
				return IsSkillUsable(item.skill_id, nullptr, true);
		}

		if (Game_Temp::battle_running) {
			switch (item.type) {
				case RPG::Item::Type_medicine:
					return !item.occasion_field1;
				case RPG::Item::Type_switch:
					return item.occasion_battle;
			}
		} else {
			switch (item.type) {
				case RPG::Item::Type_medicine:
				case RPG::Item::Type_material:
				case RPG::Item::Type_book:
					return true;
				case RPG::Item::Type_switch:
					return item.occasion_field2;
			}
		}
	}

	return false;
}

bool Game_Party::UseItem(int item_id, Game_Actor* target) {
	bool was_used = false;

	if (target) {
		if (IsItemUsable(item_id, target)) {
			was_used = target->UseItem(item_id);
		}
	} else {
		std::vector<Game_Actor*> actors = GetActors();
		std::vector<Game_Actor*>::iterator it;
		for (it = actors.begin(); it != actors.end(); ++it) {
			if (IsItemUsable(item_id, (*it))) {
				was_used |= (*it)->UseItem(item_id);
			}
		}
	}

	if (was_used) {
		ConsumeItemUse(item_id);
	}

	return was_used;
}

bool Game_Party::IsSkillUsable(int skill_id, const Game_Actor* target, bool from_item) const {
	if (skill_id <= 0 || skill_id > (int)Data::skills.size()) {
		return false;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	if (target && !target->IsSkillUsable(skill_id)) {
		return false;
	}

	if (skill.type == RPG::Skill::Type_escape) {
		return !Game_Temp::battle_running && Game_System::GetAllowEscape() && Game_Targets::HasEscapeTarget();
	} else if (skill.type == RPG::Skill::Type_teleport) {
		return !Game_Temp::battle_running && Game_System::GetAllowTeleport() && Game_Targets::HasTeleportTarget();
	} else if (skill.type == RPG::Skill::Type_normal ||
		skill.type >= RPG::Skill::Type_subskill) {
		int scope = skill.scope;

		if (Game_Temp::battle_running) {
			return true;
		}

		// Self targeting skills can not cure states only (except if called by an item).
		// RPG_RT logic...

		if (scope == RPG::Skill::Scope_self) {
			return from_item || skill.affect_hp || skill.affect_sp;
		}

		if (scope == RPG::Skill::Scope_ally ||
			scope == RPG::Skill::Scope_party) {

			return from_item ||
				skill.affect_hp ||
				skill.affect_sp ||
				!skill.state_effects.empty();
		}
	} else if (skill.type == RPG::Skill::Type_switch) {
		if (Game_Temp::battle_running) {
			return skill.occasion_battle;
		}

		return skill.occasion_field;
	}

	return false;
}

bool Game_Party::UseSkill(int skill_id, Game_Actor* source, Game_Actor* target) {
	bool was_used = false;

	if (target) {
		was_used = target->UseSkill(skill_id);
	}
	else {
		std::vector<Game_Actor*> actors = GetActors();
		std::vector<Game_Actor*>::iterator it;
		for (it = actors.begin(); it != actors.end(); ++it) {
			was_used |= (*it)->UseSkill(skill_id);
		}
	}

	if (was_used) {
		source->SetSp(source->GetSp() - source->CalculateSkillCost(skill_id));
	}

	return was_used;
}

void Game_Party::AddActor(int actor_id) {
	if (IsActorInParty(actor_id))
		return;
	if (data.party.size() >= 4)
		return;
	data.party.push_back((int16_t)actor_id);
	data.party_size = data.party.size();
	Main_Data::game_player->Refresh();
}

void Game_Party::RemoveActor(int actor_id) {
	if (!IsActorInParty(actor_id))
		return;
	data.party.erase(std::find(data.party.begin(), data.party.end(), actor_id));
	data.party_size = data.party.size();
	Main_Data::game_player->Refresh();
}

void Game_Party::Clear() {
	data.party.clear();
	data.party_size = 0;
}

bool Game_Party::IsActorInParty(int actor_id) {
	return std::find(data.party.begin(), data.party.end(), actor_id) != data.party.end();
}

int Game_Party::GetActorPositionInParty(int actor_id) {
	std::vector<short>::iterator it = std::find(data.party.begin(), data.party.end(), actor_id);

	return it != data.party.end() ? std::distance(data.party.begin(), it) : -1;
}

int Game_Party::GetGold() {
	return data.gold;
}

int Game_Party::GetSteps() {
	return data.steps;
}

std::vector<Game_Actor*> Game_Party::GetActors() const {
	std::vector<Game_Actor*> actors;
	std::vector<int16_t>::const_iterator it;
	for (it = data.party.begin(); it != data.party.end(); ++it)
		actors.push_back(Game_Actors::GetActor(*it));
	return actors;
}

int Game_Party::GetBattleCount() {
	return data.battles;
}

int Game_Party::GetWinCount() {
	return data.victories;
}

int Game_Party::GetDefeatCount() {
	return data.defeats;
}

int Game_Party::GetRunCount() {
	return data.escapes;
}


void Game_Party::ApplyDamage(int damage) {
	if (damage <= 0) {
		return;
	}

	std::vector<Game_Actor*> actors = GetActors();

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); i != actors.end(); ++i) {
		Game_Actor* actor = *i;
		actor->SetHp(actor->GetHp() - damage);
	}
}

void Game_Party::SetTimer(int which, int seconds) {
	switch (which) {
		case Timer1:
			data.timer1_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
			break;
		case Timer2:
			data.timer2_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
			break;
	}
}

void Game_Party::StartTimer(int which, bool visible, bool battle) {
	switch (which) {
		case Timer1:
			data.timer1_active = true;
			data.timer1_visible = visible;
			data.timer1_battle = battle;
			break;
		case Timer2:
			data.timer2_active = true;
			data.timer2_visible = visible;
			data.timer2_battle = battle;
			break;
	}
}

void Game_Party::StopTimer(int which) {
	switch (which) {
		case Timer1:
			data.timer1_active = false;
			data.timer1_visible = false;
			break;
		case Timer2:
			data.timer2_active = false;
			data.timer2_visible = false;
			break;
	}
}

void Game_Party::UpdateTimers() {
	bool battle = Game_Temp::battle_running;
	if (data.timer1_active && (data.timer1_battle || !battle) && data.timer1_secs > 0) {
		data.timer1_secs--;
		if (data.timer1_secs % DEFAULT_FPS == 0) {
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
		}
		if (data.timer1_secs == 0) {
			StopTimer(Timer1);
		}
	}
	if (data.timer2_active && (data.timer2_battle || !battle) && data.timer2_secs > 0) {
		data.timer2_secs--;
		if (data.timer2_secs % DEFAULT_FPS == 0) {
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
		}
		if (data.timer2_secs == 0) {
			StopTimer(Timer2);
		}
	}
}

int Game_Party::GetTimer(int which) {
	switch (which) {
		case Timer1:
			return (int)std::ceil(data.timer1_secs / (float)DEFAULT_FPS);
		case Timer2:
			return (int)std::ceil(data.timer2_secs / (float)DEFAULT_FPS);
		default:
			return 0;
	}
}

int Game_Party::GetTimerFrames(int which, bool& visible, bool& battle) {
	switch (which) {
		case Timer1:
			visible = data.timer1_visible;
			battle = data.timer1_battle;
			return data.timer1_secs;
		case Timer2:
			visible = data.timer2_visible;
			battle = data.timer2_battle;
			return data.timer2_secs;
		default:
			return 0;
	}
}

int Game_Party::GetAverageLevel() {
	int party_lvl = 0;

	std::vector<Game_Actor*> actors = GetActors();
	std::vector<Game_Actor*>::iterator it;

	if (actors.empty()) {
		return 0;
	}

	for (it = actors.begin(); it != actors.end(); ++it) {
		party_lvl += (*it)->GetLevel();
	}

	return party_lvl / (int)actors.size();
}

int Game_Party::GetFatigue() {
	std::vector<Game_Actor*> actors = GetActors();
	std::vector<Game_Actor*>::iterator it;

	if (actors.empty()) {
		return 0;
	}

	int hp = 0;
	int total_hp = 0;
	int sp = 0;
	int total_sp = 0;
	for (Game_Actor* a : actors) {
		hp += a->GetHp();
		total_hp += a->GetMaxHp();
		sp += a->GetSp();
		total_sp += a->GetMaxSp();
	}

	// SP is always 33.3% of fatigue, which means a 0 SP actor is never above 66%
	if (total_sp == 0) {
		total_sp = 1;
	}

	return (int)std::ceil(100 - 100.0f * (((float)hp / total_hp * 2.0f + (float)sp / total_sp) / 3.0f));
}
