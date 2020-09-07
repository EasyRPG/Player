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

	int MaxStatBattleValue() const override;

	int MaxStatBaseValue() const override;

	Point GetOriginalPosition() const override;

	/**
	 * Gets probability that a state can be inflicted on this actor.
	 *
	 * @param state_id State to test
	 * @return Probability of state infliction
	 */
	int GetStateProbability(int state_id) const override;

	/**
	 * Gets attribute multiplier when the enemy is damaged.
	 *
	 * @param attribute_id Attribute to test
	 * @return Attribute resistence
	 */
	int GetAttributeModifier(int attribute_id) const override;

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
	 * @return attack.
	 */
	int GetBaseAtk(int = kWeaponAll) const override;

	/**
	 * Gets the defense for the current level.
	 *
	 * @return defense.
	 */
	int GetBaseDef(int = kWeaponAll) const override;

	/**
	 * Gets the spirit for the current level.
	 *
	 * @return spirit.
	 */
	int GetBaseSpi(int = kWeaponAll) const override;

	/**
	 * Gets the agility for the current level.
	 *
	 * @return agility.
	 */
	int GetBaseAgi(int = kWeaponAll) const override;

	int GetHue() const override;

	int GetHp() const override;
	void SetHp(int _hp) override;

	int GetSp() const override;
	void SetSp(int _sp) override;

	void Transform(int new_enemy_id);

	int GetHitChance(int = kWeaponAll) const override;
	float GetCriticalHitChance(int = kWeaponAll) const override;
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

	bool IsActionValid(const lcf::rpg::EnemyAction& action);
	const lcf::rpg::EnemyAction* ChooseRandomAction();
	bool IsInParty() const override;

protected:
	const lcf::rpg::Enemy* enemy = nullptr;
	const lcf::rpg::TroopMember* troop_member = nullptr;
	std::vector<int16_t> states;
	int hp = 0;
	int sp = 0;
	int cycle = 0;
	int flying_offset = 0;
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

inline int Game_Enemy::GetBaseMaxHp() const {
	return enemy->max_hp;
}

inline int Game_Enemy::GetBaseMaxSp() const {
	return enemy->max_sp;
}

inline int Game_Enemy::GetBaseAtk(int) const {
	return enemy->attack;
}

inline int Game_Enemy::GetBaseDef(int) const {
	return enemy->defense;
}

inline int Game_Enemy::GetBaseSpi(int) const {
	return enemy->spirit;
}

inline int Game_Enemy::GetBaseAgi(int) const {
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

#endif
