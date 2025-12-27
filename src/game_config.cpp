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

#include "game_config.h"
#include "cmdline_parser.h"
#include "filefinder.h"
#include "filesystem_stream.h"
#include "input_buttons.h"
#include "keys.h"
#include "options.h"
#include "output.h"
#include "input.h"
#include "player.h"
#include <lcf/inireader.h>
#include <cstring>

#ifdef _WIN32
#  include <shlobj.h>
#endif

#ifdef USE_LIBRETRO
#   include "platform/libretro/ui.h"
#endif

namespace {
	std::string config_path;
	std::string soundfont_path;
	std::string font_path;

	struct {
		bool started = false;
		std::string path;
		Filesystem_Stream::OutputStream handle;
	} logging;

#if USE_SDL == 1
	// For SDL1 hardcode a different config file because it uses a completely different mapping for gamepads
	std::string_view config_name = "config_sdl1.ini";
#else
	std::string_view config_name = EASYRPG_CONFIG_NAME;
#endif
}

void Game_ConfigPlayer::Hide() {
#if !defined(HAVE_FREETYPE) || defined(__ANDROID__)
	// FIXME (Android): URI encoded SAF paths are not supported
	font1.SetOptionVisible(false);
	font1_size.SetOptionVisible(false);
	font2.SetOptionVisible(false);
	font2_size.SetOptionVisible(false);
#endif
	if (automatic_screenshots.IsOptionVisible()) {
		automatic_screenshots_interval.SetLocked(!automatic_screenshots.Get());
	}
}

void Game_ConfigVideo::Hide() {
	// Options that are platform dependent are opt-in
	// Implementors must invoke SetOptionVisible() when supported

	// Always enabled by default:
	// - renderer (name of the renderer)

	vsync.SetOptionVisible(false);
	fullscreen.SetOptionVisible(false);
	fps_limit.SetOptionVisible(false);
	window_zoom.SetOptionVisible(false);
	scaling_mode.SetOptionVisible(false);
	stretch.SetOptionVisible(false);
	touch_ui.SetOptionVisible(false);
	pause_when_focus_lost.SetOptionVisible(false);
	game_resolution.SetOptionVisible(false);
	screen_scale.SetOptionVisible(false);
}

void Game_ConfigAudio::Hide() {
	// Music and SE volume control are opt-out
	// Nothing else available currently
}

void Game_ConfigInput::Hide() {
	// These features are handled by our input system but showing them only
	// makes sense on hardware with the required sticks and buttons
	gamepad_swap_ab_and_xy.SetOptionVisible(false);
	gamepad_swap_analog.SetOptionVisible(false);
	gamepad_swap_dpad_with_buttons.SetOptionVisible(false);
}

Game_Config Game_Config::Create(CmdlineParser& cp) {
	Game_Config cfg;

	// Set platform specific defaults
#if USE_SDL >= 2
	cfg.video.scaling_mode.Set(ConfigEnum::ScalingMode::Bilinear);
#endif

#if defined(__WIIU__)
	cfg.input.gamepad_swap_ab_and_xy.Set(true);
#endif

#if defined(USE_CUSTOM_FILEBUF) || defined(USE_LIBRETRO)
	// Disable logging by default on
	// - platforms with slow IO or bad FS drivers
	// - libretro because the frontend handles the logging
	cfg.player.log_enabled.Set(false);
#endif

	cp.Rewind();

	config_path = GetConfigPath(cp);
	std::string config_file;
	if (!config_path.empty()) {
		config_file = FileFinder::MakePath(config_path, config_name);
	}
	else if (FileFinder::Root().Exists(config_name)) {
		config_file = ToString(config_name);
	}

	auto cli_config = FileFinder::Root().OpenOrCreateInputStream(config_file);
	if (!cli_config) {
		config_path.clear();
		auto global_config = GetGlobalConfigFileInput();
		if (global_config) {
			cfg.LoadFromStream(global_config);
		} else {
			// Game_Config only loads an empty layout
			cfg.input.buttons = Input::GetDefaultButtonMappings();
		}
	} else {
		cfg.LoadFromStream(cli_config);
	}

	cp.Rewind();
	cfg.LoadFromArgs(cp);

	return cfg;
}

