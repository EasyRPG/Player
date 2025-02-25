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

// All of this code is unused on EMSCRIPTEN. *Do not use it*!
#ifndef EMSCRIPTEN

#include "exe_reader.h"
#include "image_bmp.h"
#include "output.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <lcf/reader_util.h>

namespace {
	// hashes of known RPG_RT startup logos
	std::array<uint32_t, 5> logo_crc32 = { 0xdf3d86a7, 0x2ece66f9, 0x2fe0de56, 0x25c4618f, 0x91b2635a };
}

EXEReader::EXEReader(Filesystem_Stream::InputStream core) : corefile(std::move(core)) {
	// The Incredibly Dumb PE parser (tm)
	// Extracts data from the resource section for engine detection and can read ExFont.
	uint32_t ofs = GetU32(0x3C); // PE header offset
	uint16_t machine = GetU16(ofs + 4);

	switch (machine) {
		case 0x14c:
			file_info.machine_type = MachineType::x86;
			break;
		case 0x8664:
			file_info.machine_type = MachineType::amd64;
			break;
		default:
			Output::Debug("EXEReader: Unsupported machine type ({:#x})", machine);
			file_info.machine_type = MachineType::Unknown;
			return;
	}

	// The largest known exe has 11 segments, guard against bogus section data here
	uint16_t sections = std::min<uint16_t>(GetU16(ofs + 6), 11);
	uint32_t optional_header = ofs + 0x18;
	uint32_t oh_magic = GetU16(optional_header);

	bool format_pe32;

	switch (oh_magic) {
		case 0x10b:
			format_pe32 = true;
			break;
		case 0x20b:
			// PE32+ (for 64 bit executables)
			format_pe32 = false;
			break;
		default:
			Output::Debug("EXEReader: Unknown PE header magic ({:#x})", oh_magic);
			file_info.machine_type = MachineType::Unknown;
			return;
	}

	uint32_t sectionsOfs = optional_header + GetU16(ofs + 0x14); // skip opt header
	uint32_t data_directory_ofs = (format_pe32 ? 0x60 : 0x70);
	resource_rva = GetU32(optional_header + data_directory_ofs + 16);
	file_info.entrypoint = format_pe32 ? GetU32(optional_header + 0x10) : 0;

	if (!resource_rva) {
		// Is some kind of encrypted EXE -> Give up
		return;
	}
	while (sections) {
		uint32_t secName = GetU32(sectionsOfs);
		uint32_t sectVs = GetU32(sectionsOfs + 0x08);
		uint32_t sectRs = GetU32(sectionsOfs + 0x10);

		if (sectRs > sectVs) {
			// Actually a problem in some files.
			sectVs = sectRs;
		}

		if (secName == 0x45444F43) { // CODE
			file_info.code_size = sectVs;
			file_info.code_ofs = GetU32(sectionsOfs + 0x14);
		} else if (secName == 0x52454843) { // CHER(RY)
			file_info.cherry_size = sectVs;
		} else if (secName == 0x50454547) { // GEEP
			file_info.geep_size = sectVs;
		} else if (secName == 0x30585055) { // UPX0
			Output::Debug("EXEReader: EXE is UPX compressed. Engine detection could be incorrect.");
		}

		uint32_t sectRva = GetU32(sectionsOfs + 0x0C);
		uint32_t sectRdptr = GetU32(sectionsOfs + 0x14);
		if (resource_ofs == 0 && (sectRva <= resource_rva) && ((sectRva + sectVs) > resource_rva)) {
			// Resources located.
			resource_ofs = sectRdptr + (resource_rva - sectRva);
		}
		sections--;
		sectionsOfs += 0x28;
	}
}

