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
#include "destiny.h"

#include <sstream>
#include <vector>

#ifndef EMSCRIPTEN
#include "exe_reader.h"
#endif // !EMSCRIPTEN
#include "output.h"



// Definitions
using destiny_dword_list = std::vector<int>;
using destiny_float_list = std::vector<double>;
using destiny_string_list = std::vector<std::string>;


namespace Destiny {
	enum Language {
		DEUTSCH = 0,
		ENGLISH,
	};

	struct Version {
		uint16_t major;
		uint16_t minor;

		Version()
			: major(0), minor(0) {}
		Version(uint32_t version) {
			major = version >> 0x10;
			minor = version & 0xFFFF;
		}

		std::string toString() {
			std::stringstream ss;

			ss << major << '.' << minor;
			return ss.str();
		}
	};

	// Destiny containers
	static destiny_dword_list dwords;
	static destiny_float_list floats;
	static destiny_string_list strings;

	// Destiny data
	static Version dllVersion;
	static Version gameVersion;
	static Language language;
	static uint32_t extra;
}


// Implementations
void Destiny::Load()
{
	// Do not load Destiny whether player cannot find "Destiny.dll"
	if (!FileFinder::Game().Exists(DESTINY_DLL))
		return;

	uint32_t dllVersion = 0;
	uint32_t language = 0;
	uint32_t gameVersion = 0;
	uint32_t extra = 0;
	uint32_t dwordSize = 0;
	uint32_t floatSize = 0;
	uint32_t stringSize = 0;

#ifndef EMSCRIPTEN
	Filesystem_Stream::InputStream exe = FileFinder::Game().OpenFile(EXE_NAME);

	if (exe) {
		exe.seekg(0x00030689, std::ios_base::beg);
		exe.read(reinterpret_cast<char*>(&stringSize), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&floatSize), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&dwordSize), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&extra), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&gameVersion), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&language), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
		exe.read(reinterpret_cast<char*>(&dllVersion), sizeof(uint32_t));
		exe.seekg(1, std::ios_base::cur);
	}
#else
	// TODO [Dr.XGB]: Find to manage Destiny initialization parameters on emscripten
	dllVersion = 0x20000;
	language = ENGLISH;
	gameVersion = 0x20000107;
	extra = 0x01;
	dwordSize = floatSize = stringSize = 0x64;
#endif // !EMSCRIPTEN

	Initialize(dllVersion, language, gameVersion, extra, dwordSize, floatSize, stringSize);
}

void Destiny::Initialize(uint32_t _dllVersion, uint32_t _language, uint32_t _gameVersion, uint32_t _extra, uint32_t _dwordSize, uint32_t _floatSize, uint32_t _stringSize)
{
	dllVersion = Version(_dllVersion);
	gameVersion = Version(_gameVersion);
	language = (Language)_language;
	extra = _extra;
	dwords.resize(_dwordSize);
	floats.resize(_floatSize);
	strings.resize(_stringSize);

	Output::Debug("Destiny Initialized");
	Output::Debug("DLL Version: {}", dllVersion.toString());
	Output::Debug("Dwords: {}", _dwordSize);
	Output::Debug("Floats: {}", _floatSize);
	Output::Debug("Strings: {}", _stringSize);
}
