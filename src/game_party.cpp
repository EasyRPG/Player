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
#include <array>
#include <cassert>
#include <cmath>
#include "system.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_player.h"
#include "game_battle.h"
#include "game_targets.h"
#include "game_system.h"
#include "reader_util.h"
#include "output.h"

Game_Party::Game_Party() {
	SetupNewGame();
}

void Game_Party::SetupNewGame() {
	data.Setup();
	RemoveInvalidData();
}

void Game_Party::SetupFromSave(RPG::SaveInventory save) {
	data = std::move(save);
	RemoveInvalidData();

	// Old versions of player didn't sort the inventory, this ensures inventory is sorted
	// as our Game_Party code relies on that. Items in RPG_RT are always sorted in the inventory.
	if (!std::is_sorted(data.item_ids.begin(), data.item_ids.end())) {
		Output::Debug("Loaded Save Game with unsorted inventory! Sorting ...");
		// Resort the inventory.
		struct ItemData { int id; int count; int usage; };

		auto& ids = data.item_ids;
		auto& counts = data.item_counts;
		auto& usages = data.item_usage;

		auto num_items = std::min(ids.size(), std::min(counts.size(), usages.size()));
		std::vector<ItemData> items;
		for (size_t i = 0; i < num_items; ++i) {
			items.push_back(ItemData{ids[i], counts[i], usages[i]});
		}

		std::sort(items.begin(), items.end(), [](const ItemData& l, const ItemData& r) { return l.id < r.id; });

		ids.clear();
		counts.clear();
		usages.clear();

		for (auto& itd: items) {
			ids.push_back(itd.id);
			counts.push_back(itd.count);
			usages.push_back(itd.usage);
		}
	}
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

	for (auto& btdata : Data::system.battletest_data) {
		AddActor(btdata.actor_id);
		Game_Actor* actor = Game_Actors::GetActor(btdata.actor_id);

		// Filter garbage btdata inserted by the editor
		std::array<int, 5> ids = {{ btdata.weapon_id, btdata.shield_id, btdata.armor_id, btdata.helmet_id, btdata.accessory_id }};
		std::replace_if(ids.begin(), ids.end(), [] (const int& item_id) {
			return ReaderUtil::GetElement(Data::items, item_id) == nullptr;
		}, 0);

		actor->SetEquipment(RPG::Item::Type_weapon, ids[0]);
		actor->SetEquipment(RPG::Item::Type_shield, ids[1]);
		actor->SetEquipment(RPG::Item::Type_armor, ids[2]);
		actor->SetEquipment(RPG::Item::Type_helmet, ids[3]);
		actor->SetEquipment(RPG::Item::Type_accessory, ids[4]);
		actor->ChangeLevel(btdata.level, nullptr);
		actor->SetHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
	}

	Main_Data::game_player->Refresh();
}

void Game_Party::GetItems(std::vector<int>& item_list) {
	item_list.clear();

	std::vector<int16_t>::iterator it;
	for (it = data.item_ids.begin(); it != data.item_ids.end(); ++it)
		item_list.push_back(*it);
}

int Game_Party::GetItemCount(int item_id) const {
	auto ip = GetItemIndex(item_id);
	return ip.second ? data.item_counts[ip.first] : 0;
}

int Game_Party::GetEquippedItemCount(int item_id) const {
	int number = 0;
	if (item_id > 0) {
		for (int i = 0; i < (int) data.party.size(); i++) {
			Game_Actor* actor = Game_Actors::GetActor(data.party[i]);
			number += actor->GetItemCount(item_id);
		}
	}
	return number;
}

void Game_Party::GainGold(int n) {
	data.gold = data.gold + n;
	data.gold = std::min<int32_t>(std::max<int32_t>(data.gold, 0), 999999);
}

void Game_Party::LoseGold(int n) {
	data.gold = data.gold - n;
	data.gold = std::min<int32_t>(std::max<int32_t>(data.gold, 0), 999999);
}

void Game_Party::AddItem(int item_id, int amount) {
	if (item_id < 1 || item_id > (int) Data::items.size()) {
		Output::Debug("Can't add item to party. %04d is not a valid item ID.",
						item_id);
		return;
	}

	auto ip = GetItemIndex(item_id);
	auto idx = ip.first;
	auto has = ip.second;
	if (!has) {
		if (amount > 0) {
			amount = std::min(amount, 99);
			data.item_ids.insert(data.item_ids.begin() + idx, (int16_t)item_id);
			data.item_counts.insert(data.item_counts.begin() + idx, (uint8_t)amount);
			data.item_usage.insert(data.item_usage.begin() + idx, 0);
		}
		return;
	}

	int total_items = data.item_counts[idx] + amount;

	if (total_items <= 0) {
		data.item_ids.erase(data.item_ids.begin() + idx);
		data.item_counts.erase(data.item_counts.begin() + idx);
		data.item_usage.erase(data.item_usage.begin() + idx);
		return;
	}

	data.item_counts[idx] = (uint8_t)std::min(total_items, 99);
	// If the item was removed, the number of uses resets.
	// (Adding an item never changes the number of uses, even when
	// you already have x99 of them.)
	if (amount < 0) {
		data.item_usage[idx] = 0;
	}
}

