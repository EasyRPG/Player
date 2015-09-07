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
#include <cstdlib>
#include "data.h"
#include "rpg_enemy.h"
#include "game_battle.h"
#include "game_enemy.h"
#include "game_party.h"
#include "game_switches.h"

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

int Game_Enemy::GetStateProbability(int state_id) {
	int rate = 3; // C - default

	if (state_id <= Data::enemies[enemy_id].state_ranks.size()) {
		rate = Data::enemies[enemy_id].state_ranks[state_id - 1];
	}

	return GetStateRate(state_id, rate);
}

const std::string& Game_Enemy::GetName() const {
	return enemy->name;
}

const std::string& Game_Enemy::GetSpriteName() const {
	return enemy->battler_name;
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
	enemy = &Data::enemies[enemy_id - 1];
}

int Game_Enemy::GetBattleAnimationId() const {
	return 0;
}

int Game_Enemy::GetHitChance() const {
	return enemy->miss ? 70 : 90;
}

int Game_Enemy::GetCriticalHitChance() const {
	return enemy->critical_hit ? enemy->critical_hit_chance : 0;
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
			int count = 0;
			/* TODO
			for (std::vector<Battle::Enemy>::const_iterator it = Game_Battle::enemies.begin(); it != Game_Battle::enemies.end(); it++)
				if (it->game_enemy->Exists())
					count++;*/
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
	int total = 0;
	for (int i = 0; i < (int) actions.size(); ++i) {
		const RPG::EnemyAction& action = actions[i];
		if (IsActionValid(action)) {
			valid.push_back(i);
			total += action.rating;
		}
	}

	if (total == 0) {
		return NULL;
	}

	int which = rand() % total;
	for (std::vector<int>::const_iterator it = valid.begin(); it != valid.end(); ++it) {
		const RPG::EnemyAction& action = actions[*it];
		if (which >= action.rating) {
			which -= action.rating;
			continue;
		}

		return &action;
	}

	return NULL;
}
