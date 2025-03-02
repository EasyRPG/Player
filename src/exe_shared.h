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

#include <map>
#include <string>
#include <vector>
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

	enum class EmbeddedStringTypes {
		Battle_DamageToEnemy,
		Battle_DamageToAlly,
		Battle_HpSpRecovery,
		Battle_StatDecrease,
		Battle_StatIncrease,
		Battle_AbsorbEnemy,
		Battle_AbsorbAlly,
		Battle_UseItem,
		Msg_LevelUp,
		Menu_ExpMaxedOut,
		Menu_2k3ActorStatus_ExpMaxedOut,
		LAST
	};

	static constexpr auto kEmbeddedStringTypes = lcf::makeEnumTags<EmbeddedStringTypes>(
		"Battle_DamageToEnemy",
		"Battle_DamageToAlly",
		"Battle_HpSpRecovery",
		"Battle_StatDecrease",
		"Battle_StatIncrease",
		"Battle_AbsorbEnemy",
		"Battle_AbsorbAlly",
		"Battle_UseItem",
		"Msg_LevelUp",
		"Menu_ExpMaxedOut",
		"Menu_2k3ActorStatus_ExpMaxedOut"
	);

	static_assert(kEmbeddedStringTypes.size() == static_cast<size_t>(EmbeddedStringTypes::LAST));

	enum class KnownPatches {
		UnlockPics,
		CommonThisEvent,
		BreakLoopFix,
		AutoEnterPatch,
		BetterAEP,
		PicPointer,
		PicPointer_R,
		DirectMenu,
		MonSca,
		MonScaPlus,
		EXPlus,
		EXPlusPlus,

		LAST
	};

	static constexpr auto kKnownPatches = lcf::makeEnumTags<KnownPatches>(
		"UnlockPics",
		"CommonThisEvent",
		"BreakLoopFix",
		"AutoEnterPatch",
		"BetterAEP",
		"PicPointer 2.5b",
		"PicPointer Restruct",
		"DirectMenu",
		"MonSca",
		"MonScaPlus",
		"EXPlus",
		"EXPlusPlus"
	);

	static_assert(kKnownPatches.size() == static_cast<size_t>(KnownPatches::LAST));

	struct PatchSetupInfo {
		KnownPatches patch_type;
		std::vector<int32_t> customizations;

		PatchSetupInfo()
			: patch_type(static_cast<KnownPatches>(-1)), customizations({}) {
		}

		PatchSetupInfo(KnownPatches patch_type)
			: patch_type(patch_type), customizations({}) {
		}

		PatchSetupInfo(KnownPatches patch_type, std::vector<int32_t> customizations)
		: patch_type(patch_type), customizations(customizations) {
		}
	};

	struct EngineCustomization {
		std::map<GameConstantType, int32_t> constant_overrides;
		std::map<EmbeddedStringTypes, std::string> strings;
		std::map<KnownPatches, PatchSetupInfo> runtime_patches;
	};
}

#endif
