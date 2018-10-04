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

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#  include <Shellapi.h>
#  ifndef _DEBUG
#    include <winioctl.h>
#    include <dbghelp.h>
     static void InitMiniDumpWriter();
#  endif
#elif defined(GEKKO)
#  include <fat.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
#elif defined(PSP2)
#  include <psp2/kernel/processmgr.h>
#elif defined(_3DS)
#  include <3ds.h>
#elif defined(SWITCH)
#  include <switch.h>
#endif

#include "async_handler.h"
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
#include "utils.h"
#include "version.h"

namespace Player {
	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool window_flag;
	bool fps_flag;
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
	bool mouse_flag;
	bool touch_flag;
	std::string encoding;
	std::string escape_symbol;
	int engine;
	std::string game_title;
	int frames;
	std::string replay_input_path;
	std::string record_input_path;
	int speed_modifier = 3;
#ifdef EMSCRIPTEN
	std::string emscripten_game_name;
#endif
#ifdef _3DS
	bool is_3dsx;
#endif
}

namespace {
	double start_time;
	double next_frame;

	// Overwritten by --encoding
	std::string forced_encoding;

	FileRequestBinding system_request_id;
	FileRequestBinding save_request_id;
	FileRequestBinding map_request_id;
}

void Player::Init(int argc, char *argv[]) {
	frames = 0;

	// Must be called before the first call to Output
	Graphics::Init();

	// Display a nice version string
	std::stringstream header;
	std::string addtl_ver(PLAYER_ADDTL);
	header << "EasyRPG Player " << PLAYER_VERSION;
	if (!addtl_ver.empty())
		header << " " << addtl_ver;
	header << " started";
	Output::Debug(header.str().c_str());

	unsigned int header_width = header.str().length();
	header.str("");
	header << std::setfill('=') << std::setw(header_width) << "=";
	Output::Debug(header.str().c_str());

#ifdef GEKKO
	// Init libfat (Mount SD/USB)
	if (!fatInitDefault()) {
		Output::Error("Couldn't mount any storage medium!");
	}
#elif defined(_3DS)
	romfsInit();
#endif

#if (defined(_WIN32) && defined(NDEBUG) && defined(WINVER) && WINVER >= 0x0600)
	InitMiniDumpWriter();
#endif

	Utils::SeedRandomNumberGenerator(time(NULL));

	ParseCommandLine(argc, argv);

#ifdef EMSCRIPTEN
	Output::IgnorePause(true);

	// Create initial directory structure in our private area
	// Retrieve save directory from persistent storage
	EM_ASM(

		FS.mkdir("easyrpg");
		FS.chdir("easyrpg");

		var dirs = ['Backdrop', 'Battle', 'Battle2', 'BattleCharSet', 'BattleWeapon', 'CharSet', 'ChipSet', 'FaceSet', 'Frame', 'GameOver', 'Monster', 'Movie', 'Music', 'Panorama', 'Picture', 'Sound', 'System', 'System2', 'Title', 'Save'];
		dirs.forEach(function(dir) { FS.mkdir(dir) });

		FS.mount(Module.EASYRPG_FS, {}, 'Save');

		FS.syncfs(true, function(err) {
		});
	);
#endif

	Main_Data::Init();

	DisplayUi.reset();

	if(! DisplayUi) {
		DisplayUi = BaseUi::CreateUi
			(SCREEN_TARGET_WIDTH,
			 SCREEN_TARGET_HEIGHT,
			 !window_flag,
			 RUN_ZOOM);
	}

	Input::Init(replay_input_path, record_input_path);
}

void Player::Run() {
	Scene::Push(std::shared_ptr<Scene>(static_cast<Scene*>(new Scene_Logo())));
	Graphics::UpdateSceneCallback();

	reset_flag = false;

	// Reset frames before starting
	FrameReset();

	// Main loop
#ifdef EMSCRIPTEN
	emscripten_set_main_loop(Player::MainLoop, 0, 0);
#elif defined(_3DS)
	while (aptMainLoop() && (Graphics::IsTransitionPending() || Scene::instance->type != Scene::Null))
	{
		hidScanInput();
		Player::MainLoop();
	}
#elif defined(SWITCH)
	while (appletMainLoop() && (Graphics::IsTransitionPending() || Scene::instance->type != Scene::Null))
		MainLoop();
#else
	while (Graphics::IsTransitionPending() || Scene::instance->type != Scene::Null)
		MainLoop();
#endif
}

