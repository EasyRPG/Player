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

/**
 * This class manages global engine configuration.
 * For game specific settings see Game_ConfigGame.
 *
 * @see Game_ConfigGame
 */

#include "config_param.h"
#include "filesystem.h"
#include "options.h"
#include "input_buttons.h"
#include "utils.h"

class CmdlineParser;

namespace ConfigEnum {
	enum class ScalingMode {
		/** Nearest neighbour to fit screen */
		Nearest,
		/** Like NN but only scales to integers */
		Integer,
		/** Integer followed by Bilinear downscale to fit screen */
		Bilinear,
	};

	enum class GameResolution {
		/** 320x240 */
		Original,
		/** 416x240 */
		Widescreen,
		/** 560x240 */
		Ultrawide
	};

	enum class StartupLogos {
		None,
		Custom,
		All
	};

	enum class StartupLangSelect {
		Never,
		/* Shows language screen when no saves are found */
		FirstStartup,
		/* Always show the language screen before the title */
		Always
	};

	enum class ShowFps {
		/** Do not show */
		OFF,
		/** When windowed: Title bar, fullscreen: Overlay */
		ON,
		/** Always overlay */
		Overlay
	};
};

#ifdef HAVE_FLUIDLITE
#define EP_FLUID_NAME "FluidLite"
#else
#define EP_FLUID_NAME "FluidSynth"
#endif

struct Game_ConfigPlayer {
	StringConfigParam autobattle_algo{ "", "", "", "", "" };
	StringConfigParam enemyai_algo{ "", "", "", "", "" };
	BoolConfigParam settings_autosave{ "Save settings on exit", "Automatically save the settings on exit", "Player", "SettingsAutosave", false };
	BoolConfigParam settings_in_title{ "Show settings on title screen", "Display settings menu item on the title screen", "Player", "SettingsInTitle", false };
	BoolConfigParam settings_in_menu{ "Show settings in menu", "Display settings menu item on the menu screen", "Player", "SettingsInMenu", false };
	EnumConfigParam<ConfigEnum::StartupLogos, 3> show_startup_logos{
		"Startup Logos", "Logos that are displayed on startup", "Player", "StartupLogos", ConfigEnum::StartupLogos::Custom,
		Utils::MakeSvArray("None", "Custom", "All"),
		Utils::MakeSvArray("none", "custom", "all"),
		Utils::MakeSvArray("Do not show any additional logos", "Show custom logos bundled with the game", "Show all logos, including the original from RPG Maker")};
	PathConfigParam font1 { "Font 1", "The game chooses whether it wants font 1 or 2", "Player", "Font1", "" };
	RangeConfigParam<int> font1_size { "Font 1 Size", "", "Player", "Font1Size", 12, 6, 16};
	PathConfigParam font2 { "Font 2", "The game chooses whether it wants font 1 or 2", "Player", "Font2", "" };
	RangeConfigParam<int> font2_size { "Font 2 Size", "", "Player", "Font2Size", 12, 6, 16};
	EnumConfigParam<ConfigEnum::StartupLangSelect, 3> lang_select_on_start {
		"Startup Language Menu", "Show language menu before booting up a game", "Player", "StartupLangSelect", ConfigEnum::StartupLangSelect::FirstStartup,
		Utils::MakeSvArray("Never", "First Start", "Always"),
		Utils::MakeSvArray("never", "FirstStartup", "always"),
		Utils::MakeSvArray("Never show language menu on start", "Show on first start (when no save files are found)", "Always show language menu prior to the title screen") };
	BoolConfigParam lang_select_in_title{ "Show language menu on title screen", "Display language menu item on the title screen", "Player", "LanguageInTitle", true };
	BoolConfigParam log_enabled{ "Logging", "Write diagnostic messages into a logfile", "Player", "Logging", true };
	RangeConfigParam<int> screenshot_scale { "Screenshot scaling factor", "Scale screenshots by the given factor", "Player", "ScreenshotScale", 1, 1, 24};
	BoolConfigParam screenshot_timestamp{ "Screenshot timestamp", "Add the current date and time to the file name", "Player", "ScreenshotTimestamp", true };
	BoolConfigParam automatic_screenshots{ "Automatic screenshots", "Periodically take screenshots", "Player", "AutomaticScreenshots", false };
	RangeConfigParam<int> automatic_screenshots_interval{ "Screenshot interval", "The interval between automatic screenshots (seconds)", "Player", "AutomaticScreenshotsInterval", 30, 1, 999999 };
	BoolConfigParam prefer_easyrpg_map_files{ "Prefer EasyRPG map files", "Attempt to load EasyRPG map files (.emu) first and fall back to RPG Maker map files (.lmu)", "Player", "PreferEasyRpgMapFiles", true };

	void Hide();
};