FilesystemView Game_Config::GetGlobalConfigFilesystem() {
	// FIXME: Game specific configs?
	std::string path;

	if (config_path.empty()) {
#ifdef __wii__
		path = "/data/easyrpg-player";
#elif defined(__WIIU__)
		path = "fs:/vol/external01/wiiu/data/easyrpg-player";
#elif defined(__SWITCH__)
		path = "/switch/easyrpg-player";
#elif defined(__3DS__)
		path = "sdmc:/data/easyrpg-player";
#elif defined(__vita__)
		path = "ux0:/data/easyrpg-player";
#elif defined(__PS4__)
		path = "/data/easyrpg-player/";
#elif defined(USE_LIBRETRO)
		const char* dir = nullptr;
		if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir) {
			path = FileFinder::MakePath(dir, "easyrpg-player");
		}
#elif defined(__ANDROID__)
		// Never called, passed as argument on startup
#elif defined(_WIN32)
		PWSTR knownPath;
		const auto hresult = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &knownPath);
		if (SUCCEEDED(hresult)) {
			path = Utils::FromWideString(knownPath);
			CoTaskMemFree(knownPath);
		} else {
			Output::Debug("Config: SHGetKnownFolderPath failed");
		}

		if (!path.empty()) {
			path = FileFinder::MakePath(path, FileFinder::MakePath(ORGANIZATION_NAME, APPLICATION_NAME));
		}
#else
		char* home = getenv("XDG_CONFIG_HOME");
		if (home) {
			path = home;
		} else {
			home = getenv("HOME");
			if (home) {
				path = FileFinder::MakePath(home, ".config");
			}
		}

		if (!path.empty()) {
			path = FileFinder::MakePath(path, FileFinder::MakePath(ORGANIZATION_NAME, APPLICATION_NAME));
		}
#endif
	} else {
		path = config_path;
	}

	auto print_err = [&path]() {
		if (path.empty()) {
			Output::Warning("Could not determine config path");
		} else {
			Output::Warning("Could not access config path {}", path);
		}
	};

	if (path.empty()) {
		print_err();
		return {};
	}

	if (!FileFinder::Root().MakeDirectory(path, true)) {
		print_err();
		return {};
	}

	auto fs = FileFinder::Root().Create(path);

	if (!fs) {
		print_err();
		return {};
	}

	return fs;
}

Filesystem_Stream::InputStream Game_Config::GetGlobalConfigFileInput() {
	auto fs = GetGlobalConfigFilesystem();

	if (fs) {
		return fs.OpenOrCreateInputStream(config_name, std::ios_base::in);
	}

	return Filesystem_Stream::InputStream();
}

FilesystemView Game_Config::GetSoundfontFilesystem() {
	std::string path = soundfont_path;
	if (path.empty()) {
		path = FileFinder::MakePath(GetGlobalConfigFilesystem().GetFullPath(), "Soundfont");
	}

	if (!FileFinder::Root().MakeDirectory(path, true)) {
		Output::Warning("Could not create soundfont path {}", path);
		return {};
	}

	return FileFinder::Root().Create(path);
}


FilesystemView Game_Config::GetFontFilesystem() {
	std::string path = font_path;
	if (path.empty()) {
		path = FileFinder::MakePath(GetGlobalConfigFilesystem().GetFullPath(), "Font");
	}

	if (!FileFinder::Root().MakeDirectory(path, true)) {
		Output::Warning("Could not create fount path {}", path);
		return {};
	}

	return FileFinder::Root().Create(path);
}

Filesystem_Stream::OutputStream Game_Config::GetGlobalConfigFileOutput() {
	auto fs = GetGlobalConfigFilesystem();

	if (fs) {
		return fs.OpenOutputStream(config_name, std::ios_base::out);
	}

	return Filesystem_Stream::OutputStream();
}