static std::vector<uint8_t> ExtractExFont(Filesystem_Stream::InputStream& corefile, uint32_t position, uint32_t len) {
	std::vector<uint8_t> exfont;
	constexpr int header_size = 14; // Size of BITMAPFILEHEADER
	exfont.resize(len + header_size);

	corefile.seekg(position, std::ios_base::beg);
	corefile.read(reinterpret_cast<char*>(exfont.data()) + header_size, len);
	if (corefile.gcount() != len) {
		Output::Debug("EXEReader: ExFont: Error reading resource (read {}, expected {})", corefile.gcount(), len);
		return {};
	}

	auto* exfont_data = reinterpret_cast<const uint8_t*>(exfont.data()) + header_size;
	auto* e = exfont_data + len;
	auto header = ImageBMP::ParseHeader(exfont_data, e);

	// As it turns out, EXFONTs appear to operate on all the same restrictions as an ordinary BMP.
	// Bitmap resources lack the BITMAPFILEHEADER. This header must be generated based on the BITMAPINFOHEADER.
	// And the header that's going to be prepended.
	int header_len = header_size + header.size;
	if (header.depth != 8) {
		Output::Debug("EXEReader: ExFont: Unsupported depth {}", header.depth);
		return {};
	}
	header_len += header.num_colors * 4;

	// 0 (these are in decimal)
	int pos = 0;
	exfont[pos++] = 'B';
	exfont[pos++] = 'M';
	// 2
	uint32_t totallen = exfont.size();
	exfont[pos++] = (totallen) & 0xFF;
	exfont[pos++] = (totallen >> 8) & 0xFF;
	exfont[pos++] = (totallen >> 16) & 0xFF;
	exfont[pos++] = (totallen >> 24) & 0xFF;
	// 6 - Reserved data
	exfont[pos++] = 'E';
	exfont[pos++] = 'x';
	exfont[pos++] = 'F';
	exfont[pos++] = 'n';
	// 10
	exfont[pos++] = (header_len) & 0xFF;
	exfont[pos++] = (header_len >> 8) & 0xFF;
	exfont[pos++] = (header_len >> 16) & 0xFF;
	exfont[pos++] = (header_len >> 24) & 0xFF;

	// Check if the ExFont is the original through a fast hash function
	auto crc = crc32(0, exfont.data() + header_size, exfont.size() - header_size);
	if (crc != 0x86bc6c68) {
		Output::Debug("EXEReader: Custom ExFont found");
	}
	return exfont;
}

static uint32_t exe_reader_roffset(uint32_t bas, uint32_t ofs) {
	return bas + (ofs ^ 0x80000000);
}

std::vector<uint8_t> EXEReader::GetExFont() {
	corefile.clear();

	auto bitmapDBase = ResOffsetByType(2);
	if (bitmapDBase == 0) {
		Output::Debug("EXEReader: BITMAP not found.");
		return {};
	}

	// Looking for a named entry.
	uint16_t resourcesNDEs = GetU16(bitmapDBase + 0x0C) + (uint32_t) GetU16(bitmapDBase + 0x0E);
	uint32_t resourcesNDEbase = bitmapDBase + 0x10;
	while (resourcesNDEs) {
		uint32_t name = GetU32(resourcesNDEbase);
		// Actually a name?
		if (name & 0x80000000) {
			name = exe_reader_roffset(resource_ofs, name);

			if (ResNameCheck(name, "EXFONT")) {
				uint32_t dataent = GetU32(resourcesNDEbase + 4);
				if (dataent & 0x80000000) {
					dataent = exe_reader_roffset(resource_ofs, dataent);
					dataent = resource_ofs + GetU32(dataent + 0x14);
				}
				uint32_t filebase = (GetU32(dataent) - resource_rva) + resource_ofs;
				uint32_t filesize = GetU32(dataent + 0x04);
				Output::Debug("EXEReader: EXFONT resource found (DE {:#x}; {:#x}; len {:#x})", dataent, filebase, filesize);
				return ExtractExFont(corefile, filebase, filesize);
			}
		}
		resourcesNDEbase += 8;
		resourcesNDEs--;
	}
	Output::Debug("EXEReader: EXFONT not found in dbase at {:#x}", bitmapDBase);
	return {};
}

