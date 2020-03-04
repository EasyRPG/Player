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
#include <thread>

#ifdef _WIN32
#  include "util_win.h"
#  include <Windows.h>
#  include <Shellapi.h>
#elif defined(GEKKO)
#  include <fat.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
#elif defined(PSP2)
#  include <psp2/kernel/processmgr.h>
#elif defined(_3DS)
#  include <3ds.h>
#elif defined(__SWITCH__)
#  include <switch.h>
#endif

#include "async_handler.h"
#include "audio.h"
#include "cache.h"
#include "filefinder.h"
#include "game_actors.h"
#include "game_battle.h"
#include "game_map.h"
#include "game_message.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_targets.h"
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
#include "utils.h"
#include "version.h"
#include "game_quit.h"
#include "scene_title.h"
#include "instrumentation.h"
#include "transition.h"
#include "scope_guard.h"
#include "baseui.h"
#include "game_clock.h"

#ifndef EMSCRIPTEN
// This is not used on Emscripten.
#include "exe_reader.h"
#endif

namespace Player {
	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool window_flag;
	bool fps_flag;
	bool fps_render_window = false;
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
	uint32_t escape_char;
	int engine;
	std::string game_title;
	std::shared_ptr<Meta> meta;
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
	Game_Clock::time_point start_time;
	Game_Clock::time_point next_frame;

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

#if defined(_WIN32)
	WindowsUtils::InitMiniDumpWriter();
#endif

	Game_Clock::logClockInfo();
	Utils::SeedRandomNumberGenerator(time(NULL));

	ParseCommandLine(argc, argv);

#ifdef EMSCRIPTEN
	Output::IgnorePause(true);

	// Create initial directory structure in our private area
	// Retrieve save directory from persistent storage
	EM_ASM(({
		var dirs = ['Backdrop', 'Battle', 'Battle2', 'BattleCharSet', 'BattleWeapon', 'CharSet', 'ChipSet', 'FaceSet', 'Frame', 'GameOver', 'Monster', 'Movie', 'Music', 'Panorama', 'Picture', 'Sound', 'System', 'System2', 'Title', 'Save'];
		dirs.forEach(function(dir) { FS.mkdir(dir) });

		FS.mount(Module.EASYRPG_FS, {}, 'Save');

		FS.syncfs(true, function(err) {
		});
	}));
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
	Instrumentation::Init("EasyRPG-Player");
	Scene::Push(std::make_shared<Scene_Logo>());
	Graphics::UpdateSceneCallback();

	reset_flag = false;

	// Reset frames before starting
	FrameReset(Game_Clock::now());

	start_time = Game_Clock::now();

	// Main loop
	// libretro invokes the MainLoop through a retro_run-callback
#ifndef USE_LIBRETRO
	while (Transition::instance().IsActive() || Scene::instance->type != Scene::Null) {
#  if defined(_3DS)
		if (!aptMainLoop())
			Exit();
#  elif defined(__SWITCH__)
		if(!appletMainLoop())
			Exit();
#  endif
		MainLoop();
	}
#endif
}

void Player::MainLoop() {
	Instrumentation::FrameScope iframe;

	next_frame = start_time + Game_Clock::GetSimulationTimeStep();

	auto speed_modifier = GetSpeedModifier();
	for (int i = 0; i < speed_modifier; ++i) {
		Scene::old_instances.clear();
		Scene::instance->MainFunction();
	}

	Player::Draw();

	Scene::old_instances.clear();

	start_time = next_frame;

	if (!Transition::instance().IsActive() && Scene::instance->type == Scene::Null) {
		Exit();
		return;
	}

	// Don't use sleep when the port uses an external timing source
#if !defined(USE_LIBRETRO)
	auto cur_time = Game_Clock::now();
	// Still time after graphic update? Yield until it's time for next one.
	if (cur_time < next_frame) {
		iframe.End();
		Game_Clock::SleepFor(next_frame - cur_time);
	}
#endif
}

