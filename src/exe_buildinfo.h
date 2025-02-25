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

#ifndef EP_EXE_BUILDINFO_H
#define EP_EXE_BUILDINFO_H

#include <cstdint>
#include <lcf/enum_tags.h>
#include "utils.h"

namespace EXE::BuildInfo {

	enum class EngineType {
		UnknownEngine = 0,
		RPG2000,
		RPG2003,
		RM2k3_MP,
		EasyRPG
	};

	static constexpr auto kEngineTypes = lcf::makeEnumTags<EngineType>(
		"Unknown",
		"RPG2000",
		"RPG2003",
		"RM2k3_MP",
		"EasyRPG"
	);

	enum KnownEngineBuildVersions {
		UnknownBuild = 0,

		RM2K_20000306,
		RM2K_2000XXXX_UNK,
		RM2K_20000507,
		RM2K_20000619,
		RM2K_20000711,
		RM2K_20001113,
		RM2K_20001115,
		RM2K_20001227,
		RM2K_20010505,
		RM2K_20030327,
		RM2K_20030625,
		RM2KE_160,
		RM2KE_161,
		RM2KE_162,

		RM2K3_100,
		RM2K3_UNK_1,
		RM2K3_UNK_2,
		RM2K3_1021_1021,
		RM2K3_1030_1030_1,
		RM2K3_1030_1030_2,
		RM2K3_1030_1040,
		RM2K3_1050_1050_1,
		RM2K3_1050_1050_2,
		RM2K3_1060_1060,
		RM2K3_1070_1070,
		RM2K3_1080_1080,
		RM2K3_1091_1091,

		LAST_RM2K = RM2KE_162,
		LAST_RM2K3 = RM2K3_1091_1091,
		LAST
	};

	static constexpr auto kKnownEngineBuildDescriptions = lcf::makeEnumTags<KnownEngineBuildVersions>(
		"UnknownBuild",

		"1.00",
		"2000-Unk",
		"2000-05-07",
		"2000-06-19",
		"2000-07-11",
		"2000-11-13",
		"2000-11-15",
		"1.07",
		"1.10",
		"1.50 (VALUE!)",
		"1.51 (VALUE!)",
		"1.60 (English)",
		"1.61 (English)",
		"1.62 (English)",

		"1.0.0",
		"1.0.1_UNK1",
		"1.0.1_UNK2",
		"1.0.2.1",
		"1.0.3.1",
		"1.0.3.2",
		"1.0.4.0",
		"1.0.5.1",
		"1.0.5.2",
		"1.0.6",
		"1.0.7",
		"1.0.8",
		"1.0.9.1"
	);

	static_assert(kKnownEngineBuildDescriptions.size() == static_cast<size_t>(KnownEngineBuildVersions::LAST));

	constexpr size_t count_known_engine_builds = KnownEngineBuildVersions::LAST;
	constexpr size_t count_known_rm2k_builds = 1 + KnownEngineBuildVersions::LAST_RM2K - KnownEngineBuildVersions::RM2K_20000306;
	constexpr size_t count_known_rm2k3_builds = 1 + KnownEngineBuildVersions::LAST_RM2K3 - KnownEngineBuildVersions::RM2K3_100;

	struct EngineBuildInfo {
		EngineType engine_type;
		int32_t code_size;
		int32_t entrypoint;

		constexpr EngineBuildInfo() :
			engine_type(EngineType::UnknownEngine), code_size(0), entrypoint(0) {
		}

		constexpr EngineBuildInfo(EngineType engine_type, int32_t code_size, int32_t entrypoint) :
			engine_type(engine_type), code_size(code_size), entrypoint(entrypoint) {
		}
	};

	using engine_buildinfo_map = std::array<std::pair<KnownEngineBuildVersions, EngineBuildInfo>, count_known_engine_builds>;