std::vector<std::vector<uint8_t>> EXEReader::GetLogos() {
	corefile.clear();

	if (!resource_ofs) {
		return {};
	}

	if (Player::player_config.show_startup_logos.Get() == ConfigEnum::StartupLogos::None) {
		return {};
	}

	std::vector<std::vector<uint8_t>> logos;

	uint32_t resourcesIDEs = GetU16(resource_ofs + 0x0C);
	if (resourcesIDEs == 1) {
		uint32_t resourcesIDEbase = resource_ofs + 0x10;
		if (ResNameCheck(exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase)), "XYZ")) {
			uint32_t xyz_base = exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
			uint16_t xyz_logos = std::min<uint16_t>(GetU16(xyz_base + 0x0C), 9);
			uint32_t xyz_logo_base = xyz_base + 0x10;

			bool only_custom_logos = (Player::player_config.show_startup_logos.Get() == ConfigEnum::StartupLogos::Custom);
			std::string res_name = "LOGOX";

			for (int i = 0; i <= xyz_logos; ++i) {
				uint32_t name = GetU32(xyz_logo_base);
				// Actually a name?
				if (name & 0x80000000) {
					name = exe_reader_roffset(resource_ofs, name);
					res_name.back() = '1' + i;

					if (ResNameCheck(name, res_name.c_str()) || (i == 0 && ResNameCheck(name, "LOGO"))) {
						uint32_t dataent = GetU32(xyz_logo_base + 4);
						if (dataent & 0x80000000) {
							dataent = exe_reader_roffset(resource_ofs, dataent);
							dataent = resource_ofs + GetU32(dataent + 0x14);
						}
						uint32_t filebase = (GetU32(dataent) - resource_rva) + resource_ofs;
						uint32_t filesize = GetU32(dataent + 0x04);
						Output::Debug("EXEReader: {} resource found (DE {:#x}; {:#x}; len {:#x})", res_name, dataent, filebase, filesize);
						std::vector<uint8_t> logo;
						logo.resize(filesize);

						corefile.seekg(filebase, std::ios_base::beg);
						corefile.read(reinterpret_cast<char*>(logo.data()), filesize);
						if (logo.size() < 8 || strncmp(reinterpret_cast<char*>(logo.data()), "XYZ1", 4) != 0) {
							Output::Debug("EXEReader: {}: Not a XYZ image", res_name);
							return {};
						}

						if (corefile.gcount() != filesize) {
							Output::Debug("EXEReader: {}: Error reading resource (read {}, expected {})", res_name, corefile.gcount(), filesize);
							return {};
						}

						if (only_custom_logos) {
							auto crc = static_cast<uint32_t>(crc32(0, logo.data(), logo.size()));
							if (std::find(logo_crc32.begin(), logo_crc32.end(), crc) == logo_crc32.end()) {
								logos.push_back(logo);
							}
						} else {
							logos.push_back(logo);
						}
					}
				}

				xyz_logo_base += 8;
			}
		}
	}

	return logos;
}