Filesystem_Stream::OutputStream& Game_Config::GetLogFileOutput() {
	// Invalid stream that consumes the output when logging is disabled or an error occurs
	static Filesystem_Stream::OutputStream noop_stream;

	if (!Player::player_config.log_enabled.Get()) {
		return noop_stream;
	}

	if (!logging.started) {
		logging.started = true;

		std::string path;

		if (logging.path.empty()) {
	#if defined(_WIN32)
			PWSTR knownPath;
			const auto hresult = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &knownPath);
			if (SUCCEEDED(hresult)) {
				path = Utils::FromWideString(knownPath);
				CoTaskMemFree(knownPath);
			} else {
				Output::Debug("LogFile: SHGetKnownFolderPath failed");
			}
	#elif defined(SYSTEM_DESKTOP_LINUX_BSD_MACOS)
			char* home = getenv("XDG_STATE_HOME");
			if (home) {
				path = home;
			} else {
				home = getenv("HOME");
				if (home) {
					path = FileFinder::MakePath(home, ".local/state");
				}
			}
	#endif

			if (path.empty()) {
				// Fallback: Use the config directory
				// Can still fail in the rare case that the config path is invalid
				if (auto fs = GetGlobalConfigFilesystem(); fs) {
					path = fs.GetFullPath();
				}
			}

			if (!path.empty()) {
				path = FileFinder::MakePath(path, OUTPUT_FILENAME);
			}
		} else {
			path = logging.path;
		}

		auto print_err = [&path]() {
			if (path.empty()) {
				Output::Warning("Could not determine logfile path");
			} else {
				Output::Warning("Could not access logfile path {}", path);
			}
		};

		if (path.empty()) {
			print_err();
			return noop_stream;
		}

#ifndef ANDROID
		// Make Directory not supported on Android, assume the path exists
		if (!FileFinder::Root().MakeDirectory(FileFinder::GetPathAndFilename(path).first, true)) {
			print_err();
			return noop_stream;
		}
#endif

		logging.handle = FileFinder::Root().OpenOutputStream(path, std::ios_base::out | std::ios_base::app);

		if (!logging.handle) {
			Output::Warning("Could not open logfile {}", path);
			return logging.handle;
		}

		logging.path = path;
	}

	return logging.handle;
}

void Game_Config::CloseLogFile() {
	if (!Game_Config::GetLogFileOutput()) {
		return;
	}

	Game_Config::GetLogFileOutput().Close();

	// Truncate the logfile when it is too large
	const std::streamoff log_size = 1024 * 1024; // 1 MB
	std::vector<char> buf(log_size);

	auto in = FileFinder::Root().OpenInputStream(logging.path);
	if (in) {
		in.seekg(0, std::ios_base::end);
		if (in.tellg() > log_size) {
			in.seekg(-log_size, std::ios_base::end);
			// skip current incomplete line
			std::string line;
			Utils::ReadLine(in, line);

			// Read the remaining logfile into the buffer
			in.read(buf.data(), buf.size());
			size_t read = in.gcount();
			in.Close();

			// Truncate the logfile and write the data into the logfile
			auto out = FileFinder::Root().OpenOutputStream(logging.path);
			if (out) {
				out.write(buf.data(), read);
			}
		}
	}

	logging.started = false;
	logging.handle = Filesystem_Stream::OutputStream();
}

std::string Game_Config::GetConfigPath(CmdlineParser& cp) {
	std::string path;

	while (!cp.Done()) {
		CmdlineArg arg;
		if (cp.ParseNext(arg, 1, "--config-path", 'c')) {
			if (arg.NumValues() > 0) {
				path = arg.Value(0);
				path = FileFinder::MakeCanonical(path, 0);
			}
			continue;
		}

		cp.SkipNext();
	}

	if (!path.empty()) {
		if (!FileFinder::Root().MakeDirectory(path, true)) {
			Output::Debug("Could not create global config directory {}", path);
			path.clear();
		}
	}

	return path;
}