void Player::MainLoop() {
	Scene::instance->MainFunction();

	Scene::old_instances.clear();

	if (!Graphics::IsTransitionPending() && Scene::instance->type == Scene::Null) {
		Exit();
	}
}

void Player::Pause() {
	Audio().BGM_Pause();
}

void Player::Resume() {
	Input::ResetKeys();
	Audio().BGM_Resume();
	FrameReset();
}

void Player::Update(bool update_scene) {
	// available ms per frame, game logic expects 60 fps
	static const double framerate_interval = 1000.0 / Graphics::GetDefaultFps();
	next_frame = start_time + framerate_interval;

#ifdef EMSCRIPTEN
	// Ticks in emscripten are unreliable due to how the main loop works:
	// This function is only called 60 times per second instead of theoretical
	// 1000s of times.
	Graphics::Draw();
#else
	double cur_time = (double)DisplayUi->GetTicks();
	if (cur_time < next_frame) {
		Graphics::Draw();
		cur_time = (double)DisplayUi->GetTicks();
		// Still time after graphic update? Yield until it's time for next one.
		if (cur_time < next_frame) {
			DisplayUi->Sleep((uint32_t)(next_frame - cur_time));
		}
	}
#endif

	// Input Logic:
	if (Input::IsTriggered(Input::TOGGLE_FPS)) {
		fps_flag = !fps_flag;
	}
	if (Input::IsTriggered(Input::TAKE_SCREENSHOT)) {
		Output::TakeScreenshot();
	}
	if (Input::IsTriggered(Input::SHOW_LOG)) {
		Output::ToggleLog();
	}
	if (Input::IsTriggered(Input::RESET)) {
		reset_flag = true;
	}
	if (Input::IsTriggered(Input::TOGGLE_ZOOM)) {
		DisplayUi->ToggleZoom();
	}
	if (Input::IsTriggered(Input::TOGGLE_FULLSCREEN)) {
		DisplayUi->ToggleFullscreen();
	}

	// Update Logic:
	DisplayUi->ProcessEvents();

	if (exit_flag) {
		Scene::PopUntil(Scene::Null);
	} else if (reset_flag) {
		reset_flag = false;
		if (Scene::Find(Scene::Title) && Scene::instance->type != Scene::Title) {
			Game_Temp::restart_title_cache = true;
			Scene::PopUntil(Scene::Title);
			// Fade out music and stop sound effects before returning
			Game_System::BgmFade(800);
			Audio().SE_Stop();
			// Do not update this scene until it's properly set up in the next main loop
			update_scene = false;
		}
	}

	Audio().Update();
	Input::Update();

	std::shared_ptr<Scene> old_instance = Scene::instance;

	int speed_modifier = GetSpeedModifier();

	for (int i = 0; i < speed_modifier; ++i) {
		Graphics::Update();
		if (update_scene) {
			Scene::instance->Update();
			++frames;

			// Scene changed or webplayer waits for files.
			// Not save to Update again, setup code must run:
			if (&*old_instance != &*Scene::instance || AsyncHandler::IsImportantFilePending()) {
				break;
			}
		}
	}

	start_time = next_frame;
}

void Player::FrameReset() {
	// When update started
	start_time = (double)DisplayUi->GetTicks();

	// available ms per frame, game logic expects 60 fps
	static const double framerate_interval = 1000.0 / Graphics::GetDefaultFps();

	// When next frame is expected
	next_frame = start_time + framerate_interval;

	Graphics::FrameReset();
}

int Player::GetFrames() {
	return frames;
}

void Player::Exit() {
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();

	BitmapRef surface = DisplayUi->GetDisplaySurface();
	std::string message = "It's now safe to turn off\n      your browser.";

	Text::Draw(*surface, 84, DisplayUi->GetHeight() / 2 - 30, Color(221, 123, 64, 255), Font::Default(), message);
	DisplayUi->UpdateDisplay();
#endif

	Player::ResetGameObjects();
	Font::Dispose();
	Graphics::Quit();
	FileFinder::Quit();
	Output::Quit();
	DisplayUi.reset();

#ifdef PSP2
	sceKernelExitProcess(0);
#elif defined(_3DS)
	romfsExit();
#endif
}

void Player::ParseCommandLine(int argc, char *argv[]) {
#ifdef _3DS
	is_3dsx = argc > 0;
#endif
#if defined(_WIN32) && !defined(__WINRT__)
	LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);
#endif

	engine = EngineNone;
#ifdef EMSCRIPTEN
	window_flag = true;
