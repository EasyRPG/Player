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

#ifndef EP_GAME_ENEMY_H
#define EP_GAME_ENEMY_H

// Headers
#include "game_battler.h"
#include "sprite_enemy.h"
#include "player.h"
#include <lcf/rpg/enemy.h>
#include <lcf/rpg/enemyaction.h>
#include <lcf/rpg/troopmember.h>

/**
 * Represents a single enemy in the battle scene
 */
class Game_Enemy final : public Game_Battler
{
public:
	explicit Game_Enemy(const lcf::rpg::TroopMember* tm);

	const std::vector<int16_t>& GetStates() const override;
	std::vector<int16_t>& GetStates() override;

	int MaxHpValue() const override;

	int MaxSpValue() const override;

	int MaxStatBattleValue() const override;

	int MaxStatBaseValue() const override;

	Point GetOriginalPosition() const override;

	void ResetBattle() override;

	/**
	 * Gets probability that a state can be inflicted on this actor.
	 *
	 * @param state_id State to test
	 * @return Probability of state infliction
	 */
	int GetStateProbability(int state_id) const override;

	/**
	 * Gets the base attribute rate when actor is damaged.
	 * 
	 * @param attribute_id Attribute to query
	 * @return Attribute rate
	 */
	int GetBaseAttributeRate(int attribute_id) const override;

	/**
	 * Checks if the enemy is immune to attribute downshifts.
	 *
	 * @return if the enemy is immune to attribute downshifts.
	 */
	bool IsImmuneToAttributeDownshifts() const override;

	/**
	 * Gets the enemy ID.
	 *
	 * @return Enemy ID
	 */
	int GetId() const override;

	/**
	 * Gets the characters name
	 *
	 * @return Character name
	 */
	StringView GetName() const override;

	/** @return The troop member id in the battle lineup */
	int GetTroopMemberId() const;

	/**
	 * Gets the filename of the enemy sprite
	 *
	 * @return Filename of enemy sprite
	 */
	StringView GetSpriteName() const override;

	/**
	 * Gets the maximum HP for the current level.
	 *
	 * @return maximum HP.
	 */
	int GetBaseMaxHp() const override;

	/**
	 * Gets the maximum SP for the current level.
	 *
	 * @return maximum SP.
	 */
	int GetBaseMaxSp() const override;

	/**
	 * Gets the attack for the current level.
	 *
	 * @param weapon which weapons to include in calculating result.
	 * @return attack.
	 */
	int GetBaseAtk(Weapon = WeaponAll) const override;

	/**
	 * Gets the defense for the current level.
	 *
	 * @param weapon which weapons to include in calculating result.
	 * @return defense.
	 */
	int GetBaseDef(Weapon = WeaponAll) const override;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @param weapon which weapons to include in calculating result.
	 * @return spirit.
	 */
	int GetBaseSpi(Weapon = WeaponAll) const override;

	/**
	 * Gets the agility for the current level.
	 *
	 * @param weapon which weapons to include in calculating result.
	 * @return agility.
	 */
	int GetBaseAgi(Weapon = WeaponAll) const override;

	int GetHue() const;

	int GetHp() const override;
	int SetHp(int _hp) override;

	int GetSp() const override;
	int SetSp(int _sp) override;

	void Transform(int new_enemy_id);

	/**
	 * Gets the chance to hit for a normal attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return hit rate. [0-100]
	 */
	int GetHitChance(Weapon = WeaponAll) const override;

	/**
	 * Gets the chance to hit for a normal attack.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return hit rate. [0-100]
	 */
	float GetCriticalHitChance(Weapon = WeaponAll) const override;

	/**
	 * Tests if the enemy has a weapon that grants attack all
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return true if a weapon is having attack all attribute
	 */
	bool HasAttackAll(Weapon weapon = WeaponAll) const override;

	/**
	 * Tests if the enemy has a weapon which ignores evasion.
	 *
	 * @param weapon Which weapons to include in calculating result.
	 * @return If the actor has weapon that ignores evasion
	 */
	bool AttackIgnoresEvasion(Weapon weapon = WeaponAll) const override;

	/**
	 * @return If the enemy is protected against critical hits.
	 */
	bool PreventsCritical() const override;

	/**
	 * @return If the enemy has an increased physical evasion rate.
	 */
	bool HasPhysicalEvasionUp() const override;

	int GetBattleAnimationId() const override;

	int GetExp() const;

	int GetMoney() const;

	int GetFlyingOffset() const override;

	void UpdateBattle() override;

	bool IsTransparent() const;

	/**
	 * Get's the ID of the item the enemy drops when defeated.
	 *
	 * @return Dropped item ID, or 0 if no drop
	 */
	int GetDropId() const;

	/**
	 * Get's the probability that the enemy's item will be
	 * dropped.
	 *
	 * @return Probability of drop as a percent (0-100)
	 */
	int GetDropProbability() const;

	BattlerType GetType() const override;

