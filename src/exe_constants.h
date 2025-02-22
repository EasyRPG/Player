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

#include <cstdint>
#include <vector>
#include "player.h"

namespace ExeConstants {

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

	using patch_config = std::map<Player::GameConstantType, int32_t>;

	constexpr size_t MAX_SIZE_CHK_PRE = 4;

	struct CodeAddressInfo {
		int32_t default_val;
		uint8_t size_val;
		size_t code_offset;
		size_t size_pre_data;
		std::array<uint8_t, MAX_SIZE_CHK_PRE> pre_data;

		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), size_pre_data(0), pre_data({ 0,0,0,0 }) {
		}
		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset, uint8_t pre_data) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), size_pre_data(1), pre_data({ pre_data, 0, 0, 0 }) {
		}
		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset, std::array<uint8_t, 2> pre_data) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), size_pre_data(2), pre_data({ pre_data[0], pre_data[1], 0, 0 }) {
		}
		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset, std::array<uint8_t, 3> pre_data) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), size_pre_data(3), pre_data({ pre_data[0], pre_data[1], pre_data[2], 0 }) {
		}
		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset, std::array<uint8_t, 4> pre_data) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), size_pre_data(4), pre_data({ pre_data[0], pre_data[1], pre_data[2], pre_data[3]}) {
		}
	};

	using code_address = std::pair<Player::GameConstantType, CodeAddressInfo>;
	using code_address_map = std::array<code_address, static_cast<size_t>(Player::GameConstantType::LAST)>;

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

#define DEPENDS_ON_PREVIOUS { 0xFF, 0xFF, 0x00, 0xFF }

	constexpr auto magic_prev = std::array<uint8_t, 4 >(DEPENDS_ON_PREVIOUS);

	template<typename T, Player::GameConstantType C>
	constexpr code_address map(T default_val, size_t code_offset, uint8_t pre_1) {
		return { C, CodeAddressInfo(default_val, sizeof(T), code_offset, pre_1)};
	}
	template<typename T, Player::GameConstantType C>
	constexpr code_address map(T default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2) {
		return { C, CodeAddressInfo(default_val, sizeof(T), code_offset,  std::array<uint8_t, 2> { pre_1, pre_2 }) };
	}
	template<typename T, Player::GameConstantType C>
	constexpr code_address map(T default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2, uint8_t pre_3) {
		return { C, CodeAddressInfo(default_val, sizeof(T), code_offset,  std::array<uint8_t, 3> { pre_1, pre_2, pre_3 }) };
	}
	template<typename T, Player::GameConstantType C>
	constexpr code_address map(T default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2, uint8_t pre_3, uint8_t pre_4) {
		return { C, CodeAddressInfo(default_val, sizeof(T), code_offset,  std::array<uint8_t, 4> { pre_1, pre_2, pre_3, pre_4 }) };
	}
	template<Player::GameConstantType C>
	constexpr code_address not_def() {
		return { C, CodeAddressInfo(0, 0, 0) };
	}
}

namespace ExeConstants::RT_2K {

	using T = Player::GameConstantType;
	
	constexpr code_address_map const_addresses_103b = {{
		map<int32_t, T::MinVarLimit>            ( -999999, 0x085A0C, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            (  999999, 0x085A36, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x06D039, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x06D040, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x06D05B, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x06D062, SUB_EDX_EBX, MOV_ECX),

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

	constexpr code_address_map const_addresses_105b = {{
		map<int32_t, T::MinVarLimit>            ( -999999, 0x0846A8, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            (  999999, 0x0846D2, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x06E491, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x06E498, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x06E4B3, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x06E4BA, SUB_EDX_EBX, MOV_ECX),

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

	constexpr code_address_map const_addresses_106 = {{
		map<int32_t, T::MinVarLimit>            ( -999999, 0x085D78, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            (  999999, 0x085DA2, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x06D5B9, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x06D5C0, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x06D5DB, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x06D5E2, SUB_EDX_EBX, MOV_ECX),

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
}

namespace ExeConstants::RT_2K3 {

	using T = Player::GameConstantType;

	constexpr code_address_map const_addresses_104 = {{
		map<int32_t, T::MinVarLimit>            (-9999999, 0x0A60B3, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            ( 9999999, 0x0A60DD, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x08AC49, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x08AC50, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x08AC6B, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x08AC72, SUB_EDX_EBX, MOV_ECX),

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
	
	constexpr code_address_map const_addresses_106 = {{
		map<int32_t, T::MinVarLimit>            (-9999999, 0x0AC4F7, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            ( 9999999, 0x0AC521, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x08FB6D, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x08FB74, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x08FB8F, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x08FB96, SUB_EDX_EBX, MOV_ECX),

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
	
	constexpr code_address_map const_addresses_108 = {{
		map<int32_t, T::MinVarLimit>            (-9999999, 0x0AC76B, CMP_DWORD_ESP, 0x10),
		map<int32_t, T::MaxVarLimit>            ( 9999999, 0x0AC795, CMP_DWORD_ESP, 0x10),

		map<int32_t, T::TitleX>                 (     160, 0x08FC21, MOV_EDX),
		map<int32_t, T::TitleY>                 (     148, 0x08FC28, SUB_EDX_EBX, MOV_ECX),
		map<int32_t, T::TitleHiddenX>           (     160, 0x08FC43, MOV_EDX),
		map<int32_t, T::TitleHiddenY>           (      88, 0x08FC4A, SUB_EDX_EBX, MOV_ECX),

		map<int32_t, T::MaxActorHP>             (    9999, 0x0B652B, MOV_ECX), /* 0x0B858B */
		map<int32_t, T::MaxActorSP>             (     999, 0x0B659D, MOV_ECX), /* 0x0B85AD */
		not_def<T::MaxEnemyHP>(),
		not_def<T::MaxEnemySP>(),

		map<int32_t, T::MaxAtkBaseValue>        (     999, 0x0B6636, MOV_ECX), /* 0xB85CC */
		map<int32_t, T::MaxDefBaseValue>        (     999, 0x0B689C, MOV_ECX), /* 0xB85EB */
		map<int32_t, T::MaxSpiBaseValue>        (     999, 0x0B694C, MOV_ECX), /* 0xB860A */
		map<int32_t, T::MaxAgiBaseValue>        (     999, 0x0B69F2, MOV_ECX), /* 0xB8629 */
		
		map<int32_t, T::MaxAtkBattleValue>      (    9999, 0x0BEF3C, MOV_ECX),
		map<int32_t, T::MaxDefBattleValue>      (    9999, 0x0BF008, MOV_ECX),
		map<int32_t, T::MaxSpiBattleValue>      (    9999, 0x0BF0D1, MOV_ECX),
		map<int32_t, T::MaxAgiBattleValue>      (    9999, 0x0BF16D, MOV_ECX),

		map<int32_t, T::MaxDamageValue>         (    9999, 0x09C43C, MOV_EAX),
		map<int32_t, T::MaxExpValue>            ( 9999999, 0x0B60C3, CMP_ESI), /* 0xB8482 */
		map<int32_t, T::MaxGoldValue>           (  999999, 0x0A5B54, ADD_EDX_ESI, MOV_EAX),
		map<uint8_t, T::MaxItemCount>           (      99, 0x092399, CMP_EAX_BYTE),
		map<uint8_t, T::MaxSaveFiles>           (      16, 0x08FB34, CMP_EBX_BYTE),
		not_def<T::MaxLevel>(),
	}};
}

namespace ExeConstants {
	using T = Player::GameConstantType;

	std::map<KnownPatchConfigurations, patch_config> known_patch_configurations = {
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
}

#endif
