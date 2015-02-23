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
#include "audio.h"
#include "cache.h"
#include "filefinder.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_message.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_variables.h"
#include "graphics.h"
#include "inireader.h"
#include "input.h"
#include "ldb_reader.h"
#include "lmt_reader.h"
#include "lsd_reader.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "reader_lcf.h"
#include "reader_util.h"
#include "scene_battle.h"
#include "scene_logo.h"
#include "scene_map.h"
#include "scene_title.h"
#include "system.h"
#include "utils.h"

#include <algorithm>
#include <set>
#include <locale>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef GEKKO
	#include <fat.h>
#endif
#if (defined(_WIN32) && defined(NDEBUG))
	#include <windows.h>
	#include <winioctl.h>
	#include <dbghelp.h>
	static void InitMiniDumpWriter();
#endif
#ifdef EMSCRIPTEN
	#include <emscripten.h>
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
	int load_game_id;
	int party_x_position;
	int party_y_position;
	std::vector<int> party_members;
	int start_map_id;
	bool no_rtp_flag;
	bool no_audio_flag;
	std::string encoding;
	std::string escape_symbol;
	EngineType engine;
	std::string game_title;
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

#if (defined(_WIN32) && defined(NDEBUG) && defined(WINVER) && WINVER >= 0x0600)
	InitMiniDumpWriter();
#endif

#ifdef EMSCRIPTEN
	Output::IgnorePause(true);
	
	emscripten_set_canvas_size(SCREEN_TARGET_WIDTH * 2, SCREEN_TARGET_HEIGHT * 2);

	// Retrieve save directory from persistent storage
	EM_ASM(
		FS.mkdir('/Save');
		FS.mount(IDBFS, {}, '/Save');
	
		FS.syncfs(true, function(err) {
		});
	);
#endif

	srand(time(NULL));

	ParseCommandLine(argc, argv);

	if (Main_Data::project_path.empty()) {
		// Not overwritten by --project-path
		Main_Data::Init();
	}

	FileFinder::Init();

	DisplayUi.reset();

	if(! DisplayUi) {
		DisplayUi = BaseUi::CreateUi
			(SCREEN_TARGET_WIDTH,
			 SCREEN_TARGET_HEIGHT,
			 game_title,
			 !window_flag,
			 RUN_ZOOM);
	}

	init = true;
}

void Player::Run() {
	Scene::Push(EASYRPG_MAKE_SHARED<Scene>());

	Scene::Push(EASYRPG_SHARED_PTR<Scene>(static_cast<Scene*>(new Scene_Logo())));

	reset_flag = false;

	// Reset frames before starting
	Graphics::FrameReset();

	// Main loop
#ifdef EMSCRIPTEN
	emscripten_set_main_loop(Player::MainLoop, 0, 0);
#else
	while (Graphics::IsTransitionPending() || Scene::instance->type != Scene::Null)
		Player::MainLoop();
#endif
}

