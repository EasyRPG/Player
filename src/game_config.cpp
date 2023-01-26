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
#include "input_buttons.h"
#include "output.h"
#include "input.h"
#include <lcf/inireader.h>
#include <cstring>

#ifdef _WIN32
#  include <shlobj.h>
#endif

#ifdef USE_LIBRETRO
#   include "platform/libretro/ui.h"
#endif

namespace {
	StringView config_name = "config.ini";
}

void Game_ConfigPlayer::Hide() {
	// Game specific settings unsupported
}

void Game_ConfigVideo::Hide() {
	// Options that are platform dependent are opt-in
	// Implementors must invoke SetOptionVisible() when supported

	// Always enabled by default:
	// - renderer (name of the renderer)
	// - show_fps (Rendering of FPS, engine feature)

	vsync.SetOptionVisible(false);
	fullscreen.SetOptionVisible(false);
	fps_limit.SetOptionVisible(false);
	fps_render_window.SetOptionVisible(false);
	window_zoom.SetOptionVisible(false);
	scaling_mode.SetOptionVisible(false);
	stretch.SetOptionVisible(false);
	touch_ui.SetOptionVisible(false);
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

#if USE_SDL >= 2
	cfg.video.scaling_mode.Set(ScalingMode::Bilinear);
#endif

	cp.Rewind();

	auto arg_path = GetConfigPath(cp);
	if (!arg_path.empty()) {
		arg_path = FileFinder::MakePath(arg_path, config_name);
	}

	auto cli_config = FileFinder::Root().OpenOrCreateInputStream(arg_path);
	if (!cli_config) {
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

#ifdef GEKKO
	path = "sd:/data/easyrpg-player";
#elif defined(__SWITCH__)
	path = "/switch/easyrpg-player";
#elif defined(__3DS__)
	path = "sdmc:/data/easyrpg-player";
#elif defined(__vita__)
	path = "ux0:/data/easyrpg-player";
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

Filesystem_Stream::OutputStream Game_Config::GetGlobalConfigFileOutput() {
	auto fs = GetGlobalConfigFilesystem();

	if (fs) {
		return fs.OpenOutputStream(config_name, std::ios_base::out);
	}

	return Filesystem_Stream::OutputStream();
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
	while (!cp.Done()) {
		CmdlineArg arg;
		long li_value = 0;
		if (cp.ParseNext(arg, 0, "--vsync")) {
			video.vsync.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-vsync")) {
			video.vsync.Set(false);
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
			video.show_fps.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-show-fps")) {
			video.show_fps.Set(false);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--fps-render-window")) {
			video.fps_render_window.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-fps-render-window")) {
			video.fps_render_window.Set(false);
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
	if (ini.HasValue("video", "vsync")) {
		video.vsync.Set(ini.GetBoolean("video", "vsync", false));
	}
	if (ini.HasValue("video", "fullscreen")) {
		video.fullscreen.Set(ini.GetBoolean("video", "fullscreen", false));
	}
	if (ini.HasValue("video", "show-fps")) {
		video.show_fps.Set(ini.GetBoolean("video", "show-fps", false));
	}
	if (ini.HasValue("video", "fps-render-window")) {
		video.fps_render_window.Set(ini.GetBoolean("video", "fps-render-window", false));
	}
	if (ini.HasValue("video", "fps-limit")) {
		video.fps_limit.Set(ini.GetInteger("video", "fps-limit", 0));
	}
	if (ini.HasValue("video", "window-zoom")) {
		video.window_zoom.Set(ini.GetInteger("video", "window-zoom", 0));
	}
	if (ini.HasValue("video", "scaling-mode")) {
		video.scaling_mode.Set(static_cast<ScalingMode>(ini.GetInteger("video", "scaling-mode", 0)));
	}
	if (ini.HasValue("video", "stretch")) {
		video.stretch.Set(ini.GetBoolean("video", "stretch", 0));
	}
	if (ini.HasValue("video", "touch-ui")) {
		video.touch_ui.Set(ini.GetBoolean("video", "touch-ui", 1));
	}
	if (ini.HasValue("video", "window-x") && ini.HasValue("video", "window-y") && ini.HasValue("video", "window-width") && ini.HasValue("video", "window-height")) {
		video.window_x.Set(ini.GetInteger("video", "window-x", 0));
		video.window_y.Set(ini.GetInteger("video", "window-y", 0));
		video.window_width.Set(ini.GetInteger("video", "window-width", SCREEN_TARGET_WIDTH));
		video.window_height.Set(ini.GetInteger("video", "window-height", SCREEN_TARGET_HEIGHT));
	}

	/** AUDIO SECTION */
	if (ini.HasValue("audio", "music-volume")) {
		audio.music_volume.Set(ini.GetInteger("audio", "music-volume", 100));
	}
	if (ini.HasValue("audio", "sound-volume")) {
		audio.sound_volume.Set(ini.GetInteger("audio", "sound-volume", 100));
	}

	/** INPUT SECTION */
	input.buttons = Input::GetDefaultButtonMappings();
	auto& mappings = input.buttons;

	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		auto name = Input::kButtonNames.tag(button);
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
					const auto& kNames = Input::Keys::kNames;
					auto it = std::find(kNames.begin(), kNames.end(), key);
					if (it != Input::Keys::kNames.end()) {
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
				const auto& kNames = Input::Keys::kNames;
				auto it = std::find(kNames.begin(), kNames.end(), key);
				if (it != Input::Keys::kNames.end()) {
					mappings.Add({button, static_cast<Input::Keys::InputKey>(std::distance(kNames.begin(), it))});
				}
			}
		}
	}

	if (ini.HasValue("input", "gamepad-swap-analog")) {
		input.gamepad_swap_analog.Set(ini.GetInteger("input", "gamepad-swap-analog", 0));
	}
	if (ini.HasValue("input", "gamepad-swap-dpad")) {
		input.gamepad_swap_dpad_with_buttons.Set(ini.GetInteger("input", "gamepad-swap-dpad", 0));
	}
	if (ini.HasValue("input", "gamepad-swap-abxy")) {
		input.gamepad_swap_ab_and_xy.Set(ini.GetInteger("input", "gamepad-swap-abxy", 0));
	}

	/** PLAYER SECTION */
	if (ini.HasValue("player", "settings-autosave")) {
		player.settings_autosave.Set(ini.GetBoolean("player", "settings-autosave", 0));
	}
	if (ini.HasValue("player", "settings-in-title")) {
		player.settings_in_title.Set(ini.GetBoolean("player", "settings-in-title", 0));
	}
	if (ini.HasValue("player", "settings-in-menu")) {
		player.settings_in_title.Set(ini.GetBoolean("player", "settings-in-menu", 0));
	}
	if (ini.HasValue("player", "autobattle-algo")) {
		player.autobattle_algo.Set(ini.GetString("player", "autobattle-algo", ""));
	}
	if (ini.HasValue("player", "enemyai-algo")) {
		player.enemyai_algo.Set(ini.GetString("player", "enemyai-algo", ""));
	}
}

void Game_Config::WriteToStream(Filesystem_Stream::OutputStream& os) const {
	/** VIDEO SECTION */

	os << "[video]\n";
	if (video.vsync.IsOptionVisible()) {
		os << "vsync=" << int(video.vsync.Get()) << "\n";
	}
	if (video.fullscreen.IsOptionVisible()) {
		os << "fullscreen=" << int(video.fullscreen.Get()) << "\n";
	}
	if (video.show_fps.IsOptionVisible()) {
		os << "show-fps=" << int(video.show_fps.Get()) << "\n";
	}
	if (video.fps_render_window.IsOptionVisible()) {
		os << "fps-render-window=" << int(video.fps_render_window.Get()) << "\n";
	}
	if (video.fps_limit.IsOptionVisible()) {
		os << "fps-limit=" << video.fps_limit.Get() << "\n";
	}
	if (video.window_zoom.IsOptionVisible()) {
		os << "window-zoom=" << video.window_zoom.Get() << "\n";
	}
	if (video.scaling_mode.IsOptionVisible()) {
		os << "scaling-mode=" << int(video.scaling_mode.Get()) << "\n";
	}
	if (video.stretch.IsOptionVisible()) {
		os << "stretch=" << int(video.stretch.Get()) << "\n";
	}
	if (video.touch_ui.IsOptionVisible()) {
		os << "touch-ui=" << int(video.touch_ui.Get()) << "\n";
	}
	// only preserve when toggling between window and fullscreen is supported
	if (video.fullscreen.IsOptionVisible()) {
		os << "window-x=" << int(video.window_x.Get()) << "\n";
		os << "window-y=" << int(video.window_y.Get()) << "\n";
		os << "window-width=" << int(video.window_width.Get()) << "\n";
		os << "window-height=" << int(video.window_height.Get()) << "\n";
	}
	os << "\n";

	/** AUDIO SECTION */
	os << "[audio]\n";

	if (audio.music_volume.IsOptionVisible()) {
		os << "music-volume=" << audio.music_volume.Get() << "\n";
	}
	if (audio.sound_volume.IsOptionVisible()) {
		os << "sound-volume=" << audio.sound_volume.Get() << "\n";
	}
	os << "\n";

	/** INPUT SECTION */
	os << "[input]\n";

	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		auto name = Input::kButtonNames.tag(button);
		os << name << "=";

		std::stringstream ss;
		bool first = true;
		for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button;++ki) {
			if (!first) {
				os << ",";
			}
			first = false;

			auto key = static_cast<Input::Keys::InputKey>(ki->second);
			auto kname = Input::Keys::kNames.tag(key);
			os << kname;
		}

		os << "\n";
	}

	if (input.gamepad_swap_analog.IsOptionVisible()) {
		os << "gamepad-swap-analog=" << int(input.gamepad_swap_analog.Get()) << "\n";
	}
	if (input.gamepad_swap_dpad_with_buttons.IsOptionVisible()) {
		os << "gamepad-swap-dpad=" << int(input.gamepad_swap_dpad_with_buttons.Get()) << "\n";
	}
	if (input.gamepad_swap_ab_and_xy.IsOptionVisible()) {
		os << "gamepad-swap-abxy=" << int(input.gamepad_swap_ab_and_xy.Get()) << "\n";
	}

	os << "\n";

	/** PLAYER SECTION */
	os << "[player]\n";
	//os << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	//os << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";
	os << "settings-autosave=" << player.settings_autosave.Get() << "\n";
	os << "settings-in-title=" << player.settings_in_title.Get() << "\n";
	os << "settings-in-menu=" << player.settings_in_menu.Get() << "\n";

	os << "\n";
}