const EXEReader::FileInfo& EXEReader::GetFileInfo() {
	corefile.clear();

	file_info.logos = GetLogoCount();

	auto versionDBase = ResOffsetByType(16);
	if (versionDBase == 0) {
		return file_info;
	}

	uint16_t resourcesNDEs = GetU16(versionDBase + 0x0C) + (uint32_t) GetU16(versionDBase + 0x0E);
	uint32_t resourcesNDEbase = versionDBase + 0x10;
	while (resourcesNDEs) {
		uint32_t id = GetU32(resourcesNDEbase);
		if (id == 1) {
			id = exe_reader_roffset(resource_ofs, id);

			uint32_t dataent = GetU32(resourcesNDEbase + 4);
			if (dataent & 0x80000000) {
				dataent = exe_reader_roffset(resource_ofs, dataent);
				dataent = resource_ofs + GetU32(dataent + 0x14);
			}
			uint32_t filebase = (GetU32(dataent) - resource_rva) + resource_ofs;
			uint32_t filesize = GetU32(dataent + 0x04);

			std::vector<uint8_t> version_info(filesize);
			corefile.seekg(filebase, std::ios_base::beg);
			corefile.read(reinterpret_cast<char*>(version_info.data()), filesize);

			// The start of VS_FIXEDFILEINFO structure is aligned on a 32 bit boundary
			// Instead of calculating search for the signature
			std::array<uint8_t, 4> signature = {0xBD, 0x04, 0xEF, 0xFE};

			auto sig_it = std::search(version_info.begin(), version_info.end(), signature.begin(), signature.end());
			if (sig_it != version_info.end()) {
				uint32_t product_version_off = std::distance(version_info.begin(), sig_it + 16);
				uint32_t version_high = GetU32(filebase + product_version_off);
				uint32_t version_low = GetU32(filebase + product_version_off + 4);

				file_info.version = (static_cast<uint64_t>(version_high) << 32) | version_low;
				file_info.version_str = fmt::format("{}.{}.{}.{}", (version_high >> 16) & 0xFFFF, version_high & 0xFFFF, (version_low >> 16) & 0xFFFF, version_low & 0xFFFF);
			}

			std::array<uint8_t, 30> easyrpg_player_str = {
				0x45, 0x00, 0x61, 0x00, 0x73, 0x00, 0x79, 0x00, 0x52, 0x00, 0x50, 0x00, 0x47, 0x00, 0x20, 0x00,
				0x50, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x79, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00
			};
			auto ep_it = std::search(version_info.begin(), version_info.end(), easyrpg_player_str.begin(), easyrpg_player_str.end());
			file_info.is_easyrpg_player = ep_it != version_info.end();

			Output::Debug("EXEReader: VERSIONINFO resource found (DE {:#x}; {:#x}; len {:#x})", dataent, filebase, filesize);
			return file_info;
		}
		resourcesNDEbase += 8;
		resourcesNDEs--;
	}
	Output::Debug("EXEReader: VERSIONINFO not found in dbase at {:#x}", versionDBase);
	return file_info;
}

uint8_t EXEReader::GetU8(uint32_t i) {
	corefile.seekg(i, std::ios_base::beg);
	int ch = corefile.get();
	if (ch == -1)
		ch = 0;
	return (uint8_t) ch;
}

uint16_t EXEReader::GetU16(uint32_t i) {
	uint16_t v = GetU8(i);
	v |= ((uint32_t) GetU8(i + 1)) << 8;
	return v;
}

uint32_t EXEReader::GetU32(uint32_t i) {
	uint32_t v = GetU16(i);
	v |= ((uint32_t) GetU16(i + 2)) << 16;
	return v;
}

uint32_t EXEReader::ResOffsetByType(uint32_t type) {
	// Part 2 of the resource grabber.
	if (!resource_ofs) {
		return 0;
	}
	// For each ID/Name entry in the outer...
	uint32_t resourcesIDEs = GetU16(resource_ofs + 0x0C) + (uint32_t) GetU16(resource_ofs + 0x0E);
	uint32_t resourcesIDEbase = resource_ofs + 0x10;
	while (resourcesIDEs) {
		if (GetU32(resourcesIDEbase) == type) {
			return exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
		}
		resourcesIDEbase += 8;
		resourcesIDEs--;
	}
	return 0;
}

uint32_t EXEReader::GetLogoCount() {
	if (!resource_ofs) {
		return 0;
	}

	uint32_t resourcesIDEs = GetU16(resource_ofs + 0x0C);
	if (resourcesIDEs == 1) {
		uint32_t resourcesIDEbase = resource_ofs + 0x10;
		if (ResNameCheck(exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase)), "XYZ")) {
			uint32_t xyz_logo_base = exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
			return static_cast<uint32_t>(GetU16(xyz_logo_base + 0x0C));
		}
	}
	return 0;
}

bool EXEReader::ResNameCheck(uint32_t i, const char* p) {
	if (GetU16(i) != strlen(p))
		return false;
	while (*p) {
		i += 2;
		if (GetU16(i) != *p)
			return false;
		p++;
	}
	return true;
}

void EXEReader::FileInfo::Print() const {
	Output::Debug("RPG_RT information: version={} logos={} code={:#x} entrypoint={:#x} cherry={:#x} geep={:#x} arch={} easyrpg={}", version_str, logos, code_size, entrypoint, cherry_size, geep_size, kMachineTypes[machine_type], is_easyrpg_player);
}

