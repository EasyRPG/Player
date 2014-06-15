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
#include "system.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_player.h"
#include "game_battle.h"
#include "game_temp.h"
#include "output.h"
#include "util_macro.h"

static RPG::SaveInventory& data = Main_Data::game_data.inventory;

Game_Party::Game_Party() {
	data.Setup();
}

Game_Actor& Game_Party::operator[] (const int index) {
	std::vector<Game_Actor*> actors = GetActors();

	if (index < 0 || (size_t)index >= actors.size()) {
		assert(false && "Subscript out of range");
	}

	return *actors[index];
}

int Game_Party::GetBattlerCount() const {
	return GetActors().size();
}

void Game_Party::SetupBattleTestMembers() {
	data.party.clear();
	
	std::vector<RPG::TestBattler>::const_iterator it;
	for (it = Data::system.battletest_data.begin();
		it != Data::system.battletest_data.end(); ++it) {
		AddActor(it->actor_id);
		Game_Actor* actor = Game_Actors::GetActor(it->actor_id);
		actor->SetEquipment(0, it->weapon_id);
		actor->SetEquipment(1, it->shield_id);
		actor->SetEquipment(1, it->armor_id);
		actor->SetEquipment(1, it->helmet_id);
		actor->SetEquipment(1, it->accessory_id);
		actor->ChangeLevel(it->level, false);
		actor->SetHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
	}

	Main_Data::game_player->Refresh();
}

void Game_Party::GetItems(std::vector<int>& item_list) {
	item_list.clear();

	std::vector<int16_t>::iterator it;
	for (it = data.item_ids.begin(); it != data.item_ids.end(); it++)
		item_list.push_back(*it);
}

int Game_Party::GetItemCount(int item_id, bool get_equipped) {
	if (get_equipped && item_id > 0) {
		int number = 0;
		for (int i = 0; i < (int) data.party.size(); i++) {
			Game_Actor* actor = Game_Actors::GetActor(data.party[i]);
			if (actor->GetWeaponId() == item_id) {
				++number;
			}
			if (actor->GetShieldId() == item_id) {
				++number;
			}
			if (actor->GetArmorId() == item_id) {
				++number;
			}
			if (actor->GetHelmetId() == item_id) {
				++number;
			}
			if (actor->GetAccessoryId() == item_id) {
				++number;
			}
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
		Output::Warning("Can't add item to party.\n%04d is not a valid item ID.",
						item_id);
		return;
	}

	for (int i = 0; i < (int) data.item_ids.size(); i++) {
		if (data.item_ids[i] != item_id)
			continue;

		int total_items = data.item_counts[i] + amount;

		if (total_items <= 0) {
			data.item_ids.erase(data.item_ids.begin() + i);
			data.item_counts.erase(data.item_counts.begin() + i);
			data.item_usage.erase(data.item_usage.begin() + i);
			return;
		}

		data.item_counts[i] = (uint8_t)std::min(total_items, 99);
		return;
	}

	// Item isn't in the inventory yet
	
	if (amount <= 0) {
		return;
	}

	data.item_ids.push_back((int16_t)item_id);
	data.item_counts.push_back((uint8_t)std::min(amount, 99));
	data.item_usage.push_back((uint8_t)Data::items[item_id - 1].uses);
}

void Game_Party::RemoveItem(int item_id, int amount) {
	AddItem(item_id, -amount);
}

bool Game_Party::IsItemUsable(int item_id) {
	RPG::Item& item = Data::items[item_id - 1];

	if (item_id > 0 && item_id <= (int)Data::items.size() && data.party.size() > 0) {
		if (Game_Temp::battle_running) {
			if (item.type == RPG::Item::Type_medicine) {
				return !item.occasion_field1;
			}
			else if (item.type == RPG::Item::Type_switch) {
				return item.occasion_battle;
			}
			else if (item.type == RPG::Item::Type_special) {
				// ToDo: Proper check
				return true;
			}
		} else {
			if (item.type == RPG::Item::Type_medicine ||
				item.type == RPG::Item::Type_material ||
				item.type == RPG::Item::Type_book) {
					return true;
			} else if (item.type == RPG::Item::Type_switch) {
				return item.occasion_field2;
			}
		}
	}

	return false;
}

bool Game_Party::UseItem(int item_id, Game_Actor* target) {
	bool was_used = false;

	if (target) {
		was_used = target->UseItem(item_id);
	} else {
		std::vector<Game_Actor*> actors = GetActors();
		std::vector<Game_Actor*>::iterator it;
		for (it = actors.begin(); it != actors.end(); ++it) {
			was_used |= (*it)->UseItem(item_id);
		}
	}

	// Todo usage count
	if (was_used) {
		RemoveItem(item_id, 1);
	}

	return was_used;
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
	Main_Data::game_player->Refresh();
}

void Game_Party::RemoveActor(int actor_id) {
	if (!IsActorInParty(actor_id))
		return;
	data.party.erase(std::find(data.party.begin(), data.party.end(), actor_id));
	Main_Data::game_player->Refresh();
}

void Game_Party::Clear() {
	data.party.clear();
}

bool Game_Party::IsActorInParty(int actor_id) {
	return std::find(data.party.begin(), data.party.end(), actor_id) != data.party.end();
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
	for (it = data.party.begin(); it != data.party.end(); it++)
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

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); i != actors.end(); i++) {
		Game_Actor* actor = *i;
		actor->SetHp(actor->GetHp() - damage);
	}
}

void Game_Party::SetTimer(int which, int seconds) {
	switch (which) {
		case Timer1:
			data.timer1_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(true);
			break;
		case Timer2:
			data.timer2_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(true);
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
	if (data.timer1_active && (!data.timer1_battle || !battle) && data.timer1_secs > 0) {
		data.timer1_secs--;
		Game_Map::SetNeedRefresh(true);
	} 
	if (data.timer2_active && (!data.timer2_battle || !battle) && data.timer2_secs > 0) {
		data.timer2_secs--;
		Game_Map::SetNeedRefresh(true);
	}
}

int Game_Party::ReadTimer(int which) {
	switch (which) {
		case Timer1:
			return data.timer1_secs;
		case Timer2:
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

	return party_lvl /= actors.size();
}

int Game_Party::GetFatigue() {
	int party_exh = 0;
	std::vector<Game_Actor*> actors = GetActors();
	std::vector<Game_Actor*>::iterator it;

	if (actors.empty()) {
		return 0;
	}

	for (it = actors.begin(); it != actors.end(); ++it) {
		// FIXME: this is what the help file says, but it looks wrong
		party_exh += 100 - (200 * (*it)->GetHp() / (*it)->GetMaxHp() -
			100 * (*it)->GetSp() / (*it)->GetMaxSp() / 3);
	}

	return party_exh /= actors.size();
}
