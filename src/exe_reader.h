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

#ifndef EP_EXE_READER_H
#define EP_EXE_READER_H

#include <cstdint>
#include <string>
#include <istream>
#include <vector>
#include "bitmap.h"
#include "player.h"

/**
 * Extracts resources from an EXE.
 * The istream given is still owned by the parent,
 *  but it is in use by the reader.
 */
class EXEReader {
public:
	// uint32_t is used here as:
	// 1. everywhere else uses "unsigned", which is equally as odd...
	// 2. max offset value is this size

	EXEReader(Filesystem_Stream::InputStream core);

	// Extracts an EXFONT resource with BMP header if present
	// and returns exfont buffer on success.
	std::vector<uint8_t> GetExFont();

	struct FileInfo {
		uint64_t version = 0;
		int logos = 0;
		std::string version_str;
		uint32_t code_size = 0;
		uint32_t cherry_size = 0;
		uint32_t geep_size = 0;
		bool is_i386 = true;
		bool is_easyrpg_player = false;

		int GetEngineType(bool& is_maniac_patch) const;
		void Print() const;
	};

	const FileInfo& GetFileInfo();

private:
	// Bounds-checked unaligned reader primitives.
	// In case of out-of-bounds, returns 0 - this will usually result in a harmless error at some other level,
	//  or a partial correct interpretation.
	uint8_t GetU8(uint32_t point);
	uint16_t GetU16(uint32_t point);
	uint32_t GetU32(uint32_t point);

	uint32_t ResOffsetByType(uint32_t type);
	uint32_t GetLogoCount();
	bool ResNameCheck(uint32_t namepoint, const char* name);

	// 0 if resource section was unfindable.
	uint32_t resource_ofs = 0;
	uint32_t resource_rva = 0;

	FileInfo file_info;
	Filesystem_Stream::InputStream corefile;
};

#endif