	constexpr engine_buildinfo_map known_engine_builds = {{
			{ UnknownBuild,      { EngineType::UnknownEngine, 0, 0 } },

			{ RM2K_20000306,     { EngineType::RPG2000, 0x96400, 0x097220 } },
			{ RM2K_2000XXXX_UNK, { EngineType::RPG2000, 0x96600, 0x0972C4 } },
			{ RM2K_20000507,     { EngineType::RPG2000, 0x96600, 0x0972D8 } },
			{ RM2K_20000619,     { EngineType::RPG2000, 0x96C00, 0x097940 } },
			{ RM2K_20000711,     { EngineType::RPG2000, 0x96E00, 0x097B50 } },
			{ RM2K_20001113,     { EngineType::RPG2000, 0x96800, 0x0975E4 } },
			{ RM2K_20001115,     { EngineType::RPG2000, 0x96800, 0x0975EC } },
			{ RM2K_20001227,     { EngineType::RPG2000, 0x96A00, 0x097744 } },
			{ RM2K_20010505,     { EngineType::RPG2000, 0x96A00, 0x097710 } },
			{ RM2K_20030327,     { EngineType::RPG2000, 0x9BC00, 0x09CA80 } },
			{ RM2K_20030625,     { EngineType::RPG2000, 0x9BE00, 0x09CC30 } },
			{ RM2KE_160,         { EngineType::RPG2000, 0x9C000, 0x09CC78 } },
			{ RM2KE_161,         { EngineType::RPG2000, 0x9CA00, 0x09D708 } },
			{ RM2KE_162,         { EngineType::RPG2000, 0x9CC00, 0x09D930 } },

			{ RM2K3_100,         { EngineType::RPG2003, 0xBD600, 0x0BE3F4 } },
			{ RM2K3_UNK_1,       { EngineType::RPG2003, 0xBF200, 0x0BFF50 } },
			{ RM2K3_UNK_2,       { EngineType::RPG2003, 0xBE800, 0x0BF6A8 } },
			{ RM2K3_1021_1021,   { EngineType::RPG2003, 0xBF400, 0x0C00CC } },
			{ RM2K3_1030_1030_1, { EngineType::RPG2003, 0xBFE00, 0x0C0B90 } },
			{ RM2K3_1030_1030_2, { EngineType::RPG2003, 0xC0A00, 0x0C1878 } },
			{ RM2K3_1030_1040,   { EngineType::RPG2003, 0xC0800, 0x0C14D8 } },
			{ RM2K3_1050_1050_1, { EngineType::RPG2003, 0xC7400, 0x0C81C4 } },
			{ RM2K3_1050_1050_2, { EngineType::RPG2003, 0xC7400, 0x0C81F0 } },
			{ RM2K3_1060_1060,   { EngineType::RPG2003, 0xC8A00, 0x0C9804 } },
			{ RM2K3_1070_1070,   { EngineType::RPG2003, 0xC8C00, 0x0C9984 } },
			{ RM2K3_1080_1080,   { EngineType::RPG2003, 0xC8E00, 0x0C9C1C } },
			{ RM2K3_1091_1091,   { EngineType::RPG2003, 0xC9000, 0x0C9D24 } }
	}};

	template<size_t S>
	class fixed_size_byte_array {
	public:
		template <typename... Args>
		constexpr fixed_size_byte_array(Args... args)
			: _size(sizeof...(args)), _data(init_fixedsize_array<sizeof...(args)>({ args... })) {
		}
		constexpr size_t size() const { return _size; }

		constexpr const uint8_t operator[](const size_t off) const noexcept {
			return _data[off];
		}
	private:
		template<size_t N>
		static constexpr std::array<uint8_t, S> init_fixedsize_array(const std::array<const uint8_t, N> input) {
			std::array<uint8_t, S> result{};
			for (size_t i = 0; i < N; ++i) {
				result[i] = input[i];
			}
			return result;
		}

		size_t _size;
		std::array<uint8_t, S> _data;
	};

	constexpr size_t MAX_SIZE_CHK_PRE = 4;

	using validate_const_data = fixed_size_byte_array<MAX_SIZE_CHK_PRE>;

	class CodeAddressInfo {
	public:
		int32_t default_val;
		uint8_t size_val;
		size_t code_offset;
		validate_const_data pre_data;

		template<typename... Args>
		constexpr CodeAddressInfo(int32_t default_val, uint8_t size_val, size_t code_offset, Args... args) :
			default_val(default_val), size_val(size_val), code_offset(code_offset), pre_data(validate_const_data{ static_cast<const uint8_t>(args)... }) {
		}
	};

	class CodeAddressStringInfo {
	public:
		size_t code_offset;
		uint32_t crc_jp, crc_en;
		validate_const_data pre_data;

		template<typename... Args>
		constexpr CodeAddressStringInfo(size_t code_offset, uint32_t crc_jp, uint32_t crc_en, Args... args) :
			code_offset(code_offset), crc_jp(crc_jp), crc_en(crc_en), pre_data(validate_const_data{ static_cast<const uint8_t>(args)... }) {
		}
	};

	constexpr size_t MAX_SIZE_CHK_PATCH_SEGMENT = 8;

	using patch_segment_data = fixed_size_byte_array<MAX_SIZE_CHK_PATCH_SEGMENT>;

	class PatchDetectionInfo {
	public:
		size_t chk_segment_offset;
		patch_segment_data chk_segment_data;
		size_t extract_var_offset;

		constexpr PatchDetectionInfo(size_t chk_segment_offset, patch_segment_data chk_segment_data) :
			chk_segment_offset(chk_segment_offset), chk_segment_data(chk_segment_data), extract_var_offset(0) {
		}
		constexpr PatchDetectionInfo(size_t chk_segment_offset, patch_segment_data chk_segment_data, size_t extract_var_offset) :
			chk_segment_offset(chk_segment_offset), chk_segment_data(chk_segment_data), extract_var_offset(extract_var_offset) {
		}
	};
}

#endif