void Game_Config::LoadFromArgs(CmdlineParser& cp) {
	font_path.clear();
	soundfont_path.clear();

	while (!cp.Done()) {
		CmdlineArg arg;
		long li_value = 0;
		std::string str_value;

		if (cp.ParseNext(arg, 0, {"--vsync", "--no-vsync"})) {
			video.vsync.Set(arg.ArgIsOn());
			continue;
		}
		if (cp.ParseNext(arg, 1, "--fps-limit")) {
			if (arg.ParseValue(0, li_value)) {
				video.fps_limit.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-fps-limit")) {
			video.fps_limit.Set(0);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--show-fps")) {
			video.fps.Set(ConfigEnum::ShowFps::ON);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-show-fps")) {
			video.fps.Set(ConfigEnum::ShowFps::OFF);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--fps-render-window")) {
			video.fps.Set(ConfigEnum::ShowFps::Overlay);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--pause-focus-lost")) {
			video.pause_when_focus_lost.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-pause-focus-lost")) {
			video.pause_when_focus_lost.Set(false);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--window")) {
			video.fullscreen.Set(false);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--fullscreen")) {
			video.fullscreen.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 1, "--window-zoom")) {
			if (arg.ParseValue(0, li_value)) {
				video.window_zoom.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, {"--stretch", "--no-stretch"})) {
			video.stretch.Set(arg.ArgIsOn());
			continue;
		}
		if (cp.ParseNext(arg, 1, "--scaling")) {
			if (arg.ParseValue(0, str_value)) {
				video.scaling_mode.SetFromString(str_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--game-resolution")) {
			if (arg.ParseValue(0, str_value)) {
				video.game_resolution.SetFromString(str_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--autobattle-algo")) {
			std::string svalue;
			if (arg.ParseValue(0, svalue)) {
				player.autobattle_algo.Set(std::move(svalue));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--enemyai-algo")) {
			std::string svalue;
			if (arg.ParseValue(0, svalue)) {
				player.enemyai_algo.Set(std::move(svalue));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--music-volume")) {
			if (arg.ParseValue(0, li_value)) {
				audio.music_volume.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--sound-volume")) {
			if (arg.ParseValue(0, li_value)) {
				audio.sound_volume.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--soundfont")) {
			if (arg.NumValues() > 0) {
				audio.soundfont.Set(arg.Value(0));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--font1")) {
			if (arg.NumValues() > 0) {
				player.font1.Set(FileFinder::MakeCanonical(arg.Value(0), 0));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--font1-size")) {
			if (arg.ParseValue(0, li_value)) {
				player.font1_size.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--font2")) {
			if (arg.NumValues() > 0) {
				player.font2.Set(FileFinder::MakeCanonical(arg.Value(0), 0));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--font2-size")) {
			if (arg.ParseValue(0, li_value)) {
				player.font2_size.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--soundfont-path")) {
			if (arg.NumValues() > 0) {
				soundfont_path = FileFinder::MakeCanonical(arg.Value(0), 0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--font-path")) {
			if (arg.NumValues() > 0) {
				font_path = FileFinder::MakeCanonical(arg.Value(0), 0);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--log-file")) {
			if (arg.NumValues() > 0) {
				logging.path = FileFinder::MakeCanonical(arg.Value(0), 0);
			}
			continue;
		}

		cp.SkipNext();
	}
}

void Game_Config::LoadFromStream(Filesystem_Stream::InputStream& is) {
	lcf::INIReader ini(is);

	if (ini.ParseError()) {
		Output::Debug("Failed to parse ini config file {}", is.GetName());
		return;
	}

	/** VIDEO SECTION */
	video.vsync.FromIni(ini);
	video.fullscreen.FromIni(ini);
	video.fps.FromIni(ini);
	video.fps_limit.FromIni(ini);
	video.window_zoom.FromIni(ini);
	video.scaling_mode.FromIni(ini);
	video.stretch.FromIni(ini);
	video.touch_ui.FromIni(ini);
	video.pause_when_focus_lost.FromIni(ini);
	video.game_resolution.FromIni(ini);
	video.screen_scale.FromIni(ini);

	if (ini.HasValue("Video", "WindowX") && ini.HasValue("Video", "WindowY") && ini.HasValue("Video", "WindowWidth") && ini.HasValue("Video", "WindowHeight")) {
		video.window_x.FromIni(ini);
		video.window_y.FromIni(ini);
		video.window_width.FromIni(ini);
		video.window_height.FromIni(ini);
	}

	/** AUDIO SECTION */
	audio.music_volume.FromIni(ini);
	audio.sound_volume.FromIni(ini);
	audio.fluidsynth_midi.FromIni(ini);
	audio.wildmidi_midi.FromIni(ini);
	audio.native_midi.FromIni(ini);
	audio.soundfont.FromIni(ini);

	/** INPUT SECTION */
	input.buttons = Input::GetDefaultButtonMappings();
	auto& mappings = input.buttons;

	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		auto name = Input::kInputButtonNames.tag(button);
		if (ini.HasValue("input", name)) {
			auto values = ini.GetString("input", name, "");
			mappings.RemoveAll(button);

			auto keys = Utils::Tokenize(values, [](char32_t c) { return c == ','; });

			// When it is a protected (important) button with zero mappings keep the default
			// For all other buttons having no mapping is fine
			bool has_mapping = false;
			if (Input::IsProtectedButton(button)) {
				// Check for protected (important) buttons if they have more than zero mappings
				for (const auto& key: keys) {
					Input::Keys::InputKey k;
					if (Input::Keys::kInputKeyNames.etag(key.c_str(), k)) {
						has_mapping = true;
						break;
					}
				}

				// If not, keep the default mapping
				if (!has_mapping) {
					continue;
				}
			}

			// Load mappings from ini
			for (const auto& key: keys) {
				Input::Keys::InputKey k;
				if (Input::Keys::kInputKeyNames.etag(key.c_str(), k)) {
					mappings.Add({button, k});
				}
			}
		}
	}

	input.gamepad_swap_analog.FromIni(ini);
	input.gamepad_swap_dpad_with_buttons.FromIni(ini);
	input.gamepad_swap_ab_and_xy.FromIni(ini);
	input.speed_modifier_a.FromIni(ini);
	input.speed_modifier_b.FromIni(ini);

	/** PLAYER SECTION */
	player.settings_autosave.FromIni(ini);
	player.settings_in_title.FromIni(ini);
	player.settings_in_menu.FromIni(ini);
	player.lang_select_on_start.FromIni(ini);
	player.lang_select_in_title.FromIni(ini);
	player.show_startup_logos.FromIni(ini);
	player.font1.FromIni(ini);
	player.font1_size.FromIni(ini);
	player.font2.FromIni(ini);
	player.font2_size.FromIni(ini);
	player.log_enabled.FromIni(ini);
	player.screenshot_scale.FromIni(ini);
	player.screenshot_timestamp.FromIni(ini);
	player.automatic_screenshots.FromIni(ini);
	player.automatic_screenshots_interval.FromIni(ini);
	player.prefer_easyrpg_map_files.FromIni(ini);
}

void Game_Config::WriteToStream(Filesystem_Stream::OutputStream& os) const {
	/** VIDEO SECTION */

	os << "[Video]\n";
	video.vsync.ToIni(os);
	video.fullscreen.ToIni(os);
	video.fps.ToIni(os);
	video.fps_limit.ToIni(os);
	video.window_zoom.ToIni(os);
	video.scaling_mode.ToIni(os);
	video.stretch.ToIni(os);
	video.touch_ui.ToIni(os);
	video.pause_when_focus_lost.ToIni(os);
	video.game_resolution.ToIni(os);
	video.screen_scale.ToIni(os);

	// only preserve when toggling between window and fullscreen is supported
	if (video.fullscreen.IsOptionVisible()) {
		video.window_x.ToIni(os);
		video.window_y.ToIni(os);
		video.window_width.ToIni(os);
		video.window_height.ToIni(os);
	}
	os << "\n";

	/** AUDIO SECTION */
	os << "[Audio]\n";

	audio.music_volume.ToIni(os);
	audio.sound_volume.ToIni(os);
	audio.fluidsynth_midi.ToIni(os);
	audio.wildmidi_midi.ToIni(os);
	audio.native_midi.ToIni(os);
	audio.soundfont.ToIni(os);

	os << "\n";

	/** INPUT SECTION */
	os << "[Input]\n";

	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		auto name = Input::kInputButtonNames.tag(button);
		os << name << "=";

		std::stringstream ss;
		bool first = true;
		for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button;++ki) {
			if (!first) {
				os << ",";
			}
			first = false;

			auto key = static_cast<Input::Keys::InputKey>(ki->second);
			auto kname = Input::Keys::kInputKeyNames.tag(key);
			os << kname;
		}

		os << "\n";
	}

	input.gamepad_swap_analog.ToIni(os);
	input.gamepad_swap_dpad_with_buttons.ToIni(os);
	input.gamepad_swap_ab_and_xy.ToIni(os);
	input.speed_modifier_a.ToIni(os);
	input.speed_modifier_b.ToIni(os);

	os << "\n";

	/** PLAYER SECTION */
	os << "[Player]\n";
	//os << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	//os << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";

	player.settings_autosave.ToIni(os);
	player.settings_in_title.ToIni(os);
	player.settings_in_menu.ToIni(os);
	player.lang_select_on_start.ToIni(os);
	player.lang_select_in_title.ToIni(os);
	player.show_startup_logos.ToIni(os);
	player.font1.ToIni(os);
	player.font1_size.ToIni(os);
	player.font2.ToIni(os);
	player.font2_size.ToIni(os);
	player.log_enabled.ToIni(os);
	player.screenshot_scale.ToIni(os);
	player.screenshot_timestamp.ToIni(os);
	player.automatic_screenshots.ToIni(os);
	player.automatic_screenshots_interval.ToIni(os);
	player.prefer_easyrpg_map_files.ToIni(os);

	os << "\n";
}
