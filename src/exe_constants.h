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

#ifndef EP_EXE_CONSTANTS_H
#define EP_EXE_CONSTANTS_H

#include "exe_buildinfo.h"
#include "exe_shared.h"

namespace EXE::Constants {
	using namespace BuildInfo;
	using GameConstantType = EXE::Shared::GameConstantType;

	enum class KnownPatchConfigurations {
		Rm2k3_Italian_WD_108,		// Italian "WhiteDragon" patch
		QP_StatDelimiter,
		LAST
	};

	static constexpr auto kKnownPatchConfigurations = lcf::makeEnumTags<KnownPatchConfigurations>(
		"Rm2k3 Italian 1.08",
		"QuickPatch StatDelimiter"
	);

	static_assert(kKnownPatchConfigurations.size() == static_cast<size_t>(KnownPatchConfigurations::LAST));

	using patch_config = std::map<GameConstantType, int32_t>;
	using T = GameConstantType;

	const std::map<KnownPatchConfigurations, patch_config> known_patch_configurations = {
		{
			KnownPatchConfigurations::Rm2k3_Italian_WD_108, {
				{ T::MinVarLimit,	-999999999 },
				{ T::MaxVarLimit,	 999999999 },
				{ T::MaxEnemyHP,     999999999 },
				{ T::MaxEnemySP,     999999999 },
				{ T::MaxActorHP,         99999 },
				{ T::MaxActorSP,          9999 },
				{ T::MaxAtkBaseValue,     9999 },
				{ T::MaxDefBaseValue,     9999 },
				{ T::MaxSpiBaseValue,     9999 },
				{ T::MaxAgiBaseValue,     9999 },
				{ T::MaxDamageValue,     99999 },
				{ T::MaxGoldValue,     9999999 }
		}},{
			KnownPatchConfigurations::QP_StatDelimiter, {
				{ T::MaxActorHP,       9999999 },
				{ T::MaxActorSP,       9999999 },
				{ T::MaxAtkBaseValue,   999999 },
				{ T::MaxDefBaseValue,   999999 },
				{ T::MaxSpiBaseValue,   999999 },
				{ T::MaxAgiBaseValue,   999999 },
				{ T::MaxAtkBattleValue, 999999 },
				{ T::MaxDefBattleValue, 999999 },
				{ T::MaxSpiBattleValue, 999999 },
				{ T::MaxAgiBattleValue, 999999 }
		}}
	};

	using code_address = std::pair<GameConstantType, CodeAddressInfo>;
	using code_address_map = std::array<code_address, static_cast<size_t>(GameConstantType::LAST)>;

#define ADD_EAX_ESI 0x03, 0xC6
#define ADD_EDX_ESI 0x03, 0xD6
#define MOV_EAX		0xB8
#define MOV_ECX		0xB9
#define MOV_EDX		0xBA
#define SUB_EDX_EBX 0x2B, 0xD3
#define CMP_DWORD_ESP 0x81, 0x7C, 0x24
#define CMP_ESI		0x81, 0xFE
#define CMP_EAX_BYTE 0x83, 0xF8
#define CMP_EBX_BYTE 0x83, 0xFB

//#define DEPENDS_ON_PREVIOUS { 0xFF, 0xFF, 0x00, 0xFF }
//
//	constexpr auto magic_prev = std::array<uint8_t, 4 >(DEPENDS_ON_PREVIOUS);

	template<typename T, GameConstantType C, typename... Args>
	constexpr code_address map(T default_val, size_t code_offset, Args&&... args) {
		return { C, CodeAddressInfo(default_val, sizeof(T), code_offset, std::forward<decltype(args)>(args)...) };
	}

	template<GameConstantType C>
	constexpr code_address not_def() {
		return { C, CodeAddressInfo(0, 0, 0) };
	}

	using engine_code_adresses_rm2k = std::array<std::pair<KnownEngineBuildVersions, code_address_map>, static_cast<size_t>(count_known_rm2k_builds)>;
	using engine_code_adresses_rm2k3 = std::array<std::pair<KnownEngineBuildVersions, code_address_map>, static_cast<size_t>(count_known_rm2k3_builds)>;