void Game_Party::RemoveItem(int item_id, int amount) {
	AddItem(item_id, -amount);
}

void Game_Party::ConsumeItemUse(int item_id) {
	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);

	if (!item) {
		Output::Warning("ConsumeItemUse: Invalid item ID %d.", item_id);
		return;
	}

	switch (item->type) {
		case RPG::Item::Type_normal:
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			return;
	}

	if (item->uses == 0) {
		// Unlimited uses
		return;
	}

	auto ip = GetItemIndex(item_id);
	auto idx = ip.first;
	auto has = ip.second;

	if (!has) {
		return;
	}

	data.item_usage[idx]++;

	if (data.item_usage[idx] >= item->uses) {
		if (data.item_counts[idx] == 1) {
			// We just used up the last one
			data.item_ids.erase(data.item_ids.begin() + idx);
			data.item_counts.erase(data.item_counts.begin() + idx);
			data.item_usage.erase(data.item_usage.begin() + idx);
		} else {
			data.item_counts[idx]--;
			data.item_usage[idx] = 0;
		}
	}
}

bool Game_Party::IsItemUsable(int item_id, const Game_Actor* target) const {
	if (target && !target->IsItemUsable(item_id)) {
		return false;
	}

	const RPG::Item* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("IsItemUsable: Invalid item ID %d", item_id);
		return false;
	}

	if (data.party.size() == 0) {
		return false;
	}

	switch (item->type) {
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			if (item->use_skill) {
				auto* skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
				if (skill && (
							skill->type == RPG::Skill::Type_escape
							|| skill->type == RPG::Skill::Type_teleport
							|| skill->type == RPG::Skill::Type_switch
							)
				   ) {
					return false;
				}
				return IsSkillUsable(item->skill_id, nullptr, true);
			}
			return false;
		case RPG::Item::Type_special:
			return IsSkillUsable(item->skill_id, nullptr, true);
	}

	if (Game_Battle::IsBattleRunning()) {
		switch (item->type) {
			case RPG::Item::Type_medicine:
				return !item->occasion_field1;
			case RPG::Item::Type_switch:
				return item->occasion_battle;
		}
	} else {
		switch (item->type) {
			case RPG::Item::Type_medicine:
			case RPG::Item::Type_material:
			case RPG::Item::Type_book:
				return true;
			case RPG::Item::Type_switch:
				return item->occasion_field2;
		}
	}

	return false;
}

