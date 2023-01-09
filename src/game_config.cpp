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
#include "output.h"
#include "input.h"
#include <lcf/inireader.h>
#include <cstring>

#ifdef _WIN32
#  include <shlobj.h>
#endif

namespace {
	StringView config_name = "config.ini";
}

Game_Config Game_Config::Create(CmdlineParser& cp) {
	Game_Config cfg;
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
#elif defined(_3DS)
	path = "sdmc:/data/easyrpg-player";
#elif defined(PSP2)
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

	auto print_err = []() {
		Output::Debug("Could not determine global config path");
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
		video.stretch.Set(ini.GetInteger("video", "stretch", 0));
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
	if (video.vsync.Enabled()) {
		os << "vsync=" << int(video.vsync.Get()) << "\n";
	}
	if (video.fullscreen.Enabled()) {
		os << "fullscreen=" << int(video.fullscreen.Get()) << "\n";
	}
	if (video.show_fps.Enabled()) {
		os << "show-fps=" << int(video.show_fps.Get()) << "\n";
	}
	if (video.fps_render_window.Enabled()) {
		os << "fps-render-window=" << int(video.fps_render_window.Get()) << "\n";
	}
	if (video.fps_limit.Enabled()) {
		os << "fps-limit=" << video.fps_limit.Get() << "\n";
	}
	if (video.window_zoom.Enabled()) {
		os << "window-zoom=" << video.window_zoom.Get() << "\n";
	}
	if (video.stretch.Enabled()) {
		os << "stretch=" << int(video.stretch.Get()) << "\n";
	}
	if (video.scaling_mode.Enabled()) {
		os << "scaling-mode=" << int(video.scaling_mode.Get()) << "\n";
	}
	os << "\n";

	/** AUDIO SECTION */
	os << "[audio]\n";

	if (audio.music_volume.Enabled()) {
		os << "music-volume=" << audio.music_volume.Get() << "\n";
	}
	if (audio.sound_volume.Enabled()) {
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

	if (input.gamepad_swap_analog.Enabled()) {
		os << "gamepad-swap-analog=" << int(input.gamepad_swap_analog.Get()) << "\n";
	}
	if (input.gamepad_swap_dpad_with_buttons.Enabled()) {
		os << "gamepad-swap-dpad=" << int(input.gamepad_swap_dpad_with_buttons.Get()) << "\n";
	}
	if (input.gamepad_swap_ab_and_xy.Enabled()) {
		os << "gamepad-swap-abxy=" << int(input.gamepad_swap_ab_and_xy.Get()) << "\n";
	}

	os << "\n";

	/** PLAYER SECTION */
	os << "[player]\n";
	os << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	os << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";
	os << "\n";
}
