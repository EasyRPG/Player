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

#ifndef EP_OPTIONS_H
#define EP_OPTIONS_H

/** Window title to show. */
#define GAME_TITLE "EasyRPG Player"

// These two defines control the name of the folder structure in the global
// configuration directory
/** Parent folder name for global configuration */
#define ORGANIZATION_NAME "EasyRPG"

/** Subfolder name for global configuration */
#define APPLICATION_NAME "Player"

/** Website shown on the logo scene */
#define WEBSITE_ADDRESS "easyrpg.org"

/** Targeted screen default width. */
#define SCREEN_TARGET_WIDTH 320

/** Targeted screen default height. */
#define SCREEN_TARGET_HEIGHT 240

/** Menus width */
#define MENU_WIDTH 320
#define MENU_HEIGHT 240

/** MessageBox dimension and offset */
#define MESSAGE_BOX_WIDTH 320
#define MESSAGE_BOX_HEIGHT 80

/** Working with hi resolutions. default 16 */
#define TILE_SIZE 16

/** Targeted screen bits per pixel. */
#define SCREEN_TARGET_BPP 32

/**
 * Pause the game process when the player window
 * looses its focus.
 */
#define PAUSE_GAME_WHEN_FOCUS_LOST 1

/**
 * Pause the audio process when the player window
 * looses its focus.
 */
#define PAUSE_AUDIO_WHEN_FOCUS_LOST 1

/** INI configuration filename. */
#define INI_NAME "RPG_RT.ini"
#define EASYRPG_INI_NAME "EasyRPG.ini"

/** Prefix for .ldb and .lmt files; used when guessing non-standard extensions. */
#define RPG_RT_PREFIX "RPG_RT"
#define EASY_RT_PREFIX "EASY_RT"

/** Suffixes for LDB/LMT/LMU files */
#define SUFFIX_LDB "ldb"
#define SUFFIX_LMT "lmt"
#define SUFFIX_LMU "lmu"

/** Suffixes for their EasyRPG equivalents */
#define SUFFIX_EDB "edb"
#define SUFFIX_EMT "emt"
#define SUFFIX_EMU "emu"

/** lcf::Database filename. */
#define DATABASE_NAME RPG_RT_PREFIX "." SUFFIX_LDB
#define DATABASE_NAME_EASYRPG EASY_RT_PREFIX "." SUFFIX_EDB

/** Map tree filename. */
#define TREEMAP_NAME RPG_RT_PREFIX "." SUFFIX_LMT
#define TREEMAP_NAME_EASYRPG EASY_RT_PREFIX "." SUFFIX_EMT

/** File name for additional metadata, such as multi-game save imports. */
#define META_NAME "Meta.ini"

/**
 * RPG_RT.exe (official engine) filename.
 * Not used by emscripten.
 */
#define EXE_NAME "RPG_RT.exe"

/** Default fps rate. */
#define DEFAULT_FPS 60

/** Enables or disables font smoothing. */
#define FONT_SMOOTHING 0

// OUTPUT_TYPE
//		OUTPUT_NONE - no output
//		OUTPUT_CONSOLE - print to console
//		OUTPUT_FILE - write to file
//		OUTPUT_MSGBOX - create pop up message box
//		OUTPUT_SCREEN - write to screen
//

/** Name of the file for output. */
#ifndef OUTPUT_FILENAME
#  define OUTPUT_FILENAME "easyrpg_log.txt"
#endif

#define USE_KEYBOARD
#define USE_MOUSE
#define USE_JOYSTICK
#define USE_JOYSTICK_AXIS
#define USE_TOUCH

#endif
