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

#ifndef EP_EXE_STRINGS_H
#define EP_EXE_STRINGS_H

#include "exe_buildinfo.h"
#include "exe_shared.h"

namespace EXE::Strings {
	using namespace BuildInfo;
	using EmbeddedStringTypes = EXE::Shared::EmbeddedStringTypes;

	using string_address = std::pair<EmbeddedStringTypes, CodeAddressStringInfo>;
	using string_address_map = std::array<string_address, static_cast<size_t>(EmbeddedStringTypes::LAST)>;

	template<EmbeddedStringTypes C, typename... Args>
	constexpr string_address map(size_t code_offset, uint32_t crc_jp, uint32_t crc_en,  Args&&... args) {
		return { C, CodeAddressStringInfo(code_offset, crc_jp, crc_en, std::forward<decltype(args)>(args)...) };
	}

	template<EmbeddedStringTypes C>
	constexpr string_address not_def() {
		return { C, CodeAddressStringInfo(0, 0, 0, 0) };
	}

	using Str = EmbeddedStringTypes;

#define FFFFFFFF 0xFF, 0xFF, 0xFF, 0xFF

	//TODO: offsets: 0x07B848, 0x07B860 (are these strings used anywhere?)
	constexpr string_address_map string_addresses_rm2k_151 = {{
			map<Str::Battle_DamageToEnemy>(0x07AEA4, 0x9D4F8EDE, 0xB55355BF, FFFFFFFF),
			map<Str::Battle_DamageToAlly>(0x07AEB8, 0x6900AACD, 0xB55355BF, FFFFFFFF),
			map<Str::Battle_HpSpRecovery>(0x07AFF0, 0x7C968020, 0x5299B765, FFFFFFFF),
			map<Str::Battle_StatDecrease>(0x07B1E0, 0x7C968020, 0x5299B765, FFFFFFFF),
			map<Str::Battle_StatIncrease>(0x07B43C, 0x7C968020, 0x5299B765, FFFFFFFF),
			map<Str::Battle_AbsorbEnemy>(0x07B65C, 0xFB3336D9, 0x5299B765, FFFFFFFF),
			map<Str::Battle_AbsorbAlly>(0x07B674, 0x3ABDE919, 0x5299B765, FFFFFFFF),
			not_def<Str::Battle_UseItem>(),
			map<Str::Msg_LevelUp>(0x0882CC, 0x0DED4C01, 0x5299B765, FFFFFFFF),
			map<Str::Menu_ExpMaxedOut>(0x07C678, 0x2770FF3E, 0x2770FF3E, FFFFFFFF),  //TODO
			not_def<Str::Menu_2k3ActorStatus_ExpMaxedOut>()
	}};
}

#endif