void Player::MainLoop() {
	Scene::instance->MainFunction();
	for (size_t i = 0; i < Scene::old_instances.size(); ++i) {
		Graphics::Pop();
	}
	Scene::old_instances.clear();

	if (!Graphics::IsTransitionPending() && Scene::instance->type == Scene::Null) {
		Player::Exit();
	}
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
	if (Input::IsTriggered(Input::SHOW_LOG)) {
		Output::ToggleLog();
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
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();

	BitmapRef surface = DisplayUi->GetDisplaySurface();
	std::string error = "You can turn off your browser now.";

	Text::Draw(*surface, 55, DisplayUi->GetHeight() / 2 - 6, Color(255, 255, 255, 255), error);
	DisplayUi->UpdateDisplay();
#endif

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
	engine = EngineNone;
	window_flag = false;
	debug_flag = false;
	hide_title_flag = false;
	exit_flag = false;
	reset_flag = false;
	battle_test_flag = false;
	battle_test_troop_id = 0;
	new_game_flag = false;
	load_game_id = -1;
	party_x_position = -1;
	party_y_position = -1;
	start_map_id = -1;
	no_rtp_flag = false;
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
			++it;
			if (it == args.end()) {
				return;
			}
			battle_test_flag = true;
			battle_test_troop_id = atoi((*it).c_str());
			if (battle_test_troop_id == 0) {
				--it;
				battle_test_troop_id = (argc > 4) ? atoi(argv[4]) : 0;
			}
		}
		else if (*it == "--battle-test") {
			++it;
			if (it == args.end()) {
				return;
			}
			battle_test_flag = true;
			battle_test_troop_id = atoi((*it).c_str());
		}
		else if (*it == "--project-path") {
			++it;
			if (it == args.end()) {
				return;
			}
			// case sensitive
			Main_Data::project_path = argv[it - args.begin() + 1];
		}
		else if (*it == "--new-game") {
			new_game_flag = true;
		}
		else if (*it == "--load-game-id") {
			++it;
			if (it == args.end()) {
				return;
			}
			load_game_id = atoi((*it).c_str());
		}
		/*else if (*it == "--load-game") {
			// load game by filename
		}
		else if (*it == "--database") {
			// overwrite database file
		}
		else if (*it == "--map-tree") {
			// overwrite map tree file
		}
		else if (*it == "--start-map") {
			// overwrite start map by filename
		}*/
		else if (*it == "--seed") {
			++it;
			if (it == args.end()) {
				return;
			}
			srand(atoi((*it).c_str()));
		}
		else if (*it == "--start-map-id") {
			++it;
			if (it == args.end()) {
				return;
			}
			start_map_id = atoi((*it).c_str());
		}
		else if (*it == "--start-position") {
			++it;
			if (it == args.end() || it == args.end()-1) {
				return;
			}
			party_x_position = atoi((*it).c_str());
			++it;
			party_y_position = atoi((*it).c_str());
		}
		else if (*it == "--start-party") {
			while (++it != args.end() && isdigit((*it)[0])) {
				party_members.push_back(atoi((*it).c_str()));
			}
			--it;
		}
		else if (*it == "--engine") {
			++it;
			if (it == args.end()) {
				return;
			}
			if (*it == "rpg2k" || *it == "2000") {
				engine = EngineRpg2k;
			}
			else if (*it == "rpg2k3" || *it == "2003") {
				engine = EngineRpg2k3;
			}
		}
		else if (*it == "--encoding") {
			++it;
			if (it == args.end()) {
				return;
			}
			encoding = *it;
		}
		else if (*it == "--disable-audio") {
			no_audio_flag = true;
		}
		else if (*it == "--disable-rtp") {
			no_rtp_flag = true;
		}
		else if (*it == "--version" || *it == "-v") {
			PrintVersion();
			exit(0);
		}
		else if (*it == "--help" || *it == "-h" || *it == "/?") {
			PrintUsage();
			exit(0);
		}
	}
}

void Player::CreateGameObjects() {
	static bool init = false;
	if (!init) {
		Player::GetEncoding();
		escape_symbol = ReaderUtil::Recode("\\", encoding);
		if (escape_symbol.empty()) {
			Output::Error("Invalid encoding: %s.", encoding.c_str());
		}

		Player::LoadDatabase();

		INIReader ini(FileFinder::FindDefault(INI_NAME));
		if (ini.ParseError() != -1) {
			std::string title = ini.Get("RPG_RT", "GameTitle", GAME_TITLE);
			game_title = ReaderUtil::Recode(title, encoding);
			no_rtp_flag = ini.Get("RPG_RT", "FullPackageFlag", "0") == "1"? true : no_rtp_flag;
		}

		if (Player::engine == EngineNone) {
			if (Data::system.ldb_id == 2003) {
				Output::Debug("Switching to Rpg2003 Interpreter");
				Player::engine = Player::EngineRpg2k3;
			}
			else {
				Player::engine = Player::EngineRpg2k;
			}
		}

		if (!no_rtp_flag) {
			FileFinder::InitRtpPaths();
		}
	}
	init = true;

	Main_Data::game_data.Setup();

	Game_Actors::Init();
	Game_Map::Init();
	Game_Message::Init();
	Game_Switches.Reset();
	Game_System::Init();
	Game_Temp::Init();
	Game_Variables.Reset();
	Main_Data::game_enemyparty.reset(new Game_EnemyParty());
	Main_Data::game_party.reset(new Game_Party());
	Main_Data::game_player.reset(new Game_Player());
	Main_Data::game_screen.reset(new Game_Screen());

	Graphics::FrameReset();
}

