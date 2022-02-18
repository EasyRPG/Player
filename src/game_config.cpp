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
#include <lcf/inireader.h>
#include <cstring>

namespace {
	StringView config_name = "config.ini";
}

Game_Config Game_Config::Create(CmdlineParser& cp) {
	Game_Config cfg;
	cp.Rewind();
	auto arg_path = GetConfigPath(cp);
	auto cli_config = FileFinder::Root().OpenInputStream(arg_path);

	if (!cli_config) {
		auto global_config = GetGlobalConfigFileInput();
		if (!global_config) {
			return cfg;
		}
		cfg.LoadFromStream(global_config);
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
	path = "sd:/data";
#elif defined(__SWITCH__)
	path = "/switch/easyrpg-player";
#elif defined(_3DS)
	path = "sdmc:/data";
#elif defined(PSP2)
	path = "ux0:/data/easyrpg-player";
#elif defined(USE_LIBRETRO)
	const char* dir = nullptr;
	if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir) {
		path = std::string(dir);
	}
#elif defined(__ANDROID__)
	// FIXME
#elif defined(_WIN32)
	// FIXME
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
#endif

	auto print_err = []() {
		Output::Debug("Could not determine global config path");
	};

	if (path.empty()) {
		print_err();
		return {};
	}

	std::string sub_path = FileFinder::MakePath(ORGANIZATION_NAME, APPLICATION_NAME);
	path = FileFinder::MakePath(path, sub_path);

	auto fs = FileFinder::Root().Create(path);
	if (!fs) {
		// Attempt creating directories and try again
		if (FileFinder::Root().MakeDirectory(path, true)) {
			fs = FileFinder::Root().Create(path);
		}
	}

	if (!fs) {
		print_err();
		return {};
	}

	return fs;
}

Filesystem_Stream::InputStream Game_Config::GetGlobalConfigFileInput() {
	auto fs = GetGlobalConfigFilesystem();

	if (fs) {
		auto is = fs.OpenInputStream(config_name, std::ios_base::in);
		if (!is) {
			// Create the file
			{
				fs.OpenOutputStream(config_name);
			}
			is = fs.OpenInputStream(config_name, std::ios_base::in);
		}

		return is;
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
		if (cp.ParseNext(arg, 1, "--config", 'c')) {
			if (arg.NumValues() > 0) {
				path = arg.Value(0);
			}
			continue;
		}

		cp.SkipNext();
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

	/** PLAYER SECTION */

	if (ini.HasValue("player", "autobattle-algo")) {
		player.autobattle_algo.Set(ini.GetString("player", "autobattle-algo", ""));
	}
	if (ini.HasValue("player", "enemyai-algo")) {
		player.enemyai_algo.Set(ini.GetString("player", "enemyai-algo", ""));
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

	/** AUDIO SECTION */

	/** INPUT SECTION */
}

void Game_Config::WriteToStream(Filesystem_Stream::OutputStream& os) const {
	/** PLAYER SECTION */
	os << "[player]\n";
	os << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	os << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";
	os << "\n";

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
	os << "\n";

	/** AUDIO SECTION */

	/** INPUT SECTION */
}

