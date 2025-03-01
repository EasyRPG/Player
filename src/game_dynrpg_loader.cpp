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

// Headers
#include "game_dynrpg_loader.h"
#include "output.h"
#include "player.h"
#include "exe_constants.h"
#include "exe_patches.h"
#include <ini.h>

#include <fstream>

namespace {
	std::map<std::string, std::string> ini_quickpatches;
	std::map<EXE::Patches::KnownPatches, EXE::BuildInfo::PatchDetectionInfo> known_patches;

	int ini_handler_qp(void* user, const char* section, const char* name, const char* value) {
		if (std::strcmp(section, DYNRPG_INI_SECTION_QUICKPATCHES) == 0) {
			ini_quickpatches[name] = value;
		}
		return 1;
	}

	template<typename T>
	bool try_read_hex_str(std::string& str, T& out_val) {
		auto non_ws_found = std::find_if(str.begin(), str.end(), [](auto p) { return !std::isxdigit(p); });
		if (non_ws_found != str.end()) {
			return false;
		}
		if constexpr (std::is_same<T, long>::value) {
			out_val = std::stol(str, nullptr, 16);
		} else {
			out_val = std::stoi(str, nullptr, 16);
		}
		return true;
	}

	bool read_ips_offset(Filesystem_Stream::InputStream& is, int& pos, uint32_t& out_addr, uint32_t& out_size, bool& is_rle) {
		is_rle = false;

		is.seekg(pos++, std::ios_base::beg);
		out_addr = is.get() << 16;
		is.seekg(pos++, std::ios_base::beg);
		out_addr |= is.get() << 8;
		is.seekg(pos++, std::ios_base::beg);
		out_addr |= is.get();

		if (out_addr == 0x454f46) { //EOF
			return false;
		}

		is.seekg(pos++, std::ios_base::beg);
		out_size = is.get() << 8;
		is.seekg(pos++, std::ios_base::beg);
		out_size |= is.get();

		if (out_size == 0) {
			// RLE encoded
			is.seekg(pos++, std::ios_base::beg);
			out_size = is.get() << 8;
			is.seekg(pos++, std::ios_base::beg);
			out_size |= is.get();
			is_rle = true;
		}
		return true;
	}

	int read_int32(Filesystem_Stream::InputStream& is, int pos) {
		int v;
		is.seekg(pos++, std::ios_base::beg);
		v = is.get();
		is.seekg(pos++, std::ios_base::beg);
		v |= is.get() << 8;
		is.seekg(pos++, std::ios_base::beg);
		v |= is.get() << 8;
		is.seekg(pos++, std::ios_base::beg);
		v |= is.get() << 8;
		return v;
	}
}

std::vector<EXE::Shared::PatchSetupInfo> DynRpg_Loader::DetectRuntimePatches(EXE::BuildInfo::KnownEngineBuildVersions build_version) {
	auto dir_contents = FileFinder::Game().ListDirectory(DYNRPG_FOLDER_PATCHES);

	if (!dir_contents) {
		return {};
	}

	if (build_version == EXE::BuildInfo::UnknownBuild) {
		// If not otherwise specified, DynRPG should always be Rm2k3 v1.08
		build_version = EXE::BuildInfo::RM2K3_1080_1080;
	}

	known_patches.clear();
	for (auto& p : EXE::Patches::GetPatchesForBuildVersion(build_version)) {
		known_patches[p.first] = p.second;
	}

	std::vector<EXE::Shared::PatchSetupInfo> result;

	for (auto item : *dir_contents) {
		auto fis = FileFinder::Game().OpenFile(DYNRPG_FOLDER_PATCHES, item.first);
		if (!fis) {
			continue;
		}
		bool is_ips = true;
		int i = 0;
		for (i = 0; i < magic_ips.size(); i++) {
			fis.seekg(i, std::ios_base::beg);
			if (fis.get() != magic_ips[i]) {
				is_ips = false;
				break;
			}
		}
		if (!is_ips) {
			continue;
		}

		auto patch = ReadIPS(item.first, fis);
		if (patch.patch_type != invalid_patch.patch_type) {
			result.push_back(patch);
		}
	}

	return result;
}