void Player::Pause() {
	Audio().BGM_Pause();
}

void Player::Resume() {
	Input::ResetKeys();
	Audio().BGM_Resume();
	FrameReset(Game_Clock::now());
}

void Player::Update(bool update_scene) {
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
	if (Input::IsTriggered(Input::TOGGLE_ZOOM)) {
		DisplayUi->ToggleZoom();
	}
	if (Input::IsTriggered(Input::TOGGLE_FULLSCREEN)) {
		DisplayUi->ToggleFullscreen();
	}

	if (Main_Data::game_quit) {
		reset_flag |= Main_Data::game_quit->ShouldQuit();
	}

	// Update Logic:
	DisplayUi->ProcessEvents();

	std::shared_ptr<Scene> old_instance = Scene::instance;

	if (exit_flag) {
		Scene::PopUntil(Scene::Null);
	} else if (reset_flag && !Scene::IsAsyncPending()) {
		reset_flag = false;
		if (Scene::ReturnToTitleScene()) {
			// Fade out music and stop sound effects before returning
			Game_System::BgmFade(800);
			Audio().SE_Stop();
			// Do not update this scene until it's properly set up in the next main loop
			update_scene = false;
		}
	}

	Audio().Update();
	Input::Update();

	if (Main_Data::game_quit) {
		Main_Data::game_quit->Update();
	}

	auto& transition = Transition::instance();
	auto was_transition_pending = transition.IsActive();

	transition.Update();

	// If we aren't waiting on a transition, but we are waiting for scene delay.
	if (!was_transition_pending) {
		Scene::instance->UpdateDelayFrames();
	}
	if (update_scene) {
		Scene::instance->Update();
		// Async file loading or transition. Don't increment the frame
		// counter as we now have to "suspend" and "resume"
		if (Scene::IsAsyncPending()) {
			old_instance->SetAsyncFromMainLoop();
			return;
		}
		IncFrame();
	}
}

void Player::Draw() {
	Graphics::Update();

	auto cur_time = Game_Clock::now();
	if (cur_time < next_frame) {
		Graphics::Draw(*DisplayUi->GetDisplaySurface());
		DisplayUi->UpdateDisplay();
	}
}

void Player::IncFrame() {
	++frames;
	Game_System::IncFrameCounter();
}

void Player::FrameReset(Game_Clock::time_point now) {
	// When next frame is expected
	next_frame = now + Game_Clock::GetSimulationTimeStep();

	Graphics::FrameReset(now);
}

int Player::GetFrames() {
	return frames;
}

