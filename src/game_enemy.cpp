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
#include "data.h"
#include "rpg_enemy.h"
#include "game_battle.h"
#include "game_enemy.h"
#include "game_party.h"
#include "game_switches.h"
#include "output.h"
#include "utils.h"

Game_Enemy::Game_Enemy(int enemy_id) : Game_Battler() {
	Setup(enemy_id);
}

void Game_Enemy::Setup(int enemy_id) {
	Transform(enemy_id);
	hp = GetMaxHp();
	sp = GetMaxSp();
	x = 0;
	y = 0;
	hidden = false;
}

const std::vector<int16_t>& Game_Enemy::GetStates() const {
	return states;
}

std::vector<int16_t>& Game_Enemy::GetStates() {
	return states;
}

const std::string& Game_Enemy::GetName() const {
	return enemy->name;
}

const std::string& Game_Enemy::GetSpriteName() const {
	return enemy->battler_name;
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

	return GetAttributeRate(attribute_id, rate);
}

int Game_Enemy::GetId() const {
	return enemy_id;
}

int Game_Enemy::GetBaseMaxHp() const {
	return enemy->max_hp;
}

int Game_Enemy::GetBaseMaxSp() const {
	return enemy->max_sp;
}

int Game_Enemy::GetBaseAtk() const {
	return enemy->attack;
}

int Game_Enemy::GetBaseDef() const {
	return enemy->defense;
}

int Game_Enemy::GetBaseSpi() const {
	return enemy->spirit;
}

int Game_Enemy::GetBaseAgi() const {
	return enemy->agility;
}

int Game_Enemy::GetHp() const {
	return hp;
}

int Game_Enemy::GetSp() const {
	return sp;
}

void Game_Enemy::SetHp(int _hp) {
	hp = std::min(std::max(_hp, 0), GetMaxHp());
}

void Game_Enemy::ChangeHp(int hp) {
	SetHp(GetHp() + hp);

	if (this->hp == 0) {
		// Death
		SetGauge(0);
		SetDefending(false);
		SetCharged(false);
		RemoveAllStates();
		AddState(1);
	} else {
		// Back to life
		RemoveState(1);
	}
}

void Game_Enemy::SetSp(int _sp) {
	sp = std::min(std::max(_sp, 0), GetMaxSp());
}

int Game_Enemy::GetBattleX() const {
	return (x*SCREEN_TARGET_WIDTH/320);
}

int Game_Enemy::GetBattleY() const {
	return (y*SCREEN_TARGET_HEIGHT/240);
}

void Game_Enemy::SetBattleX(int new_x) {
	x = new_x;
}

void Game_Enemy::SetBattleY(int new_y) {
	y = new_y;
}

int Game_Enemy::GetHue() const {
	return enemy->battler_hue;
}

void Game_Enemy::SetHidden(bool _hidden) {
	hidden = _hidden;
}

bool Game_Enemy::IsHidden() const {
	return hidden;
}

void Game_Enemy::Transform(int new_enemy_id) {
	enemy_id = new_enemy_id;

	if (enemy_id <= 0 || enemy_id > static_cast<int>(Data::enemies.size())) {
		// Some games (e.g. Battle 5 in Embric) have invalid monsters in the battle.
		// This case will fail in RPG Maker and the game will exit with an error message.
		// Create a warning instead and continue the battle.
		Output::Warning("Enemy id %d invalid", new_enemy_id);
		enemy_id = 1;
		// This generates an invisible monster with 0 HP and a minor memory leak
		enemy = new RPG::Enemy();
	} else {
		enemy = &Data::enemies[enemy_id - 1];
	}
}

int Game_Enemy::GetBattleAnimationId() const {
	return 0;
}

int Game_Enemy::GetHitChance() const {
	return enemy->miss ? 70 : 90;
}

float Game_Enemy::GetCriticalHitChance() const {
	return enemy->critical_hit ? (1.0f / enemy->critical_hit_chance) : 0.0f;
}

Game_Battler::BattlerType Game_Enemy::GetType() const {
	return Game_Battler::Type_Enemy;
}

int Game_Enemy::GetExp() const {
	return enemy->exp;
}

int Game_Enemy::GetMoney() const {
	return enemy->gold;
}

int Game_Enemy::GetDropId() const {
	return enemy->drop_id;
}

int Game_Enemy::GetDropProbability() const {
	return enemy->drop_prob;
}

bool Game_Enemy::IsActionValid(const RPG::EnemyAction& action) {
	if (action.kind == action.Kind_skill) {
		if (!IsSkillUsable(action.skill_id)) {
			return false;
		}
	}

	switch (action.condition_type) {
	case RPG::EnemyAction::ConditionType_always:
		return true;
	case RPG::EnemyAction::ConditionType_switch:
		return Game_Switches[action.switch_id];
	case RPG::EnemyAction::ConditionType_turn:
		{
			int turns = Game_Battle::GetTurn();
			return Game_Battle::CheckTurns(turns, action.condition_param2, action.condition_param1);
		}
	case RPG::EnemyAction::ConditionType_actors:
		{
			std::vector<Game_Battler*> battlers;
			GetParty().GetActiveBattlers(battlers);
			int count = (int)battlers.size();
			return count >= action.condition_param1 && count <= action.condition_param2;
		}
	case RPG::EnemyAction::ConditionType_hp:
		{
			int hp_percent = GetHp() * 100 / GetMaxHp();
			return hp_percent >= action.condition_param1 && hp_percent <= action.condition_param2;
		}
	case RPG::EnemyAction::ConditionType_sp:
		{
			int sp_percent = GetSp() * 100 / GetMaxSp();
			return sp_percent >= action.condition_param1 && sp_percent <= action.condition_param2;
		}
	case RPG::EnemyAction::ConditionType_party_lvl:
		{
			int party_lvl = Main_Data::game_party->GetAverageLevel();
			return party_lvl >= action.condition_param1 && party_lvl <= action.condition_param2;
		}
	case RPG::EnemyAction::ConditionType_party_fatigue:
		{
			int party_exh = Main_Data::game_party->GetFatigue();
			return party_exh >= action.condition_param1 && party_exh <= action.condition_param2;
		}
	default:
		return true;
	}
}

const RPG::EnemyAction* Game_Enemy::ChooseRandomAction() {
	if (IsCharged()) {
		return &normal_atk;
	}

	const std::vector<RPG::EnemyAction>& actions = enemy->actions;
	std::vector<int> valid;
	int highest_rating = 0;
	for (int i = 0; i < (int) actions.size(); ++i) {
		const RPG::EnemyAction& action = actions[i];
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
		const RPG::EnemyAction& action = actions[*it];
		if (which >= action.rating) {
			which -= action.rating;
			continue;
		}

		return &action;
	}

	return nullptr;
}
