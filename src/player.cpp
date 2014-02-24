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
#include "player.h"
#include "audio.h"
#include "cache.h"
#include "filefinder.h"
#include "game_actors.h"
#include "game_message.h"
#include "game_map.h"
#include "game_party.h"
#include "game_player.h"
#include "game_system.h"
#include "game_temp.h"
#include "graphics.h"
#include "input.h"
#include "ldb_reader.h"
#include "lmt_reader.h"
#include "main_data.h"
#include "output.h"
#include "reader_lcf.h"
#include "scene_logo.h"
#include "scene_map.h"
#include "scene_title.h"
#include "scene_battle.h"
#include "system.h"
#include "utils.h"

#include <algorithm>
#include <set>
#include <locale>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#ifdef GEKKO
	#include <fat.h>
#endif
#if (defined(_WIN32) && defined(NDEBUG))
	#include <windows.h>
	#include <winioctl.h>
	#include <dbghelp.h>
	static void InitMiniDumpWriter();
#endif

namespace Player {
	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool window_flag;
	bool battle_test_flag;
	int battle_test_troop_id;
	bool new_game_flag;
	int party_x_position;
	int party_y_position;
	int start_map_id;
	bool no_audio_flag;
	EngineType engine;
}

void Player::Init(int argc, char *argv[]) {
	static bool init = false;

	if (init) return;

#ifdef GEKKO
	// Init libfat (Mount SD/USB)
	if (!fatInitDefault()) {
		Output::Error("Couldn't mount any storage medium!");
	}
#endif

	Main_Data::Init();

#if (defined(_WIN32) && defined(NDEBUG) && defined(WINVER) && WINVER >= 0x0600)
	InitMiniDumpWriter();
#endif

	ParseCommandLine(argc, argv);

	engine = EngineRpg2k;

	FileFinder::Init();

	DisplayUi.reset();

	if(! DisplayUi) {
		DisplayUi = BaseUi::CreateUi
			(SCREEN_TARGET_WIDTH,
			 SCREEN_TARGET_HEIGHT,
			 GAME_TITLE,
			 !window_flag,
			 RUN_ZOOM);
	}

	init = true;
}

void Player::Run() {
	Scene::Push(EASYRPG_MAKE_SHARED<Scene>());

	if (Player::debug_flag) {
		// Scene_Logo does setup in non-debug mode
		CreateGameObjects();
		SetupPlayerSpawn();
		Scene::Push(EASYRPG_SHARED_PTR<Scene>(
			static_cast<Scene*>(new Scene_Title())));
		if (Player::new_game_flag) {
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_Map>());
		}
	}
	else {
		Scene::Push(EASYRPG_SHARED_PTR<Scene>(
			static_cast<Scene*>(new Scene_Logo())));
	}

	reset_flag = false;

	// Reset frames before starting
	Graphics::FrameReset();

	// Main loop
	while (Scene::instance->type != Scene::Null) {
		Scene::instance->MainFunction();
		for (size_t i = 0; i < Scene::old_instances.size(); ++i) {
			Graphics::Pop();
		}
		Scene::old_instances.clear();
	}

	Player::Exit();
}

void Player::Pause() {
	Audio().BGM_Pause();
}

void Player::Resume() {
	Input::ResetKeys();
	Audio().BGM_Resume();
	Graphics::FrameReset();
}

void Player::Update() {
	if (Input::IsTriggered(Input::TOGGLE_FPS)) {
		Graphics::fps_on_screen = !Graphics::fps_on_screen;
	}
	if (Input::IsTriggered(Input::TAKE_SCREENSHOT)) {
		Output::TakeScreenshot();
	}

	DisplayUi->ProcessEvents();

	if (exit_flag) {
		Scene::PopUntil(Scene::Null);
	} else if (reset_flag) {
		reset_flag = false;
		if (Scene::instance->type != Scene::Logo) {
			Scene::PopUntil(Scene::Title);
		}
	}
}

