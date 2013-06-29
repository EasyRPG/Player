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
#include "game_enemy.h"

Game_Enemy::Game_Enemy(int enemy_id) {
	Setup(enemy_id);
}

void Game_Enemy::Setup(int enemy_id) {
	Transform(enemy_id);
	hp = GetMaxHp();
	sp = GetMaxSp();
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

	if (hp == 0) {
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
	return x;
}

int Game_Enemy::GetBattleY() const {
	return y;
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

Game_Battler::BattlerType Game_Enemy::GetType() const {
	return Game_Battler::Type_Enemy;
}
