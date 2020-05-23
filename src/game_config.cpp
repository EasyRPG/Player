#include "game_config.h"
#include "cmdline_parser.h"
#include "inireader.h"
#include "output.h"
#include <cstring>
#include <fstream>

Game_Config Game_Config::Create(CmdlineParser& cp) {
	Game_Config cfg;
	auto default_path = GetDefaultConfigPath();
	cp.Rewind();
	auto arg_path = GetConfigPath(cp);
	const auto& path = (arg_path.empty()) ? default_path : arg_path;

	if (!path.empty()) {
		cfg.loadFromConfig(path);
	}

	cp.Rewind();
	cfg.loadFromArgs(cp);

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

void Game_Config::loadFromArgs(CmdlineParser& cp) {
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
		if (cp.ParseNext(arg, 0, "--no-show-fps")) {
			video.show_fps.Set(false);
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

		cp.SkipNext();
	}
}

void Game_Config::loadFromConfig(const std::string& path) {
	this->config_path = path;

	INIReader ini(path);

	if (ini.ParseError()) {
		Output::Debug("Failed to parse ini config file {}", path);
		return;
	}

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
}

void Game_Config::WriteToConfig(const std::string& path) const {
	std::ofstream of;
	of.open(path);

	if (!of.is_open()) {
		Output::Debug("Failed to open {} for writing : {}", path, strerror(errno));
		return;
	}

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
		of << "show-fps=" << int(video.fps_render_window.Get()) << "\n";
	}
	if (video.fps_limit.Enabled()) {
		of << "fps-limit=" << video.fps_limit.Get() << "\n";
	}
	if (video.window_zoom.Enabled()) {
		of << "window-zoom=" << video.window_zoom.Get() << "\n";
	}
	of << "\n";
}