EXE::Shared::PatchSetupInfo DynRpg_Loader::ReadIPS(std::string const& item_name, Filesystem_Stream::InputStream& is) {
	int i = magic_ips.size();

	EXE::Shared::KnownPatches patch_type;
	EXE::BuildInfo::PatchDetectionInfo const* patch_info = nullptr;
	do {
		uint32_t address = 0, size = 0;
		bool is_rle = false;
		if (!read_ips_offset(is, i, address, size, is_rle)) {
			break;
		}
		auto patch_it = std::find_if(known_patches.begin(), known_patches.end(), [&](auto& p) {
			auto& ofs = p.second.chk_segment_offset;
			return ofs >= address && ofs < (address + size);
		});
		if (patch_it != known_patches.end()) {
			patch_type = patch_it->first;
			patch_info = &patch_it->second;

			int ofs = patch_info->chk_segment_offset - address;

			for (int j = 0; j < patch_info->chk_segment_data.size(); ++j) {
				if (!is_rle) {
					is.seekg(i + j + ofs, std::ios_base::beg);
				}
				if (is.get() != patch_info->chk_segment_data[j]) {
					patch_info = nullptr;
					break;
				}
			}
			if (patch_info != nullptr) {
				break;
			}
		}
		i += size;
	} while (is.get() != -1);

	if (patch_info == nullptr) {
		Output::Warning("DynRPG Loader: Encountered unknown IPS patch '{}'", item_name);
		return invalid_patch;
	}

	if (patch_info->extract_var_offset == 0) {
		Output::Debug("DynRPG Loader: Applying patch '{}'", EXE::Shared::kKnownPatches.tag(patch_type));
		return EXE::Shared::PatchSetupInfo { patch_type, 0 };
	}

	i = magic_ips.size();
	int var_id = 0;
	do {
		uint32_t address = 0, size = 0;
		bool is_rle = false;
		if (!read_ips_offset(is, i, address, size, is_rle)) {
			break;
		}

		if (patch_info->extract_var_offset >= address && patch_info->extract_var_offset <= (address + size)) {
			int section_ofs = patch_info->extract_var_offset - address;
			var_id = read_int32(is, i + section_ofs);
		}

		auto patch_it = std::find_if(known_patches.begin(), known_patches.end(), [&](auto& p) {
			auto& ofs = p.second.extract_var_offset;
			return ofs >= address && ofs < (address + size);
		});
		if (patch_it != known_patches.end()) {
			patch_info = &patch_it->second;
		}
		i += size;
	} while (is.get() != -1);

	if (var_id == 0) {
		Output::Debug("DynRPG Loader: Could not extract variable values for known IPS patch '{}'", item_name);
		return invalid_patch;
	}

	Output::Debug("DynRPG Loader: Applying patch '{}' with var value '{}'", EXE::Shared::kKnownPatches.tag(patch_type), var_id);

	return EXE::Shared::PatchSetupInfo { patch_type, var_id };
}

