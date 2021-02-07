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
#  include "platform/windows/utils.h"
#  include <windows.h>
#  include <shellapi.h>
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
#include "rand.h"
#include "cmdline_parser.h"
#include "dynrpg.h"
#include "filefinder.h"
#include "filefinder_rtp.h"
#include "fileext_guesser.h"
#include "game_actors.h"
#include "game_battle.h"
#include "game_map.h"
#include "game_message.h"
#include "game_enemyparty.h"
#include "game_ineluki.h"
#include "game_party.h"
#include "game_player.h"
#include "game_switches.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_targets.h"
#include "graphics.h"
#include <lcf/inireader.h>
#include "input.h"
#include <lcf/ldb/reader.h>
#include <lcf/lmt/reader.h>
#include <lcf/lsd/reader.h>
#include "main_data.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_lcf.h>
#include <lcf/reader_util.h>
#include "scene_battle.h"
#include "scene_logo.h"
#include "scene_map.h"
#include "utils.h"
#include "version.h"
#include "game_quit.h"
#include "scene_title.h"
#include "instrumentation.h"
#include "transition.h"
#include <lcf/scope_guard.h>
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
	bool new_game_flag;
	int load_game_id;
	int party_x_position;
	int party_y_position;
	std::vector<int> party_members;
	int start_map_id;
	bool no_rtp_flag;
	bool no_audio_flag;
	bool is_easyrpg_project;
	bool mouse_flag;
	bool touch_flag;
	std::string encoding;
	std::string escape_symbol;
	uint32_t escape_char;
	int engine;
	std::string game_title;
	int patch;
	std::shared_ptr<Meta> meta;
	FileExtGuesser::RPG2KFileExtRemap fileext_map;
	Translation translation;
	int frames;
	std::string replay_input_path;
	std::string record_input_path;
	std::string command_line;
	int speed_modifier = 3;
	Game_ConfigPlayer player_config;
#ifdef EMSCRIPTEN
	std::string emscripten_game_name;
#endif
#ifdef _3DS
	bool is_3dsx;
#endif
}

namespace {
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
	Output::Debug("{}", header.str());

	unsigned int header_width = header.str().length();
	header.str("");
	header << std::setfill('=') << std::setw(header_width) << "=";
	Output::Debug("{}", header.str());

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
	Rand::SeedRandomNumberGenerator(time(NULL));

	auto cfg = ParseCommandLine(argc, argv);

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
		DisplayUi = BaseUi::CreateUi(SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, cfg.video);
	}

	auto buttons = Input::GetDefaultButtonMappings();
	auto directions = Input::GetDefaultDirectionMappings();

	Input::Init(std::move(buttons), std::move(directions), replay_input_path, record_input_path);
	Input::AddRecordingData(Input::RecordingData::CommandLine, command_line);

	player_config = std::move(cfg.player);
}

void Player::Run() {
	Instrumentation::Init("EasyRPG-Player");
	Scene::Push(std::make_shared<Scene_Logo>());
	Graphics::UpdateSceneCallback();

	reset_flag = false;

	Game_Clock::ResetFrame(Game_Clock::now());

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

	const auto frame_time = Game_Clock::now();
	Game_Clock::OnNextFrame(frame_time);

	Player::UpdateInput();

	int num_updates = 0;
	while (Game_Clock::NextGameTimeStep()) {
		if (num_updates > 0) {
			Player::UpdateInput();
		}

		Scene::old_instances.clear();
		Scene::instance->MainFunction();

		++num_updates;
	}
	if (num_updates == 0) {
		// If no logical frames ran, we need to update the system keys only.
		Input::UpdateSystem();
	}

	Player::Draw();

	Scene::old_instances.clear();

	if (!Transition::instance().IsActive() && Scene::instance->type == Scene::Null) {
		Exit();
		return;
	}

	auto frame_limit = DisplayUi->GetFrameLimit();
	if (frame_limit == Game_Clock::duration()) {
#ifdef EMSCRIPTEN
		emscripten_sleep(0);
#endif
		return;
	}

	// Still time after graphic update? Yield until it's time for next one.
	auto now = Game_Clock::now();
	auto next = frame_time + frame_limit;
	if (Game_Clock::now() < next) {
		iframe.End();
		Game_Clock::SleepFor(next - now);
	} else {
#ifdef EMSCRIPTEN
		// Yield back to browser once per frame
		emscripten_sleep(0);
#endif
	}
}

