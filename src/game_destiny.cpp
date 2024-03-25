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
#include "game_destiny.h"

#ifndef EMSCRIPTEN
#include "exe_reader.h"
#endif // !EMSCRIPTEN
#include "output.h"


using lcf::ToString;
using lcf::rpg::EventCommand;


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//				Game_Destiny implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void Game_Destiny::Load()
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
	// TODO [Dr.XGB]: Find to manage Destiny initialization parameters on Emscripten
	dllVersion = 0x20000;
	language = ENGLISH;
	gameVersion = 0x20000107;
	extra = 0x01;
	dwordSize = floatSize = stringSize = 0x64;
#endif // !EMSCRIPTEN

	Initialize(dllVersion, language, gameVersion, extra, dwordSize, floatSize, stringSize);
}

Game_Destiny::Game_Destiny()
{
	_dllVersion = {};
	_gameVersion = {};
	_language = Destiny::Language::DEUTSCH;
	_extra = 0U;
	_decimalComma = false;
}

Game_Destiny::~Game_Destiny()
{
	Terminate();
}

void Game_Destiny::Initialize(
	uint32_t dllVersion,
	uint32_t language,
	uint32_t gameVersion,
	uint32_t extra,
	uint32_t dwordSize,
	uint32_t floatSize,
	uint32_t stringSize)
{
	_dllVersion = Destiny::Version(dllVersion);
	_gameVersion = Destiny::Version(gameVersion);
	_language = static_cast<Destiny::Language>(language);
	_extra = extra;
	_decimalComma = _language == Destiny::Language::ENGLISH;

	// Init containers
	_dwords.resize(dwordSize);
	_floats.resize(floatSize);
	_strings.resize(stringSize);

	// TODO: Init File container
	// TODO: Init ClientSocket container

	// Debug
	Output::Debug("Destiny Initialized");
	Output::Debug("Language: {}", _decimalComma ? "English" : "Deutsch");
	Output::Debug("DLL Version: {}", _dllVersion.toString());
	Output::Debug("Dwords: {}", dwordSize);
	Output::Debug("Floats: {}", floatSize);
	Output::Debug("Strings: {}", stringSize);
}

void Game_Destiny::Terminate()
{
	_dwords.clear();
	_floats.clear();
	_strings.clear();

	// TODO: Clear File container
	// TODO: Clear ClientSocket container
}

std::string Game_Destiny::MakeString(lcf::rpg::SaveEventExecFrame& scriptData)
{
	std::string code;

	int32_t& current = scriptData.current_command;
	const std::vector<EventCommand>& cmdList = scriptData.commands;
	std::vector<EventCommand>::const_iterator it = cmdList.begin() + current++;

	code = ToString((*it++).string).substr(1);
	while (it != cmdList.cend() && it->code == static_cast<int32_t>(EventCommand::Code::Comment_2)) {
		code += '\n';
		code += ToString((*it++).string);
		++current;
	}

	return code;
}

bool Game_Destiny::Interpret(const std::string& code)
{
	// TODO [Dr.XGB]: DestinyScript Interpret
	return true;
}