void Player::Exit() {
	Main_Data::Cleanup();
	Graphics::Quit();
	FileFinder::Quit();
	DisplayUi.reset();
	
#ifdef __ANDROID__
	// Workaround Segfault under Android
	exit(0);
#endif
}

void Player::ParseCommandLine(int argc, char *argv[]) {
	window_flag = false;
	debug_flag = false;
	hide_title_flag = false;
	exit_flag = false;
	reset_flag = false;
	battle_test_flag = false;
	new_game_flag = false;
	party_x_position = -1;
	party_y_position = -1;
	start_map_id = -1;
	no_audio_flag = false;

	std::vector<std::string> args;

	for (int i = 1; i < argc; ++i) {
		args.push_back(Utils::LowerCase(argv[i]));
	}

	std::vector<std::string>::const_iterator it;

	for (it = args.begin(); it != args.end(); ++it) {
		if (*it == "window" || *it == "--window") {
			window_flag = true;
		}
		else if (*it == "testplay" || *it == "--test-play") {
			debug_flag = true;
		}
		else if (*it == "hidetitle" || *it == "--hide-title") {
			hide_title_flag = true;
		}
		else if (*it == "battletest") {
			battle_test_flag = true;
			battle_test_troop_id = (argc > 4) ? atoi(argv[4]) : 0;
		}
		else if (*it == "--battle-test") {
			++it;
			if (it != args.end()) {
				battle_test_flag = true;
				battle_test_troop_id = atoi((*it).c_str());
			}
		}
		else if (*it == "--new-game") {
			new_game_flag = true;
		}
		else if (*it == "--map-id") {
			++it;
			if (it != args.end()) {
				start_map_id = atoi((*it).c_str());
			}
		}
		else if (*it == "--position") {
			++it;
			if (it != args.end()) {
				party_x_position = atoi((*it).c_str());
			}
			++it;
			if (it != args.end()) {
				party_y_position = atoi((*it).c_str());
			}
		}
		else if (*it == "--no-audio") {
			no_audio_flag = true;
		}
	}
}

void Player::CreateGameObjects() {
	LoadDatabase();

	static bool init = false;
	if (!init) {
		if (Data::system.ldb_id == 2003) {
			Output::Debug("Switching to Rpg2003 Interpreter");
			Player::engine = Player::EngineRpg2k3;
		}

		FileFinder::InitRtpPaths();
	}
	init = true;

	Main_Data::game_data.Setup();

	Game_System::Init();
	Game_Temp::Init();
	Main_Data::game_screen.reset(new Game_Screen());
	Game_Actors::Init();
	Game_Party::Init();
	Game_Message::Init();
	Game_Map::Init();
	Main_Data::game_player.reset(new Game_Player());

	Graphics::FrameReset();
}

void Player::LoadDatabase() {
	// Load Database
	Data::Clear();

	if(! FileFinder::IsRPG2kProject(FileFinder::GetProjectTree())) {
		Output::Debug("%s is not an RPG2k project", Main_Data::project_path.c_str());
	}

	if (!LDB_Reader::Load(FileFinder::FindDefault(DATABASE_NAME),
		ReaderUtil::GetEncoding(FileFinder::FindDefault(INI_NAME)))) {
		Output::ErrorStr(LcfReader::GetError());
	}
	if (!LMT_Reader::Load(FileFinder::FindDefault(TREEMAP_NAME),
		ReaderUtil::GetEncoding(FileFinder::FindDefault(INI_NAME)))) {
		Output::ErrorStr(LcfReader::GetError());
	}
}