bool Game_Party::UseItem(int item_id, Game_Actor* target) {
	bool was_used = false;

	auto* item = ReaderUtil::GetElement(Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use item with invalid ID %d", item_id);
		return false;
	}

	bool do_skill = (item->type == RPG::Item::Type_special)
		|| (item->use_skill && (
				item->type == RPG::Item::Type_weapon
				|| item->type == RPG::Item::Type_shield
				|| item->type == RPG::Item::Type_armor
				|| item->type == RPG::Item::Type_helmet
				|| item->type == RPG::Item::Type_accessory
				)
				);

	const RPG::Skill* skill = nullptr;
	if (do_skill) {
		skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
		if (skill == nullptr) {
			Output::Warning("UseItem: Can't use item %d skill with invalid ID %d", item->ID, item->skill_id);
			return false;
		}
	}

	const Game_Actor* fixed_source = nullptr;
	if (skill && skill->scope != RPG::Skill::Scope_self) {
		fixed_source = GetHighestLeveledActorWhoCanUse(item);
		if (fixed_source == nullptr) {
			return false;
		}
	}

	if (target) {
		const auto* source = fixed_source ? fixed_source : target;
		if (IsItemUsable(item_id, source)) {
			was_used = target->UseItem(item_id, source);
		}
	} else {
		for (auto* actor: GetActors()) {
			const auto* source = fixed_source ? fixed_source : actor;
			if (IsItemUsable(item_id, source)) {
				was_used |= actor->UseItem(item_id, source);
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

	if (target && !target->IsSkillUsable(skill_id)) {
		return false;
	}

	const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_id);
	if (!skill) {
		Output::Warning("IsSkillUsable: Can't use skill with invalid ID %d", skill_id);
		return false;
	}

	if (skill->type == RPG::Skill::Type_escape) {
		return !Game_Battle::IsBattleRunning() && Game_System::GetAllowEscape() && Game_Targets::HasEscapeTarget();
	} else if (skill->type == RPG::Skill::Type_teleport) {
		return !Game_Battle::IsBattleRunning() && Game_System::GetAllowTeleport() && Game_Targets::HasTeleportTarget();
	} else if (skill->type == RPG::Skill::Type_normal ||
		skill->type >= RPG::Skill::Type_subskill) {
		int scope = skill->scope;

		if (Game_Battle::IsBattleRunning()) {
			return true;
		}

		// Self targeting skills can not cure states only (except if called by an item).
		// RPG_RT logic...

		if (scope == RPG::Skill::Scope_self) {
			return from_item || skill->affect_hp || skill->affect_sp;
		}

		if (scope == RPG::Skill::Scope_ally ||
			scope == RPG::Skill::Scope_party) {

			if (from_item || skill->affect_hp || skill->affect_sp) {
				return true;
			}
			for (size_t i = 0; i < skill->state_effects.size(); ++i) {
				auto& state = Data::states[i];
				if (skill->state_effects[i] && state.type == RPG::State::Persistence_persists) {
					return true;
				}
			}
			return false;
		}
	} else if (skill->type == RPG::Skill::Type_switch) {
		if (Game_Battle::IsBattleRunning()) {
			return skill->occasion_battle;
		}

		return skill->occasion_field;
	}

	return false;
}

bool Game_Party::UseSkill(int skill_id, Game_Actor* source, Game_Actor* target) {
	bool was_used = false;

	if (target) {
		was_used = target->UseSkill(skill_id, source);
	}
	else {
		std::vector<Game_Actor*> actors = GetActors();
		std::vector<Game_Actor*>::iterator it;
		for (it = actors.begin(); it != actors.end(); ++it) {
			was_used |= (*it)->UseSkill(skill_id, source);
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

int Game_Party::GetActorPositionInParty(int actor_id) {
	std::vector<short>::iterator it = std::find(data.party.begin(), data.party.end(), actor_id);

	return it != data.party.end() ? std::distance(data.party.begin(), it) : -1;
}

std::vector<Game_Actor*> Game_Party::GetActors() const {
	std::vector<Game_Actor*> actors;
	std::vector<int16_t>::const_iterator it;
	for (it = data.party.begin(); it != data.party.end(); ++it)
		actors.push_back(Game_Actors::GetActor(*it));
	return actors;
}

Game_Actor* Game_Party::GetActor(int idx) const {
	if (idx < static_cast<int>(data.party.size())) {
		return Game_Actors::GetActor(data.party[idx]);
	}
	return nullptr;
}

void Game_Party::ApplyDamage(int damage, bool lethal) {
	if (damage <= 0) {
		return;
	}

	std::vector<Game_Actor*> actors = GetActors();

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); i != actors.end(); ++i) {
		Game_Actor* actor = *i;
		actor->ChangeHp(lethal? -damage : - std::max<int>(0, std::min<int>(damage, actor->GetHp() - 1)));
	}
}

void Game_Party::SetTimer(int which, int seconds) {
	switch (which) {
		case Timer1:
			data.timer1_frames = seconds * DEFAULT_FPS + (DEFAULT_FPS - 1);
			Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
			break;
		case Timer2:
			data.timer2_frames = seconds * DEFAULT_FPS + (DEFAULT_FPS -1);
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
	const bool battle = Game_Battle::IsBattleRunning();
	bool seconds_changed = false;

	if (data.timer1_active && (data.timer1_battle || !battle) && data.timer1_frames > 0) {
		data.timer1_frames = data.timer1_frames - 1;

		const int seconds = data.timer1_frames / DEFAULT_FPS;
		const int mod_frames = data.timer1_frames % DEFAULT_FPS;
		seconds_changed |= (mod_frames == (DEFAULT_FPS - 1));

		if (seconds == 0) {
			StopTimer(Timer1);
		}
	}

	if (data.timer2_active && (data.timer2_battle || !battle) && data.timer2_frames > 0) {
		data.timer2_frames = data.timer2_frames - 1;

		const int seconds = data.timer2_frames / DEFAULT_FPS;
		const int mod_frames = data.timer2_frames % DEFAULT_FPS;
		seconds_changed |= (mod_frames == (DEFAULT_FPS - 1));

		if (seconds == 0) {
			StopTimer(Timer2);
		}
	}

	if (seconds_changed) {
		Game_Map::SetNeedRefresh(Game_Map::Refresh_Map);
	}
}

int Game_Party::GetTimerSeconds(int which) {
	return GetTimerFrames(which) / DEFAULT_FPS;
}

int Game_Party::GetTimerFrames(int which) {
	switch (which) {
		case Timer1:
			return data.timer1_frames;
		case Timer2:
			return data.timer2_frames;
		default:
			return 0;
	}
}

bool Game_Party::GetTimerVisible(int which, bool in_battle) {
	bool visible = false;
	bool battle = false;
	switch (which) {
		case Timer1:
			visible = data.timer1_visible;
			battle = data.timer1_battle;
			break;
		case Timer2:
			visible = data.timer2_visible;
			battle = data.timer2_battle;
			break;
	}
	return visible && (!in_battle || battle);
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

void Game_Party::RemoveInvalidData() {
	// Remove non existing actors
	std::vector<int16_t> temp_party;
	std::swap(temp_party, data.party);
	std::vector<int16_t>::iterator it;
	for (it = temp_party.begin(); it != temp_party.end(); ++it) {
		if (Game_Actors::ActorExists(*it)) {
			data.party.push_back(*it);
		} else {
			Output::Warning("Removing invalid party member %d", *it);
		}
	}

	// Remove non existing items
	for (it = data.item_ids.begin(); it != data.item_ids.end(); ) {
		if (!ReaderUtil::GetElement(Data::items, *it)) {
			Output::Warning("Removing invalid item %d from party", *it);
			it = data.item_ids.erase(it);
		} else {
			++it;
		}
	}
}

std::vector<int16_t> Game_Party::GetInflictedStates() const {
	std::vector<int16_t> states;

	for (auto actor : GetActors()) {
		std::vector<int16_t> actor_states = actor->GetInflictedStates();
		states.insert(states.end(), actor_states.begin(), actor_states.end());
	}

	if (!states.empty()) {
		std::sort(states.begin(), states.end());
		states.erase(std::unique(states.begin(), states.end()), states.end());
	}
	return states;
}

bool Game_Party::ApplyStateDamage() {
	bool damage = false;
	std::vector<int16_t> states = GetInflictedStates();

	const auto steps = GetSteps();

	for (auto state_id : states) {
		RPG::State *state = ReaderUtil::GetElement(Data::states, state_id);

		if (state->hp_change_map_steps > 0
				&& state->hp_change_map_val > 0
				&& ((steps % state->hp_change_map_steps) == 0)
				) {
			for (auto actor : GetActors()) {
				if (actor->HasState(state_id)) {
					if (state->hp_change_type == RPG::State::ChangeType_lose) {
						actor->ChangeHp(-std::max<int>(0, std::min<int>(state->hp_change_map_val, actor->GetHp() - 1)));
						damage = true;
					}
					else if (state->hp_change_type == RPG::State::ChangeType_gain) {
						actor->ChangeHp(state->hp_change_map_val);
					}
				}
			}
		}

		if (state->sp_change_map_steps > 0
				&& state->sp_change_map_val > 0
				&& ((steps % state->sp_change_map_steps) == 0)
		   ){
			for (auto actor : GetActors()) {
				if (actor->HasState(state_id)) {
					if (state->sp_change_type == RPG::State::ChangeType_lose) {
						actor->ChangeSp(-state->sp_change_map_val);
						damage = true;
					}
					else if (state->sp_change_type == RPG::State::ChangeType_gain) {
						actor->ChangeSp(state->sp_change_map_val);
					}
				}
			}
		}
	}

	return damage;
}

bool Game_Party::IsAnyControllable() {
	for (auto& actor: GetActors()) {
		if (actor->IsControllable()) {
			return true;
		}
	}
	return false;
}

Game_Actor* Game_Party::GetHighestLeveledActorWhoCanUse(const RPG::Item* item) const {
	Game_Actor* best = nullptr;

	for (auto* actor : GetActors()) {
		if (actor->CanAct()
				&& actor->IsItemUsable(item->ID)
				&& (best == nullptr || best->GetLevel() < actor->GetLevel())) {
			best = actor;
		}
	}
	return best;
}

std::pair<int,bool> Game_Party::GetItemIndex(int item_id) const {
	auto& ids = data.item_ids;
	auto iter = std::lower_bound(ids.begin(), ids.end(), item_id);
	return std::make_pair(iter - ids.begin(), (iter != ids.end() && *iter == item_id));
}
