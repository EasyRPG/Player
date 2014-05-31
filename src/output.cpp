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

#ifdef GEKKO
	#include <unistd.h>
#endif

#ifdef __ANDROID__
	#include <android/log.h>
#endif

#include "filefinder.h"
#include "font.h"
#include "graphics.h"
#include "input.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include "main_data.h"
#include "message_overlay.h"

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>

void boost::throw_exception(std::exception const& exp) {
	Output::Error("exception: %s", exp.what());
}
#endif

namespace output_anon {
	std::ofstream LOG_FILE(FileFinder::MakePath(Main_Data::project_path, OUTPUT_FILENAME).c_str(), std::ios_base::out | std::ios_base::app);

	std::ostream& output_time() {
		std::time_t t = std::time(NULL);
		char timestr[100];
		strftime(timestr, 100, "[%Y-%m-%d %H:%M:%S] ", std::localtime(&t));
		return LOG_FILE << timestr;
	}

	bool ignore_pause = false;

	MessageOverlay* message_overlay = NULL;
}

using namespace output_anon;

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

static void WriteLog(std::string const& type, std::string const& msg) {
	output_time() << type << ": " << msg << std::endl;

	#ifdef __ANDROID__
		__android_log_print(type == "Error" ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO, "EasyRPG Player", "%s", msg.c_str());
	#endif
}

static void HandleErrorOutput(const std::string& err) {
	// Drawing directly on the screen because message_overlay is not visible
	// when faded out
	BitmapRef surface = DisplayUi->GetDisplaySurface();
	surface->FillRect(surface->GetRect(), Color(255, 0, 0, 128));

	std::string error = "Error:\n";
	error += err;
	error += "\n\nEasyRPG Player will close now. Press any key...";

	Text::Draw(*surface, 11, 11, Color(0, 0, 0, 255), error);
	Text::Draw(*surface, 10, 10, Color(255, 255, 255, 255), error);
	DisplayUi->UpdateDisplay();

	if (ignore_pause) { return; }

	Input::ResetKeys();
	while (!Input::IsAnyPressed()) {
		DisplayUi->Sleep(1);
		DisplayUi->ProcessEvents();

		if (Player::exit_flag) break;

		Input::Update();
	}
	Input::ResetKeys();
	Graphics::FrameReset();
	DisplayUi->UpdateDisplay();
}

static void PrepareScreenOutput() {
	if (!message_overlay) {
		message_overlay = new MessageOverlay();
	}
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

void Output::ToggleLog() {
	PrepareScreenOutput();
	static bool show_log = true;
	message_overlay->SetShowAll(show_log);
	show_log = !show_log;
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
	WriteLog("Error", err);
	static bool recursive_call = false;
	if (!recursive_call && DisplayUi) {
		recursive_call = true;
		PrepareScreenOutput();
		HandleErrorOutput(err);
	} else {
		// Fallback to Console if the display is not ready yet
		std::cout << err << std::endl;
		std::cout << std::endl;
		std::cout << "EasyRPG Player will close now.";
#ifdef GEKKO
		// Wii stdin is non-blocking
		sleep(5);
#else
		std::cout << " Press any key..." << std::endl;
		std::cin.get();
#endif
	}

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
	PrepareScreenOutput();
	WriteLog("Warning", warn);
	message_overlay->AddMessage(warn, Color(255, 255, 0, 255));
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
	PrepareScreenOutput();
	WriteLog("Info", msg);
	message_overlay->AddMessage(msg, Color(255, 255, 255, 255));
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
	WriteLog("Debug", msg);
}
#endif