#else
	window_flag = false;
#endif
	fps_flag = false;
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
	mouse_flag = false;
	touch_flag = false;

	std::vector<std::string> args;

	std::stringstream ss;
	for (int i = 1; i < argc; ++i) {
		ss << argv[i] << " ";
#if defined(_WIN32) && !defined(__WINRT__)
		args.push_back(Utils::LowerCase(Utils::FromWideString(argv_w[i])));
#else
		args.push_back(Utils::LowerCase(argv[i]));
#endif
	}
	Output::Debug("CLI: %s", ss.str().c_str());

	std::vector<std::string>::const_iterator it;

	for (it = args.begin(); it != args.end(); ++it) {
		ss << *it << " ";
	}

	for (it = args.begin(); it != args.end(); ++it) {
		if (*it == "window" || *it == "--window") {
			window_flag = true;
		}
		else if (*it == "--show-fps") {
			fps_flag = true;
		}
		else if (*it == "--enable-mouse") {
			mouse_flag = true;
		}
		else if (*it == "--enable-touch") {
			touch_flag = true;
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
#ifdef _WIN32
			Main_Data::SetProjectPath(*it);
			BOOL cur_dir = SetCurrentDirectory(Utils::ToWideString(Main_Data::GetProjectPath()).c_str());
			if (cur_dir) {
				Main_Data::SetProjectPath(".");
			}
#else
			// case sensitive
			Main_Data::SetProjectPath(argv[it - args.begin() + 1]);
#endif
		}
		else if (*it == "--save-path") {
			++it;
			if (it == args.end()) {
				return;
			}
			// case sensitive
			Main_Data::SetSavePath(argv[it - args.begin() + 1]);
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
			Utils::SeedRandomNumberGenerator(atoi((*it).c_str()));
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
			else if (*it == "rpg2kv150" || *it == "2000v150") {
				engine = EngineRpg2k | EngineMajorUpdated;
			}
			else if (*it == "rpg2ke" || *it == "2000e") {
				engine = EngineRpg2k | EngineMajorUpdated | EngineEnglish;
			}
			else if (*it == "rpg2k3" || *it == "2003") {
				engine = EngineRpg2k3;
			}
			else if (*it == "rpg2k3v105" || *it == "2003v105") {
				engine = EngineRpg2k3 | EngineMajorUpdated;
			}
			else if (*it == "rpg2k3e") {
				engine = EngineRpg2k3 | EngineMajorUpdated | EngineEnglish;
			}
		}
		else if (*it == "--record-input") {
			++it;
			if (it == args.end()) {
				return;
			}
			record_input_path = *it;
		}
		else if (*it == "--replay-input") {
			++it;
			if (it == args.end()) {
				return;
			}
			replay_input_path = *it;
		}
		else if (*it == "--encoding") {
			++it;
			if (it == args.end()) {
				return;
			}
			forced_encoding = *it;
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
#ifdef EMSCRIPTEN
		else if (*it == "--game") {
			++it;
			if (it == args.end()) {
				return;
			}
			emscripten_game_name = *it;
		}
#endif
	}
}

static void OnSystemFileReady(FileRequestResult* result) {
	Game_System::SetSystemName(result->file);
}