void Player::Pause() {
	Audio().BGM_Pause();
}

void Player::Resume() {
	Input::ResetKeys();
	Audio().BGM_Resume();
	Game_Clock::ResetFrame(Game_Clock::now());
}

void Player::UpdateInput() {
	// Input Logic:
	if (Input::IsSystemTriggered(Input::TOGGLE_FPS)) {
		DisplayUi->ToggleShowFps();
	}
	if (Input::IsSystemTriggered(Input::TAKE_SCREENSHOT)) {
		Output::TakeScreenshot();
	}
	if (Input::IsSystemTriggered(Input::SHOW_LOG)) {
		Output::ToggleLog();
	}
	if (Input::IsSystemTriggered(Input::TOGGLE_ZOOM)) {
		DisplayUi->ToggleZoom();
	}
	float speed = 1.0;
	if (Input::IsSystemPressed(Input::FAST_FORWARD)) {
		speed = Input::IsPressed(Input::PLUS) ? 10 : speed_modifier;
	}
	Game_Clock::SetGameSpeedFactor(speed);

	if (Main_Data::game_quit) {
		reset_flag |= Main_Data::game_quit->ShouldQuit();
	}

	// Update Logic:
	DisplayUi->ProcessEvents();
}

void Player::Update(bool update_scene) {
	std::shared_ptr<Scene> old_instance = Scene::instance;

	if (exit_flag) {
		Scene::PopUntil(Scene::Null);
	} else if (reset_flag && !Scene::IsAsyncPending()) {
		reset_flag = false;
		if (Scene::ReturnToTitleScene()) {
			// Fade out music and stop sound effects before returning
			Main_Data::game_system->BgmFade(800);
			Audio().SE_Stop();
			// Do not update this scene until it's properly set up in the next main loop
			update_scene = false;
		}
	}

	if (update_scene) {
		IncFrame();
	}

	Audio().Update();
	Input::Update();

	// Game events can query full screen status and change their behavior, so this needs to
	// be a game key and not a system key.
	if (Input::IsTriggered(Input::TOGGLE_FULLSCREEN)) {
		DisplayUi->ToggleFullscreen();
	}

	if (Main_Data::game_quit) {
		Main_Data::game_quit->Update();
	}

	auto& transition = Transition::instance();

	if (transition.IsActive()) {
		transition.Update();
	} else {
		// If we aren't waiting on a transition, but we are waiting for scene delay.
		Scene::instance->UpdateDelayFrames();
	}

	if (update_scene) {
		if (Main_Data::game_ineluki) {
			Main_Data::game_ineluki->Update();
		}

		Scene::instance->Update();
	}
}

void Player::Draw() {
	Graphics::Update();
	Graphics::Draw(*DisplayUi->GetDisplaySurface());
	DisplayUi->UpdateDisplay();
}

