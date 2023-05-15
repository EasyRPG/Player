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

EXEReader::EXEReader(Filesystem_Stream::InputStream core) : corefile(std::move(core)) {
	// The Incredibly Dumb PE parser (tm)
	// Extracts data from the resource section for engine detection and can read ExFont.
	uint32_t ofs = GetU32(0x3C);
	uint16_t machine = GetU16(ofs + 4);
	if (machine != 0x14c) {
		// FIXME: Newer versions of Maniac Patch can be 64 Bit
		Output::Debug("EXEReader: Machine type is not i386 ({:#x})", machine);
		file_info.is_i386 = false;
		return;
	}

	// The largest known exe has 11 segments, guard against bogus section data here
	uint16_t sections = std::min<uint16_t>(GetU16(ofs + 6), 11);
	uint32_t sectionsOfs = ofs + 0x18 + GetU16(ofs + 0x14);
	resource_rva = GetU32(ofs + 0x88);
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
		} else if (secName == 0x52454843) { // CHER(RY)
			file_info.cherry_size = sectVs;
		} else if (secName == 0x50454547) { // GEEP
			file_info.geep_size = sectVs;
		} else if (secName == 0x30585055) { // UPX0
			Output::Debug("EXE is UPX compressed. Engine detection could be incorrect.");
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

static uint32_t djb2_hash(char* str, size_t length) {
	uint32_t hash = 5381;
	for (size_t i = 0; i < length; ++i) {
		hash = ((hash << 5) + hash) + (uint8_t)str[i];
	}
	return hash;
}

static std::vector<uint8_t> exe_reader_perform_exfont_save(Filesystem_Stream::InputStream& corefile, uint32_t position, uint32_t len) {
	std::vector<uint8_t> exfont;
	constexpr int header_size = 14; // Size of BITMAPFILEHEADER
	exfont.resize(len + header_size);

	corefile.seekg(position, std::ios_base::beg);
	corefile.read(reinterpret_cast<char*>(exfont.data()) + header_size, len);
	if (corefile.gcount() != len) {
		Output::Debug("ExFont: Error reading resource (read {}, expected {})", corefile.gcount(), len);
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
		Output::Debug("ExFont: Unsupported depth {}", header.depth);
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
	if (djb2_hash((char*)exfont.data() + header_size, exfont.size() - header_size) != 0x491e19de) {
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
				return exe_reader_perform_exfont_save(corefile, filebase, filesize);
			}
		}
		resourcesNDEbase += 8;
		resourcesNDEs--;
	}
	Output::Debug("EXEReader: EXFONT not found in dbase at {:#x}", bitmapDBase);
	return {};
}

const EXEReader::FileInfo& EXEReader::GetFileInfo() {
	corefile.clear();

	file_info.logos = GetLogoCount();

	auto versionDBase = ResOffsetByType(16);
	if (versionDBase == 0) {
		Output::Debug("EXEReader: VERSIONINFO not found");
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
	// For each ID/Name entry in the outer...
	uint32_t resourcesIDEs = GetU16(resource_ofs + 0x0C);
	if (resourcesIDEs == 1) {
		uint32_t resourcesIDEbase = resource_ofs + 0x10;
		if (ResNameCheck(exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase)), "XYZ")) {
			uint32_t xyz_logo_base = exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
			return static_cast<uint32_t>(GetU16(xyz_logo_base + 0x0C));
			return exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
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
	Output::Debug("RPG_RT information: version={} logos={} code={:#x} cherry={:#x} geep={:#x} i386={} easyrpg={}", version_str, logos, code_size, cherry_size, geep_size, is_i386, is_easyrpg_player);
}

int EXEReader::FileInfo::GetEngineType(bool& is_maniac_patch) const {
	is_maniac_patch = false;

	if (is_easyrpg_player || !is_i386) {
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
				return Player::EngineRpg2k3;
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

#endif
