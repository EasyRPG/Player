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

#ifndef EP_GAME_CONSTANTS_H
#define EP_GAME_CONSTANTS_H

// Headers
#include <cstdint>
#include <lcf/enum_tags.h>
#include <unordered_map>
#include "game_variables.h"

/**
 * Implements Constants (e.g. stat caps) used by the engine and allows
 * overwriting them based on engine version or patches.
 */
class Game_Constants {
public:
	Game_Constants() = default;

	/** RPG2k level cap */
	static const constexpr int32_t max_level_2k = 50;
	/** RPG2k3 level cap */
	static const constexpr int32_t max_level_2k3 = 99;

	enum class ConstantType {
		MinVarLimit,
		MaxVarLimit,
		MaxActorHP,
		MaxActorSP,
		MaxStatBaseValue,
		MaxStatBattleValue,
		MaxDamageValue,
		MaxExpValue,
		MaxLevel,
		MaxGoldValue,
		MaxItemCount,
		MaxSaveFiles
	};

	static constexpr auto kConstantType = lcf::makeEnumTags<ConstantType>(
		"MinVarLimit",
		"MaxVarLimit",
		"MaxActorHP",
		"MaxActorSP",
		"MaxStatBaseValue",
		"MaxStatBattleValue",
		"MaxDamageValue",
		"MaxExpValue",
		"MaxLevel",
		"MaxGoldValue",
		"MaxItemCount",
		"MaxSaveFiles"
	);

	/** @return Variable upper and lower limit; default: ±999,999 (2k), ±9,999,9999 (2k3) */
	std::array<Game_Variables::Var_t, 2> GetVariableLimits();

	/** @return Max HP for Actors; default: 999 (2k), 9999 (2k3) */
	int32_t MaxActorHpValue();

	/** @return Max SP for Actors (default: 999) */
	int32_t MaxActorSpValue();

	/** @return Max HP for Enemies; default: uncapped */
	int32_t MaxEnemyHpValue();

	/** @return Max SP for Enemies; default: uncapped */
	int32_t MaxEnemySpValue();

	/** @return Max base stat (Attack, Defense, Spirit, Agility) value; default: 999 */
	int32_t MaxStatBaseValue();

	/** @return Max stat (Attack, Defense, Spirit, Agility) with modifiers value; default: 9999 */
	int32_t MaxStatBattleValue();

	/** @return Max attack damage; default: 999 (2k), 9999 (2k3) */
	int32_t MaxDamageValue();

	/** @return Max Experience; default: 999,999 (2k), 9,999,999 (2k3) */
	int32_t MaxExpValue();

	/** @return Level cap; default: 50 (2k), 99 (2k3) */
	int32_t MaxLevel();

	/** @return Gold limit; default: 999,999 */
	int32_t MaxGoldValue();

	/** @return Max amount per item; default: 99 */
	int32_t MaxItemCount();

	/** @return Save slots displayed in the Save/Load scenes; default: 15 */
	int32_t MaxSaveFiles();

	/**
	 * Overrides one of the constants.
	 *
	 * @param const_type Constant to override
	 * @param value new constant value
	 */
	void OverrideGameConstant(ConstantType const_type, int32_t value);

	/**
	 * Prints a list of overwritten constants to the log
	 */
	void PrintActiveOverrides();

	/** Patches detected by the EXE Reader */
	enum class KnownPatchConfigurations {
		Rm2k3_Italian_WD_108,		// Italian "WhiteDragon" patch
		StatDelimiter,
		LAST
	};

	static constexpr auto kKnownPatchConfigurations = lcf::makeEnumTags<KnownPatchConfigurations>(
		"Rm2k3 Italian 1.08",
		"StatDelimiter"
	);

	static_assert(kKnownPatchConfigurations.size() == static_cast<size_t>(KnownPatchConfigurations::LAST));

	using T = ConstantType;

	/** Constants of known patches */
	inline static const std::unordered_map<KnownPatchConfigurations, std::unordered_map<T, int32_t>> known_patch_configurations = {
		{
			KnownPatchConfigurations::Rm2k3_Italian_WD_108, {
				{ T::MinVarLimit,	-999999999 },
				{ T::MaxVarLimit,	 999999999 },
				{ T::MaxActorHP,	     99999 },
				{ T::MaxActorSP,          9999 },
				{ T::MaxStatBaseValue,    9999 },
				{ T::MaxDamageValue,     99999 },
				{ T::MaxGoldValue,     9999999 }
			}
		},{
			KnownPatchConfigurations::StatDelimiter, {
				{ T::MaxActorHP,           9999999 },
				{ T::MaxActorSP,           9999999 },
				{ T::MaxStatBaseValue,      999999 },
				{ T::MaxStatBattleValue,    999999 }
			}
		}
	};

	private:
		bool TryGetOverriddenConstant(ConstantType const_type, int32_t& out_value);

		std::unordered_map<ConstantType, int32_t> constant_overrides;
};

#endif
