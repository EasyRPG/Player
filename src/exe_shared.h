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

#ifndef EP_EXE_SHARED_H
#define EP_EXE_SHARED_H

#include <lcf/enum_tags.h>

namespace EXE::Shared {

	enum class GameConstantType {
		MinVarLimit,
		MaxVarLimit,
		MaxActorHP,
		MaxActorSP,
		MaxEnemyHP,
		MaxEnemySP,

		MaxAtkBaseValue,
		MaxDefBaseValue,
		MaxSpiBaseValue,
		MaxAgiBaseValue,

		MaxAtkBattleValue,
		MaxDefBattleValue,
		MaxSpiBattleValue,
		MaxAgiBattleValue,

		MaxDamageValue,
		MaxExpValue,
		MaxLevel,
		MaxGoldValue,
		MaxItemCount,
		MaxSaveFiles,

		/** X-coordinate of the title scene command window (HAlign: Center) */
		TitleX,
		/** Y-coordinate of the title scene command window (VAlign: Top) */
		TitleY,
		/** X-coordinate of the title scene command window when the title graphic is hidden (HAlign: Center) */
		TitleHiddenX,
		/** Y-coordinate of the title scene command window when the title graphic is hidden (VAlign: Top) */
		TitleHiddenY,

		LAST
	};

	static constexpr auto kGameConstantType = lcf::makeEnumTags<GameConstantType>(
		"MinVarLimit",
		"MaxVarLimit",
		"MaxActorHP",
		"MaxActorSP",
		"MaxEnemyHP",
		"MaxEnemySP",

		"MaxAtkBaseValue",
		"MaxDefBaseValue",
		"MaxSpiBaseValue",
		"MaxAgiBaseValue",

		"MaxAtkBattleValue",
		"MaxDefBattleValue",
		"MaxSpiBattleValue",
		"MaxAgiBattleValue",

		"MaxDamageValue",
		"MaxExpValue",
		"MaxLevel",
		"MaxGoldValue",
		"MaxItemCount",
		"MaxSaveFiles",
		"TitleCmdWnd_X",
		"TitleCmdWnd_Y",
		"TitleHiddenCmdWnd_X",
		"TitleHiddenCmdWnd_Y"
	);

	static_assert(kGameConstantType.size() == static_cast<size_t>(GameConstantType::LAST));
}

#endif
