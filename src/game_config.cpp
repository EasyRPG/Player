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

Game_Config Game_Config::Create(CmdlineParser& cp) {
	Game_Config cfg;
	auto default_path = GetDefaultConfigPath();
	cp.Rewind();
	auto arg_path = GetConfigPath(cp);
	const auto& path = (arg_path.empty()) ? default_path : arg_path;

	if (!path.empty()) {
		cfg.LoadFromConfig(path);
	}

	cp.Rewind();
	cfg.LoadFromArgs(cp);

	return cfg;
}

std::string Game_Config::GetDefaultConfigPath() {
	// FIXME: Platform specific add this.
	// FIXME: Game specific configs?

	return "";
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

void Game_Config::LoadFromConfig(const std::string& path) {
	this->config_path = path;

	auto is = FileFinder::Root().OpenInputStream(path);
	if (!is) {
		Output::Debug("Ini config file {} not found", path);
		return;
	}

	lcf::INIReader ini(is);

	if (ini.ParseError()) {
		Output::Debug("Failed to parse ini config file {}", path);
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

void Game_Config::WriteToConfig(const std::string& path) const {
	auto of = FileFinder::Root().OpenOutputStream(path);

	if (!of) {
		Output::Debug("Failed to open {} for writing: {}", path, strerror(errno));
		return;
	}

	/** PLAYER SECTION */
	of << "[player]\n";
	of << "autobattle-algo=" << player.autobattle_algo.Get() << "\n";
	of << "enemyai-algo=" << player.enemyai_algo.Get() << "\n";
	of << "\n";

	/** VIDEO SECTION */

	of << "[video]\n";
	if (video.vsync.Enabled()) {
		of << "vsync=" << int(video.vsync.Get()) << "\n";
	}
	if (video.fullscreen.Enabled()) {
		of << "fullscreen=" << int(video.fullscreen.Get()) << "\n";
	}
	if (video.show_fps.Enabled()) {
		of << "show-fps=" << int(video.show_fps.Get()) << "\n";
	}
	if (video.fps_render_window.Enabled()) {
		of << "fps-render-window=" << int(video.fps_render_window.Get()) << "\n";
	}
	if (video.fps_limit.Enabled()) {
		of << "fps-limit=" << video.fps_limit.Get() << "\n";
	}
	if (video.window_zoom.Enabled()) {
		of << "window-zoom=" << video.window_zoom.Get() << "\n";
	}
	of << "\n";

	/** AUDIO SECTION */

	/** INPUT SECTION */
}