int EXEReader::FileInfo::GetEngineType(bool& is_maniac_patch) const {
	is_maniac_patch = false;

	if (is_easyrpg_player || machine_type == MachineType::Unknown) {
		return Player::EngineNone;
	}

	if (version_str.empty()) {
		// RPG2k and Rpg2k3 < 1.0.2.1 has no VERSIONINFO
		if (logos == 3) {
			// three logos only appear in old RPG2k
			return Player::EngineRpg2k;
		} else if (logos == 1) {
			// VALUE! or Rpg2k3 < 1.0.2.1
			// Check CODE segment size to be sure
			if (code_size > 0xB0000) {
				if (code_size >= 0xC7400) {
					// Code segment size for >= 1.0.5.0
					// In theory this check is unnecessary because this version has a VERSIONINFO.
					// However the modified exe shipped with Ahriman's Prophecy is a 1.0.8.0 without a VERSIONINFO.
					return Player::EngineRpg2k3 | Player::EngineMajorUpdated;
				} else {
					return Player::EngineRpg2k3;
				}
			}

			return Player::EngineRpg2k | Player::EngineMajorUpdated;
		}

		return Player::EngineNone;
	}

	std::array<decltype(version), 4> ver = {
		(version >> 48) & 0xFF,
		(version >> 32) & 0xFF,
		(version >> 16) & 0xFF,
		(version & 0xFF)};

	if (logos == 0) {
		// New version of Maniac Patch is version 1.1.2.1 and is an rewrite of the engine
		// Has no logos, no CODE segment and no CHERRY segment
		if (ver[0] == 1 && ver[1] == 1 && ver[2] == 2 && ver[3] == 1 && code_size == 0 && cherry_size == 0) {
			is_maniac_patch = true;
			return Player::EngineRpg2k3 | Player::EngineMajorUpdated | Player::EngineEnglish;
		}

		return Player::EngineNone;
	}

	// Everything else with a VERSIONINFO must have one logo
	if (logos != 1) {
		return Player::EngineNone;
	}

	if (ver[0] == 1) {
		if (ver[1] == 6) {
			// English release of RPG Maker 2000 has a version info (thanks Cherry!)
			return Player::EngineRpg2k | Player::EngineMajorUpdated | Player::EngineEnglish;
		} else if (ver[1] == 0) {
			// Everything else with a version info is RPG Maker 2003
			if (ver[2] < 5) {
				return Player::EngineRpg2k3;
			} else {
				return Player::EngineRpg2k3 | Player::EngineMajorUpdated;
			}
		} else if (ver[1] == 1) {
			if (ver[2] == 2 && ver[3] == 1) {
				// Old versions of Maniac Patch are a hacked 1.1.2.1
				// The first versions have a GEEP segment (No idea what this abbreviation means)
				// Later versions have no GEEP segment but an enlarged CHERRY segment
				is_maniac_patch = (geep_size > 0 || cherry_size > 0x10000);
			}

			return Player::EngineRpg2k3 | Player::EngineMajorUpdated | Player::EngineEnglish;
		}
	}

	return Player::EngineNone;
}

