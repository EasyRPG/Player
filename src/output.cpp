/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdarg>
#include <exception>

#include "graphics.h"
#include "input.h"
#include "msgbox.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "time.hpp"

////////////////////////////////////////////////////////////
#include <boost/config.hpp>

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>

void boost::throw_exception(std::exception const& exp) {
	Output::Error("exception: %s", exp.what());
}
#endif


////////////////////////////////////////////////////////////
static void HandleScreenOutput(std::string msg, bool is_error) {
	std::stringstream ss;
	ss << msg << "\n\n";
	if (is_error) {
		ss << "EasyRPG Player will close now.\nPress any key to exit...";
	} else {
		ss << "Press any key to continue...";
	}
	DisplayUi->DrawScreenText(ss.str(), 10, 30 + 10);
	DisplayUi->UpdateDisplay();
	Input::ResetKeys();
	while (!Input::IsAnyPressed()) {
		Time::Sleep(1);
		DisplayUi->ProcessEvents();

		if (Player::exit_flag) break;

		Input::Update();
	}
	Input::ResetKeys();
	Graphics::FrameReset();
	Graphics::Update();
}

////////////////////////////////////////////////////////////
void Output::Error(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];
	vsprintf(str, fmt, args);

	Output::ErrorStr((std::string)str);

	va_end(args);
}
void Output::Error(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];
	vsprintf(str, fmt, args);

	Output::ErrorStr((std::string)str);

	va_end(args);
}
void Output::ErrorStr(std::string err) {
	#if OUTPUT_TYPE == OUTPUT_CONSOLE
		printf("%s\n", err.c_str());
		std::cout << std::endl;
		std::cout << "EasyRPG Player will close now. Press any key..." << std::endl;
	#elif OUTPUT_TYPE == OUTPUT_FILE
		std::ofstream file;
		file.open(OUTPUT_FILENAME, std::ios::out | std::ios::app);
		file << err;
		file.close();
	#elif OUTPUT_TYPE == OUTPUT_MSGBOX
		if (DisplayUi != NULL) {
			DisplayUi->ShowCursor(true);
		}
		MsgBox::Error(err, GAME_TITLE);
	#elif OUTPUT_TYPE == OUTPUT_SCREEN
		if (DisplayUi != NULL) {
			DisplayUi->DrawScreenText("Error:", 10, 30, Color(255, 0, 0, 0));
			HandleScreenOutput(err, true);
		} else {
			// Fallback to Console if the display is not ready yet
			printf("%s\n", err.c_str());
			std::cout << std::endl;
			std::cout << "EasyRPG Player will close now. Press any key..." << std::endl;
			std::cin.get();
		}
	#endif
	Player::Exit();
	exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////
void Output::Warning(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::WarningStr((std::string)str);

	va_end(args);
}
void Output::Warning(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::WarningStr((std::string)str);

	va_end(args);
}
void Output::WarningStr(std::string warn) {
	#if OUTPUT_TYPE == OUTPUT_CONSOLE
		printf("%s\n", warn.c_str());
	#elif OUTPUT_TYPE == OUTPUT_FILE
		std::ofstream file;
		file.open(OUTPUT_FILENAME, std::ios::out | std::ios::app);
		file << warn;
		file.close();
	#elif OUTPUT_TYPE == OUTPUT_MSGBOX
		bool last = DisplayUi->ShowCursor(true);
		MsgBox::Warning(warn, GAME_TITLE);
		DisplayUi->ShowCursor(last);
		Graphics::FrameReset();
	#elif OUTPUT_TYPE == OUTPUT_SCREEN
		DisplayUi->DrawScreenText("Warning:", 10, 30, Color(255, 255, 0, 0));
		HandleScreenOutput(warn, false);
	#endif
}

////////////////////////////////////////////////////////////
void Output::Post(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::PostStr((std::string)str);

	va_end(args);
}
void Output::Post(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::PostStr((std::string)str);

	va_end(args);
}
void Output::PostStr(std::string msg) {
	#if OUTPUT_TYPE == OUTPUT_CONSOLE
		printf("%s\n", msg.c_str());
	#elif OUTPUT_TYPE == OUTPUT_FILE
		std::ofstream file;
		file.open(OUTPUT_FILENAME, std::ios::out | std::ios::app);
		file << msg;
		file.close();
	#elif OUTPUT_TYPE == OUTPUT_MSGBOX
		bool last = DisplayUi->ShowCursor(true);
		MsgBox::OK(msg, GAME_TITLE);
		DisplayUi->ShowCursor(last);
		Graphics::FrameReset();
	#elif OUTPUT_TYPE == OUTPUT_SCREEN
		DisplayUi->DrawScreenText("Info:", 10, 30, Color(255, 255, 0, 0));
		HandleScreenOutput(msg, false);
	#endif
}

////////////////////////////////////////////////////////////
#ifdef _DEBUG
void Output::Debug(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::DebugStr((std::string)str);

	va_end(args);
}
void Output::Debug(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::DebugStr((std::string)str);

	va_end(args);
}
void Output::DebugStr(std::string msg) {
	printf("Debug: %s\n", msg.c_str());
}
#else
void Output::Debug(char*, ...) {
}
void Output::Debug(const char*, ...) {
}
void Output::DebugStr(std::string) {
}
#endif