void Player::CreateGameObjects() {
	GetEncoding();
	escape_symbol = ReaderUtil::Recode("\\", encoding);
	if (escape_symbol.empty()) {
		Output::Error("Invalid encoding: %s.", encoding.c_str());
	}

	std::string game_path = Main_Data::GetProjectPath();
	std::string save_path = Main_Data::GetSavePath();
	if (game_path == save_path) {
		Output::Debug("Using %s as Game and Save directory", game_path.c_str());
	} else {
		Output::Debug("Using %s as Game directory", game_path.c_str());
		Output::Debug("Using %s as Save directory", save_path.c_str());
	}

	LoadDatabase();

	std::string ini_file = FileFinder::FindDefault(INI_NAME);

	INIReader ini(ini_file);
	if (ini.ParseError() != -1) {
		std::string title = ini.Get("RPG_RT", "GameTitle", GAME_TITLE);
		game_title = ReaderUtil::Recode(title, encoding);
		no_rtp_flag = ini.Get("RPG_RT", "FullPackageFlag", "0") == "1"? true : no_rtp_flag;
	}

	std::stringstream title;
	if (!game_title.empty()) {
		Output::Debug("Loading game %s", game_title.c_str());
		title << game_title << " - ";
	} else {
		Output::Warning("Could not read game title.");
	}
	title << GAME_TITLE;
	DisplayUi->SetTitle(title.str());

	if (engine == EngineNone) {
		if (Data::system.ldb_id == 2003) {
			engine = EngineRpg2k3;

			if (FileFinder::FindDefault("ultimate_rt_eb.dll").empty()) {
				// Heuristic: Detect if game was converted from 2000 to 2003 and
				// no typical 2003 feature was used at all (breaks .flow e.g.)
				if (Data::classes.size() == 1 &&
					Data::classes[0].name.empty() &&
					Data::system.menu_commands.empty() &&
					Data::system.system2_name.empty() &&
					Data::battleranimations.size() == 1 &&
					Data::battleranimations[0].name.empty()) {
					engine = EngineRpg2k;
					Output::Debug("Using RPG2k Interpreter (heuristic)");
				} else {
					Output::Debug("Using RPG2k3 Interpreter");
				}
			} else {
				engine |= EngineEnglish;
				Output::Debug("Using RPG2k3 (English release, v1.11) Interpreter");
			}
		} else {
			engine = EngineRpg2k;
			Output::Debug("Using RPG2k Interpreter");
			if (Data::data.version >= 1) {
				engine |= EngineEnglish | EngineMajorUpdated;
				Output::Debug("RM2k >= v.1.61 (English release) detected");
			}
		}
		if (!(engine & EngineMajorUpdated)) {
			if (FileFinder::IsMajorUpdatedTree()) {
				engine |= EngineMajorUpdated;
				Output::Debug("RPG2k >= v1.50 / RPG2k3 >= v1.05 detected");
			} else {
				Output::Debug("RPG2k < v1.50 / RPG2k3 < v1.05 detected");
			}
		}
	}
	Output::Debug("Engine configured as: 2k=%d 2k3=%d 2k3Legacy=%d MajorUpdated=%d Eng=%d", Player::IsRPG2k(), Player::IsRPG2k3(), Player::IsRPG2k3Legacy(), Player::IsMajorUpdatedVersion(), Player::IsEnglish());

	if (!no_rtp_flag) {
		FileFinder::InitRtpPaths();
	}

	ResetGameObjects();
}

void Player::ResetGameObjects() {
	if (Data::system.system_name != Game_System::GetSystemName()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("System", Data::system.system_name);
		request->SetImportantFile(true);
		system_request_id = request->Bind(&OnSystemFileReady);
		request->Start();
	}

	// The init order is important
	Main_Data::Cleanup();

	Main_Data::game_data.Setup();
	// Prevent a crash when Game_Map wants to reset the screen content
	// because Setup() modified pictures array
	Main_Data::game_screen.reset(new Game_Screen());

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

	FrameReset();
}

