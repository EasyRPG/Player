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

	constexpr size_t MAX_SIZE_CHK_PRE = 4;

	struct CodeAddressInfoU32 {
		int32_t default_val;
		size_t code_offset;
		size_t size_pre_data;
		std::array<uint8_t, MAX_SIZE_CHK_PRE> pre_data;

		constexpr CodeAddressInfoU32(int32_t default_val, size_t code_offset) :
			default_val(default_val), code_offset(code_offset), size_pre_data(0), pre_data({ 0,0,0,0 }) {
		}
		constexpr CodeAddressInfoU32(int32_t default_val, size_t code_offset, uint8_t pre_data) :
			default_val(default_val), code_offset(code_offset), size_pre_data(1), pre_data({ pre_data, 0, 0, 0 }) {
		}
		constexpr CodeAddressInfoU32(int32_t default_val, size_t code_offset, std::array<uint8_t, 2> pre_data) :
			default_val(default_val), code_offset(code_offset), size_pre_data(2), pre_data({ pre_data[0], pre_data[1], 0, 0 }) {
		}
		constexpr CodeAddressInfoU32(int32_t default_val, size_t code_offset, std::array<uint8_t, 3> pre_data) :
			default_val(default_val), code_offset(code_offset), size_pre_data(3), pre_data({ pre_data[0], pre_data[1], pre_data[2], 0 }) {
		}
		constexpr CodeAddressInfoU32(int32_t default_val, size_t code_offset, std::array<uint8_t, 4> pre_data) :
			default_val(default_val), code_offset(code_offset), size_pre_data(4), pre_data({ pre_data[0], pre_data[1], pre_data[2], pre_data[3]}) {
		}
	};

	using code_address = std::pair<Player::GameConstantType, struct CodeAddressInfoU32>;
	using code_address_map = std::array<code_address, static_cast<size_t>(Player::GameConstantType::LAST)>;

#define ADD_EAX_ESI 0x03, 0xC6
#define MOV_EBX		0xB9
#define MOV_ECX		0xB9
#define MOV_EDX		0xBA
#define SUB_EDX_EBX 0x2B, 0xD3
#define CMP_DWORD_ESP 0x81, 0x7C, 0x24

#define DEPENDS_ON_PREVIOUS { 0xFF, 0xFF, 0x00, 0xFF }

	constexpr auto magic_prev = std::array<uint8_t, 4 >(DEPENDS_ON_PREVIOUS);

	template<Player::GameConstantType T>
	constexpr code_address map(int32_t default_val, size_t code_offset, uint8_t pre_1) {
		return { T, CodeAddressInfoU32(default_val, code_offset, pre_1) };
	}
	template<Player::GameConstantType T>
	constexpr code_address map(int32_t default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2) {
		return { T, CodeAddressInfoU32(default_val, code_offset,  std::array<uint8_t, 2> { pre_1, pre_2 }) };
	}
	template<Player::GameConstantType T>
	constexpr code_address map(int32_t default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2, uint8_t pre_3) {
		return { T, CodeAddressInfoU32(default_val, code_offset,  std::array<uint8_t, 3> { pre_1, pre_2, pre_3 }) };
	}
	template<Player::GameConstantType T>
	constexpr code_address map(int32_t default_val, size_t code_offset, uint8_t pre_1, uint8_t pre_2, uint8_t pre_3, uint8_t pre_4) {
		return { T, CodeAddressInfoU32(default_val, code_offset,  std::array<uint8_t, 4> { pre_1, pre_2, pre_3, pre_4 }) };
	}
	template<Player::GameConstantType T>
	constexpr code_address not_def() {
		return { T, CodeAddressInfoU32(0, 0) };
	}
}

namespace ExeConstants::RT_2K {

	using T = Player::GameConstantType;
	
	constexpr code_address_map const_addresses_103b = {{
		map<T::MinVarLimit>            ( -999999, 0x08560C, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            (  999999, 0x085636, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x06CC39, MOV_EDX),
		map<T::TitleY>                 (     148, 0x06CC40, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x06CC5B, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x06CC62, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};

	constexpr code_address_map const_addresses_105b = {{
		map<T::MinVarLimit>            ( -999999, 0x0842A4, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            (  999999, 0x0842D2, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x06E091, MOV_EDX),
		map<T::TitleY>                 (     148, 0x06E098, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x06E0B3, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x06CC62, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};

	constexpr code_address_map const_addresses_106 = {{
		map<T::MinVarLimit>            ( -999999, 0x085978, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            (  999999, 0x0859A2, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x06D1B9, MOV_EDX),
		map<T::TitleY>                 (     148, 0x06D1C0, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x06D1DB, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x06D1E2, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};
}

namespace ExeConstants::RT_2K3 {

	using T = Player::GameConstantType;

	constexpr code_address_map const_addresses_104 = {{
		map<T::MinVarLimit>            (-9999999, 0x0A5CB3, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            ( 9999999, 0x0A5CDD, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x08A849, MOV_EDX),
		map<T::TitleY>                 (     148, 0x08A850, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x08A86B, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x08A872, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};
	
	constexpr code_address_map const_addresses_106 = {{
		map<T::MinVarLimit>            (-9999999, 0x0AC0F7, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            ( 9999999, 0x0AC121, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x08F76D, MOV_EDX),
		map<T::TitleY>                 (     148, 0x08F774, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x08F78F, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x08F796, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};
	
	constexpr code_address_map const_addresses_108 = {{
		map<T::MinVarLimit>            (-9999999, 0x0AC36B, CMP_DWORD_ESP, 0x10),
		map<T::MaxVarLimit>            ( 9999999, 0x0AC395, CMP_DWORD_ESP, 0x10),

		map<T::TitleX>                 (     160, 0x08F821, MOV_EDX),
		map<T::TitleY>                 (     148, 0x08F828, SUB_EDX_EBX, MOV_EBX),
		map<T::TitleHiddenX>           (     160, 0x08F843, MOV_EDX),
		map<T::TitleHiddenY>           (      88, 0x08F84A, SUB_EDX_EBX, MOV_EBX),

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
		not_def<T::MaxLevel>(),
		not_def<T::MaxGoldValue>(),
		not_def<T::MaxItemCount>(),
		not_def<T::MaxSaveFiles>()
	}};
}
#endif
