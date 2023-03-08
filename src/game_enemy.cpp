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
#include <cmath>
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
#include "rand.h"

Game_Enemy::Game_Enemy(const lcf::rpg::TroopMember* member)
	: troop_member(member)
{
	if (troop_member == nullptr) {
		return;
	}
	Transform(troop_member->enemy_id);

	hp = GetMaxHp();
	sp = GetMaxSp();
	ResetBattle();

	SetHidden(troop_member->invisible);
	SetBattlePosition(GetOriginalPosition());
}

int Game_Enemy::MaxHpValue() const {
	auto& val = lcf::Data::system.easyrpg_max_enemy_hp;
	if (val == -1) {
		return Player::IsRPG2k() ? 9999 : 99999;
	}
	return val;
}

int Game_Enemy::MaxSpValue() const {
	auto& val = lcf::Data::system.easyrpg_max_enemy_sp;
	if (val == -1) {
		return 9999;
	}
	return val;
}

int Game_Enemy::MaxStatBattleValue() const {
	auto& val = lcf::Data::system.easyrpg_max_stat_battle_value;
	if (val == -1) {
		return 9999;
	}
	return val;
}

int Game_Enemy::MaxStatBaseValue() const {
	auto& val = lcf::Data::system.easyrpg_max_stat_base_value;
	if (val == -1) {
		return 999;
	}
	return val;
}

int Game_Enemy::GetStateProbability(int state_id) const {
	int rate = 1; // Enemies have only B as the default state rank

	if (state_id >= 1 && state_id <= (int)enemy->state_ranks.size()) {
		rate = enemy->state_ranks[state_id - 1];
	}

	return GetStateRate(state_id, rate);
}

int Game_Enemy::GetBaseAttributeRate(int attribute_id) const {
	int rate = 2; // C - default

	if (attribute_id >= 1 && attribute_id <= (int)enemy->attribute_ranks.size()) {
		rate = enemy->attribute_ranks[attribute_id - 1];
	}

	return rate;
}

bool Game_Enemy::IsImmuneToAttributeDownshifts() const {
	return enemy->easyrpg_immune_to_attribute_downshifts;
}

int Game_Enemy::SetHp(int _hp) {
	hp = Utils::Clamp(_hp, 0, GetMaxHp());
	return hp;
}

int Game_Enemy::SetSp(int _sp) {
	sp = Utils::Clamp(_sp, 0, GetMaxSp());
	return sp;
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

	auto* sprite = GetEnemyBattleSprite();
	if (sprite) {
		sprite->Refresh();
	}
}

int Game_Enemy::GetHitChance(Weapon) const {
	if (enemy->easyrpg_hit != -1) {
		return enemy->easyrpg_hit;
	} else {
		return enemy->miss ? 70 : 90;
	}
}

float Game_Enemy::GetCriticalHitChance(Weapon) const {
	return enemy->critical_hit ? (1.0f / enemy->critical_hit_chance) : 0.0f;
}

bool Game_Enemy::HasAttackAll(Weapon) const {
	return enemy->easyrpg_attack_all;
}

bool Game_Enemy::AttackIgnoresEvasion(Weapon) const {
	return enemy->easyrpg_ignore_evasion;
}

bool Game_Enemy::PreventsCritical() const {
	return enemy->easyrpg_prevent_critical;
}

bool Game_Enemy::HasPhysicalEvasionUp() const {
	return enemy->easyrpg_raise_evasion;
}

int Game_Enemy::GetFlyingOffset() const {
	// 2k does not support flying, albeit mentioned in the help file
	if (!Player::IsRPG2k3() || !IsFlying()) {
		return 0;
	}

	const auto frame = GetBattleFrameCounter();
	auto offset = Utils::RoundTo<int>(std::sin(2 * M_PI * static_cast<double>(frame) / 256.0) * 4.0);
	return offset;
}

void Game_Enemy::UpdateBattle() {
	if (blink_timer > 0) --blink_timer;
	if (death_timer > 0) --death_timer;
	if (explode_timer > 0) --explode_timer;

	Game_Battler::UpdateBattle();
}

void Game_Enemy::ResetBattle() {
	Game_Battler::ResetBattle();

	blink_timer = 0;
	death_timer = 0;
	explode_timer = 0;
}