void Player::LoadDatabase() {
	// Load Database
	Data::Clear();

	if (!FileFinder::IsRPG2kProject(*FileFinder::GetDirectoryTree()) &&
		!FileFinder::IsEasyRpgProject(*FileFinder::GetDirectoryTree())) {
		// Unlikely to happen because of the game browser only launches valid games

		Output::Debug("%s is not a supported project", Main_Data::GetProjectPath().c_str());

		Output::Error("%s\n\n%s\n\n%s\n\n%s","No valid game was found.",
			"EasyRPG must be run from a game folder containing\nRPG_RT.ldb and RPG_RT.lmt.",
			"This engine only supports RPG Maker 2000 and 2003\ngames.",
			"RPG Maker XP, VX, VX Ace and MV are NOT supported.");
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

static void OnMapSaveFileReady(FileRequestResult*) {
	Game_Map::SetupFromSave();

	Main_Data::game_player->MoveTo(
		Main_Data::game_data.party_location.position_x,
		Main_Data::game_data.party_location.position_y
		);
	Main_Data::game_player->Refresh();

	RPG::Music current_music = Main_Data::game_data.system.current_music;
	Game_System::BgmStop();
	Game_System::BgmPlay(current_music);
}

void Player::LoadSavegame(const std::string& save_name) {
	std::unique_ptr<RPG::Save> save = LSD_Reader::Load(save_name, encoding);

	if (!save.get()) {
		Output::Error("%s", LcfReader::GetError().c_str());
	}

	Main_Data::game_data = *save.get();
	Main_Data::game_data.system.Fixup();

	Game_Actors::Fixup();
	Main_Data::game_party->RemoveInvalidData();

	int map_id = save->party_location.map_id;

	FileRequestAsync* map = Game_Map::RequestMap(map_id);
	save_request_id = map->Bind(&OnMapSaveFileReady);
	map->SetImportantFile(true);

	FileRequestAsync* system = AsyncHandler::RequestFile("System", Game_System::GetSystemName());
	system->SetImportantFile(true);
	system_request_id = system->Bind(&OnSystemFileReady);

	map->Start();
	system->Start();
}

static void OnMapFileReady(FileRequestResult*) {
	int map_id = Player::start_map_id == -1 ?
		Data::treemap.start.party_map_id : Player::start_map_id;
	int x_pos = Player::party_x_position == -1 ?
		Data::treemap.start.party_x : Player::party_x_position;
	int y_pos = Player::party_y_position == -1 ?
		Data::treemap.start.party_y : Player::party_y_position;
	if (Player::party_members.size() > 0) {
		Main_Data::game_party->Clear();
		std::vector<int>::iterator member;
		for (member = Player::party_members.begin(); member != Player::party_members.end(); ++member) {
			Main_Data::game_party->AddActor(*member);
		}
	}

	Game_Map::Setup(map_id);
	Main_Data::game_player->MoveTo(x_pos, y_pos);
	Main_Data::game_player->Refresh();
	Game_Map::PlayBgm();
}

void Player::SetupPlayerSpawn() {
	int map_id = Player::start_map_id == -1 ?
		Data::treemap.start.party_map_id : Player::start_map_id;

	FileRequestAsync* request = Game_Map::RequestMap(map_id);
	map_request_id = request->Bind(&OnMapFileReady);
	request->SetImportantFile(true);
	request->Start();
}

std::string Player::GetEncoding() {
	encoding = forced_encoding;

	// command line > ini > detection > current locale
	if (encoding.empty()) {
		std::string ini = FileFinder::FindDefault(INI_NAME);
		encoding = ReaderUtil::GetEncoding(ini);
	}

	if (encoding.empty() || encoding == "auto") {
		encoding = "";

		std::string ldb = FileFinder::FindDefault(DATABASE_NAME);

		std::vector<std::string> encodings;
		std::ifstream is(ldb, std::ios::binary);
		// Stream required due to a liblcf api change:
		// When a string is passed the encoding of the string is detected
		if (is) {
			encodings = ReaderUtil::DetectEncodings(is);
		}

#ifndef EMSCRIPTEN
		for (std::string& enc : encodings) {
			// Heuristic: Check if encoded title and system name matches the one on the filesystem
			// When yes is a good encoding. Otherwise try the next ones.

			escape_symbol = ReaderUtil::Recode("\\", enc);
			if (escape_symbol.empty()) {
				// Bad encoding
				Output::Debug("Bad encoding: %s. Trying next.", enc.c_str());
				continue;
			}

			if ((Data::system.title_name.empty() ||
					!FileFinder::FindImage("Title", ReaderUtil::Recode(Data::system.title_name, enc)).empty()) &&
				(Data::system.system_name.empty() ||
					!FileFinder::FindImage("System", ReaderUtil::Recode(Data::system.system_name, enc)).empty())) {
				// Looks like a good encoding
				encoding = enc;
				break;
			} else {
				Output::Debug("Detected encoding: %s. Files not found. Trying next.", enc.c_str());
			}
		}
#endif

		if (!encodings.empty() && encoding.empty()) {
			// No encoding found that matches the files, maybe RTP missing.
			// Use the first one instead
			encoding = encodings[0];
		}

		escape_symbol = "";

		if (!encoding.empty()) {
			Output::Debug("Detected encoding: %s", encoding.c_str());
		} else {
			Output::Debug("Encoding not detected");
			encoding = ReaderUtil::GetLocaleEncoding();
		}
	}

	return encoding;
}

int Player::GetSpeedModifier() {
	if (Input::IsPressed(Input::FAST_FORWARD)) {
		return Input::IsPressed(Input::PLUS) ? 10 : speed_modifier;
	}

	return 1;
}

void Player::PrintVersion() {
	std::string additional(PLAYER_ADDTL);
	std::stringstream version;

	version << PLAYER_VERSION;

	if (!additional.empty())
		version << " " << additional;

	std::cout << "EasyRPG Player " << version.str() << std::endl;
}

void Player::PrintUsage() {
	std::cout <<
R"(EasyRPG Player - An open source interpreter for RPG Maker 2000/2003 games.
Options:
      --battle-test N      Start a battle test with monster party N.
      --disable-audio      Disable audio (in case you prefer your own music).
      --disable-rtp        Disable support for the Runtime Package (RTP).
      --encoding N         Instead of auto detecting the encoding or using
                           the one in RPG_RT.ini, the encoding N is used.
                           Use "auto" for automatic detection.
      --engine ENGINE      Disable auto detection of the simulated engine.
                           Possible options:
                            rpg2k      - RPG Maker 2000 engine (v1.00 - v1.10)
                            rpg2kv150  - RPG Maker 2000 engine (v1.50 - v1.51)
                            rpg2ke     - RPG Maker 2000 (English release) engine (v1.61)
                            rpg2k3     - RPG Maker 2003 engine (v1.00 - v1.04)
                            rpg2k3v105 - RPG Maker 2003 engine (v1.05 - v1.09a)
                            rpg2k3e    - RPG Maker 2003 (English release) engine
      --fullscreen         Start in fullscreen mode.
      --show-fps           Enable frames per second counter.
      --enable-mouse       Use mouse click for decision and scroll wheel for lists
      --enable-touch       Use one/two finger tap for decision/cancel
      --hide-title         Hide the title background image and center the
                           command menu.
      --load-game-id N     Skip the title scene and load SaveN.lsd
                           (N is padded to two digits).
      --new-game           Skip the title scene and start a new game directly.
      --project-path PATH  Instead of using the working directory the game in
                           PATH is used.
      --record-input PATH  Record all button input to a log file at PATH.
      --replay-input PATH  Replays button presses from an input log generated by
                           --record-input.
      --save-path PATH     Instead of storing save files in the game directory
                           they are stored in PATH. The directory must exist.
                           When using the game browser all games will share
                           the same save directory!
      --seed N             Seeds the random number generator with N.
      --start-map-id N     Overwrite the map used for new games and use.
                           MapN.lmu instead (N is padded to four digits).
                           Incompatible with --load-game-id.
      --start-position X Y Overwrite the party start position and move the
                           party to position (X, Y).
                           Incompatible with --load-game-id.
      --start-party A B... Overwrite the starting party members with the actors
                           with IDs A, B, C...
                           Incompatible with --load-game-id.
      --test-play          Enable TestPlay mode.
      --window             Start in window mode.
  -v, --version            Display program version and exit.
  -h, --help               Display this help and exit.

For compatibility with the legacy RPG Maker runtime the following arguments
are supported:
      BattleTest N         Same as --battle-test. When N is not a valid number
                           the 4th argument is used as the party id.
      HideTitle            Same as --hide-title.
      TestPlay             Same as --test-play.
      Window               Same as --window.

Game related parameters (e.g. new-game and load-game-id) don't work correctly when the
startup directory does not contain a valid game (and the game browser loads)

Alex, EV0001 and the EasyRPG authors wish you a lot of fun!)" << std::endl;
}

bool Player::IsRPG2k() {
	return (engine & EngineRpg2k) == EngineRpg2k;
}

bool Player::IsRPG2k3Legacy() {
	return (engine == EngineRpg2k3 || engine == (EngineRpg2k3 | EngineMajorUpdated));
}

bool Player::IsRPG2k3() {
	return (engine & EngineRpg2k3) == EngineRpg2k3;
}

bool Player::IsMajorUpdatedVersion() {
	return (engine & EngineMajorUpdated) == EngineMajorUpdated;
}

bool Player::IsRPG2k3E() {
	return ((engine & EngineRpg2k3) == EngineRpg2k3)
		&& ((engine & EngineEnglish) == EngineEnglish);
}

bool Player::IsRPG2kE() {
	return ((engine & EngineRpg2k) == EngineRpg2k)
		&& ((engine & EngineEnglish) == EngineEnglish);
}

bool Player::IsEnglish() {
	return (engine & EngineEnglish) == EngineEnglish;
}

bool Player::IsCP932() {
	return (encoding == "ibm-943_P15A-2003" || encoding == "932");
}

bool Player::IsCP949() {
	return (encoding == "windows-949-2000" ||
			encoding == "949");
}

bool Player::IsBig5() {
	return (encoding == "Big5" || encoding == "950");
}

bool Player::IsCP936() {
	return (encoding == "windows-936-2000" ||
			encoding == "936");
}

bool Player::IsCJK() {
	return (IsCP932() || IsCP949() ||
			IsBig5() || IsCP936());
}

bool Player::IsCP1251() {
	return (encoding == "ibm-5347_P100-1998" ||
			encoding == "windows-1251" || encoding == "1251");
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