void Player::LoadDatabase() {
	// Load Database
	Data::Clear();

	if (!FileFinder::IsRPG2kProject(FileFinder::GetProjectTree()) &&
		!FileFinder::IsEasyRpgProject(FileFinder::GetProjectTree())) {
		Output::Debug("%s is not a supported project", Main_Data::project_path.c_str());
	}

	// Try loading EasyRPG project files first, then fallback to normal RPG Maker
	std::string edb = FileFinder::FindDefault(DATABASE_NAME_EASYRPG);
	std::string emt = FileFinder::FindDefault(TREEMAP_NAME_EASYRPG);

	bool easyrpg_project = !edb.empty() && !emt.empty();

	if (easyrpg_project) {
		if (!LDB_Reader::LoadXml(edb)) {
			Output::ErrorStr(LcfReader::GetError());
		}
		if (!LMT_Reader::LoadXml(emt)) {
			Output::ErrorStr(LcfReader::GetError());
		}
	}
	else {
		std::string ldb = FileFinder::FindDefault(DATABASE_NAME);
		std::string lmt = FileFinder::FindDefault(TREEMAP_NAME);

		if (!LDB_Reader::Load(ldb, encoding)) {
			Output::ErrorStr(LcfReader::GetError());
		}
		if (!LMT_Reader::Load(lmt, encoding)) {
			Output::ErrorStr(LcfReader::GetError());
		}
	}
}

void Player::LoadSavegame(const std::string& save_name) {
	std::auto_ptr<RPG::Save> save = LSD_Reader::Load(save_name, encoding);

	if (!save.get()) {
		Output::Error("%s", LcfReader::GetError().c_str());
	}

	RPG::SaveSystem system = Main_Data::game_data.system;

	Main_Data::game_data = *save.get();

	Main_Data::game_data.party_location.Fixup();
	Main_Data::game_data.system.Fixup();
	Main_Data::game_data.screen.Fixup();
	Game_Actors::Fixup();

	Game_Map::SetupFromSave();

	Main_Data::game_player->MoveTo(
		save->party_location.position_x, save->party_location.position_y);
	Main_Data::game_player->Refresh();

	RPG::Music current_music = Main_Data::game_data.system.current_music;
	Game_System::BgmStop();
	Game_System::BgmPlay(current_music);
}

void Player::SetupPlayerSpawn() {
	int map_id = Player::start_map_id == -1 ?
		Data::treemap.start.party_map_id : Player::start_map_id;
	int x_pos = Player::party_x_position == -1 ?
		Data::treemap.start.party_x : Player::party_x_position;
	int y_pos = Player::party_y_position == -1 ?
		Data::treemap.start.party_y : Player::party_y_position;
	if (party_members.size() > 0) {
		Main_Data::game_party->Clear();
		std::vector<int>::iterator member;
		for (member = party_members.begin(); member != party_members.end(); ++member) {
			Main_Data::game_party->AddActor(*member);
		}
	}

	Game_Map::Setup(map_id);
	Main_Data::game_player->MoveTo(x_pos, y_pos);
	Main_Data::game_player->Refresh();
	Game_Map::PlayBgm();
}

std::string Player::GetEncoding() {
	if (encoding.empty()) {
		encoding = ReaderUtil::GetEncoding(FileFinder::FindDefault(INI_NAME));
	} else {
		return encoding;
	}
	if (encoding.empty()) {
		encoding = ReaderUtil::DetectEncoding(FileFinder::FindDefault(DATABASE_NAME));
	} else {
		return encoding;
	}
	if (encoding.empty()) {
		Output::Debug("Encoding not detected");
		encoding = ReaderUtil::GetLocaleEncoding();
	} else {
		Output::Debug("Detected encoding: %s", encoding.c_str());
	}

	return encoding;
}

