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

#include "exe_reader.h"
#include "filefinder.h"
#include "bitmap.h"
#include "output.h"
#include <iostream>
#include <fstream>

EXEReader::EXEReader(std::istream & core) : corefile(core) {
	// The Incredibly Dumb Resource Grabber (tm)
	// The idea is that this code will eventually be moved to happen earlier or broken down as-needed.
	// Since EXFONT is the only thing that matters right now, it's the only thing handled.
	uint32_t ofs = GetU32(0x3C);
	uint16_t sections = GetU16(ofs + 6);
	uint32_t sectionsOfs = ofs + 0x18 + GetU16(ofs + 0x14);
	resource_rva = GetU32(ofs + 0x88);
	if (!resource_rva) {
		resource_ofs = 0;
		return;
	}
	while (sections) {
		uint32_t sectVs = GetU32(sectionsOfs + 0x08);
		uint32_t sectRs = GetU32(sectionsOfs + 0x10);
		if (sectRs > sectVs) {
			// Actually a problem in some files.
			sectVs = sectRs;
		}
		uint32_t sectRva = GetU32(sectionsOfs + 0x0C);
		uint32_t sectRdptr = GetU32(sectionsOfs + 0x14);
		if ((sectRva <= resource_rva) && ((sectRva + sectVs) > resource_rva)) {
			// Resources located.
			resource_ofs = sectRdptr + (resource_rva - sectRva);
			break;
		}
		sections--;
		sectionsOfs += 0x28;
	}
	if (sections == 0) {
		resource_rva = 0;
		resource_ofs = 0;
	}
}

EXEReader::~EXEReader() {
}

static bool exe_reader_perform_exfont_save(std::string f, std::istream & corefile, uint32_t pos, uint32_t len) {
	corefile.seekg(pos, std::ios_base::beg);
	// Solely for calculating position of actual data
	uint32_t hdrL = corefile.get();
	hdrL |= ((uint32_t) corefile.get()) << 8;
	hdrL |= ((uint32_t) corefile.get()) << 16;
	hdrL |= ((uint32_t) corefile.get()) << 24;
	// As it turns out, EXFONTs appear to operate on all the same restrictions as an ordinary BMP.
	// Given this particular resource is loaded by the RPG Maker half of the engine, this makes the usual amount of sense.
	// This means 256 palette entries. Without fail. Even though only two are used, the first and last.
	// Since this is a packed bitmap, there's nothing else to worry about.
	hdrL += 256 * 4;
	// And the header that's going to be prepended.
	hdrL += 14;

	std::shared_ptr<std::fstream> exfilex = FileFinder::openUTF8(f, std::ios_base::binary | std::ios_base::out);
	std::fstream & exfile = *exfilex;
	if (exfile.fail()) {
		Output::Debug("EXEReader::GetExfont : Unable to open exfont file");
		return false;
	}

	// 0 (these are in decimal)
	exfile.put('B');
	exfile.put('M');
	// 2
	uint32_t totallen = len + 14;
	exfile.put((totallen) & 0xFF);
	exfile.put((totallen >> 8) & 0xFF);
	exfile.put((totallen >> 16) & 0xFF);
	exfile.put((totallen >> 24) & 0xFF);
	// 6
	exfile.put('E');
	exfile.put('x');
	exfile.put('F');
	exfile.put('n');
	// 10
	exfile.put((hdrL) & 0xFF);
	exfile.put((hdrL >> 8) & 0xFF);
	exfile.put((hdrL >> 16) & 0xFF);
	exfile.put((hdrL >> 24) & 0xFF);

	corefile.seekg(pos, std::ios_base::beg);
	while (len > 0) {
		int v = corefile.get();
		if (v == -1)
			break;
		exfile.put(v);
		len--;
	}
	
	exfile.close();
	return true;
}

static uint32_t exe_reader_roffset(uint32_t bas, uint32_t ofs) {
	return bas + (ofs ^ 0x80000000);
}

bool EXEReader::GetExfont(std::string file) {
	// Part 2 of the resource grabber.
	if (!resource_ofs) {
		Output::Debug("EXEReader::GetExfont : No resource section.");
		return false;
	}
	// For each ID/Name entry in the outer...
	uint32_t resourcesIDEs = GetU16(resource_ofs + 0x0C) + (uint32_t) GetU16(resource_ofs + 0x0E);
	uint32_t resourcesIDEbase = resource_ofs + 0x10;
	while (resourcesIDEs) {
		// This can only be 2 if valid. Don't worry about it.
		if (GetU32(resourcesIDEbase) == 2) {
			uint32_t bitmapDBase = exe_reader_roffset(resource_ofs, GetU32(resourcesIDEbase + 4));
			// Looking for a named entry.
			uint16_t resourcesNDEs = GetU16(bitmapDBase + 0x0C) + (uint32_t) GetU16(bitmapDBase + 0x0E);
			uint32_t resourcesNDEbase = bitmapDBase + 0x10;
			while (resourcesNDEs) {
				uint32_t name = GetU32(resourcesNDEbase);
				// Actually a name?
				if (name & 0x80000000) {
					name = exe_reader_roffset(resource_ofs, name);
					// Output::Debug("EXEReader::GetExfont : Name at %x.", name);
					if (ResNameCheck(name, "EXFONT")) {
						uint32_t dataent = GetU32(resourcesNDEbase + 4);
						if (dataent & 0x80000000) {
							dataent = exe_reader_roffset(resource_ofs, dataent);
							dataent = resource_ofs + GetU32(dataent + 0x14);
						}
						uint32_t filebase = (GetU32(dataent) - resource_rva) + resource_ofs;
						uint32_t filesize = GetU32(dataent + 0x04);
						Output::Debug("EXEReader::GetExfont : EXFONT found (DE %x ; %x ; len %x), committing to extraction.", dataent, filebase, filesize);
						return exe_reader_perform_exfont_save(file, corefile, filebase, filesize);
					}
				}
				resourcesNDEbase += 8;
				resourcesNDEs--;
			}
			Output::Debug("EXEReader::GetExfont : EXFONT not found in dbase at %x.", bitmapDBase);
			return false;
		}
		resourcesIDEbase += 8;
		resourcesIDEs--;
	}
	Output::Debug("EXEReader::GetExfont : BITMAP not found.");
	return false;
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

bool EXEReader::ResNameCheck(uint32_t i, const char * p) {
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
