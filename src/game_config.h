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

#ifndef EP_GAME_CONFIG_H
#define EP_GAME_CONFIG_H

#include "config_param.h"
#include "filesystem.h"
#include "options.h"
#include "input_buttons.h"

class CmdlineParser;

enum class ScalingMode {
	/** Nearest neighbour to fit screen */
	Nearest,
	/** Like NN but only scales to integers */
	Integer,
	/** Integer followed by Bilinear downscale to fit screen */
	Bilinear,
};

struct Game_ConfigPlayer {
	StringConfigParam autobattle_algo{ "" };
	StringConfigParam enemyai_algo{ "" };
};

struct Game_ConfigVideo {
	LockedConfigParam<std::string> renderer{ "auto" };
	BoolConfigParam vsync{ true };
	BoolConfigParam fullscreen{ true };
	BoolConfigParam show_fps{ false };
	BoolConfigParam fps_render_window{ false };
	BoolConfigParam stretch_width{ true };
	RangeConfigParam<int> fps_limit{ DEFAULT_FPS, 0, std::numeric_limits<int>::max() };
	RangeConfigParam<int> window_zoom{ 2, 1, std::numeric_limits<int>::max() };
	EnumConfigParam<ScalingMode> scaling_mode{ ScalingMode::Bilinear };
};

struct Game_ConfigAudio {
	RangeConfigParam<int> music_volume{ 100, 0, 100 };
	RangeConfigParam<int> sound_volume{ 100, 0, 100 };
};

struct Game_ConfigInput {
	BoolConfigParam gamepad_swap_analog{ false };
	BoolConfigParam gamepad_swap_dpad_with_buttons{ false };
	BoolConfigParam gamepad_swap_ab_and_xy{ false };
	Input::ButtonMappingArray buttons;
};

struct Game_Config {
	/** Gameplay subsystem options */
	Game_ConfigPlayer player;

	/** Video subsystem options */
	Game_ConfigVideo video;

	/** Audio subsystem options */
	Game_ConfigAudio audio;

	/** Input subsystem options */
	Game_ConfigInput input;

	/**
	 * Create an application config. This first determines the config file path if any,
	 * loads the config file, then loads command line arguments.
	 */
	static Game_Config Create(CmdlineParser& cp);

	/** @return config file path from command line args if found */
	static std::string GetConfigPath(CmdlineParser& cp);

	/**
	 * Returns the a filesystem view to the global config directory
	 */
	static FilesystemView GetGlobalConfigFilesystem();

	/**
	 * Returns a handle to the global config file for reading.
	 * The file is created if it does not exist.
	 *
	 * @return handle to the global file
	 */
	static Filesystem_Stream::InputStream GetGlobalConfigFileInput();

	/**
	 * Returns a handle to the global config file for writing.
	 * The file is created if it does not exist.
	 *
	 * @return handle to the global file
	 */
	static Filesystem_Stream::OutputStream GetGlobalConfigFileOutput();

	/**
	 * Load configuration values from a stream;
	 *
	 * @param is stream to read from.
	 * @post values of this are updated with values found in the stream.
	 */
	void LoadFromStream(Filesystem_Stream::InputStream& is);

	/**
	 * Load configuration values from a command line arguments.
	 *
	 * @param cp the command line parser to use.
	 * @post values of this are updated with values found in command line args.
	 */
	void LoadFromArgs(CmdlineParser& cp);

	/**
	 * Writes our configuration to the given stream.
	 *
	 * @param os stream to write to
	 */
	void WriteToStream(Filesystem_Stream::OutputStream& os) const;
};

#endif
