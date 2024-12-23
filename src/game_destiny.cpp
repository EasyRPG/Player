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
#include "filefinder.h"

#ifndef EMSCRIPTEN
#include "exe_reader.h"
#endif // !EMSCRIPTEN
#include "output.h"

using Destiny::InterpretFlag;
using Destiny::MainFunctions::Interpreter;

using lcf::ToString;
using lcf::rpg::EventCommand;
using lcf::rpg::SaveEventExecFrame;


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//				Game_Destiny implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void Game_Destiny::Load()
{
	// Do not load Destiny whether player cannot find "Destiny.dll"
	if (! FileFinder::Game().Exists(DESTINY_DLL))
	{
		return;
	}

	uint32_t dllVersion = 0;
	uint32_t language = 0;
	uint32_t gameVersion = 0;
	uint32_t extra = 0;
	uint32_t dwordSize = 0;
	uint32_t floatSize = 0;
	uint32_t stringSize = 0;

#ifndef EMSCRIPTEN
	Filesystem_Stream::InputStream exe = FileFinder::Game().OpenFile(EXE_NAME);

	if (exe)
	{
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
	// TODO [XGB]: Find to manage Destiny initialization parameters on Emscripten
	dllVersion = 0x20000;
	language = Destiny::Language::ENGLISH;
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
	_trueColor = 0U;
	_decimalComma = false;
	_rm2k3 = false;
	_protect = false;
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
	uint32_t stringSize
)
{
	_dllVersion = Destiny::Version(dllVersion);
	_gameVersion = Destiny::Version(gameVersion);
	_language = language <= 1
		? static_cast<Destiny::Language>(language)
		: Destiny::Language::ENGLISH;
	_extra = extra;
	_decimalComma = _language == Destiny::Language::DEUTSCH;

	EvaluateExtraFlags();
	CheckVersionInfo();

	// Init containers
	_dwords.resize(dwordSize);
	_floats.resize(floatSize);
	_strings.resize(stringSize);

	// TODO: Init File container
	// TODO: Init ClientSocket container

	// Debug
	Output::Debug("[Destiny] Initialized");
	Output::Debug("[Destiny] Language: {}", _decimalComma ? "Deutsch" : "English");
	Output::Debug("[Destiny] DLL Version: {}", _dllVersion.toString());
	Output::Debug("[Destiny] Dwords: {}", dwordSize);
	Output::Debug("[Destiny] Floats: {}", floatSize);
	Output::Debug("[Destiny] Strings: {}", stringSize);
	Output::Debug("[Destiny] RPG Maker version: {}", _rm2k3 ? 2003 : 2000);
}

void Game_Destiny::Terminate()
{
	_dwords.clear();
	_floats.clear();
	_strings.clear();

	// TODO [XGB]: Clear File container
	// TODO [XGB]: Clear ClientSocket container
}

bool Game_Destiny::Main(SaveEventExecFrame& frame)
{
	const char* script;
	InterpretFlag flag;

	script = _interpreter.MakeString(frame);
	flag = InterpretFlag::IF_EXIT;

	_interpreter.CleanUpData();
	_interpreter.LoadInterpretStack();

	while (! _interpreter.IsEndOfScript())
	{
		if (_interpreter.IsEndOfLine())
		{
			_interpreter.ScriptNextChar();
		}

		flag = _interpreter.Interpret();
		if (flag == InterpretFlag::IF_EXIT)
		{
			break;
		}
	}

	Output::Debug("DestinyScript Code:\n{}", script);
	_interpreter.FreeString();

	return true;
}

void Game_Destiny::EvaluateExtraFlags()
{
	_trueColor = (_extra & Destiny::DF_TRUECOLOR) << 9;
	_protect = _extra & Destiny::DF_PROTECT;
}

void Game_Destiny::CheckVersionInfo()
{
	uint32_t gameVersionMajor;

	gameVersionMajor = _gameVersion.major;

	if (! (gameVersionMajor == 0x2000 || gameVersionMajor == 0x2003))
	{
		Output::Error("[Destiny]: {} is not a valid version", gameVersionMajor);
	}

	_rm2k3 = gameVersionMajor == 0x2003;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//			MainFunctions / Interpreter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Interpreter::Interpreter()
{
	CleanUpData();
	_scriptPtr = nullptr;
}

const char* Interpreter::MakeString(SaveEventExecFrame& frame)
{
	std::string code;

	int32_t& current = frame.current_command;
	const std::vector<EventCommand>& cmdList = frame.commands;
	std::vector<EventCommand>::const_iterator it = cmdList.begin() + current++;

	code = ToString((*it++).string);

	while (it != cmdList.cend() && it->code == static_cast<int32_t>(EventCommand::Code::Comment_2))
	{
		code += '\n';
		code += ToString((*it++).string);
		++current;
	}

	_destinyScript = code;
	return _scriptPtr = _destinyScript.data();
}

void Interpreter::FreeString()
{
	_destinyScript = "";
	_scriptPtr = _destinyScript.data();
}

void Interpreter::SkipWhiteSpace()
{
	while (IsWhiteSpace(*_scriptPtr))
	{
		++_scriptPtr;
	}
}

size_t Interpreter::GetWordLen()
{
	char* endPtr = _scriptPtr;

	while (IsWordChar(*endPtr))
	{
		++endPtr;
	}

	return endPtr - _scriptPtr;
}

InterpretFlag Interpreter::Interpret()
{
	/*char* code;
	uint8_t flags[4];
	InterpretFlag returnType;

	size_t wordLen;

	code = nullptr;
	returnType = IF_COMMAND;*/

	SkipSpace();
	//wordLen = GetWordLen();

	return IF_EXIT;
}

void Interpreter::LoadInterpretStack()
{
	//
}

void Interpreter::SkipSpace()
{
	bool finished = false;
	char next = '\0';

	while (! finished)
	{
		if (! IsWhiteSpace(*++_scriptPtr))
		{
			if (*_scriptPtr == '/')
			{
				next = *(_scriptPtr + 1);

				if (next == '/')
				{
					finished = LineComment();
				}
				else if (next == '*')
				{
					finished = BlockComment();
				}
			}
			else
			{
				finished = true;
			}
		}
	}
}

bool Interpreter::LineComment()
{
	_scriptPtr += 2;

	while (*(++_scriptPtr))
	{
		if (*_scriptPtr == 0x0A)
		{
			return false;
		}
	}

	return true;
}

bool Interpreter::BlockComment()
{
	_scriptPtr += 2;

	while (*(++_scriptPtr))
	{
		if (*_scriptPtr == '*' && *(++_scriptPtr) == '/')
		{
			return false;
		}
	}

	return true;
}
