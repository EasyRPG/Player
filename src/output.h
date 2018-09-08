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

#ifndef EP_OUTPUT_H
#define EP_OUTPUT_H

// Headers
#include <string>
#include <iosfwd>

#ifdef __MORPHOS__
#undef Debug
#endif

/**
 * Output Namespace.
 */
namespace Output {
	/**
	 * Closes the log file handle and trims the file.
	 */
	void Quit();

	/**
	 * Takes screenshot and save it to Main_Data::GetProjectPath().
	 *
	 * @return true if success, otherwise false.
	 */
	bool TakeScreenshot();

	/**
	 * Takes screenshot and save it to specified file.
	 *
	 * @param file file to save.
	 * @return true if success, otherwise false.
	 */
	bool TakeScreenshot(std::string const& file);

	/**
	 * Takes screenshot and save it to specified stream.
	 *
	 * @param os output stream that PNG will be stored.
	 * @return true if success, otherwise false.
	 */
	bool TakeScreenshot(std::ostream& os);

	/**
	 * Shows/Hides the output log overlay.
	 */
	void ToggleLog();

	/**
	 * Ignores pause in Warning and Error.
	 *
	 * @param val whether to ignore pause.
	 */
	void IgnorePause(bool val);

	/**
	 * Displays a string with formatted string.
	 *
	 * @param fmt formatted string to display.
	 */
	void Post(char* fmt, ...);

	/**
	 * Displays a string with formatted string.
	 *
	 * @param fmt formatted string to display.
	 */
	void Post(const char* fmt, ...);

	/**
	 * Displays a string msg.
	 *
	 * @param msg string to display.
	 */
	void PostStr(std::string const& msg);

	/**
	 * Display a warning with formatted string.
	 *
	 * @param fmt formatted warning to display.
	 */
	void Warning(const char* fmt, ...);

	/**
	 * Display a warning.
	 *
	 * @param warn : warning to display.
	 */
	void WarningStr(std::string const& warn);

	/**
	 * Raises an error message with formatted string and
	 * closes the player afterwards.
	 *
	 * @param fmt formatted error to display.
	 */
	[[noreturn]] void Error(const char* fmt, ...);

	/**
	 * Display an error message and closes the player
	 * afterwards.
	 *
	 * @param err error to display.
	 */
	[[noreturn]] void ErrorStr(std::string const& err);

	/**
	 * Prints a debug message to the console.
	 *
	 * @param fmt formatted debug text to display.
	 */
	void Debug(const char* fmt, ...);

	/**
	 * Prints a debug message to the console.
	 *
	 * @param msg formatted debug text to display.
	 */
	void DebugStr(std::string const& msg);

#ifdef GEKKO
	/**
	 * Helper function to disable the console on Wii
	 * and redirect to USB Gekko, if present.
	 */
	void WiiSetConsole();
#endif
}

#endif