struct Game_ConfigVideo {
	LockedConfigParam<std::string> renderer{ "Renderer", "The rendering engine", "auto" };
	BoolConfigParam vsync{ "V-Sync", "Toggle V-Sync mode (Recommended: ON)", "Video", "Vsync", true };
	BoolConfigParam fullscreen{ "Fullscreen", "Toggle between fullscreen and window mode", "Video", "Fullscreen", true };
	EnumConfigParam<ConfigEnum::ShowFps, 3> fps{
		"FPS counter", "How to display the FPS counter", "Video", "Fps", ConfigEnum::ShowFps::OFF,
		Utils::MakeSvArray("[OFF]", "[ON]", "Overlay"),
		Utils::MakeSvArray("off", "on", "overlay"),
		Utils::MakeSvArray("Do not show the FPS counter", "Show the FPS counter", "Always show the FPS counter inside the window")};
	RangeConfigParam<int> fps_limit{ "Frame Limiter", "Toggle the frames per second limit (Recommended: 60)", "Video", "FpsLimit", DEFAULT_FPS, 0, 99999 };
	ConfigParam<int> window_zoom{ "Window Zoom", "Toggle the window zoom level", "Video", "WindowZoom", 2 };
	EnumConfigParam<ConfigEnum::ScalingMode, 3> scaling_mode{ "Scaling method", "How the output is scaled", "Video", "ScalingMode", ConfigEnum::ScalingMode::Nearest,
		Utils::MakeSvArray("Nearest", "Integer", "Bilinear"),
		Utils::MakeSvArray("nearest", "integer", "bilinear"),
		Utils::MakeSvArray("Scale to screen size (Causes scaling artifacts)", "Scale to multiple of the game resolution", "Like Nearest, but output is blurred to avoid artifacts")};
	BoolConfigParam stretch{ "Stretch", "Stretch to the width of the window/screen", "Video", "Stretch", false };
	BoolConfigParam pause_when_focus_lost{ "Pause when focus lost", "Pause the program when it is in the background", "Video", "PauseWhenFocusLost", true };
	BoolConfigParam touch_ui{ "Touch Ui", "Display the touch ui", "Video", "TouchUi", true };
	EnumConfigParam<ConfigEnum::GameResolution, 3> game_resolution{ "Resolution", "Game resolution. Changes require a restart.", "Video", "GameResolution", ConfigEnum::GameResolution::Original,
		Utils::MakeSvArray("Original (Recommended)", "Widescreen (Experimental)", "Ultrawide (Experimental)"),
		Utils::MakeSvArray("original", "widescreen", "ultrawide"),
		Utils::MakeSvArray("The default resolution (320x240, 4:3)", "Can cause glitches (416x240, 16:9)", "Can cause glitches (560x240, 21:9)")};
	RangeConfigParam<int> screen_scale{ "Scaling", "Adjust screen scaling (Overscan/Underscan)", "Video", "ScreenScale", 100, 50, 150 };

	// These are never shown and are used to restore the window to the previous position
	ConfigParam<int> window_x{ "", "", "Video", "WindowX", -1 };
	ConfigParam<int> window_y{ "", "", "Video", "WindowY", -1 };
	ConfigParam<int> window_width{ "", "", "Video", "WindowWidth", -1 };
	ConfigParam<int> window_height{ "", "", "Video", "WindowHeight", -1 };

	void Hide();
};

struct Game_ConfigAudio {
	RangeConfigParam<int> music_volume{ "BGM Volume", "Volume of the background music", "Audio", "MusicVolume", 100, 0, 100 };
	RangeConfigParam<int> sound_volume{ "SFX Volume", "Volume of the sound effects", "Audio", "SoundVolume", 100, 0, 100 };
	BoolConfigParam fluidsynth_midi { EP_FLUID_NAME " (SF2)", "Play MIDI using SF2 soundfonts", "Audio", "Fluidsynth", true };
	BoolConfigParam wildmidi_midi { "WildMidi (GUS)", "Play MIDI using GUS patches", "Audio", "WildMidi", true };
	BoolConfigParam native_midi { "Native MIDI", "Play MIDI through the operating system ", "Audio", "NativeMidi", true };
	LockedConfigParam<std::string> fmmidi_midi { "FmMidi", "Play MIDI using the built-in MIDI synthesizer", "[Always ON]" };
	PathConfigParam soundfont { "Soundfont", "Soundfont to use for " EP_FLUID_NAME, "Audio", "Soundfont", "" };

	void Hide();
};

struct Game_ConfigInput {
	RangeConfigParam<int> speed_modifier_a{ "Fast Forward A: Speed", "Set fast forward A speed", "Input", "SpeedModifierA", 3, 2, 100 };
	RangeConfigParam<int> speed_modifier_b{ "Fast Forward B: Speed", "Set fast forward B speed", "Input", "SpeedModifierB", 10, 2, 100 };
	BoolConfigParam gamepad_swap_analog{ "Gamepad: Swap Analog Sticks", "Swap left and right stick", "Input", "GamepadSwapAnalog", false };
	BoolConfigParam gamepad_swap_dpad_with_buttons{ "Gamepad: Swap D-Pad with buttons", "Swap D-Pad with ABXY-Buttons", "Input", "GamepadSwapDpad", false };
	BoolConfigParam gamepad_swap_ab_and_xy{ "Gamepad: Swap AB and XY", "Swap A and B with X and Y", "Input", "GamepadSwapAbxy", false };
	Input::ButtonMappingArray buttons;

	void Hide();
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
	 * Returns the filesystem view to the soundfont directory
	 * By default this is config/Soundfont
	 */
	static FilesystemView GetSoundfontFilesystem();

	/**
	 * Returns the filesystem view to the font directory
	 * By default this is config/Font
	 */
	static FilesystemView GetFontFilesystem();

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

	static Filesystem_Stream::OutputStream& GetLogFileOutput();

	static void CloseLogFile();

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
