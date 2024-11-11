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
#include "scene_battle.h"
#include <lcf/reader_util.h>
#include "output.h"
#include "algo.h"

Game_Party::Game_Party() {
}

void Game_Party::SetupNewGame() {
	Clear();

	data.party = lcf::Data::system.party;
	RemoveInvalidData();
}

void Game_Party::SetupFromSave(lcf::rpg::SaveInventory save) {
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

int Game_Party::GetVisibleBattlerCount() const {
	int visible = 0;
	for (const auto& actor: GetActors()) {
		visible += !actor->IsHidden();
	}
	return visible;
}

void Game_Party::SetupBattleTest() {
	Clear();

	for (auto& btdata : lcf::Data::system.battletest_data) {
		AddActor(btdata.actor_id);
		Game_Actor* actor = Main_Data::game_actors->GetActor(btdata.actor_id);

		// Filter garbage btdata inserted by the editor
		// The upper 16 bit look like uninitialized data
		std::array<int, 5> ids = {{
			btdata.weapon_id & 0xFFFF,
			btdata.shield_id & 0xFFFF,
			btdata.armor_id & 0xFFFF,
			btdata.helmet_id & 0xFFFF,
			btdata.accessory_id & 0xFFFF }};
		std::replace_if(ids.begin(), ids.end(), [] (const int& item_id) {
			return lcf::ReaderUtil::GetElement(lcf::Data::items, item_id) == nullptr;
		}, 0);

		actor->SetEquipment(lcf::rpg::Item::Type_weapon, ids[0]);
		actor->SetEquipment(lcf::rpg::Item::Type_shield, ids[1]);
		actor->SetEquipment(lcf::rpg::Item::Type_armor, ids[2]);
		actor->SetEquipment(lcf::rpg::Item::Type_helmet, ids[3]);
		actor->SetEquipment(lcf::rpg::Item::Type_accessory, ids[4]);
		actor->ChangeLevel(btdata.level, nullptr);
		actor->SetHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
	}
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
	if (item_id >= 0) {
		for (int i = 0; i < (int) data.party.size(); i++) {
			Game_Actor* actor = Main_Data::game_actors->GetActor(data.party[i]);
			number += actor->GetItemCount(item_id);
		}
	}
	return number;
}

int Game_Party::GetItemTotalCount(int item_id) const {
	return GetItemCount(item_id) + GetEquippedItemCount(item_id);
}

int Game_Party::GetMaxItemCount(int item_id) const {
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item || item->easyrpg_max_count == -1) {
		return (lcf::Data::system.easyrpg_max_item_count == -1 ? 99 : lcf::Data::system.easyrpg_max_item_count);
	} else {
		return item->easyrpg_max_count;
	}
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
	if (item_id < 1 || item_id > (int) lcf::Data::items.size()) {
		Output::Debug("Can't add item to party. {} is not a valid item ID.", item_id);
		return;
	}

	int item_limit = GetMaxItemCount(item_id);

	auto ip = GetItemIndex(item_id);
	auto idx = ip.first;
	auto has = ip.second;
	if (!has) {
		if (amount > 0) {
			amount = std::min(amount, item_limit);
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

	data.item_counts[idx] = (uint8_t)std::min(total_items, item_limit);
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
	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);

	if (!item) {
		Output::Warning("ConsumeItemUse: Invalid item ID {}.", item_id);
		return;
	}

	switch (item->type) {
		case lcf::rpg::Item::Type_normal:
		case lcf::rpg::Item::Type_weapon:
		case lcf::rpg::Item::Type_shield:
		case lcf::rpg::Item::Type_armor:
		case lcf::rpg::Item::Type_helmet:
		case lcf::rpg::Item::Type_accessory:
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

	const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("IsItemUsable: Invalid item ID {}", item_id);
		return false;
	}

	const auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
	const bool in_battle = Game_Battle::IsBattleRunning();

	if (item->use_skill) {
		// RPG_RT BUG: Does not check if skill is usable.
		return skill &&
			(in_battle
			 || skill->scope == lcf::rpg::Skill::Scope_self
			 || skill->scope == lcf::rpg::Skill::Scope_ally
			 || skill->scope == lcf::rpg::Skill::Scope_party);
	}

	switch (item->type) {
		case lcf::rpg::Item::Type_medicine:
			return !in_battle || !item->occasion_field1;
		case lcf::rpg::Item::Type_material:
		case lcf::rpg::Item::Type_book:
			return !in_battle;
		case lcf::rpg::Item::Type_switch:
			return in_battle ? item->occasion_battle : item->occasion_field2;
		case lcf::rpg::Item::Type_special:
			if (skill && Algo::IsSkillUsable(*skill, false)) {
				// RPG_RT requires one actor in the party and alive who can use the item.
				// But only if the item invokes a normal or subskill. This check is
				// not performed for escape, teleport, or switch skills!
				if (!Algo::IsNormalOrSubskill(*skill)) {
					return true;
				} else {
					for (auto* actor: GetActors()) {
						if (actor->CanAct() && actor->IsItemUsable(item_id)) {
							return true;
						}
					}
				}
			}
			return false;
		default:
			break;
	}

	return false;
}

bool Game_Party::UseItem(int item_id, Game_Actor* target) {
	bool was_used = false;

	auto* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
	if (!item) {
		Output::Warning("UseItem: Can't use item with invalid ID {}", item_id);
		return false;
	}

	bool do_skill = (item->type == lcf::rpg::Item::Type_special)
		|| (item->use_skill && (
				item->type == lcf::rpg::Item::Type_weapon
				|| item->type == lcf::rpg::Item::Type_shield
				|| item->type == lcf::rpg::Item::Type_armor
				|| item->type == lcf::rpg::Item::Type_helmet
				|| item->type == lcf::rpg::Item::Type_accessory
				)
				);

	const lcf::rpg::Skill* skill = nullptr;
	if (do_skill) {
		skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
		if (skill == nullptr) {
			Output::Warning("UseItem: Can't use item {} skill with invalid ID {}", item->ID, item->skill_id);
			return false;
		}
	}

	const Game_Actor* fixed_source = nullptr;
	if (skill && skill->scope != lcf::rpg::Skill::Scope_self) {
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
	if (skill_id <= 0 || skill_id > (int)lcf::Data::skills.size()) {
		return false;
	}

	if (target && !target->IsSkillUsable(skill_id)) {
		return false;
	}

	const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, skill_id);
	if (!skill) {
		Output::Warning("IsSkillUsable: Can't use skill with invalid ID {}", skill_id);
		return false;
	}

	if (skill->type == lcf::rpg::Skill::Type_escape) {
		return !Game_Battle::IsBattleRunning() && Main_Data::game_system->GetAllowEscape() && Main_Data::game_targets->HasEscapeTarget() && !Main_Data::game_player->IsFlying();
	} else if (skill->type == lcf::rpg::Skill::Type_teleport) {
		return !Game_Battle::IsBattleRunning() && Main_Data::game_system->GetAllowTeleport() && Main_Data::game_targets->HasTeleportTargets() && !Main_Data::game_player->IsFlying();
	} else if (Algo::IsNormalOrSubskill(*skill)) {
		int scope = skill->scope;

		if (Game_Battle::IsBattleRunning()) {
			return true;
		}

		// Self targeting skills can not cure states only (except if called by an item).
		// RPG_RT logic...

		if (scope == lcf::rpg::Skill::Scope_self) {
			return from_item || skill->affect_hp || skill->affect_sp;
		}

		if (scope == lcf::rpg::Skill::Scope_ally ||
			scope == lcf::rpg::Skill::Scope_party) {

			if (from_item || skill->affect_hp || skill->affect_sp) {
				return true;
			}
			for (size_t i = 0; i < skill->state_effects.size(); ++i) {
				auto& state = lcf::Data::states[i];
				if (skill->state_effects[i] && state.type == lcf::rpg::State::Persistence_persists) {
					return true;
				}
			}
			return false;
		}
	} else if (skill->type == lcf::rpg::Skill::Type_switch) {
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
		source->SetHp(source->GetHp() - source->CalculateSkillHpCost(skill_id));
	}

	return was_used;
}

void Game_Party::AddActor(int actor_id) {
	auto* actor = Main_Data::game_actors->GetActor(actor_id);
	if (!actor) {
		return;
	}

	if (IsActorInParty(actor_id))
		return;
	if (data.party.size() >= 4)
		return;
	data.party.push_back((int16_t)actor_id);
	Main_Data::game_player->ResetGraphic();

	auto scene = Scene::Find(Scene::Battle);
	if (scene) {
		scene->OnPartyChanged(actor, true);
	}
}

void Game_Party::RemoveActor(int actor_id) {
	if (!IsActorInParty(actor_id))
		return;
	data.party.erase(std::find(data.party.begin(), data.party.end(), actor_id));
	Main_Data::game_player->ResetGraphic();

	auto* actor = Main_Data::game_actors->GetActor(actor_id);
	if (!actor) {
		return;
	}

	auto scene = Scene::Find(Scene::Battle);
	if (scene) {
		scene->OnPartyChanged(actor, false);
	}
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
		actors.push_back(Main_Data::game_actors->GetActor(*it));
	return actors;
}

Game_Actor* Game_Party::GetActor(int idx) const {
	if (idx >= 0 && idx < static_cast<int>(data.party.size())) {
		return Main_Data::game_actors->GetActor(data.party[idx]);
	}
	return nullptr;
}

void Game_Party::ApplyDamage(int damage, bool lethal) {
	if (damage <= 0) {
		return;
	}

	for (auto* actor: GetActors()) {
		actor->ChangeHp(-damage, lethal);
	}
}

void Game_Party::SetTimer(int which, int seconds) {
	switch (which) {
		case Timer1:
			data.timer1_frames = seconds * DEFAULT_FPS + (DEFAULT_FPS - 1);
			Game_Map::SetNeedRefresh(true);
			break;
		case Timer2:
			data.timer2_frames = seconds * DEFAULT_FPS + (DEFAULT_FPS -1);
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
		Game_Map::SetNeedRefresh(true);
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
	const auto& actors = GetActors();

	if (actors.empty()) {
		return 0;
	}

	int hp = 0;
	int total_hp = 0;
	int sp = 0;
	int total_sp = 0;
	for (auto* a : actors) {
		hp += a->GetHp();
		total_hp += a->GetMaxHp();
		sp += a->GetSp();
		total_sp += a->GetMaxSp();
	}

	// SP is always 33.3% of fatigue, which means a 0 SP actor is never above 66%
	if (total_sp == 0) {
		total_sp = 1;
	}

	auto p = Utils::RoundTo<int>(100.0f * ((static_cast<double>(hp) / total_hp * 2.0 + static_cast<double>(sp) / total_sp) / 3.0f));
	return 100 - p;
}

void Game_Party::RemoveInvalidData() {
	// Remove non existing actors
	std::vector<int16_t> temp_party;
	std::swap(temp_party, data.party);
	std::vector<int16_t>::iterator it;
	for (it = temp_party.begin(); it != temp_party.end(); ++it) {
		if (Main_Data::game_actors->ActorExists(*it)) {
			data.party.push_back(*it);
		} else {
			Output::Debug("Removing invalid party member {}", *it);
		}
	}

	// Remove non existing items
	for (it = data.item_ids.begin(); it != data.item_ids.end(); ) {
		if (!lcf::ReaderUtil::GetElement(lcf::Data::items, *it)) {
			Output::Debug("Removing invalid item {} from party", *it);
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
		lcf::rpg::State *state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);

		if (state->hp_change_map_steps > 0
				&& state->hp_change_map_val > 0
				&& ((steps % state->hp_change_map_steps) == 0)
				) {
			for (auto actor : GetActors()) {
				if (actor->HasState(state_id)) {
					if (state->hp_change_type == lcf::rpg::State::ChangeType_lose) {
						actor->ChangeHp(-state->hp_change_map_val, false);
						damage = true;
					}
					else if (state->hp_change_type == lcf::rpg::State::ChangeType_gain) {
						actor->ChangeHp(state->hp_change_map_val, false);
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
					if (state->sp_change_type == lcf::rpg::State::ChangeType_lose) {
						actor->ChangeSp(-state->sp_change_map_val);
						damage = true;
					}
					else if (state->sp_change_type == lcf::rpg::State::ChangeType_gain) {
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

Game_Actor* Game_Party::GetHighestLeveledActorWhoCanUse(const lcf::rpg::Item* item) const {
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
