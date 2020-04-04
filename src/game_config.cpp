#include "game_config.h"
#include "cmdline_parser.h"
#include "inireader.h"
#include "output.h"
#include <cstring>
#include <fstream>

Game_Config Game_Config::Create(int* argc, char** argv) {
	Game_Config cfg;
	auto default_path = GetDefaultConfigPath();
	auto arg_path = GetConfigPath(argc, argv);
	const auto& path = (arg_path.empty()) ? default_path : arg_path;

	if (!path.empty()) {
		cfg.loadFromConfig(path);
	}

	cfg.loadFromArgs(argc, argv);

	return cfg;
}

std::string Game_Config::GetDefaultConfigPath() {
	// FIXME: Platform specific add this.
	// FIXME: Game specific configs?

	return "";
}

std::string Game_Config::GetConfigPath(int* argc, char** argv) {
	CmdlineParser cp(argc, argv);

	std::string path;

	while (cp.Next()) {
		if (auto* val = cp.CheckValue("--config", 'c')) {
			path = val;
			continue;
		}
	}

	return path;
}

void Game_Config::loadFromArgs(int* argc, char** argv) {
	CmdlineParser cp(argc, argv);

	while (cp.Next()) {
		if (cp.Check("--vsync")) {
			video.vsync.Set(true);
			continue;
		}
		if (cp.Check("--no-vsync")) {
			video.vsync.Set(false);
			continue;
		}
		if (auto* val = cp.CheckValue("--fps-limit")) {
			video.fps_limit.Set(atoi(val));
			continue;
		}
		if (cp.Check("--no-fps-limit")) {
			video.fps_limit.Set(0);
			continue;
		}
		if (cp.Check("--show-fps")) {
			video.show_fps.Set(true);
			continue;
		}
		if (cp.Check("--no-show-fps")) {
			video.show_fps.Set(false);
			continue;
		}
		if (cp.Check("--fps-render-window")) {
			video.fps_render_window.Set(true);
			continue;
		}
		if (cp.Check("--no-fps-render-window")) {
			video.fps_render_window.Set(false);
			continue;
		}
		if (cp.Check("--no-show-fps")) {
			video.show_fps.Set(false);
			continue;
		}
		if (cp.Check("--window")) {
			video.fullscreen.Set(false);
			continue;
		}
		if (cp.Check("--fullscreen")) {
			video.fullscreen.Set(true);
			continue;
		}
		if (auto* val = cp.CheckValue("--window-zoom")) {
			video.window_zoom.Set(atoi(val));
			continue;
		}
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
		video.vsync.Set(ini.GetBoolean("video", "fullscreen", false));
	}
	if (ini.HasValue("video", "show-fps")) {
		video.vsync.Set(ini.GetBoolean("video", "show-fps", false));
	}
	if (ini.HasValue("video", "fps-render-window")) {
		video.vsync.Set(ini.GetBoolean("video", "fps-render-window", false));
	}
	if (ini.HasValue("video", "fps-limit")) {
		video.vsync.Set(ini.GetInteger("video", "fps-limit", 0));
	}
	if (ini.HasValue("video", "window-zoom")) {
		video.vsync.Set(ini.GetInteger("video", "window-zoom", 0));
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