void Player::IncFrame() {
	++frames;
	if (Main_Data::game_system) {
		Main_Data::game_system->IncFrameCounter();
	}
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
	DynRpg::Reset();
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

Game_Config Player::ParseCommandLine(int argc, char *argv[]) {
#ifdef _3DS
	is_3dsx = argc > 0;
#endif

#if defined(_WIN32) && !defined(__WINRT__)
	int argc_w;
	LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);
#endif

	engine = EngineNone;
	patch = PatchNone;
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
	is_easyrpg_project = false;
	mouse_flag = false;
	touch_flag = false;
	Game_Battle::battle_test.enabled = false;

	std::stringstream ss;
	for (int i = 1; i < argc; ++i) {
		ss << argv[i] << " ";
	}
	Output::Debug("CLI: {}", ss.str());
	command_line = ss.str();

#if defined(_WIN32) && !defined(__WINRT__)
	CmdlineParser cp(argc, argv_w);
#else
	CmdlineParser cp(argc, argv);
#endif

	auto cfg = Game_Config::Create(cp);

	cp.Rewind();
	while (!cp.Done()) {
		CmdlineArg arg;
		long li_value = 0;
		if (cp.ParseNext(arg, 0, "window")) {
			// Legacy RPG_RT argument - window
			cfg.video.fullscreen.Set(false);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--enable-mouse")) {
			mouse_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, "--enable-touch")) {
			touch_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"testplay", "--test-play"})) {
			// Legacy RPG_RT argument - testplay
			debug_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, {"hidetitle", "--hide-title"})) {
			// Legacy RPG_RT argument - hidetitle
			hide_title_flag = true;
			continue;
		}
		if(cp.ParseNext(arg, 4, {"battletest", "--battle-test"})) {
			// Legacy RPG_RT argument - battletest
			Game_Battle::battle_test.enabled = true;
			Game_Battle::battle_test.troop_id = 0;

			if (arg.NumValues() > 0) {
				if (arg.ParseValue(0, li_value)) {
					Game_Battle::battle_test.troop_id = li_value;
				}
			}

			if (arg.NumValues() >= 4) {
				// 2k3 passes formation, condition and terrain_id as args 5-7
				if (arg.ParseValue(1, li_value)) {
					Game_Battle::battle_test.formation = static_cast<lcf::rpg::System::BattleFormation>(li_value);
				}
				if (arg.ParseValue(2, li_value)) {
					Game_Battle::battle_test.condition = static_cast<lcf::rpg::System::BattleCondition>(li_value);
				}
				if (arg.ParseValue(3, li_value)) {
					Game_Battle::battle_test.terrain_id = static_cast<lcf::rpg::System::BattleFormation>(li_value);
				}
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--project-path") && arg.NumValues() > 0) {
			if (arg.NumValues() > 0) {
#ifdef _WIN32
				Main_Data::SetProjectPath(arg.Value(0));
				BOOL cur_dir = SetCurrentDirectory(Utils::ToWideString(Main_Data::GetProjectPath()).c_str());
				if (cur_dir) {
					Main_Data::SetProjectPath(".");
				}
#else
				Main_Data::SetProjectPath(arg.Value(0));
#endif
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--save-path")) {
			if (arg.NumValues() > 0) {
				Main_Data::SetSavePath(arg.Value(0));
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--new-game")) {
			new_game_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 1, "--load-game-id")) {
			if (arg.ParseValue(0, li_value)) {
				load_game_id = li_value;
			}
			continue;
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
		if (cp.ParseNext(arg, 1, "--seed")) {
			if (arg.ParseValue(0, li_value)) {
				Rand::SeedRandomNumberGenerator(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--start-map-id")) {
			if (arg.ParseValue(0, li_value)) {
				start_map_id = li_value;
			}
			continue;
		}
		if (cp.ParseNext(arg, 2, "--start-position")) {
			if (arg.ParseValue(0, li_value)) {
				party_x_position = li_value;
			}
			if (arg.ParseValue(1, li_value)) {
				party_y_position = li_value;
			}
			continue;
		}
		if (cp.ParseNext(arg, 4, "--start-party")) {
			for (int i = 0; i < arg.NumValues(); ++i) {
				if (arg.ParseValue(i, li_value)) {
					party_members.push_back(li_value);
				}
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--engine")) {
			if (arg.NumValues() > 0) {
				const auto& v = arg.Value(0);
				if (v == "rpg2k" || v == "2000") {
					engine = EngineRpg2k;
				}
				else if (v == "rpg2kv150" || v == "2000v150") {
					engine = EngineRpg2k | EngineMajorUpdated;
				}
				else if (v == "rpg2ke" || v == "2000e") {
					engine = EngineRpg2k | EngineMajorUpdated | EngineEnglish;
				}
				else if (v == "rpg2k3" || v == "2003") {
					engine = EngineRpg2k3;
				}
				else if (v == "rpg2k3v105" || v == "2003v105") {
					engine = EngineRpg2k3 | EngineMajorUpdated;
				}
				else if (v == "rpg2k3e") {
					engine = EngineRpg2k3 | EngineMajorUpdated | EngineEnglish;
				}
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--record-input")) {
			if (arg.NumValues() > 0) {
				record_input_path = arg.Value(0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--replay-input")) {
			if (arg.NumValues() > 0) {
				replay_input_path = arg.Value(0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--encoding")) {
			if (arg.NumValues() > 0) {
				forced_encoding = arg.Value(0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--disable-audio")) {
			no_audio_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, "--disable-rtp")) {
			no_rtp_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, "--version", 'v')) {
			PrintVersion();
			exit(0);
			break;
		}
		if (cp.ParseNext(arg, 0, {"--help", "/?"}, 'h')) {
			PrintUsage();
			exit(0);
			break;
		}
#ifdef EMSCRIPTEN
		if (cp.ParseNext(arg, 1, "--game")) {
			if (arg.NumValues() > 0) {
				emscripten_game_name = arg.Value(0);
			}
			continue;
		}
#endif
		cp.SkipNext();
	}

#if defined(_WIN32) && !defined(__WINRT__)
	LocalFree(argv_w);
#endif

	return cfg;
}

void Player::CreateGameObjects() {
	// Load the meta information file.
	// Note: This should eventually be split across multiple folders as described in Issue #1210
	std::string meta_file = FileFinder::FindDefault(META_NAME);
	meta.reset(new Meta(meta_file));

	// Guess non-standard extensions (for the DB) before loading the encoding
	GuessNonStandardExtensions();

	GetEncoding();
	escape_symbol = lcf::ReaderUtil::Recode("\\", encoding);
	if (escape_symbol.empty()) {
		Output::Error("Invalid encoding: {}.", encoding);
	}
	escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();

	// Check for translation-related directories and load language names.
	translation.InitTranslations();

	std::string game_path = Main_Data::GetProjectPath();
	std::string save_path = Main_Data::GetSavePath();
	if (game_path == save_path) {
		Output::Debug("Using {} as Game and Save directory", game_path);
	} else {
		Output::Debug("Using {} as Game directory", game_path);
		Output::Debug("Using {} as Save directory", save_path);
	}

	LoadDatabase();

	bool no_rtp_warning_flag = false;
	{ // Scope lifetime of variables for ini parsing
		std::string ini_file = FileFinder::FindDefault(INI_NAME);
		auto ini_stream = FileFinder::OpenInputStream(ini_file, std::ios::ios_base::in);
		lcf::INIReader ini(ini_stream);
		if (ini.ParseError() != -1) {
			std::string title = ini.Get("RPG_RT", "GameTitle", GAME_TITLE);
			game_title = lcf::ReaderUtil::Recode(title, encoding);
			no_rtp_warning_flag = ini.Get("RPG_RT", "FullPackageFlag", "0") == "1" ? true : no_rtp_flag;
		}
	}

	std::stringstream title;
	if (!game_title.empty()) {
		Output::Debug("Loading game {}", game_title);
		title << game_title << " - ";
		Input::AddRecordingData(Input::RecordingData::GameTitle, game_title);
	} else {
		Output::Debug("Could not read game title.");
	}
	title << GAME_TITLE;
	DisplayUi->SetTitle(title.str());

	if (no_rtp_warning_flag) {
		Output::Debug("Game does not need RTP (FullPackageFlag=1)");
	}

	if (engine == EngineNone) {
		if (lcf::Data::system.ldb_id == 2003) {
			engine = EngineRpg2k3;

			if (FileFinder::FindDefault("ultimate_rt_eb.dll").empty()) {
				// Heuristic: Detect if game was converted from 2000 to 2003 and
				// no typical 2003 feature was used at all (breaks .flow e.g.)
				if (lcf::Data::classes.size() == 1 &&
					lcf::Data::classes[0].name.empty() &&
					lcf::Data::system.menu_commands.empty() &&
					lcf::Data::system.system2_name.empty() &&
					lcf::Data::battleranimations.size() == 1 &&
					lcf::Data::battleranimations[0].name.empty()) {
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
			if (lcf::Data::data.version >= 1) {
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
	Output::Debug("Engine configured as: 2k={} 2k3={} MajorUpdated={} Eng={}", Player::IsRPG2k(), Player::IsRPG2k3(), Player::IsMajorUpdatedVersion(), Player::IsEnglish());

	Main_Data::filefinder_rtp.reset(new FileFinder_RTP(no_rtp_flag, no_rtp_warning_flag));

	if (!FileFinder::FindDefault("dynloader.dll").empty()) {
		patch |= PatchDynRpg;
		Output::Warning("This game uses DynRPG and will not run properly.");
	}

	if (!FileFinder::FindDefault("accord.dll").empty()) {
		patch |= PatchManiac;
		Output::Warning("This game uses the Maniac Patch and will not run properly.");
	}

	Output::Debug("Patch configuration: dynrpg={} maniac={}", Player::IsPatchDynRpg(), Player::IsPatchManiac());

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
			auto exesp = FileFinder::OpenInputStream(exep);
			if (exesp) {
				Output::Debug("Loading ExFont from {}", exep);
				EXEReader exe_reader = EXEReader(exesp);
				Cache::exfont_custom = exe_reader.GetExFont();
			} else {
				Output::Debug("ExFont loading failed: {} not readable", exep);
			}
		} else {
			Output::Debug("ExFont loading failed: {} not found", EXE_NAME);
		}
	}
#endif
	if (!exfont_file.empty()) {
		auto exfont_stream = FileFinder::OpenInputStream(exfont_file);
		if (exfont_stream) {
			Output::Debug("Using custom ExFont: {}", exfont_file);
			Cache::exfont_custom = Utils::ReadStream(exfont_stream);
		} else {
			Output::Debug("Reading custom ExFont {} failed", exfont_file);
		}
	}

	ResetGameObjects();

	Main_Data::game_ineluki->ExecuteScriptList(FileFinder::FindDefault("autorun.script"));
}

void Player::ResetGameObjects() {
	// The init order is important
	Main_Data::Cleanup();

	Main_Data::game_switches = std::make_unique<Game_Switches>();

	auto min_var = Player::IsRPG2k3() ? Game_Variables::min_2k3 : Game_Variables::min_2k;
	auto max_var = Player::IsRPG2k3() ? Game_Variables::max_2k3 : Game_Variables::max_2k;
	Main_Data::game_variables = std::make_unique<Game_Variables>(min_var, max_var);

	// Prevent a crash when Game_Map wants to reset the screen content
	// because Setup() modified pictures array
	Main_Data::game_screen = std::make_unique<Game_Screen>();
	Main_Data::game_pictures = std::make_unique<Game_Pictures>();

	Main_Data::game_actors = std::make_unique<Game_Actors>();

	Game_Map::Init();

	Main_Data::game_system = std::make_unique<Game_System>();
	Main_Data::game_targets = std::make_unique<Game_Targets>();
	Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
	Main_Data::game_party = std::make_unique<Game_Party>();
	Main_Data::game_player = std::make_unique<Game_Player>();
	Main_Data::game_quit = std::make_unique<Game_Quit>();
	Main_Data::game_ineluki = std::make_unique<Game_Ineluki>();

	DynRpg::Reset();

	Game_Clock::ResetFrame(Game_Clock::now());

	Main_Data::game_system->ReloadSystemGraphic();

	Input::ResetMask();
}

static bool DefaultLmuStartFileExists(const DirectoryTreeView& tree) {
	// Compute map_id based on command line.
	int map_id = Player::start_map_id == -1 ? lcf::Data::treemap.start.party_map_id : Player::start_map_id;
	std::string mapName = Game_Map::ConstructMapName(map_id, false);

	// Now see if the file exists.
	return !tree.FindFile(mapName).empty();
}

void Player::GuessNonStandardExtensions() {
	// Check all conditions, but check the remap last (since it is potentially slower).
	FileExtGuesser::RPG2KNonStandardFilenameGuesser rpg2kRemap;
	if (!FileFinder::IsRPG2kProject(FileFinder::GetDirectoryTree()) &&
		!FileFinder::IsEasyRpgProject(FileFinder::GetDirectoryTree())) {

		rpg2kRemap = FileExtGuesser::GetRPG2kProjectWithRenames(FileFinder::GetDirectoryTree());
		if (rpg2kRemap.Empty()) {
			// Unlikely to happen because of the game browser only launches valid games
			Output::Debug("{} is not a supported project", Main_Data::GetProjectPath());

			Output::Error("{}\n\n{}\n\n{}\n\n{}","No valid game was found.",
				"EasyRPG must be run from a game folder containing\nRPG_RT.ldb and RPG_RT.lmt.",
				"This engine only supports RPG Maker 2000 and 2003\ngames.",
				"RPG Maker XP, VX, VX Ace and MV are NOT supported.");
		}
	}

	// At this point we haven't yet determined if this is an easyrpg project or not.
	// There are several ways to handle this, but we just put 'is_easyrpg_project' in the header
	// and calculate it here.
	// Try loading EasyRPG project files first, then fallback to normal RPG Maker
	std::string edb = FileFinder::FindDefault(DATABASE_NAME_EASYRPG);
	std::string emt = FileFinder::FindDefault(TREEMAP_NAME_EASYRPG);
	is_easyrpg_project = !edb.empty() && !emt.empty();

	// Non-standard extensions only apply to non-EasyRPG projects
	if (!is_easyrpg_project && !rpg2kRemap.Empty()) {
		fileext_map = rpg2kRemap.guessExtensions(*meta);
	} else {
		fileext_map = FileExtGuesser::RPG2KFileExtRemap();
	}
}

void Player::LoadDatabase() {
	// Load lcf::Database
	lcf::Data::Clear();

	if (is_easyrpg_project) {
		std::string edb = FileFinder::FindDefault(DATABASE_NAME_EASYRPG);
		auto edb_stream = FileFinder::OpenInputStream(edb, std::ios::ios_base::in );
		auto db = lcf::LDB_Reader::LoadXml(edb_stream);
		if (!db) {
			Output::ErrorStr(lcf::LcfReader::GetError());
		} else {
			lcf::Data::data = std::move(*db);
		}

		std::string emt = FileFinder::FindDefault(TREEMAP_NAME_EASYRPG);
		auto emt_stream = FileFinder::OpenInputStream(emt, std::ios::ios_base::in);
		auto treemap = lcf::LMT_Reader::LoadXml(emt_stream);
		if (!treemap) {
			Output::ErrorStr(lcf::LcfReader::GetError());
		} else {
			lcf::Data::treemap = std::move(*treemap);
		}
	} else {
		// Retrieve the appropriately-renamed files.
		std::string ldb = FileFinder::FindDefault(fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LDB));
		std::string lmt = FileFinder::FindDefault(fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LMT));

		auto ldb_stream = FileFinder::OpenInputStream(ldb);
		auto db = lcf::LDB_Reader::Load(ldb_stream, encoding);
		if (!db) {
			Output::ErrorStr(lcf::LcfReader::GetError());
		} else {
			lcf::Data::data = std::move(*db);
		}

		auto lmt_stream = FileFinder::OpenInputStream(lmt);
		auto treemap = lcf::LMT_Reader::Load(lmt_stream, encoding);
		if (!treemap) {
			Output::ErrorStr(lcf::LcfReader::GetError());
		} else {
			lcf::Data::treemap = std::move(*treemap);
		}

		if (Input::IsRecording()) {
			ldb_stream.clear();
			ldb_stream.seekg(0, std::ios::beg);
			lmt_stream.clear();
			lmt_stream.seekg(0, std::ios::beg);
			Input::AddRecordingData(Input::RecordingData::Hash,
									fmt::format("ldb {:#08x}", Utils::CRC32(ldb_stream)));
			Input::AddRecordingData(Input::RecordingData::Hash,
						   fmt::format("lmt {:#08x}", Utils::CRC32(lmt_stream)));
		}

		// Override map extension, if needed.
		if (!DefaultLmuStartFileExists(FileFinder::GetDirectoryTree())) {
			FileExtGuesser::GuessAndAddLmuExtension(FileFinder::GetDirectoryTree(), *meta, fileext_map);
		}
	}
}

static void OnMapSaveFileReady(FileRequestResult*, lcf::rpg::Save save) {
	auto map = Game_Map::loadMapFile(Main_Data::game_player->GetMapId());
	Game_Map::SetupFromSave(
			std::move(map),
			std::move(save.map_info),
			std::move(save.boat_location),
			std::move(save.ship_location),
			std::move(save.airship_location),
			std::move(save.foreground_event_execstate),
			std::move(save.panorama),
			std::move(save.common_events));
}

void Player::LoadSavegame(const std::string& save_name, int save_id) {
	Output::Debug("Loading Save {}", FileFinder::GetPathInsidePath(Main_Data::GetSavePath(), save_name));
	Main_Data::game_system->BgmFade(800);

	// We erase the screen now before loading the saved game. This prevents an issue where
	// if the save game has a different system graphic, the load screen would change before
	// transitioning out.
	Transition::instance().InitErase(Transition::TransitionFadeOut, Scene::instance.get(), 6);

	auto title_scene = Scene::Find(Scene::Title);
	if (title_scene) {
		static_cast<Scene_Title*>(title_scene.get())->OnGameStart();
	}

	auto save_stream = FileFinder::OpenInputStream(save_name);
	std::unique_ptr<lcf::rpg::Save> save = lcf::LSD_Reader::Load(save_stream, encoding);

	if (!save.get()) {
		Output::Error("{}", lcf::LcfReader::GetError());
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

	Output::Debug("Savegame version {} ({})", ver, verstr.str());

	if (ver > PLAYER_SAVEGAME_VERSION) {
		Output::Warning("This savegame was created with {} which is newer than the current version of EasyRPG Player ({})",
			verstr.str(), PLAYER_VERSION);
	}

	// Compatibility hacks for old EasyRPG Player saves.
	if (save->easyrpg_data.version == 0) {
		// Old savegames accidentally wrote animation_type as continuous for all events.
		save->party_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->boat_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->ship_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->airship_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
	}

	Scene::PopUntil(Scene::Title);
	Game_Map::Dispose();

	Main_Data::game_switches->SetData(std::move(save->system.switches));
	Main_Data::game_variables->SetData(std::move(save->system.variables));
	Main_Data::game_system->SetupFromSave(std::move(save->system));
	Main_Data::game_actors->SetSaveData(std::move(save->actors));
	Main_Data::game_party->SetupFromSave(std::move(save->inventory));
	Main_Data::game_screen->SetSaveData(std::move(save->screen));
	Main_Data::game_pictures->SetSaveData(std::move(save->pictures));
	Main_Data::game_targets->SetSaveData(std::move(save->targets));
	Main_Data::game_player->SetSaveData(save->party_location);

	int map_id = Main_Data::game_player->GetMapId();

	FileRequestAsync* map = Game_Map::RequestMap(map_id);
	save_request_id = map->Bind([save=std::move(*save)](auto* request) { OnMapSaveFileReady(request, std::move(save)); });
	map->SetImportantFile(true);

	Main_Data::game_system->ReloadSystemGraphic();

	map->Start();
	Scene::Push(std::make_shared<Scene_Map>(save_id));
}

static void OnMapFileReady(FileRequestResult*) {
	int map_id = Player::start_map_id == -1 ?
		lcf::Data::treemap.start.party_map_id : Player::start_map_id;
	int x_pos = Player::party_x_position == -1 ?
		lcf::Data::treemap.start.party_x : Player::party_x_position;
	int y_pos = Player::party_y_position == -1 ?
		lcf::Data::treemap.start.party_y : Player::party_y_position;
	if (Player::party_members.size() > 0) {
		Main_Data::game_party->Clear();
		for (auto& member: Player::party_members) {
			Main_Data::game_party->AddActor(member);
		}
	}

	Main_Data::game_player->MoveTo(map_id, x_pos, y_pos);
}

void Player::SetupNewGame() {
	Main_Data::game_system->BgmFade(800);
	Main_Data::game_system->ResetFrameCounter();
	auto title = Scene::Find(Scene::Title);
	if (title) {
		static_cast<Scene_Title*>(title.get())->OnGameStart();
	}

	Main_Data::game_party->SetupNewGame();
	SetupPlayerSpawn();
	Scene::Push(std::make_shared<Scene_Map>(0));
}

void Player::SetupPlayerSpawn() {
	int map_id = Player::start_map_id == -1 ?
		lcf::Data::treemap.start.party_map_id : Player::start_map_id;

	FileRequestAsync* request = Game_Map::RequestMap(map_id);
	map_request_id = request->Bind(&OnMapFileReady);
	request->SetImportantFile(true);
	request->Start();
}

void Player::SetupBattleTest() {
	BattleArgs args;
	args.troop_id = Game_Battle::battle_test.troop_id;
	args.first_strike = false;
	args.allow_escape = true;
	args.background = ToString(lcf::Data::system.battletest_background);
	args.terrain_id = 1; //Not used in 2k, for 2k3 only used to determine grid layout if formation == terrain.

	if (Player::IsRPG2k3()) {
		args.formation = Game_Battle::battle_test.formation;
		args.condition = Game_Battle::battle_test.condition;

		if (args.formation == lcf::rpg::System::BattleFormation_terrain) {
			args.terrain_id = Game_Battle::battle_test.terrain_id;
		}

		Output::Debug("BattleTest Mode 2k3 troop=({}) background=({}) formation=({}) condition=({}) terrain=({})",
				args.troop_id, args.background.c_str(), args.formation, args.condition, args.terrain_id);
	} else {
		Output::Debug("BattleTest Mode 2k troop=({}) background=({})", args.troop_id, args.background);
	}

	auto* troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, args.troop_id);
	if (troop == nullptr) {
		Output::Error("BattleTest: Invalid Monster Party ID {}", args.troop_id);
	}

	if (Game_Battle::battle_test.enabled) {
		Main_Data::game_party->SetupBattleTest();
	}

	Scene::Push(Scene_Battle::Create(std::move(args)), true);
}

std::string Player::GetEncoding() {
	encoding = forced_encoding;

	// command line > ini > detection > current locale
	if (encoding.empty()) {
		std::string ini = FileFinder::FindDefault(INI_NAME);
		auto ini_stream = FileFinder::OpenInputStream(ini);
		encoding = lcf::ReaderUtil::GetEncoding(ini_stream);
	}

	if (encoding.empty() || encoding == "auto") {
		encoding = "";

		std::string ldb = FileFinder::FindDefault(fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LDB));
		auto ldb_stream = FileFinder::OpenInputStream(ldb);
		std::vector<std::string> encodings = lcf::ReaderUtil::DetectEncodings(ldb_stream);

#ifndef EMSCRIPTEN
		for (std::string& enc : encodings) {
			// Heuristic: Check if encoded title and system name matches the one on the filesystem
			// When yes is a good encoding. Otherwise try the next ones.

			escape_symbol = lcf::ReaderUtil::Recode("\\", enc);
			if (escape_symbol.empty()) {
				// Bad encoding
				Output::Debug("Bad encoding: {}. Trying next.", enc);
				continue;
			}
			escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();

			if ((lcf::Data::system.title_name.empty() ||
					!FileFinder::FindImage("Title", lcf::ReaderUtil::Recode(lcf::Data::system.title_name, enc)).empty()) &&
				(lcf::Data::system.system_name.empty() ||
					!FileFinder::FindImage("System", lcf::ReaderUtil::Recode(lcf::Data::system.system_name, enc)).empty())) {
				// Looks like a good encoding
				encoding = enc;
				break;
			} else {
				Output::Debug("Detected encoding: {}. Files not found. Trying next.", enc);
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
			Output::Debug("Detected encoding: {}", encoding);
		} else {
			Output::Debug("Encoding not detected");
			encoding = lcf::ReaderUtil::GetLocaleEncoding();
		}
	}

	return encoding;
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
      --fps-limit          Set a custom frames per second limit. The default is 60 FPS.
                           Set to 0 to run with unlimited frames per second.
                           This option is not supported on all platforms.
      --no-vsync           Disable vertical sync and use fps-limit. Even without
                           this option, vsync may not be supported on all platforms.
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
      --autobattle-algo A  Which AutoBattle algorithm to use.
                           Possible options:
                            RPG_RT     - The default RPG_RT compatible algo, including RPG_RT bugs
                            RPG_RT+    - The default RPG_RT compatible algo, with bug fixes
                            ATTACK     - RPG_RT+ but only physical attacks, no skills
      --enemyai-algo A     Which EnemyAI algorithm to use.
                           Possible options:
                            RPG_RT     - The default RPG_RT compatible algo, including RPG_RT bugs
                            RPG_RT+    - The default RPG_RT compatible algo, with bug fixes
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

