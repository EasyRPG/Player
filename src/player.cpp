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
#include <memory>
#include <thread>
#include <csignal>

#ifdef _WIN32
#  include "platform/windows/utils.h"
#  include <windows.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
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
#include "multiplayer/game_multiplayer.h"
#include "game_variables.h"
#include "game_targets.h"
#include "game_windows.h"
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
#include "scene_settings.h"
#include "scene_title.h"
#include "instrumentation.h"
#include "transition.h"
#include <lcf/scope_guard.h>
#include "baseui.h"
#include "game_clock.h"
#include "message_overlay.h"
#include "statustext_overlay.h"
#include "multiplayer/chatui.h"
#include "multiplayer/server.h"

#ifdef __ANDROID__
#include "platform/android/android.h"
#endif

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
#include "decoder_fluidsynth.h"
#endif

#ifndef EMSCRIPTEN
// This is not used on Emscripten.
#include "exe_reader.h"
#endif

using namespace std::chrono_literals;

namespace Player {
	int screen_width = SCREEN_TARGET_WIDTH;
	int screen_height = SCREEN_TARGET_HEIGHT;
	int menu_offset_x = (screen_width - MENU_WIDTH) / 2;
	int menu_offset_y = (screen_height - MENU_HEIGHT) / 2;
	int message_box_offset_x = (screen_width - MENU_WIDTH) / 2;
	bool has_custom_resolution = false;

	bool exit_flag;
	bool reset_flag;
	bool debug_flag;
	bool hide_title_flag;
	bool server_flag;
	int load_game_id;
	int party_x_position;
	int party_y_position;
	std::vector<int> party_members;
	int start_map_id;
	bool no_rtp_flag;
	std::string rtp_path;
	bool no_audio_flag;
	bool is_easyrpg_project;
	std::string encoding;
	std::string escape_symbol;
	uint32_t escape_char;
	std::string game_title;
	std::shared_ptr<Meta> meta;
	FileExtGuesser::RPG2KFileExtRemap fileext_map;
	std::string startup_language;
	Translation translation;
	int frames;
	std::string replay_input_path;
	std::string record_input_path;
	std::string command_line;
	bool toggle_mute_flag = false;
	int volume_se = 0;
	int volume_bgm = 0;
	int speed_modifier = 3;
	int speed_modifier_plus = 10;
	int rng_seed = -1;
	Game_ConfigPlayer player_config;
	Game_ConfigGame game_config;
#ifdef EMSCRIPTEN
	std::string emscripten_game_name;
#endif
}

namespace {
	std::vector<std::string> arguments;

	// Overwritten by --encoding
	std::string forced_encoding;

	FileRequestBinding system_request_id;
	FileRequestBinding save_request_id;
	FileRequestBinding map_request_id;
}

void Player::Init(std::vector<std::string> args) {
	frames = 0;

	// Must be called before the first call to Output
	Graphics::Init();

#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif

	// First parse command line arguments
	arguments = args;
	auto cfg = ParseCommandLine();

	// Display a nice version string
	auto header = GetFullVersionString() + " started";
	Output::Debug("{}", header);
	for (auto& c : header)
		c = '=';
	Output::Debug("{}", header);

#if defined(_WIN32)
	WindowsUtils::InitMiniDumpWriter();
#endif

	Output::Debug("CLI: {}", command_line);

	if (server_flag) {
		Server().SetConfig(cfg.multiplayer);
		return;
	}

	GMI().SetConfig(cfg.multiplayer);

	Game_Clock::logClockInfo();
	if (rng_seed < 0) {
		Rand::SeedRandomNumberGenerator(time(NULL));
	} else {
		Rand::SeedRandomNumberGenerator(rng_seed);
	}

	Main_Data::Init();

	DisplayUi.reset();

	if(! DisplayUi) {
		DisplayUi = BaseUi::CreateUi(Player::screen_width, Player::screen_height, cfg);
	}

	Input::Init(cfg.input, replay_input_path, record_input_path);
	Input::AddRecordingData(Input::RecordingData::CommandLine, command_line);

	player_config = std::move(cfg.player);
}