std::map<EXE::Shared::GameConstantType, int32_t> EXEReader::GetOverridenGameConstants() {
	constexpr bool debug_const_extraction = true;

	std::map<EXE::Shared::GameConstantType, int32_t> game_constants;
	int code_offset = file_info.code_ofs - 0x400;

	auto match_surrounding_data = [&](const EXE::BuildInfo::CodeAddressInfo& info, const uint32_t const_ofs) {
		for (int i = 0; i < info.pre_data.size(); i++) {
			if (info.pre_data[i] != GetU8(const_ofs - info.pre_data.size() + i))
				return false;
		}
		/*for (int i = 0; i < info.post_data.size(); i++) {
			if (info.post_data[i] != GetU8(const_ofs + sizeof(uint32_t) + i))
				return false;
		}*/
		//Is a hit -> constant value can be extracted
		return true;
	};

	auto check_address_map = [&](const EXE::Constants::code_address_map& map) {
		uint32_t const_ofs;
		bool extract_success = false;

		for (auto it = map.begin(); it != map.end();++it) {
			auto const_type = it->first;
			auto& addr_info = it->second;

			if (addr_info.code_offset == 0) {
				// constant is not defined in this map
				continue;
			}

			const_ofs = code_offset + addr_info.code_offset;

			bool extract_constant = false;
			/*if (addr_info.pre_data == ExeConstants::magic_prev && extract_success) {
				extract_constant = true;
			} else*/
			if (match_surrounding_data(addr_info, const_ofs)) {
				extract_constant = true;
			}

			if (extract_constant) {
				int32_t value;
				switch (addr_info.size_val) {
					case 4:
						value = GetU32(const_ofs);
						break;
					case 2:
						value = GetU16(const_ofs);
						break;
					case 1:
						value = GetU8(const_ofs);
						break;
					default:
						continue;
				}

				auto it = game_constants.find(const_type);
				if (it != game_constants.end() && it->second == value) {
					// Constant override has already been applied through some other means
					continue;
				}

				if (value != addr_info.default_val || it != game_constants.end()) {
					game_constants[const_type] = value;
					Output::Debug("Read constant '{}': {} (default: {})", EXE::Shared::kGameConstantType.tag(const_type), value, addr_info.default_val);
				} else if (debug_const_extraction) {
					Output::Debug("Constant '{}' unchanged: {}", EXE::Shared::kGameConstantType.tag(const_type), value);
				}
				extract_success = true;
			} else {
				Output::Debug("Could not read constant '{}'", EXE::Shared::kGameConstantType.tag(const_type));
				extract_success = false;
			}
		}
	};

	auto apply_known_config = [&](EXE::Constants::KnownPatchConfigurations conf) {
		Output::Debug("Assuming known patch config '{}'", EXE::Constants::kKnownPatchConfigurations.tag(static_cast<int>(conf)));
		auto it_conf = EXE::Constants::known_patch_configurations.find(conf);
		assert(it_conf != EXE::Constants::known_patch_configurations.end());

		for (auto it = it_conf->second.begin(); it != it_conf->second.end(); ++it) {
			game_constants[it->first] = it->second;
		}
	};

	for (auto it = EXE::BuildInfo::known_engine_builds.begin(); it != EXE::BuildInfo::known_engine_builds.end(); ++it) {
		auto& curr_build_info = std::get<EXE::BuildInfo::EngineBuildInfo>(*it);

		if (file_info.code_size != curr_build_info.code_size || file_info.entrypoint != curr_build_info.entrypoint) {
			continue;
		}
		build_version = std::get<EXE::BuildInfo::KnownEngineBuildVersions>(*it);
		build_info = curr_build_info;
		break;
	}

	if (build_version != EXE::BuildInfo::KnownEngineBuildVersions::UnknownBuild) {
		Output::Debug("Assuming {} build '{}' for constant extraction",
			EXE::BuildInfo::kEngineTypes.tag(build_info.engine_type),
			EXE::BuildInfo::kKnownEngineBuildDescriptions.tag(build_version));

		EXE::Constants::code_address_map const* constant_addresses = nullptr;

		switch (build_info.engine_type) {
			case EXE::BuildInfo::EngineType::RPG2000:
			{
				auto& builds = EXE::Constants::known_engine_builds_rm2k;
				auto it = std::find_if(builds.begin(), builds.end(), [&](const auto& pair) {
					return pair.first == build_version;
				});
				if (it != builds.end()) {
					constant_addresses = &it->second;
				}
			}
			break;
			case EXE::BuildInfo::EngineType::RPG2003:
			{
				auto& builds = EXE::Constants::known_engine_builds_rm2k3;
				auto it = std::find_if(builds.begin(), builds.end(), [&](const auto& pair) {
					return pair.first == build_version;
				});
				if (it != builds.end()) {
					constant_addresses = &it->second;
				}
			}
			break;
		}

		switch (build_version) {
			case EXE::BuildInfo::RM2KE_162:
				if (CheckForString(0x07DEA6, "XXX" /* 3x "POP EAX" */)) {
					apply_known_config(EXE::Constants::KnownPatchConfigurations::QP_StatDelimiter);
				}
				break;
			case EXE::BuildInfo::RM2K3_1080_1080:
				if (CheckForString(0x08EFE0, "NoTitolo")) {
					apply_known_config(EXE::Constants::KnownPatchConfigurations::Rm2k3_Italian_WD_108);
				}
				if (CheckForString(0x09D679, "XXX" /* 3x "POP EAX" */)) {
					apply_known_config(EXE::Constants::KnownPatchConfigurations::QP_StatDelimiter);
				}
				break;
			case EXE::BuildInfo::RM2K3_1091_1091:
				if (CheckForString(0x09C9AD, "XXX" /* 3x "POP EAX" */)) {
					apply_known_config(EXE::Constants::KnownPatchConfigurations::QP_StatDelimiter);
				}
				break;
			default:
				break;
		}

		if (constant_addresses) {
			check_address_map(*constant_addresses);
		}
	} else {
		Output::Debug("Unknown build");
	}
	return game_constants;
}