void Player::PrintVersion() {
	std::cout << "EasyRPG Player " << PLAYER_VERSION << std::endl;
}

void Player::PrintUsage() {
	std::cout << "EasyRPG Player - An open source interpreter for RPG Maker 2000/2003 games." << std::endl << std::endl;

	std::cout << "Options:" << std::endl;
	//                                                  "                                Line end marker -> "
	std::cout << "      " << "--battle-test N      " << "Start a battle test with monster party N." << std::endl;

	std::cout << "      " << "--disable-audio      " << "Disable audio (in case you prefer your own music)." << std::endl;

	std::cout << "      " << "--disable-rtp        " << "Disable support for the Runtime Package (RTP)." << std::endl;

	std::cout << "      " << "--encoding N         " << "Instead of using the default platform encoding or" << std::endl;
	std::cout << "      " << "                     " << "the one in RPG_RT.ini the encoding N is used." << std::endl;

	std::cout << "      " << "--engine ENGINE      " << "Disable auto detection of the simulated engine." << std::endl;
	std::cout << "      " << "                     " << "Possible options:" << std::endl;
	std::cout << "      " << "                     " << " rpg2k  - RPG Maker 2000 engine" << std::endl;
	std::cout << "      " << "                     " << " rpg2k3 - RPG Maker 2003 engine" << std::endl;

	std::cout << "      " << "--fullscreen         " << "Start in fullscreen mode." << std::endl;

	std::cout << "      " << "--hide-title         " << "Hide the title background image and center the" << std::endl;
	std::cout << "      " << "                     " << "command menu." << std::endl;

	std::cout << "      " << "--load-game-id N     " << "Skip the title scene and load SaveN.lsd" << std::endl;
	std::cout << "      " << "                     " << "(N is padded to two digits)." << std::endl;

	std::cout << "      " << "--new-game           " << "Skip the title scene and start a new game directly." << std::endl;

	std::cout << "      " << "--project-path PATH  " << "Instead of using the working directory the game in" << std::endl;
	std::cout << "      " << "                     " << "PATH is used." << std::endl;

	std::cout << "      " << "--seed N            " << "Seeds the random number generator with N." << std::endl;

	std::cout << "      " << "--start-map-id N     " << "Overwrite the map used for new games and use." << std::endl;
	std::cout << "      " << "                     " << "MapN.lmu instead (N is padded to four digits)." << std::endl;
	std::cout << "      " << "                     " << "Incompatible with --load-game-id." << std::endl;

	std::cout << "      " << "--start-position X Y " << "Overwrite the party start position and move the" << std::endl;
	std::cout << "      " << "                     " << "party to position (X, Y)." << std::endl;
	std::cout << "      " << "                     " << "Incompatible with --load-game-id." << std::endl;

	std::cout << "      " << "--start-party A B... " << "Overwrite the starting party members with the actors" << std::endl;
	std::cout << "      " << "                     " << "with IDs A, B, C..." << std::endl;
	std::cout << "      " << "                     " << "Incompatible with --load-game-id." << std::endl;

	std::cout << "      " << "--test-play          " << "Enable TestPlay mode." << std::endl;

	std::cout << "      " << "--window             " << "Start in window mode." << std::endl;

	std::cout << "  -v, " << "--version            " << "Display program version and exit." << std::endl;

	std::cout << "  -h, " << "--help               " << "Display this help and exit." << std::endl << std::endl;

	std::cout << "For compatibility with the legacy RPG Maker runtime the following arguments" << std::endl;
	std::cout << "are supported:" << std::endl;
	std::cout << "      " << "BattleTest N         " << "Same as --battle-test. When N is not a valid number" << std::endl;
	std::cout << "      " << "                     " << "the 4th argument is used as the party id." << std::endl;
	std::cout << "      " << "HideTitle            " << "Same as --hide-title." << std::endl;
	std::cout << "      " << "TestPlay             " << "Same as --test-play." << std::endl << std::endl;

	std::cout << "Alex, EV0001 and the EasyRPG authors wish you a lot of fun!" << std::endl;
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
