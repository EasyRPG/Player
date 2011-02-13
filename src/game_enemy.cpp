/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#include "data.h"
#include "rpg_enemy.h"
#include "game_enemy.h"

////////////////////////////////////////////////////////////
Game_Enemy::Game_Enemy(int enemy_id) {
	Setup(enemy_id);
}

////////////////////////////////////////////////////////////
void Game_Enemy::Setup(int enemy_id) {
	//const RPG::Enemy& enemy = Data::enemies[enemy_id - 1];
	this->enemy_id = enemy_id;
	hp = GetMaxHp();
	sp = GetMaxSp();
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseMaxHp() const {
	return Data::enemies[enemy_id - 1].max_hp;
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseMaxSp() const {
	return Data::enemies[enemy_id - 1].max_sp;
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseAtk() const {
	return Data::enemies[enemy_id - 1].attack;
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseDef() const {
	return Data::enemies[enemy_id - 1].defense;
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseSpi() const {
	return Data::enemies[enemy_id - 1].spirit;
}

////////////////////////////////////////////////////////////
int Game_Enemy::GetBaseAgi() const {
	return Data::enemies[enemy_id - 1].agility;
}

////////////////////////////////////////////////////////////
void Game_Enemy::SetHidden(bool _hidden) {
	hidden = _hidden;
}

////////////////////////////////////////////////////////////
bool Game_Enemy::IsHidden() {
	return hidden;
}

////////////////////////////////////////////////////////////
void Game_Enemy::Transform(int new_enemy_id) {
	enemy_id = new_enemy_id;
}