	constexpr code_address_map empty_code_map = {{
		not_def<T::MinVarLimit>(),
		not_def<T::MaxVarLimit>(),
		not_def<T::TitleX>(),
		not_def<T::TitleY>(),
		not_def<T::TitleHiddenX>(),
		not_def<T::TitleHiddenY>(),

		not_def<T::MaxActorHP>(),
		not_def<T::MaxActorSP>(),
		not_def<T::MaxEnemyHP>(),
		not_def<T::MaxEnemySP>(),

		not_def<T::MaxAtkBaseValue>(),
		not_def<T::MaxDefBaseValue>(),
		not_def<T::MaxSpiBaseValue>(),
		not_def<T::MaxAgiBaseValue>(),

		not_def<T::MaxAtkBattleValue>(),
		not_def<T::MaxDefBattleValue>(),
		not_def<T::MaxSpiBattleValue>(),
		not_def<T::MaxAgiBattleValue>(),

		not_def<T::MaxDamageValue>(),
		not_def<T::MaxExpValue>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>(),
		not_def<T::MaxLevel>()
	}};

	constexpr engine_code_adresses_rm2k known_engine_builds_rm2k = {{
		{
			RM2K_20000306,
			empty_code_map
		}, {
			RM2K_2000XXXX_UNK,
			{{
				map<int32_t, T::MinVarLimit>        ( -999999, 0x085A0C, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        (  999999, 0x085A36, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x06D039, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x06D040, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x06D05B, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x06D062, SUB_EDX_EBX, MOV_ECX),

				not_def<T::MaxActorHP>(),
				not_def<T::MaxActorSP>(),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				not_def<T::MaxAtkBaseValue>(),
				not_def<T::MaxDefBaseValue>(),
				not_def<T::MaxSpiBaseValue>(),
				not_def<T::MaxAgiBaseValue>(),

				not_def<T::MaxAtkBattleValue>(),
				not_def<T::MaxDefBattleValue>(),
				not_def<T::MaxSpiBattleValue>(),
				not_def<T::MaxAgiBattleValue>(),

				not_def<T::MaxDamageValue>(),
				not_def<T::MaxExpValue>(),
				not_def<T::MaxGoldValue>(),
				not_def<T::MaxItemCount>(),
				not_def<T::MaxSaveFiles>(),
				not_def<T::MaxLevel>()
			}}
		}, {
			RM2K_20000507,
			empty_code_map
		},{
			RM2K_20000619,
			empty_code_map
		},{
			RM2K_20000711,
			{{
				map<int32_t, T::MinVarLimit>        ( -999999, 0x0846A8, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        (  999999, 0x0846D2, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x06E491, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x06E498, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x06E4B3, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x06E4BA, SUB_EDX_EBX, MOV_ECX),

				not_def<T::MaxActorHP>(),
				not_def<T::MaxActorSP>(),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				not_def<T::MaxAtkBaseValue>(),
				not_def<T::MaxDefBaseValue>(),
				not_def<T::MaxSpiBaseValue>(),
				not_def<T::MaxAgiBaseValue>(),

				not_def<T::MaxAtkBattleValue>(),
				not_def<T::MaxDefBattleValue>(),
				not_def<T::MaxSpiBattleValue>(),
				not_def<T::MaxAgiBattleValue>(),

				not_def<T::MaxDamageValue>(),
				not_def<T::MaxExpValue>(),
				not_def<T::MaxGoldValue>(),
				not_def<T::MaxItemCount>(),
				not_def<T::MaxSaveFiles>(),
				not_def<T::MaxLevel>()
			}}
		},{
			RM2K_20001113,
			empty_code_map
		},{
			RM2K_20001115,
			empty_code_map
		},{
			RM2K_20001227,
			{{
				map<int32_t, T::MinVarLimit>        ( -999999, 0x085D78, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        (  999999, 0x085DA2, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x06D5B9, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x06D5C0, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x06D5DB, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x06D5E2, SUB_EDX_EBX, MOV_ECX),

				not_def<T::MaxActorHP>(),
				not_def<T::MaxActorSP>(),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				not_def<T::MaxAtkBaseValue>(),
				not_def<T::MaxDefBaseValue>(),
				not_def<T::MaxSpiBaseValue>(),
				not_def<T::MaxAgiBaseValue>(),

				not_def<T::MaxAtkBattleValue>(),
				not_def<T::MaxDefBattleValue>(),
				not_def<T::MaxSpiBattleValue>(),
				not_def<T::MaxAgiBattleValue>(),

				not_def<T::MaxDamageValue>(),
				not_def<T::MaxExpValue>(),
				not_def<T::MaxGoldValue>(),
				not_def<T::MaxItemCount>(),
				not_def<T::MaxSaveFiles>(),
				not_def<T::MaxLevel>()
			}}
		},{
			RM2K_20010505,
			empty_code_map
		},{
			RM2K_20030327,
			empty_code_map
		},{
			RM2K_20030625,
			empty_code_map
		},{
			RM2KE_160,
			empty_code_map
		},{
			RM2KE_161,
			empty_code_map
		},{
			RM2KE_162,
			empty_code_map
		}
	}};

	constexpr engine_code_adresses_rm2k3 known_engine_builds_rm2k3 = {{
		{
			RM2K3_100,
			empty_code_map
		}, {
			RM2K3_UNK_1,
			empty_code_map
		},{
			RM2K3_UNK_2,
			empty_code_map
		}, {
			RM2K3_1021_1021,
			empty_code_map
		},{
			RM2K3_1030_1030_1,
			empty_code_map
		}, {
			RM2K3_1030_1030_2,
			empty_code_map
		},{
			RM2K3_1030_1040,
			{{
				map<int32_t, T::MinVarLimit>        (-9999999, 0x0A60B3, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        ( 9999999, 0x0A60DD, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x08AC49, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x08AC50, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x08AC6B, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x08AC72, SUB_EDX_EBX, MOV_ECX),

				not_def<T::MaxActorHP>(),
				not_def<T::MaxActorSP>(),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				not_def<T::MaxAtkBaseValue>(),
				not_def<T::MaxDefBaseValue>(),
				not_def<T::MaxSpiBaseValue>(),
				not_def<T::MaxAgiBaseValue>(),

				not_def<T::MaxAtkBattleValue>(),
				not_def<T::MaxDefBattleValue>(),
				not_def<T::MaxSpiBattleValue>(),
				not_def<T::MaxAgiBattleValue>(),

				not_def<T::MaxDamageValue>(),
				not_def<T::MaxExpValue>(),
				not_def<T::MaxGoldValue>(),
				not_def<T::MaxItemCount>(),
				not_def<T::MaxSaveFiles>(),
				not_def<T::MaxLevel>()
			}}
		}, {
			RM2K3_1050_1050_1,
			empty_code_map
		},{
			RM2K3_1050_1050_2,
			empty_code_map
		}, {
			RM2K3_1060_1060,
			{{
				map<int32_t, T::MinVarLimit>        (-9999999, 0x0AC4F7, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        ( 9999999, 0x0AC521, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x08FB6D, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x08FB74, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x08FB8F, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x08FB96, SUB_EDX_EBX, MOV_ECX),

				not_def<T::MaxActorHP>(),
				not_def<T::MaxActorSP>(),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				not_def<T::MaxAtkBaseValue>(),
				not_def<T::MaxDefBaseValue>(),
				not_def<T::MaxSpiBaseValue>(),
				not_def<T::MaxAgiBaseValue>(),

				not_def<T::MaxAtkBattleValue>(),
				not_def<T::MaxDefBattleValue>(),
				not_def<T::MaxSpiBattleValue>(),
				not_def<T::MaxAgiBattleValue>(),

				not_def<T::MaxDamageValue>(),
				not_def<T::MaxExpValue>(),
				not_def<T::MaxGoldValue>(),
				not_def<T::MaxItemCount>(),
				not_def<T::MaxSaveFiles>(),
				not_def<T::MaxLevel>()
			}}
		},{
			RM2K3_1070_1070,
			empty_code_map
		},{
			RM2K3_1080_1080,
			{{
				map<int32_t, T::MinVarLimit>        (-9999999, 0x0AC76B, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        ( 9999999, 0x0AC795, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x08FC21, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x08FC28, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x08FC43, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x08FC4A, SUB_EDX_EBX, MOV_ECX),

				map<int32_t, T::MaxActorHP>         (    9999, 0x0B652B, MOV_ECX), /* 0x0B8590 - 0x0B858B */
				map<int32_t, T::MaxActorSP>         (     999, 0x0B659D, MOV_ECX), /* 0x0B85B2 - 0x0B85AD */
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				map<int32_t, T::MaxAtkBaseValue>    (     999, 0x0B6636, MOV_ECX), /* 0x0B85D1 - 0xB85CC */
				map<int32_t, T::MaxDefBaseValue>    (     999, 0x0B689C, MOV_ECX), /* 0x0B85F0 - 0xB85EB */
				map<int32_t, T::MaxSpiBaseValue>    (     999, 0x0B694C, MOV_ECX), /* 0x0B860F - 0xB860A */
				map<int32_t, T::MaxAgiBaseValue>    (     999, 0x0B69F2, MOV_ECX), /* 0x0B862E - 0xB8629 */
		
				map<int32_t, T::MaxAtkBattleValue>  (    9999, 0x0BEF3C, MOV_ECX),
				map<int32_t, T::MaxDefBattleValue>  (    9999, 0x0BF008, MOV_ECX),
				map<int32_t, T::MaxSpiBattleValue>  (    9999, 0x0BF0D1, MOV_ECX),
				map<int32_t, T::MaxAgiBattleValue>  (    9999, 0x0BF16D, MOV_ECX),

				map<int32_t, T::MaxDamageValue>     (    9999, 0x09C43C, MOV_EAX),
				map<int32_t, T::MaxExpValue>        ( 9999999, 0x0B60C3, CMP_ESI),
				map<int32_t, T::MaxGoldValue>       (  999999, 0x0A5B54, ADD_EDX_ESI, MOV_EAX),
				map<uint8_t, T::MaxItemCount>       (      99, 0x092399, CMP_EAX_BYTE),
				map<uint8_t, T::MaxSaveFiles>       (      16, 0x08FB34, CMP_EBX_BYTE),
				not_def<T::MaxLevel>(),
			}}
		},{
			RM2K3_1091_1091,
			{{
				map<int32_t, T::MinVarLimit>        (-9999999, 0x0B5103, CMP_DWORD_ESP, 0x10),
				map<int32_t, T::MaxVarLimit>        ( 9999999, 0x0B512D, CMP_DWORD_ESP, 0x10),

				map<int32_t, T::TitleX>             (     160, 0x08EE15, MOV_EDX),
				map<int32_t, T::TitleY>             (     148, 0x08EE1C, SUB_EDX_EBX, MOV_ECX),
				map<int32_t, T::TitleHiddenX>       (     160, 0x08EE37, MOV_EDX),
				map<int32_t, T::TitleHiddenY>       (      88, 0x08EE3E, SUB_EDX_EBX, MOV_ECX),

				map<int32_t, T::MaxActorHP>         (    9999, 0x0AD543, MOV_ECX),
				map<int32_t, T::MaxActorSP>         (     999, 0x0AD5B5, MOV_ECX),
				not_def<T::MaxEnemyHP>(),
				not_def<T::MaxEnemySP>(),

				map<int32_t, T::MaxAtkBaseValue>    (     999, 0x0AD64E, MOV_ECX),
				map<int32_t, T::MaxDefBaseValue>    (     999, 0x0AD8B4, MOV_ECX),
				map<int32_t, T::MaxSpiBaseValue>    (     999, 0x0AD964, MOV_ECX),
				map<int32_t, T::MaxAgiBaseValue>    (     999, 0x0ADA0A, MOV_ECX),
		
				map<int32_t, T::MaxAtkBattleValue>  (    9999, 0x0A92B8, MOV_ECX),
				map<int32_t, T::MaxDefBattleValue>  (    9999, 0x0A9384, MOV_ECX),
				map<int32_t, T::MaxSpiBattleValue>  (    9999, 0x0A944D, MOV_ECX),
				map<int32_t, T::MaxAgiBattleValue>  (    9999, 0x0A94E9, MOV_ECX),

				map<int32_t, T::MaxDamageValue>     (    9999, 0x09B770, MOV_EAX),
				map<int32_t, T::MaxExpValue>        ( 9999999, 0x0AD0DB, CMP_ESI),
				map<int32_t, T::MaxGoldValue>       (  999999, 0x0A3EDC, ADD_EDX_ESI, MOV_EAX),
				map<uint8_t, T::MaxItemCount>       (      99, 0x09158D, CMP_EAX_BYTE),
				map<uint8_t, T::MaxSaveFiles>       (      16, 0x08ED28, CMP_EBX_BYTE),
				not_def<T::MaxLevel>(),
			}}
		}
	}};

	inline EXE::Constants::code_address_map const* GetConstantAddressesForBuildInfo(EXE::BuildInfo::EngineType engine_type, EXE::BuildInfo::KnownEngineBuildVersions build_version) {
		switch (engine_type) {
			case EXE::BuildInfo::EngineType::RPG2000:
			{
				auto& builds = known_engine_builds_rm2k;
				auto it = std::find_if(builds.begin(), builds.end(), [&](const auto& pair) {
					return pair.first == build_version;
				});
				if (it != builds.end()) {
					return &it->second;
				}
			}
			break;
			case EXE::BuildInfo::EngineType::RPG2003:
			{
				auto& builds = known_engine_builds_rm2k3;
				auto it = std::find_if(builds.begin(), builds.end(), [&](const auto& pair) {
					return pair.first == build_version;
				});
				if (it != builds.end()) {
					return &it->second;
				}
			}
			break;
			default:
				break;
		}
		return nullptr;
	}
}

#endif
