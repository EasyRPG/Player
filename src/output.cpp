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

// Headers
#include <cstdlib>
#include <cstdarg>
#include <ctime>

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include "filefinder.h"
#include "graphics.h"
#include "input.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include "main_data.h"

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>

void boost::throw_exception(std::exception const& exp) {
	Output::Error("exception: %s", exp.what());
}
#endif

namespace {
	std::ofstream LOG_FILE(OUTPUT_FILENAME, std::ios_base::out | std::ios_base::app);

	std::ostream& output_time() {
		std::time_t t = std::time(NULL);
		return LOG_FILE << "Local: " << std::asctime(std::localtime(&t))
						<< "UTC  : " << std::asctime(std::gmtime(&t));
	}

	bool ignore_pause = false;
}

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

static void HandleScreenOutput(char const* type, std::string const& msg, bool is_error) {
	Output::TakeScreenshot();

	output_time() << type << ":\n  " << msg << "\n";

	if(ignore_pause) { return; }

	std::stringstream ss;
	ss << type << ":\n" << msg << "\n\n";
	if (is_error) {
		ss << "EasyRPG Player will close now.\nPress any key to exit...";
	} else {
		ss << "Press any key to continue...";
	}
	DisplayUi->GetDisplaySurface()->Clear();
	DisplayUi->DrawScreenText(ss.str(), 10, 30 + 10);
	DisplayUi->UpdateDisplay();
	Input::ResetKeys();
	while (!Input::IsAnyPressed()) {
		DisplayUi->Sleep(1);
		DisplayUi->ProcessEvents();

		if (Player::exit_flag) break;

		Input::Update();
	}
	Input::ResetKeys();
	Graphics::FrameReset();
	Graphics::Update();
}


bool Output::TakeScreenshot() {
	int index = 0;
	std::string p;
	do {
		p = FileFinder::MakePath(Main_Data::project_path,
								 "screenshot_"
								 + boost::lexical_cast<std::string>(index++)
								 + ".png");
	} while(FileFinder::Exists(p));
	return TakeScreenshot(p);
}

bool Output::TakeScreenshot(std::string const& file) {
	EASYRPG_SHARED_PTR<std::fstream> ret =
		FileFinder::openUTF8(file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	return ret? Output::TakeScreenshot(*ret) : false;
}

bool Output::TakeScreenshot(std::ostream& os) {
	return DisplayUi->GetDisplaySurface()->WritePNG(os);
}

void Output::Error(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];
	vsprintf(str, fmt, args);

	Output::ErrorStr(std::string(str));

	va_end(args);
}
void Output::ErrorStr(std::string const& err) {
	if (DisplayUi) {
		DisplayUi->DrawScreenText("Error:", 10, 30, Color(255, 0, 0, 0));
		HandleScreenOutput("Error", err, true);
	} else {
		// Fallback to Console if the display is not ready yet
		printf("%s\n", err.c_str());
		std::cout << std::endl;
		std::cout << "EasyRPG Player will close now. Press any key..." << std::endl;
		std::cin.get();
	}

	Player::Exit();
	exit(EXIT_FAILURE);
}

void Output::Warning(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::WarningStr(std::string(str));

	va_end(args);
}
void Output::WarningStr(std::string const& warn) {
	DisplayUi->DrawScreenText("Warning:", 10, 30, Color(255, 255, 0, 0));
	HandleScreenOutput("Warning", warn, false);
}

void Output::Post(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::PostStr(std::string(str));

	va_end(args);
}
void Output::PostStr(std::string const& msg) {
	DisplayUi->DrawScreenText("Info:", 10, 30, Color(255, 255, 0, 0));
	HandleScreenOutput("Post", msg, false);
}

#ifdef NDEBUG
void Output::Debug(const char*, ...) {
}
void Output::DebugStr(std::string const&) {
}
#else
void Output::Debug(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];

	vsprintf(str, fmt, args);

	Output::DebugStr(std::string(str));

	va_end(args);
}
void Output::DebugStr(std::string const& msg) {
	output_time() << "Debug:\n " << msg <<std::endl;
}
#endif
