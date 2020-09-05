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
#include <lcf/data.h>
#include <lcf/rpg/enemy.h>
#include "game_battle.h"
#include "game_enemy.h"
#include "game_party.h"
#include "game_switches.h"
#include <lcf/reader_util.h>
#include "output.h"
#include "utils.h"
#include "player.h"
#include "attribute.h"

namespace {
	constexpr int levitation_frame_count = 14;
	constexpr int levitation_frame_cycle = 20;
}

Game_Enemy::Game_Enemy(const lcf::rpg::TroopMember& member)
	: troop_member(&member)
{
	Transform(troop_member->enemy_id);

	hp = GetMaxHp();
	sp = GetMaxSp();
	SetHidden(troop_member->invisible);
	cycle = Utils::GetRandomNumber(0, levitation_frame_count - 1) * levitation_frame_cycle;

	SetBattlePosition(GetOriginalPosition());
}

int Game_Enemy::MaxHpValue() const {
	return Player::IsRPG2k() ? 9999 : 99999;
}

int Game_Enemy::MaxStatBattleValue() const {
	return 9999;
}

int Game_Enemy::MaxStatBaseValue() const {
	return 999;
}

int Game_Enemy::GetStateProbability(int state_id) const {
	int rate = 2; // C - default

	if (state_id <= (int)enemy->state_ranks.size()) {
		rate = enemy->state_ranks[state_id - 1];
	}

	return GetStateRate(state_id, rate);
}

int Game_Enemy::GetAttributeModifier(int attribute_id) const {
	int rate = 2; // C - default

	if (attribute_id <= (int)enemy->attribute_ranks.size()) {
		rate = enemy->attribute_ranks[attribute_id - 1];
	}

	rate += attribute_shift[attribute_id - 1];
	if (rate < 0) {
		rate = 0;
	} else if (rate > 4) {
		rate = 4;
	}

	return Attribute::GetAttributeRate(attribute_id, rate);
}

void Game_Enemy::SetHp(int _hp) {
	hp = std::min(std::max(_hp, 0), GetMaxHp());
}

void Game_Enemy::SetSp(int _sp) {
	sp = std::min(std::max(_sp, 0), GetMaxSp());
}

Point Game_Enemy::GetOriginalPosition() const {
	return { troop_member->x, troop_member->y };
}

static lcf::rpg::Enemy makeDummyEnemy() {
	lcf::rpg::Enemy enemy;
	enemy.ID = 1;
	return enemy;
}

void Game_Enemy::Transform(int new_enemy_id) {
	enemy = lcf::ReaderUtil::GetElement(lcf::Data::enemies, new_enemy_id);

	if (!enemy) {
		// Some games (e.g. Battle 5 in Embric) have invalid monsters in the battle.
		// This case will fail in RPG Maker and the game will exit with an error message.
		// Create a warning instead and continue the battle.
		Output::Warning("Invalid enemy ID {}", new_enemy_id);
		// This generates an invisible monster with 0 HP
		static auto dummy = makeDummyEnemy();
		enemy = &dummy;
	}
}

int Game_Enemy::GetHitChance() const {
	return enemy->miss ? 70 : 90;
}

float Game_Enemy::GetCriticalHitChance() const {
	return enemy->critical_hit ? (1.0f / enemy->critical_hit_chance) : 0.0f;
}


int Game_Enemy::GetFlyingOffset() const {
	// 2k does not support flying, albeit mentioned in the help file
	if (Player::IsRPG2k3() && enemy->levitate) {
		return flying_offset;
	}
	return 0;
}

void Game_Enemy::UpdateBattle() {
	if (Player::IsRPG2k3() && enemy->levitate) {
		static const int frames[levitation_frame_count] = { 0, 0, 0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1 };

		cycle++;
		// reset animation
		if (cycle >= levitation_frame_count * levitation_frame_cycle) {
			cycle = 0;
		}
		if (cycle % levitation_frame_cycle == 0) {
			flying_offset = frames[cycle / levitation_frame_cycle];
		}
	}
	Game_Battler::UpdateBattle();
}

bool Game_Enemy::IsActionValid(const lcf::rpg::EnemyAction& action) {
	if (action.kind == action.Kind_skill) {
		if (!IsSkillUsable(action.skill_id)) {
			return false;
		}
	}

	switch (action.condition_type) {
	case lcf::rpg::EnemyAction::ConditionType_always:
		return true;
	case lcf::rpg::EnemyAction::ConditionType_switch:
		return Main_Data::game_switches->Get(action.switch_id);
	case lcf::rpg::EnemyAction::ConditionType_turn:
		{
			int turns = Game_Battle::GetTurn();
			return Game_Battle::CheckTurns(turns, action.condition_param2, action.condition_param1);
		}
	case lcf::rpg::EnemyAction::ConditionType_actors:
		{
			std::vector<Game_Battler*> battlers;
			GetParty().GetActiveBattlers(battlers);
			int count = (int)battlers.size();
			return count >= action.condition_param1 && count <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_hp:
		{
			int hp_percent = GetHp() * 100 / GetMaxHp();
			return hp_percent >= action.condition_param1 && hp_percent <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_sp:
		{
			int sp_percent = GetSp() * 100 / GetMaxSp();
			return sp_percent >= action.condition_param1 && sp_percent <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_party_lvl:
		{
			int party_lvl = Main_Data::game_party->GetAverageLevel();
			return party_lvl >= action.condition_param1 && party_lvl <= action.condition_param2;
		}
	case lcf::rpg::EnemyAction::ConditionType_party_fatigue:
		{
			int party_exh = Main_Data::game_party->GetFatigue();
			return party_exh >= action.condition_param1 && party_exh <= action.condition_param2;
		}
	default:
		return true;
	}
}

const lcf::rpg::EnemyAction* Game_Enemy::ChooseRandomAction() {
	if (IsCharged()) {
		static lcf::rpg::EnemyAction normal_atk;
		return &normal_atk;
	}

	const std::vector<lcf::rpg::EnemyAction>& actions = enemy->actions;
	std::vector<int> valid;
	int32_t highest_rating = 0;
	for (int i = 0; i < (int) actions.size(); ++i) {
		const lcf::rpg::EnemyAction& action = actions[i];
		if (IsActionValid(action)) {
			valid.push_back(i);
			highest_rating = std::max(highest_rating, action.rating);
		}
	}

	int total = 0;
	for (auto it = valid.begin(); it != valid.end();) {
		if (actions[*it].rating < highest_rating - 9) {
			it = valid.erase(it);
		} else {
			total += actions[*it].rating;
			++it;
		}
	}

	if (total == 0) {
		return nullptr;
	}

	int which = Utils::GetRandomNumber(0, total - 1);
	for (std::vector<int>::const_iterator it = valid.begin(); it != valid.end(); ++it) {
		const lcf::rpg::EnemyAction& action = actions[*it];
		if (which >= action.rating) {
			which -= action.rating;
			continue;
		}

		return &action;
	}

	return nullptr;
}