void Player::Run() {
	if (server_flag) {
		auto signal_handler = [](int signal) {
			Server().Stop();
			Output::Debug("Server: signal={}", signal);
		};
		std::signal(SIGINT, signal_handler);
		std::signal(SIGTERM, signal_handler);
		Server().Start(true);
		return;
	}

	Instrumentation::Init("EasyRPG-Player");
	Scene::Push(std::make_shared<Scene_Logo>());
	Graphics::UpdateSceneCallback();

	reset_flag = false;

	Game_Clock::ResetFrame(Game_Clock::now());

	// Main loop
	// libretro invokes the MainLoop through a retro_run-callback
#ifndef USE_LIBRETRO
	while (Transition::instance().IsActive() || (Scene::instance && Scene::instance->type != Scene::Null)) {
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

		Graphics::GetMessageOverlay().Update();
		Graphics::GetStatusTextOverlay().Update();

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
#if PAUSE_AUDIO_WHEN_FOCUS_LOST
	Audio().BGM_Pause();
#endif
}

void Player::Resume() {
	Input::ResetKeys();
#if PAUSE_AUDIO_WHEN_FOCUS_LOST
	Audio().BGM_Resume();
#endif
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
	if (Input::IsSystemTriggered(Input::TOGGLE_MUTE)) {
		Audio().ToggleMute();
	}
	float speed = 1.0;
	if (Input::IsSystemPressed(Input::FAST_FORWARD)) {
		speed = speed_modifier;
	}
	if (Input::IsSystemPressed(Input::FAST_FORWARD_PLUS)) {
		speed = speed_modifier_plus;
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
	GMI().Update();
	CUI().Update();

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

#ifdef __ANDROID__
	EpAndroid::invoke();
#endif
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
	if (player_config.settings_autosave.Get()) {
		Scene_Settings::SaveConfig(true);
	}

	Graphics::UpdateSceneCallback();
#ifdef EMSCRIPTEN
	BitmapRef surface = DisplayUi->GetDisplaySurface();
	std::string message = "It's now safe to turn off\n      your browser.";
	DisplayUi->CleanDisplay();
	Text::Draw(*surface, 84, DisplayUi->GetHeight() / 2 - 16, *Font::DefaultBitmapFont(), Color(221, 123, 64, 255), message);
	DisplayUi->UpdateDisplay();

	auto ret = FileFinder::Root().OpenOutputStream("/tmp/message.png", std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	if (ret) Output::TakeScreenshot(ret);
#endif
	Player::ResetGameObjects();
	Font::Dispose();
	DynRpg::Reset();
	Graphics::Quit();
	Output::Quit();
	FileFinder::Quit();
	DisplayUi.reset();
	GMI().Quit();
	Server().Stop();
}

Game_Config Player::ParseCommandLine() {
	debug_flag = false;
	hide_title_flag = false;
	exit_flag = false;
	reset_flag = false;
	server_flag = false;
	load_game_id = -1;
	party_x_position = -1;
	party_y_position = -1;
	start_map_id = -1;
	no_rtp_flag = false;
	no_audio_flag = false;
	is_easyrpg_project = false;
	Game_Battle::battle_test.enabled = false;

	std::stringstream ss;
	for (size_t i = 1; i < arguments.size(); ++i) {
		ss << arguments[i] << " ";
	}
	command_line = ss.str();

	CmdlineParser cp(arguments);
	auto cfg = Game_Config::Create(cp);

	bool battletest_handled = false;

	cp.Rewind();
	if (!cp.Done()) {
		// BattleTest argument handling in a RPG_RT compatible way is very ugly because the arguments do not follow
		// directly. Try to parse it and afterwards rewind the parser to parse the rest.
		CmdlineArg arg;
		long li_value = 0;

		// Legacy BattleTest handling: When BattleTest is argument 1, then the values are:
		// - arg4: troop_id
		// - arg5-7: formation, condition, terrain_id (2k3 only)
		// - arg2-3 are unrelated ("ShowTitle Window")
		if (cp.ParseNext(arg, 6, {"battletest"})) {
			Game_Battle::battle_test.enabled = true;
			Game_Battle::battle_test.troop_id = 0;

			// Starting from 3 to reach arg4 from arg1
			if (arg.NumValues() >= 3) {
				if (arg.ParseValue(2, li_value)) {
					Game_Battle::battle_test.troop_id = li_value;
				}
			}

			if (arg.NumValues() >= 6) {
				if (arg.ParseValue(3, li_value)) {
					Game_Battle::battle_test.formation = static_cast<lcf::rpg::System::BattleFormation>(li_value);
				}
				if (arg.ParseValue(4, li_value)) {
					Game_Battle::battle_test.condition = static_cast<lcf::rpg::System::BattleCondition>(li_value);
				}
				if (arg.ParseValue(5, li_value)) {
					Game_Battle::battle_test.terrain_id = static_cast<lcf::rpg::System::BattleFormation>(li_value);
				}
			}

			battletest_handled = true;
		}
	}

	cp.Rewind();
	while (!cp.Done()) {
		CmdlineArg arg;
		long li_value = 0;
		if (cp.ParseNext(arg, 0, "window")) {
			// Legacy RPG_RT argument - window
			cfg.video.fullscreen.Set(false);
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
		if(!battletest_handled && cp.ParseNext(arg, 4, {"battletest", "--battle-test"})) {
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
				} else {
					// When the argument is not a number, args5-7 were likely not specified and are something different
					// Rewind to prevent losing other args
					cp.RewindBy(3);
					continue;
				}

				if (arg.ParseValue(2, li_value)) {
					Game_Battle::battle_test.condition = static_cast<lcf::rpg::System::BattleCondition>(li_value);
				}
				if (arg.ParseValue(3, li_value)) {
					Game_Battle::battle_test.terrain_id = static_cast<lcf::rpg::System::BattleFormation>(li_value);
				}
			} else if (arg.NumValues() >= 2) {
				// Only troop-id was provided: Rewind to prevent losing other args
				cp.RewindBy(arg.NumValues() - 1);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--project-path") && arg.NumValues() > 0) {
			if (arg.NumValues() > 0) {
				auto gamefs = FileFinder::Root().Create(FileFinder::MakeCanonical(arg.Value(0), 0));
				if (!gamefs) {
					Output::Error("Invalid --project-path {}", arg.Value(0));
				}
				FileFinder::SetGameFilesystem(gamefs);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--save-path")) {
			if (arg.NumValues() > 0) {
				auto savefs = FileFinder::Root().Create(FileFinder::MakeCanonical(arg.Value(0), 0));
				if (!savefs) {
					Output::Error("Invalid --save-path {}", arg.Value(0));
				}
				FileFinder::SetSaveFilesystem(savefs);
			}
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
			if (arg.ParseValue(0, li_value) && li_value > 0) {
				rng_seed = li_value;
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
		if (cp.ParseNext(arg, 0, "--no-audio") || cp.ParseNext(arg, 0, "--disable-audio")) {
			no_audio_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-rtp") || cp.ParseNext(arg, 0, "--disable-rtp")) {
			no_rtp_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 0, "--server")) {
			server_flag = true;
			continue;
		}
		if (cp.ParseNext(arg, 1, "--rtp-path")) {
			if (arg.NumValues() > 0) {
				rtp_path = arg.Value(0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-log-color")) {
			Output::SetTermColor(false);
			continue;
		}
		if (cp.ParseNext(arg, 1, "--language")) {
			if (arg.NumValues() > 0) {
				startup_language = arg.Value(0);
				if (startup_language == "default") {
					startup_language.clear();
				}
			}
			continue;
		}
#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
		if (cp.ParseNext(arg, 1, "--soundfont")) {
			if (arg.NumValues() > 0) {
				FluidSynthDecoder::SetSoundfont(arg.Value(0));
			}
			continue;
		}
#endif
		if (cp.ParseNext(arg, 0, "--version", 'v')) {
			std::cout << GetFullVersionString() << std::endl;
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

	return cfg;
}

void Player::CreateGameObjects() {
	// Parse game specific settings
	CmdlineParser cp(arguments);
	game_config = Game_ConfigGame::Create(cp);

	// Load the meta information file.
	// Note: This should eventually be split across multiple folders as described in Issue #1210
	std::string meta_file = FileFinder::Game().FindFile(META_NAME);
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

	std::string game_path = FileFinder::GetFullFilesystemPath(FileFinder::Game());
	std::string save_path = FileFinder::GetFullFilesystemPath(FileFinder::Save());
	if (game_path == save_path) {
		Output::DebugStr("Game and Save Directory:");
		FileFinder::DumpFilesystem(FileFinder::Game());
	} else {
		Output::Debug("Game Directory:");
		FileFinder::DumpFilesystem(FileFinder::Game());
		Output::Debug("SaveDirectory:", save_path);
		FileFinder::DumpFilesystem(FileFinder::Save());
	}

	LoadDatabase();

	bool no_rtp_warning_flag = false;
	Player::has_custom_resolution = false;
	{ // Scope lifetime of variables for ini parsing
		std::string ini_file = FileFinder::Game().FindFile(INI_NAME);

		auto ini_stream = FileFinder::Game().OpenInputStream(ini_file, std::ios_base::in);
		if (ini_stream) {
			lcf::INIReader ini(ini_stream);
			if (ini.ParseError() != -1) {
				std::string title = ini.Get("RPG_RT", "GameTitle", GAME_TITLE);
				game_title = lcf::ReaderUtil::Recode(title, encoding);
				no_rtp_warning_flag = ini.Get("RPG_RT", "FullPackageFlag", "0") == "1" ? true : no_rtp_flag;
				if (ini.HasValue("RPG_RT", "WinW") || ini.HasValue("RPG_RT", "WinH")) {
					Player::screen_width = ini.GetInteger("RPG_RT", "WinW", SCREEN_TARGET_WIDTH);
					Player::screen_height = ini.GetInteger("RPG_RT", "WinH", SCREEN_TARGET_HEIGHT);
					Player::has_custom_resolution = true;
				}
			}
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

	// ExFont parsing
	Cache::exfont_custom.clear();
	// Check for bundled ExFont
	auto exfont_stream = FileFinder::OpenImage("Font", "ExFont");
	if (!exfont_stream) {
		// Backwards compatible with older Player versions
		exfont_stream = FileFinder::OpenImage(".", "ExFont");
	}

	int& engine = game_config.engine;

#ifndef EMSCRIPTEN
	// Attempt reading ExFont and version information from RPG_RT.exe (not supported on Emscripten)
	std::unique_ptr<EXEReader> exe_reader;
	auto exeis = FileFinder::Game().OpenFile(EXE_NAME);

	if (exeis) {
		exe_reader.reset(new EXEReader(std::move(exeis)));
		Cache::exfont_custom = exe_reader->GetExFont();
		if (!Cache::exfont_custom.empty()) {
			Output::Debug("ExFont loaded from RPG_RT");
		}

		if (engine == EngineNone) {
			auto version_info = exe_reader->GetFileInfo();
			version_info.Print();
			bool is_patch_maniac;
			engine = version_info.GetEngineType(is_patch_maniac);
			if (!game_config.patch_override) {
				game_config.patch_maniac.Set(is_patch_maniac);
			}
		}

		if (engine == EngineNone) {
			Output::Debug("Unable to detect version from exe");
		}
	} else {
		Output::Debug("Cannot find RPG_RT");
	}
#endif

	if (exfont_stream) {
		Output::Debug("Using custom ExFont: {}", FileFinder::GetPathInsideGamePath(exfont_stream.GetName()));
		Cache::exfont_custom = Utils::ReadStream(exfont_stream);
	}

	if (engine == EngineNone) {
		if (lcf::Data::system.ldb_id == 2003) {
			engine = EngineRpg2k3;
			if (!FileFinder::Game().FindFile("ultimate_rt_eb.dll").empty()) {
				engine |= EngineEnglish | EngineMajorUpdated;
			}
		} else {
			engine = EngineRpg2k;
			if (lcf::Data::data.version >= 1) {
				engine |= EngineEnglish | EngineMajorUpdated;
			}
		}
		if (!(engine & EngineMajorUpdated)) {
			if (FileFinder::IsMajorUpdatedTree()) {
				engine |= EngineMajorUpdated;
			}
		}
	}

	Output::Debug("Engine configured as: 2k={} 2k3={} MajorUpdated={} Eng={}", Player::IsRPG2k(), Player::IsRPG2k3(), Player::IsMajorUpdatedVersion(), Player::IsEnglish());

	Main_Data::filefinder_rtp = std::make_unique<FileFinder_RTP>(no_rtp_flag, no_rtp_warning_flag, rtp_path);

	if (!game_config.patch_override) {
		if (!FileFinder::Game().FindFile("harmony.dll").empty()) {
			game_config.patch_key_patch.Set(true);
		}

		if (!FileFinder::Game().FindFile("dynloader.dll").empty()) {
			game_config.patch_dynrpg.Set(true);
			Output::Warning("This game uses DynRPG and will not run properly.");
		}

		if (!FileFinder::Game().FindFile("accord.dll").empty()) {
			game_config.patch_maniac.Set(true);
		}
	}

	Output::Debug("Patch configuration: dynrpg={} maniac={} key-patch={} common-this={} pic-unlock={} 2k3-commands={}",
		Player::IsPatchDynRpg(), Player::IsPatchManiac(), Player::IsPatchKeyPatch(), game_config.patch_common_this_event.Get(), game_config.patch_unlock_pics.Get(), game_config.patch_rpg2k3_commands.Get());

	ResetGameObjects();

	LoadFonts();

	if (Player::IsPatchKeyPatch()) {
		Main_Data::game_ineluki->ExecuteScriptList(FileFinder::Game().FindFile("autorun.script"));
	}
}

bool Player::ChangeResolution(int width, int height) {
	if (!DisplayUi->ChangeDisplaySurfaceResolution(width, height)) {
		Output::Warning("Resolution change to {}x{} failed", width, height);
		return false;
	}

	Player::screen_width = width;
	Player::screen_height = height;
	Player::menu_offset_x = std::max<int>((Player::screen_width - MENU_WIDTH) / 2, 0);
	Player::menu_offset_y = std::max<int>((Player::screen_height - MENU_HEIGHT) / 2, 0);
	Player::message_box_offset_x = std::max<int>((Player::screen_width - MENU_WIDTH) / 2, 0);

	Graphics::GetMessageOverlay().OnResolutionChange();

	if (Main_Data::game_quit) {
		Main_Data::game_quit->OnResolutionChange();
	}

	Output::Debug("Resolution changed to {}x{}", width, height);
	return true;
}

void Player::RestoreBaseResolution() {
	if (Player::screen_width == SCREEN_TARGET_WIDTH && Player::screen_height == SCREEN_TARGET_HEIGHT) {
		return;
	}

	if (!Player::ChangeResolution(SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT)) {
		// Considering that this is the base resolution this should never fail
		Output::Error("Failed restoring base resolution");
	}
}

void Player::ResetGameObjects() {
	// The init order is important
	Main_Data::Cleanup();

	Main_Data::game_switches = std::make_unique<Game_Switches>();
	Main_Data::game_switches->SetLowerLimit(lcf::Data::switches.size());

	auto min_var = lcf::Data::system.easyrpg_variable_min_value;
	if (min_var == 0) {
		if (Player::IsPatchManiac()) {
			min_var = std::numeric_limits<Game_Variables::Var_t>::min();
		} else {
			min_var = Player::IsRPG2k3() ? Game_Variables::min_2k3 : Game_Variables::min_2k;
		}
	}
	auto max_var = lcf::Data::system.easyrpg_variable_max_value;
	if (max_var == 0) {
		if (Player::IsPatchManiac()) {
			max_var = std::numeric_limits<Game_Variables::Var_t>::max();
		} else {
			max_var = Player::IsRPG2k3() ? Game_Variables::max_2k3 : Game_Variables::max_2k;
		}
	}
	Main_Data::game_variables = std::make_unique<Game_Variables>(min_var, max_var);
	Main_Data::game_variables->SetLowerLimit(lcf::Data::variables.size());

	// Prevent a crash when Game_Map wants to reset the screen content
	// because Setup() modified pictures array
	Main_Data::game_screen = std::make_unique<Game_Screen>();
	Main_Data::game_pictures = std::make_unique<Game_Pictures>();
	Main_Data::game_windows = std::make_unique<Game_Windows>();

	Main_Data::game_actors = std::make_unique<Game_Actors>();

	Game_Map::Init();

	Main_Data::game_system = std::make_unique<Game_System>();
	Main_Data::game_targets = std::make_unique<Game_Targets>();
	Main_Data::game_enemyparty = std::make_unique<Game_EnemyParty>();
	Main_Data::game_party = std::make_unique<Game_Party>();
	Main_Data::game_player = std::make_unique<Game_Player>();
	Main_Data::game_quit = std::make_unique<Game_Quit>();
	Main_Data::game_switches_global = std::make_unique<Game_Switches>();
	Main_Data::game_variables_global = std::make_unique<Game_Variables>(min_var, max_var);
	Main_Data::game_ineluki = std::make_unique<Game_Ineluki>();

	DynRpg::Reset();

	Game_Clock::ResetFrame(Game_Clock::now());

	Main_Data::game_system->ReloadSystemGraphic();

	Input::ResetMask();
}

static bool DefaultLmuStartFileExists(const FilesystemView& fs) {
	// Compute map_id based on command line.
	int map_id = Player::start_map_id == -1 ? lcf::Data::treemap.start.party_map_id : Player::start_map_id;
	std::string mapName = Game_Map::ConstructMapName(map_id, false);

	// Now see if the file exists.
	return !fs.FindFile(mapName).empty();
}

void Player::GuessNonStandardExtensions() {
	// Check all conditions, but check the remap last (since it is potentially slower).
	FileExtGuesser::RPG2KNonStandardFilenameGuesser rpg2kRemap;
	if (!FileFinder::IsRPG2kProject(FileFinder::Game()) &&
		!FileFinder::IsEasyRpgProject(FileFinder::Game())) {

		rpg2kRemap = FileExtGuesser::GetRPG2kProjectWithRenames(FileFinder::Game());
		if (rpg2kRemap.Empty()) {
			// Unlikely to happen because of the game browser only launches valid games
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
	std::string edb = FileFinder::Game().FindFile(DATABASE_NAME_EASYRPG);
	std::string emt = FileFinder::Game().FindFile(TREEMAP_NAME_EASYRPG);
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
		std::string edb = FileFinder::Game().FindFile(DATABASE_NAME_EASYRPG);
		auto edb_stream = FileFinder::Game().OpenInputStream(edb, std::ios_base::in);
		if (!edb_stream) {
			Output::Error("Error loading {}", DATABASE_NAME_EASYRPG);
			return;
		}

		auto db = lcf::LDB_Reader::LoadXml(edb_stream);
		if (!db) {
			Output::ErrorStr(lcf::LcfReader::GetError());
			return;
		} else {
			lcf::Data::data = std::move(*db);
		}

		std::string emt = FileFinder::Game().FindFile(TREEMAP_NAME_EASYRPG);
		auto emt_stream = FileFinder::Game().OpenInputStream(emt, std::ios_base::in);
		if (!emt_stream) {
			Output::Error("Error loading {}", TREEMAP_NAME_EASYRPG);
			return;
		}

		auto treemap = lcf::LMT_Reader::LoadXml(emt_stream);
		if (!treemap) {
			Output::ErrorStr(lcf::LcfReader::GetError());
		} else {
			lcf::Data::treemap = std::move(*treemap);
		}
	} else {
		// Retrieve the appropriately-renamed files.
		std::string ldb_name = fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LDB);
		std::string ldb = FileFinder::Game().FindFile(ldb_name);
		std::string lmt_name = fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LMT);
		std::string lmt = FileFinder::Game().FindFile(lmt_name);

		auto ldb_stream = FileFinder::Game().OpenInputStream(ldb);
		if (!ldb_stream) {
			Output::Error("Error loading {}", ldb_name);
			return;
		}

		auto db = lcf::LDB_Reader::Load(ldb_stream, encoding);
		if (!db) {
			Output::ErrorStr(lcf::LcfReader::GetError());
			return;
		} else {
			lcf::Data::data = std::move(*db);
		}

		auto lmt_stream = FileFinder::Game().OpenInputStream(lmt);
		if (!lmt_stream) {
			Output::Error("Error loading {}", lmt_name);
			return;
		}

		auto treemap = lcf::LMT_Reader::Load(lmt_stream, encoding);
		if (!treemap) {
			Output::ErrorStr(lcf::LcfReader::GetError());
			return;
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
		if (!DefaultLmuStartFileExists(FileFinder::Game())) {
			FileExtGuesser::GuessAndAddLmuExtension(FileFinder::Game(), *meta, fileext_map);
		}
	}
}

void Player::LoadFonts() {
	Font::ResetDefault();
	Font::ResetNameText();

#ifdef HAVE_FREETYPE
	// Look for bundled fonts
	auto gothic = FileFinder::OpenFont("Font");
	if (gothic) {
		Font::SetDefault(Font::CreateFtFont(std::move(gothic), 12, false, false), false);
	}

	auto mincho = FileFinder::OpenFont("Font2");
	if (mincho) {
		Font::SetDefault(Font::CreateFtFont(std::move(mincho), 12, false, false), true);
	}

	auto name_text = FileFinder::OpenFont("NameText");
	if (name_text) {
		Font::SetNameText(Font::CreateFtFont(std::move(name_text), 11, false, false), false);
	}

	auto name_text_2 = FileFinder::OpenFont("NameText2");
	if (name_text_2) {
		Font::SetNameText(Font::CreateFtFont(std::move(name_text_2), 13, false, false), true);
	}
#endif
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
	Output::Debug("Loading Save {}", save_name);

	bool load_on_map = Scene::instance->type == Scene::Map;

	if (!load_on_map) {
		Main_Data::game_system->BgmFade(800);
		// We erase the screen now before loading the saved game. This prevents an issue where
		// if the save game has a different system graphic, the load screen would change before
		// transitioning out.
		Transition::instance().InitErase(Transition::TransitionFadeOut, Scene::instance.get(), 6);
	}

	auto title_scene = Scene::Find(Scene::Title);
	if (title_scene) {
		static_cast<Scene_Title*>(title_scene.get())->OnGameStart();
	}

	auto save_stream = FileFinder::Save().OpenInputStream(save_name);
	if (!save_stream) {
		Output::Error("Error loading {}", save_name);
		return;
	}

	std::unique_ptr<lcf::rpg::Save> save = lcf::LSD_Reader::Load(save_stream, encoding);

	if (!save.get()) {
		Output::ErrorStr(lcf::LcfReader::GetError());
		return;
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
			verstr.str(), Version::STRING);
	}

	// Compatibility hacks for old EasyRPG Player saves.
	if (save->easyrpg_data.version == 0) {
		// Old savegames accidentally wrote animation_type as continuous for all events.
		save->party_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->boat_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->ship_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
		save->airship_location.animation_type = Game_Character::AnimType::AnimType_non_continuous;
	}

	if (!load_on_map) {
		Scene::PopUntil(Scene::Title);
	}
	Game_Map::Dispose();

	Main_Data::game_switches->SetLowerLimit(lcf::Data::switches.size());
	Main_Data::game_switches->SetData(std::move(save->system.switches));
	Main_Data::game_variables->SetLowerLimit(lcf::Data::variables.size());
	Main_Data::game_variables->SetData(std::move(save->system.variables));
	Main_Data::game_system->SetupFromSave(std::move(save->system));
	Main_Data::game_actors->SetSaveData(std::move(save->actors));
	Main_Data::game_party->SetupFromSave(std::move(save->inventory));
	Main_Data::game_screen->SetSaveData(std::move(save->screen));
	Main_Data::game_pictures->SetSaveData(std::move(save->pictures));
	Main_Data::game_targets->SetSaveData(std::move(save->targets));
	Main_Data::game_player->SetSaveData(save->party_location);
	Main_Data::game_windows->SetSaveData(std::move(save->easyrpg_data.windows));

	int map_id = Main_Data::game_player->GetMapId();

	FileRequestAsync* map = Game_Map::RequestMap(map_id);
	save_request_id = map->Bind([save=std::move(*save)](auto* request) { OnMapSaveFileReady(request, std::move(save)); });
	map->SetImportantFile(true);

	Main_Data::game_system->ReloadSystemGraphic();

	map->Start();
	if (!load_on_map) {
		Scene::Push(std::make_shared<Scene_Map>(save_id));
	} else {
		// Increment frame counter for consistency with a normal savegame load
		IncFrame();
		static_cast<Scene_Map*>(Scene::instance.get())->StartFromSave(save_id);
	}
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
	Main_Data::game_system->BgmFade(800, true);
	Main_Data::game_system->ResetFrameCounter();
	auto title = Scene::Find(Scene::Title);
	if (title) {
		static_cast<Scene_Title*>(title.get())->OnGameStart();
	}

	Main_Data::game_system->SetAtbMode(static_cast<Game_System::AtbMode>(lcf::Data::battlecommands.easyrpg_default_atb_mode));

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
				args.troop_id, args.background, static_cast<int>(args.formation), static_cast<int>(args.condition), args.terrain_id);
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
		std::string ini = FileFinder::Game().FindFile(INI_NAME);
		auto ini_stream = FileFinder::Game().OpenInputStream(ini);
		if (ini_stream) {
			encoding = lcf::ReaderUtil::GetEncoding(ini_stream);
		}
	}

	if (encoding.empty() || encoding == "auto") {
		encoding = "";

		std::string ldb = FileFinder::Game().FindFile(fileext_map.MakeFilename(RPG_RT_PREFIX, SUFFIX_LDB));
		auto ldb_stream = FileFinder::Game().OpenInputStream(ldb);
		if (ldb_stream) {
			auto db = lcf::LDB_Reader::Load(ldb_stream);
			if (db) {
				std::vector<std::string> encodings = lcf::ReaderUtil::DetectEncodings(*db);

#ifndef EMSCRIPTEN
				for (std::string &enc : encodings) {
					// Heuristic: Check title graphic, system graphic, cursor SE, title BGM
					// Pure ASCII is skipped as it provides no added value
					escape_symbol = lcf::ReaderUtil::Recode("\\", enc);
					if (escape_symbol.empty()) {
						// Bad encoding
						Output::Debug("Bad encoding: {}. Trying next.", enc);
						continue;
					}
					escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();

					const auto& title_name = db->system.title_name;
					const auto& system_name = db->system.system_name;
					const auto& cursor_se = db->system.cursor_se.name;
					const auto& title_music = db->system.title_music.name;
					int check_max = 0;
					int check_okay = 0;

					if (db->system.show_title && !Utils::StringIsAscii(title_name)) {
						++check_max;
						check_okay += FileFinder::FindImage("Title", lcf::ReaderUtil::Recode(title_name, enc)).empty() ? 0 : 1;
					}

					if (!Utils::StringIsAscii(system_name)) {
						++check_max;
						check_okay += FileFinder::FindImage("System", lcf::ReaderUtil::Recode(system_name, enc)).empty() ? 0 : 1;
					}

					if (!Utils::StringIsAscii(cursor_se)) {
						++check_max;
						check_okay += FileFinder::FindSound(lcf::ReaderUtil::Recode(cursor_se, enc)).empty() ? 0 : 1;
					}

					if (db->system.show_title && !Utils::StringIsAscii(title_music)) {
						++check_max;
						check_okay += FileFinder::FindMusic(lcf::ReaderUtil::Recode(title_music, enc)).empty() ? 0 : 1;
					}

					if (check_max == check_okay) {
						// Looks like a good encoding
						encoding = enc;
						break;
					} else {
						Output::Debug("Detected encoding: {}. Files not found ({}/{}). Trying next.", enc, check_okay, check_max);
					}
				}
#endif
				if (!encodings.empty() && encoding.empty()) {
					// No encoding found that matches the files, maybe RTP missing.
					// Use the first one instead
					encoding = encodings.front();
				}
			}
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

std::string Player::GetFullVersionString() {
	return std::string(GAME_TITLE) + " " + Version::GetVersionString();
}

void Player::PrintUsage() {
	std::cout <<
R"(EasyRPG Player - An open source interpreter for RPG Maker 2000/2003 games.

Engine options:
 --autobattle-algo A  Which AutoBattle algorithm to use.
                      Options:
                       RPG_RT  - The default RPG_RT compatible algo, including
                                 RPG_RT bugs.
                       RPG_RT+ - The default RPG_RT compatible algo, with bug-
                                 fixes.
                       ATTACK  - Like RPG_RT+ but only physical attacks, no
                                 skills.
 -c, --config-path P  Set a custom configuration path. When not specified, the
                      configuration folder in the users home directory is used.
 --encoding N         Instead of autodetecting the encoding or using the one in
                      RPG_RT.ini, the encoding N is used.
 --enemyai-algo A     Which EnemyAI algorithm to use.
                      Options:
                       RPG_RT  - The default RPG_RT compatible algo, including
                                 RPG_RT bugs.
                       RPG_RT+ - The default RPG_RT compatible algo, with bug-
                                 fixes.
 --engine ENGINE      Disable auto detection of the simulated engine.
                      Options:
                       rpg2k      - RPG Maker 2000 (v1.00 - v1.10)
                       rpg2kv150  - RPG Maker 2000 (v1.50 - v1.51)
                       rpg2ke     - RPG Maker 2000 (English release, v1.61)
                       rpg2k3     - RPG Maker 2003 (v1.00 - v1.04)
                       rpg2k3v105 - RPG Maker 2003 (v1.05 - v1.09a)
                       rpg2k3e    - RPG Maker 2003 (English release, v1.12)
 --language LANG      Load the game translation in language/LANG folder.
 --load-game-id N     Skip the title scene and load SaveN.lsd (N is padded to
                      two digits).
 --new-game           Skip the title scene and start a new game directly.
 --no-log-color       Disable colors in terminal log.
 --no-rtp             Disable support for the Runtime Package (RTP).
 --patch PATCH...     Instead of autodetecting patches used by this game, force
                      emulation of certain patches.
                      Options:
                       common-this - "This Event" in common events
                       dynrpg      - DynRPG patch by Cherry
                       key-patch   - Key Patch by Ineluki
                       maniac      - Maniac Patch by BingShan
                       pic-unlock  - Pictures are not blocked by messages
                       rpg2k3-cmds - Support all RPG Maker 2003 event commands
                                     in any version of the engine
 --no-patch           Disable all engine patches.
 --project-path PATH  Instead of using the working directory, the game in PATH
                      is used.
 --record-input FILE  Record all button inputs to FILE.
 --replay-input FILE  Replays button presses from an input log generated by
                      --record-input.
 --rtp-path PATH      Add PATH to the RTP directory list and use this one with
                      highest precedence.
 --save-path PATH     Instead of storing save files in the game directory,
                      store them in PATH. When using the game browser all games
                      will share the same save directory!
 --seed N             Seeds the random number generator with N.

Video options:
 --fps-limit          In combination with --no-vsync sets a custom frames per
                      second limit. The default is 60 FPS. Use --no-fps-limit
                      to run with unlimited frames per second.
 --fps-render-window  Render the frames per second counter in both fullscreen
                      and windowed mode.
                      Disable with --no-fps-render-window.
 --fullscreen         Start in fullscreen mode.
 --game-resolution R  Force a different game resolution. This is experimental
                      and can cause glitches or break games!
                      Options:
                       original   - 320x240 (4:3). Recommended
                       widescreen - 416x240 (16:9)
                       ultrawide  - 560x240 (21:9)
 --scaling S          How the video output is scaled.
                      Options:
                       nearest  - Scale to screen size. Fast, but causes scaling
                                  artifacts.
                       integer  - Scales to a multiple of the game resolution.
                       bilinear - Like nearest, but applies a bilinear filter to
                                  avoid artifacts.
 --show-fps           Enable display of the frames per second counter.
                      Disable with --no-show-fps.
 --stretch            Ignore the aspect ratio and stretch video output to the
                      entire width of the screen.
                      Disable with --no-stretch.
 --vsync              Enables vertical sync if supported on this platform.
                      Disable with --no-vsync.
 --window             Start in windowed mode.

Audio options:
 --no-audio           Disable audio (in case you prefer your own music).
 --music-volume V     Set volume of background music to V (0-100).
 --sound-volume V     Set volume of sound effects to V (0-100).
 --soundfont FILE     Soundfont in sf2 format to use when playing MIDI files.

Debug options:
 --battle-test N...   Start a battle test.
                      This option supports two modes:
                      Providing a single N sets the monster party.
                      Providing four N sets: monster party, formation,
                      condition and terrain ID.
 --hide-title         Hide the title background image and center the command
                      menu.
 --start-map-id N     Overwrite the map used for new games and use MapN.lmu
                      instead (N is padded to four digits).
                      Incompatible with --load-game-id.
 --start-party A B... Overwrite the starting party members with the actors with
                      IDs A, B, C...
                      Incompatible with --load-game-id.
 --start-position X Y Overwrite the party start position and move the party to
                      position (X, Y).
                      Incompatible with --load-game-id.
 --test-play          Enable TestPlay (Debug) mode.

Other options:
 --server             Run dedicated server.
 -v, --version        Display program version and exit.
 -h, --help           Display this help and exit.

For compatibility with the legacy RPG Maker runtime the following arguments
are supported:
 BattleTest N         Same as --battle-test.
                      The argument list starts at the 4th argument.
 HideTitle            Same as --hide-title.
 TestPlay             Same as --test-play.
 Window               Same as --window.

Game related parameters (e.g. new-game and load-game-id) do not work correctly
when the startup directory does not contain a valid game (and the game browser
loads)

Alex, EV0001 and the EasyRPG authors wish you a lot of fun!)" << std::endl;
}

bool Player::IsCP932() {
	if (Tr::HasActiveTranslation() && !Tr::GetCurrentLanguageCode().empty()) {
		return Tr::GetCurrentLanguageCode() == "ja_JP";
	}

	return (encoding == "ibm-943_P15A-2003" || encoding == "932");
}

bool Player::IsCP949() {
	if (Tr::HasActiveTranslation() && !Tr::GetCurrentLanguageCode().empty()) {
		return Tr::GetCurrentLanguageCode() == "ko_KR";
	}

	return (encoding == "windows-949-2000" || encoding == "windows-949" || encoding == "949");
}

bool Player::IsBig5() {
	if (Tr::HasActiveTranslation() && !Tr::GetCurrentLanguageCode().empty()) {
		return Tr::GetCurrentLanguageCode() == "zh_TW";
	}

	return (encoding == "Big5" || encoding == "950");
}

bool Player::IsCP936() {
	if (Tr::HasActiveTranslation() && !Tr::GetCurrentLanguageCode().empty()) {
		return Tr::GetCurrentLanguageCode() == "zh_CN";
	}

	return (encoding == "windows-936-2000" || encoding == "windows-936" || encoding == "936");
}

bool Player::IsCJK() {
	return (IsCP932() || IsCP949() || IsBig5() || IsCP936());
}

bool Player::IsCP1251() {
	if (Tr::HasActiveTranslation() && !Tr::GetCurrentLanguageCode().empty()) {
		return Tr::GetCurrentLanguageCode() == "ru_RU";
	}

	return (encoding == "ibm-5347_P100-1998" || encoding == "windows-1251" || encoding == "1251");
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