std::vector<EXE::Shared::PatchSetupInfo> EXEReader::CheckForPatches() {
	std::vector<EXE::Shared::PatchSetupInfo> patches;

	int code_offset = file_info.code_ofs - 0x400;

	auto check_for_patch_segment = [&](const EXE::BuildInfo::PatchDetectionInfo& patch_info) {
		for (int i = 0; i < patch_info.chk_segment_data.size(); i++) {
			if (patch_info.chk_segment_data[i] != GetU8(code_offset + patch_info.chk_segment_offset + i))
				return false;
		}
	};

	auto apply_patches = [&](const auto& patch_detection_map) {
		for (auto it = patch_detection_map.begin(); it < patch_detection_map.end(); ++it) {
			auto patch_type = it->first;
			auto& patch_info = it->second;

			if (!check_for_patch_segment(patch_info)) {
				continue;
			}

			if (patch_info.extract_var_offset == 0) {
				Output::Debug("Detected Patch: '{}'", EXE::Shared::kKnownPatches.tag(static_cast<int>(patch_type)));
				patches.emplace_back(EXE::Shared::PatchSetupInfo { patch_type, 0 });
			} else {
				int patch_var = GetU32(code_offset + patch_info.extract_var_offset);
				if (patch_var > 0) {
					Output::Debug("Detected Patch: '{}' (VarId: {})", EXE::Shared::kKnownPatches.tag(static_cast<int>(patch_type)), patch_var);
				}
				patches.emplace_back(EXE::Shared::PatchSetupInfo{ patch_type, patch_var });
			}
		}
	};

	switch (build_version) {
		case EXE::BuildInfo::RM2K_20001227:
			apply_patches(EXE::Patches::patches_RM2K_107);
			break;
		case EXE::BuildInfo::RM2K_20010505:
			apply_patches(EXE::Patches::patches_RM2K_110);
			break;
		case EXE::BuildInfo::RM2K_20030327:
			apply_patches(EXE::Patches::patches_RM2K_150);
			break;
		case EXE::BuildInfo::RM2K_20030625:
			apply_patches(EXE::Patches::patches_RM2K_151);
			break;
		case EXE::BuildInfo::RM2KE_160:
			apply_patches(EXE::Patches::patches_RM2K_160);
			break;
		case EXE::BuildInfo::RM2KE_161:
			apply_patches(EXE::Patches::patches_RM2K_161);
			break;
		case EXE::BuildInfo::RM2KE_162:
			apply_patches(EXE::Patches::patches_RM2K_162);
			break;

		case EXE::BuildInfo::RM2K3_1080_1080:
			apply_patches(EXE::Patches::patches_RM2K3_1080);
			break;
		case EXE::BuildInfo::RM2K3_1091_1091:
			apply_patches(EXE::Patches::patches_RM2K3_1091);
			break;
		default:
			break;
	}

	return patches;
}

bool EXEReader::CheckForString(uint32_t offset, const char* p) {
	while (*p) {
		if (GetU8(file_info.code_ofs - 0x400 + offset++) != *p++)
			return false;
	}
	return true;
}
#endif
