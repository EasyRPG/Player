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
	std::string config_path;
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
	game_resolution.SetOptionVisible(false);
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

	config_path = GetConfigPath(cp);
	std::string config_file;
	if (!config_path.empty()) {
		config_file = FileFinder::MakePath(config_path, config_name);
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
		std::string str_value;

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
		if (cp.ParseNext(arg, 0, "--stretch")) {
			video.stretch.Set(true);
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-stretch")) {
			video.stretch.Set(false);
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
				audio.music_volume.Set(li_value);
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--bind-address")) {
			std::string svalue;
			if (arg.ParseValue(0, svalue)) {
				multiplayer.server_bind_address.Set(std::move(svalue));
			}
			continue;
		}
		if (cp.ParseNext(arg, 1, "--bind-address-v6")) {
			std::string svalue;
			if (arg.ParseValue(0, svalue)) {
				multiplayer.server_bind_address_v6.Set(std::move(svalue));
			}
			continue;
		}
		if (cp.ParseNext(arg, 0, "--no-heartbeats")) {
			multiplayer.no_heartbeats.Set(true);
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
	video.show_fps.FromIni(ini);
	video.fps_render_window.FromIni(ini);
	video.fps_limit.FromIni(ini);
	video.window_zoom.FromIni(ini);
	video.scaling_mode.FromIni(ini);
	video.stretch.FromIni(ini);
	video.touch_ui.FromIni(ini);
	video.game_resolution.FromIni(ini);

	if (ini.HasValue("Video", "WindowX") && ini.HasValue("Video", "WindowY") && ini.HasValue("Video", "WindowWidth") && ini.HasValue("Video", "WindowHeight")) {
		video.window_x.FromIni(ini);
		video.window_y.FromIni(ini);
		video.window_width.FromIni(ini);
		video.window_height.FromIni(ini);
	}

	/** AUDIO SECTION */
	audio.music_volume.FromIni(ini);
	audio.sound_volume.FromIni(ini);

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

	input.gamepad_swap_analog.FromIni(ini);
	input.gamepad_swap_dpad_with_buttons.FromIni(ini);
	input.gamepad_swap_ab_and_xy.FromIni(ini);

	/** PLAYER SECTION */
	player.settings_autosave.FromIni(ini);
	player.settings_in_title.FromIni(ini);
	player.settings_in_menu.FromIni(ini);

	/** MULTIPLAYER SECTION */
	multiplayer.server_auto_start.FromIni(ini);
	multiplayer.server_bind_address.FromIni(ini);
	multiplayer.server_bind_address_v6.FromIni(ini);
	multiplayer.server_max_users.FromIni(ini);
	multiplayer.server_picture_names.FromIni(ini);
	multiplayer.server_picture_prefixes.FromIni(ini);
	multiplayer.client_auto_connect.FromIni(ini);
	multiplayer.client_remote_address.FromIni(ini);
	multiplayer.client_chat_visibility.FromIni(ini);
	multiplayer.client_chat_name.FromIni(ini);
	multiplayer.client_chat_crypt_key.FromIni(ini);
	multiplayer.client_name_tag_mode.FromIni(ini);
}

void Game_Config::WriteToStream(Filesystem_Stream::OutputStream& os) const {
	/** VIDEO SECTION */

	os << "[Video]\n";
	video.vsync.ToIni(os);
	video.fullscreen.ToIni(os);
	video.show_fps.ToIni(os);
	video.fps_render_window.ToIni(os);
	video.fps_limit.ToIni(os);
	video.window_zoom.ToIni(os);
	video.scaling_mode.ToIni(os);
	video.stretch.ToIni(os);
	video.touch_ui.ToIni(os);
	video.game_resolution.ToIni(os);

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
	os << "\n";

	/** INPUT SECTION */
	os << "[Input]\n";

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

	input.gamepad_swap_analog.ToIni(os);
	input.gamepad_swap_dpad_with_buttons.ToIni(os);
	input.gamepad_swap_ab_and_xy.ToIni(os);

	os << "\n";

	/** PLAYER SECTION */
	os << "[Player]\n";
	//os << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	//os << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";

	player.settings_autosave.ToIni(os);
	player.settings_in_title.ToIni(os);
	player.settings_in_menu.ToIni(os);

	os << "\n";

	/** MULTIPLAYER SECTION */
	os << "[Multiplayer]\n";
	multiplayer.server_auto_start.ToIni(os);
	multiplayer.server_bind_address.ToIni(os);
	multiplayer.server_bind_address_v6.ToIni(os);
	multiplayer.server_max_users.ToIni(os);
	multiplayer.server_picture_names.ToIni(os);
	multiplayer.server_picture_prefixes.ToIni(os);
	multiplayer.client_auto_connect.ToIni(os);
	multiplayer.client_remote_address.ToIni(os);
	multiplayer.client_chat_visibility.ToIni(os);
	multiplayer.client_chat_name.ToIni(os);
	multiplayer.client_chat_crypt_key.ToIni(os);
	multiplayer.client_name_tag_mode.ToIni(os);

	os << "\n";
}