void Player::SetupPlayerSpawn() {
	int map_id = Player::start_map_id == -1 ?
		Data::treemap.start.party_map_id : Player::start_map_id;
	int x_pos = Player::party_x_position == -1 ?
		Data::treemap.start.party_x : Player::party_x_position;
	int y_pos = Player::party_y_position == -1 ?
		Data::treemap.start.party_y : Player::party_y_position;

	Game_Map::Setup(map_id);
	Main_Data::game_player->MoveTo(x_pos, y_pos);
	Main_Data::game_player->Refresh();
	Game_Map::Autoplay();
}

#if (defined(_WIN32) && defined(NDEBUG) && defined(WINVER) && WINVER >= 0x0600)
// Minidump code for Windows
// Original Author: Oleg Starodumov (www.debuginfo.com)
// Modified by EasyRPG Team
typedef BOOL (__stdcall *MiniDumpWriteDumpFunc) (
	IN HANDLE hProcess,
	IN DWORD ProcessId,
	IN HANDLE hFile,
	IN MINIDUMP_TYPE DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
	IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
	IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
);

static WCHAR szModulName[_MAX_FNAME];
static MiniDumpWriteDumpFunc TheMiniDumpWriteDumpFunc;

static BOOL CALLBACK MyMiniDumpCallback(PVOID,
	const PMINIDUMP_CALLBACK_INPUT pInput,
	PMINIDUMP_CALLBACK_OUTPUT pOutput
) {
	if (pInput == 0 || pOutput == 0)  {
		return false;
	}

	switch (pInput->CallbackType)
	{
		case IncludeModuleCallback:
		case IncludeThreadCallback:
		case ThreadCallback:
		case ThreadExCallback:
			return true;
		case MemoryCallback:
		case CancelCallback:
			return false;
		case ModuleCallback:
			// Are data sections available for this module?
			if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg) {
				// Exclude all modules but the player itself
				if (pInput->Module.FullPath == NULL ||
					wcsicmp(pInput->Module.FullPath, szModulName)) {
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
				}
			}
			return true;
	}

	return false;
}

static LONG __stdcall CreateMiniDump(EXCEPTION_POINTERS* pep)
{
	wchar_t szDumpName[40];

	// Get the current time
	SYSTEMTIME time;
	GetLocalTime(&time);

	// Player-YYYY-MM-DD-hh-mm-ss.dmp
#ifdef __MINGW32__
	swprintf(szDumpName, L"Player_%04d-%02d-%02d-%02d-%02d-%02d.dmp",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond);
#else
	swprintf(szDumpName, 40, L"Player_%04d-%02d-%02d-%02d-%02d-%02d.dmp",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond);
#endif

	HANDLE hFile = CreateFile(szDumpName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId           = GetCurrentThreadId();
		mdei.ExceptionPointers  = pep;
		mdei.ClientPointers     = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;
		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
		mci.CallbackParam       = 0;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
									MiniDumpWithDataSegs | MiniDumpWithHandleData |
									MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo |
									MiniDumpWithUnloadedModules );

		TheMiniDumpWriteDumpFunc(GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci);

		// Enable NTFS compression to save a lot of disk space
		DWORD res;
		DWORD format = COMPRESSION_FORMAT_DEFAULT;
		DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &format, sizeof(USHORT), NULL, 0, &res, NULL);

		CloseHandle(hFile);
	}

	// Pass to the Windows crash handler
	return EXCEPTION_CONTINUE_SEARCH;
}

static void InitMiniDumpWriter()
{
	// Prepare the Functions, when their is an exception this could fail so
	// we do this when the application is still in a clean state
	static HMODULE dbgHelp = LoadLibrary(L"dbghelp.dll");
	if (dbgHelp != NULL) {
		TheMiniDumpWriteDumpFunc = (MiniDumpWriteDumpFunc) GetProcAddress(dbgHelp, "MiniDumpWriteDump");

		if (TheMiniDumpWriteDumpFunc != NULL) {
			SetUnhandledExceptionFilter(CreateMiniDump);

			// Extract the module name
			GetModuleFileName(NULL, szModulName, _MAX_FNAME);
		}
	}
}


#endif