	bool IsInParty() const override;

	/** @return database enemy struct */
	const lcf::rpg::Enemy& GetDbEnemy() const;

	/** @return the blink animation timer */
	int GetBlinkTimer() const;

	/**
	 * Set the blink animation timer
	 *
	 * @param t the value to set.
	 */
	void SetBlinkTimer(int t = 20);

	/** @return the explode animation timer */
	int GetExplodeTimer() const;

	/**
	 * Set the explode animation timer
	 *
	 * @param t the value to set.
	 */
	void SetExplodeTimer(int t = 20);

	/** @return the death animation timer */
	int GetDeathTimer() const;

	/**
	 * Set the death animation timer
	 *
	 * @param t the value to set.
	 */
	void SetDeathTimer(int t = 36);

	/** @return true if enemy is flying */
	bool IsFlying() const;

	/** @return the id of the enemy's unarmed battle animation */
	int GetUnarmedBattleAnimationId() const;

	Sprite_Enemy* GetEnemyBattleSprite() const;

	int GetEnemyAi() const;

	/**
	 * Checks if the enemies defense skill is stronger the usual.
	 *
	 * @return true if strong defense
	 */
	bool HasStrongDefense() const override;

protected:
	const lcf::rpg::Enemy* enemy = nullptr;
	const lcf::rpg::TroopMember* troop_member = nullptr;
	std::vector<int16_t> states;
	int hp = 0;
	int sp = 0;
	int blink_timer = 0;
	int death_timer = 0;
	int explode_timer = 0;
};

inline Game_Battler::BattlerType Game_Enemy::GetType() const {
	return Game_Battler::Type_Enemy;
}

inline int Game_Enemy::GetExp() const {
	return enemy->exp;
}

inline int Game_Enemy::GetMoney() const {
	return enemy->gold;
}

inline int Game_Enemy::GetDropId() const {
	return enemy->drop_id;
}

inline int Game_Enemy::GetDropProbability() const {
	return enemy->drop_prob;
}

inline bool Game_Enemy::IsTransparent() const {
	return enemy->transparent;
}

inline int Game_Enemy::GetBattleAnimationId() const {
	return 0;
}

inline int Game_Enemy::GetHue() const {
	return enemy->battler_hue;
}

inline int Game_Enemy::GetId() const {
	return enemy->ID;
}

inline int Game_Enemy::GetTroopMemberId() const {
	return troop_member->ID;
}

inline int Game_Enemy::GetBaseMaxHp() const {
	return enemy->max_hp;
}

inline int Game_Enemy::GetBaseMaxSp() const {
	return enemy->max_sp;
}

inline int Game_Enemy::GetBaseAtk(Weapon) const {
	return enemy->attack;
}

inline int Game_Enemy::GetBaseDef(Weapon) const {
	return enemy->defense;
}

inline int Game_Enemy::GetBaseSpi(Weapon) const {
	return enemy->spirit;
}

inline int Game_Enemy::GetBaseAgi(Weapon) const {
	return enemy->agility;
}

inline int Game_Enemy::GetHp() const {
	return hp;
}

inline int Game_Enemy::GetSp() const {
	return sp;
}

inline const std::vector<int16_t>& Game_Enemy::GetStates() const {
	return states;
}

inline std::vector<int16_t>& Game_Enemy::GetStates() {
	return states;
}

inline StringView Game_Enemy::GetName() const {
	return enemy->name;
}

inline StringView Game_Enemy::GetSpriteName() const {
	return enemy->battler_name;
}

inline bool Game_Enemy::IsInParty() const {
	// Enemies can never be removed from enemy party
	return true;
}

inline const lcf::rpg::Enemy& Game_Enemy::GetDbEnemy() const {
	return *enemy;
}

inline int Game_Enemy::GetBlinkTimer() const {
	return blink_timer;
}

inline void Game_Enemy::SetBlinkTimer(int t) {
	blink_timer = t;
}

inline int Game_Enemy::GetExplodeTimer() const {
	return explode_timer;
}

inline void Game_Enemy::SetExplodeTimer(int t) {
	explode_timer = t;
}

inline int Game_Enemy::GetDeathTimer() const {
	return death_timer;
}

inline void Game_Enemy::SetDeathTimer(int t) {
	death_timer = t;
}

inline bool Game_Enemy::IsFlying() const {
	return enemy->levitate;
}

inline int Game_Enemy::GetUnarmedBattleAnimationId() const {
	return enemy->maniac_unarmed_animation;
}

inline Sprite_Enemy* Game_Enemy::GetEnemyBattleSprite() const {
	return static_cast<Sprite_Enemy*>(Game_Battler::GetBattleSprite());
}

inline int Game_Enemy::GetEnemyAi() const {
	return enemy->easyrpg_enemyai;
}

inline bool Game_Enemy::HasStrongDefense() const {
	return enemy->easyrpg_super_guard;
}

#endif