void DynRpg_Loader::ApplyQuickPatches(EXE::Shared::EngineCustomization& engine_customization, EXE::BuildInfo::KnownEngineBuildVersions build_version) {
	ini_quickpatches.clear();

	int error = ini_parse(DYNRPG_INI_NAME, ini_handler_qp, nullptr);

	if (error == -1) {
		return;
	}

	if (ini_quickpatches.size() > 0) {
		Output::Debug("Found section for QuickPatches inside DynRPG.ini.");
	}

	if (build_version == EXE::BuildInfo::UnknownBuild) {
		// If not otherwise specified, DynRPG should always be Rm2k3 v1.08
		build_version = EXE::BuildInfo::RM2K3_1080_1080;
	}

	auto const& build_info = EXE::BuildInfo::known_engine_builds[build_version].second;
	auto& constant_addresses = EXE::Constants::GetConstantAddressesForBuildInfo(build_info.engine_type, build_version);

	std::vector<EXE::Patches::patch_detection> loaded_ips_patches;

	for (auto info : engine_customization.runtime_patches) {
		auto it = std::find_if(known_patches.begin(), known_patches.end(), [&](auto& p) {
			return p.first == info.first;
		});
		if (it == known_patches.end()) {
			continue;
		}
		loaded_ips_patches.push_back({ info.first, it->second });
	}

	bool read_error = false;
	for (auto pair : ini_quickpatches) {
		auto& key_name = pair.first;
		auto& addresses_str = pair.second;

		auto qp = ParseQuickPatch(addresses_str, build_info.code_size);

		if (qp.size() == 0) {
			Output::Warning("DynRPG Loader: Could not parse QuickPatch: '{}'", key_name);
			continue;
		}

		std::map<EXE::Shared::GameConstantType, int32_t> quickpatched_constants;
		for (auto section : qp) {
			auto const_it = std::find_if(constant_addresses.begin(), constant_addresses.end(), [&section](auto& p) {
				return p.second.code_offset == section.address;
			});
			if (const_it == constant_addresses.end()) {
				read_error = true;
				break;
			}
			quickpatched_constants[const_it->first] = section.patch_val;
		}

		if (!read_error) {
			Output::Debug("DynRPG Loader: Found valid QuickPatch '{}' targeting known game constants", key_name);
			for (auto const_info : quickpatched_constants) {
				Output::Debug("DynRPG QuickPatch: Applying value '{}' for constant '{}'", const_info.second, EXE::Shared::kGameConstantType.tag(static_cast<int>(const_info.first)));
				engine_customization.constant_overrides[const_info.first] = const_info.second;
			}
			continue;
		}
		read_error = false;

		EXE::Shared::KnownPatches quickpatched_ips_type;
		int quickpatched_ips_var_id = 0;
		for (auto section : qp) {
			auto const_it = std::find_if(loaded_ips_patches.begin(), loaded_ips_patches.end(), [&section](auto& p) {
				auto& ofs = p.second.extract_var_offset;
				return ofs == section.address;
			});
			if (const_it == loaded_ips_patches.end()) {
				read_error = true;
				break;
			}
			quickpatched_ips_type = const_it->first;
			quickpatched_ips_var_id = section.patch_val;
		}

		if (!read_error) {
			Output::Debug("DynRPG Loader: Found valid QuickPatch '{}' targeting known patch constants for '{}'", key_name, EXE::Shared::kKnownPatches.tag(quickpatched_ips_type));
			for (auto& p : engine_customization.runtime_patches) {
				auto& patch = p.second;
				if (patch.patch_type != quickpatched_ips_type) {
					continue;
				}
				patch.custom_var_1 = quickpatched_ips_var_id;
			}
			continue;
		}

		Output::Warning("DynRPG Loader: Encountered unkknown QuickPatch: '{}'", key_name);
	}
}

DynRpg_Loader::DynRpgQuickPatch DynRpg_Loader::ParseQuickPatch(std::string const& line, int code_size) {
	DynRpgQuickPatch patched_sections;

	auto skip_to_next = [](std::string const& str, int& i) {
		while (i < str.size() && (str[i] != ',' || std::isspace(str[i]))) {
			i++;
		}
	};
	auto skip_whitespace = [](std::string const& str, int& i) {
		while (i < str.size() && std::isspace(str[i])) {
			i++;
		}
	};

	int str_start;
	int i = 0;

	do {
		skip_whitespace(line, i);
		str_start = i;
		skip_to_next(line, i);

		if (str_start == i) {
			return {};
		}

		auto addr_str = line.substr(str_start, i - str_start);
		QuickPatchAddress qp;

		if (!try_read_hex_str(addr_str, qp.address)) {
			return {};
		}

		if (qp.address < 0x400 || (qp.address >= 0x400000 && qp.address <= 0x400C00)) {
			// HEADER section
			return {};
		}
		if ((qp.address > (0x400 + code_size) && qp.address < 0x400000) || (qp.address >= (0x400000 + code_size))) {
			// Either DATA section or outside of bounds
			return {};
		}
		if (qp.address > 0x400C00) {
			// Convert virtual address to file offset
			qp.address -= 0x400C00;
		}

		skip_whitespace(line, ++i);
		str_start = i;
		skip_to_next(line, i);

		if (str_start == i) {
			return {};
		}

		auto patch_str = line.substr(str_start, i - str_start);
		if (patch_str[0] == '%' || patch_str[0] == '#') {
			qp.byte_count = patch_str[0] == '#' ? 4 : 1;
			qp.patch_val = atoi(patch_str.c_str() + 1);
		} else {
			if (!try_read_hex_str(patch_str, qp.patch_val)) {
				return {};
			}
			qp.byte_count = patch_str.size();
			if ((qp.byte_count % 2) == 1)
				qp.byte_count++;
			qp.byte_count /= 2;
		}
		if (qp.patch_val == 0 || qp.byte_count == 0) {
			return {};
		}
		patched_sections.push_back(qp);

		skip_whitespace(line, ++i);
	} while (i < line.size());

	return patched_sections;
}