void Player::Exit() {
	Graphics::UpdateSceneCallback();
#ifdef EMSCRIPTEN
	BitmapRef surface = DisplayUi->GetDisplaySurface();
	std::string message = "It's now safe to turn off\n      your browser.";

	Text::Draw(*surface, 84, DisplayUi->GetHeight() / 2 - 30, *Font::Default(), Color(221, 123, 64, 255), message);
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

static bool parseInt(const std::string& s, long& value) {
	auto* p = s.c_str();
	auto* e = p + s.size();
	long v = strtol(p, const_cast<char**>(&e), 10);
	if (p == e) {
		return false;
	}
	value = v;
	return true;
}

void Player::ParseCommandLine(int argc, char *argv[]) {
#ifdef _3DS
	is_3dsx = argc > 0;
#endif
#if defined(_WIN32) && !defined(__WINRT__)
	int argc_w;
	LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);
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
	new_game_flag = false;
	load_game_id = -1;
	party_x_position = -1;
	party_y_position = -1;
	start_map_id = -1;
	no_rtp_flag = false;
	no_audio_flag = false;
	mouse_flag = false;
	touch_flag = false;
	Game_Battle::battle_test.enabled = false;

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
		if (*it == "window" || *it == "--window") {
			window_flag = true;
		}
		else if (*it == "--show-fps") {
			fps_flag = true;
		}
		else if (*it == "--fps-render-window") {
			fps_render_window = true;
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
			Game_Battle::battle_test.enabled = true;
			Game_Battle::battle_test.troop_id = atoi((*it).c_str());

			if (Game_Battle::battle_test.troop_id == 0) {
				--it;
				Game_Battle::battle_test.troop_id = (argc > 4) ? atoi(argv[4]) : 0;
				// 2k3 passes formation, condition and terrain_id as args 5-7
				if (argc > 7) {
					Game_Battle::battle_test.formation = (RPG::System::BattleFormation)atoi(argv[5]);
					Game_Battle::battle_test.condition = (RPG::System::BattleCondition)atoi(argv[6]);
					Game_Battle::battle_test.terrain_id = (RPG::System::BattleFormation)atoi(argv[7]);
				}
			}
		}
		else if (*it == "--battle-test") {
			++it;
			if (it == args.end()) {
				return;
			}
			Game_Battle::battle_test.enabled = true;
			Game_Battle::battle_test.troop_id = atoi((*it).c_str());

			++it;
			// If the next 3 parameters are numbers, assume they're 2k3 formation, condition, and terrain
			long fct[3] = { 0, 0, 1 };
			for (int i = 0; i < 3; ++i) {
				if (it == args.end() || !parseInt(*it, fct[i])) {
					break;
				}
				++it;
			}

			Game_Battle::battle_test.formation = static_cast<RPG::System::BattleFormation>(fct[0]);
			Game_Battle::battle_test.condition = static_cast<RPG::System::BattleCondition>(fct[1]);
			Game_Battle::battle_test.terrain_id = fct[2];

			--it;
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

#if defined(_WIN32) && !defined(__WINRT__)
	LocalFree(argv_w);
#endif
}

void Player::CreateGameObjects() {
	GetEncoding();
	escape_symbol = ReaderUtil::Recode("\\", encoding);
	if (escape_symbol.empty()) {
		Output::Error("Invalid encoding: %s.", encoding.c_str());
	}
	escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();

	std::string game_path = Main_Data::GetProjectPath();
	std::string save_path = Main_Data::GetSavePath();
	if (game_path == save_path) {
		Output::Debug("Using %s as Game and Save directory", game_path.c_str());
	} else {
		Output::Debug("Using %s as Game directory", game_path.c_str());
		Output::Debug("Using %s as Save directory", save_path.c_str());
	}

	LoadDatabase();

	// Load the meta information file.
	// Note: This should eventually be split across multiple folders as described in Issue #1210
	std::string meta_file = FileFinder::FindDefault(META_NAME);
	meta.reset(new Meta(meta_file));

	bool no_rtp_warning_flag = false;
	std::string ini_file = FileFinder::FindDefault(INI_NAME);

	INIReader ini(ini_file);
	if (ini.ParseError() != -1) {
		std::string title = ini.GetString("RPG_RT", "GameTitle", "");
		game_title = ReaderUtil::Recode(title, encoding);
		no_rtp_warning_flag = ini.GetBoolean("RPG_RT", "FullPackageFlag", false);
	}

	std::stringstream title;
	if (!game_title.empty()) {
		Output::Debug("Loading game %s", game_title.c_str());
		title << game_title << " - ";
	} else {
		Output::Debug("Could not read game title.");
	}
	title << GAME_TITLE;
	DisplayUi->SetTitle(title.str());

	if (no_rtp_warning_flag) {
		Output::Debug("Game does not need RTP (FullPackageFlag=1)");
	}

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
	Output::Debug("Engine configured as: 2k=%d 2k3=%d MajorUpdated=%d Eng=%d", Player::IsRPG2k(), Player::IsRPG2k3(), Player::IsMajorUpdatedVersion(), Player::IsEnglish());

	FileFinder::InitRtpPaths(no_rtp_flag, no_rtp_warning_flag);

	// ExFont parsing
	Cache::exfont_custom.clear();
	// Check for bundled ExFont
	std::string exfont_file = FileFinder::FindImage(".", "ExFont");
#ifndef EMSCRIPTEN
	if (exfont_file.empty()) {
		// Attempt reading ExFont from RPG_RT.exe (not supported on Emscripten,
		// a ExFont can be manually bundled there)
		std::string exep = FileFinder::FindDefault(EXE_NAME);
		if (!exep.empty()) {
			auto exesp = FileFinder::openUTF8(exep, std::ios::binary | std::ios::in);
			if (exesp) {
				Output::Debug("Loading ExFont from %s", exep.c_str());
				EXEReader exe_reader = EXEReader(*exesp);
				Cache::exfont_custom = exe_reader.GetExFont();
			} else {
				Output::Debug("ExFont loading failed: %s not readable", exep.c_str());
			}
		} else {
			Output::Debug("ExFont loading failed: %s not found", EXE_NAME);
		}
	}
#endif
	if (!exfont_file.empty()) {
		auto exfont_stream = FileFinder::openUTF8(exfont_file, std::ios::binary | std::ios::in);
		if (exfont_stream) {
			Output::Debug("Using custom ExFont: %s", exfont_file.c_str());
			Cache::exfont_custom = Utils::ReadStream(*exfont_stream);
		} else {
			Output::Debug("Reading custom ExFont %s failed", exfont_file.c_str());
		}
	}

	ResetGameObjects();
}

void Player::ResetGameObjects() {
	Game_System::ResetSystemGraphic();

	// The init order is important
	Main_Data::Cleanup();

	Main_Data::game_data.Setup();

	Main_Data::game_switches = std::make_unique<Game_Switches>();

	auto min_var = Player::IsRPG2k3() ? Game_Variables::min_2k3 : Game_Variables::min_2k;
	auto max_var = Player::IsRPG2k3() ? Game_Variables::max_2k3 : Game_Variables::max_2k;
	Main_Data::game_variables = std::make_unique<Game_Variables>(min_var, max_var);

	// Prevent a crash when Game_Map wants to reset the screen content
	// because Setup() modified pictures array
	Main_Data::game_screen = std::make_unique<Game_Screen>();
	Main_Data::game_pictures = std::make_unique<Game_Pictures>();

	Game_Actors::Init();
	Game_Map::Init();
	Game_Message::Init();
	Game_System::Init();

	Main_Data::game_targets = std::make_unique<Game_Targets>();
	Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
	Main_Data::game_party = std::make_unique<Game_Party>();
	Main_Data::game_player = std::make_unique<Game_Player>();
	Main_Data::game_quit = std::make_unique<Game_Quit>();

	FrameReset(Game_Clock::now());
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

static void FixSaveGames() {
	// Compatibility hacks for old EasyRPG Player saves.
	if (Main_Data::game_data.easyrpg_data.version == 0) {
		// Old savegames accidentally wrote animation_type as
		// continuous for all events.
		Main_Data::game_player->SetAnimationType(Game_Character::AnimType::AnimType_non_continuous);
		Game_Map::GetVehicle(Game_Vehicle::Boat)->SetAnimationType(Game_Character::AnimType::AnimType_non_continuous);
		Game_Map::GetVehicle(Game_Vehicle::Ship)->SetAnimationType(Game_Character::AnimType::AnimType_non_continuous);
		Game_Map::GetVehicle(Game_Vehicle::Airship)->SetAnimationType(Game_Character::AnimType::AnimType_non_continuous);
	}

	if (Main_Data::game_data.easyrpg_data.version <= 600) {
		// Old savegames didn't write the vehicle chunk.
		Main_Data::game_data.boat_location.vehicle = 1;
		Main_Data::game_data.ship_location.vehicle = 2;
		Main_Data::game_data.airship_location.vehicle = 3;
	}
}

static void OnMapSaveFileReady(FileRequestResult*) {
	Game_Map::SetupFromSave();
	FixSaveGames();

	Main_Data::game_player->Refresh();
}

void Player::LoadSavegame(const std::string& save_name) {
	Output::Debug("Loading Save %s", FileFinder::GetPathInsidePath(Main_Data::GetSavePath(), save_name).c_str());
	Game_System::BgmStop();

	auto title_scene = Scene::Find(Scene::Title);
	if (title_scene) {
		static_cast<Scene_Title*>(title_scene.get())->OnGameStart();
	}

	std::unique_ptr<RPG::Save> save = LSD_Reader::Load(save_name, encoding);

	if (!save.get()) {
		Output::Error("%s", LcfReader::GetError().c_str());
	}

	std::stringstream verstr;
	int ver = save->easyrpg_data.version;
	if (ver == 0) {
		verstr << "RPG_RT or EasyRPG Player Pre-0.6.0";
	} else if (ver >= 10000) {
		verstr << "Unknown Engine";
	} else {
		verstr << "EasyRPG Player ";
		char verbuf[64];
		sprintf(verbuf, "%d.%d.%d", ver / 1000 % 10, ver / 100 % 10, ver / 10 % 10);
		verstr << verbuf;
		if (ver % 10 > 0) {
			verstr << "." << ver % 10;
		}
	}

	Output::Debug("Savegame version %d (%s)", ver, verstr.str().c_str());

	if (ver > PLAYER_SAVEGAME_VERSION) {
		Output::Warning("This savegame was created with %s which is newer than the current version of EasyRPG Player (%s)",
			verstr.str().c_str(), PLAYER_VERSION);
	}

	Scene::PopUntil(Scene::Title);
	Game_Map::Dispose();

	Main_Data::game_data = *save.get();
	Main_Data::game_data.system.Fixup();

	Game_Actors::Fixup();
	Main_Data::game_party->SetupFromSave(std::move(Main_Data::game_data.inventory));
	Main_Data::game_switches->SetData(std::move(Main_Data::game_data.system.switches));
	Main_Data::game_variables->SetData(std::move(Main_Data::game_data.system.variables));
	Main_Data::game_screen->SetSaveData(std::move(Main_Data::game_data.screen));
	Main_Data::game_pictures->SetSaveData(std::move(Main_Data::game_data.pictures));
	Main_Data::game_targets->SetSaveData(std::move(Main_Data::game_data.targets));

	int map_id = save->party_location.map_id;

	FileRequestAsync* map = Game_Map::RequestMap(map_id);
	save_request_id = map->Bind(&OnMapSaveFileReady);
	map->SetImportantFile(true);

	Game_System::ReloadSystemGraphic();

	map->Start();
	Scene::Push(std::make_shared<Scene_Map>(true));
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

	Game_Map::Setup(map_id, TeleportTarget::eParallelTeleport);
	Main_Data::game_player->MoveTo(x_pos, y_pos);
	Main_Data::game_player->Refresh();
}

void Player::SetupNewGame() {
	Game_System::BgmStop();
	Game_System::ResetFrameCounter();
	auto title = Scene::Find(Scene::Title);
	if (title) {
		static_cast<Scene_Title*>(title.get())->OnGameStart();
	}

	Main_Data::game_party->SetupNewGame();
	SetupPlayerSpawn();
	Scene::Push(std::make_shared<Scene_Map>(false));
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
			escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();
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
		escape_char = 0;

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
      --fps-render-window  Render the frames per second counter in windowed mode.
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
	return (IsCP932() || IsCP949() || IsBig5() || IsCP936());
}

bool Player::IsCP1251() {
	return (encoding == "ibm-5347_P100-1998" ||
			encoding == "windows-1251" || encoding == "1251");
}

int Player::EngineVersion() {
	if (IsRPG2k3()) return 2003;
	if (IsRPG2k()) return 2000;
	return 0;
}

std::string Player::GetEngineVersion() {
	if (EngineVersion() > 0) return std::to_string(EngineVersion());
	return std::string();
}
