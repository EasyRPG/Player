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
#include <cstdio>

#include <iostream>
#include <fstream>
#include <exception>

#ifdef GEKKO
	#include <unistd.h>
#endif

#ifdef __ANDROID__
	#include <android/log.h>
#endif

#ifdef EMSCRIPTEN
	#include <emscripten.h>
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

#include <boost/lexical_cast.hpp>

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>

void boost::throw_exception(std::exception const& exp) {
	Output::Error("exception: %s", exp.what());
}
#endif

namespace {
	std::ofstream LOG_FILE;
	bool init = false;
	
	std::ostream& output_time() {
		if (!init) {
			LOG_FILE.open(FileFinder::MakePath(Main_Data::GetProjectPath(), OUTPUT_FILENAME).c_str(), std::ios_base::out | std::ios_base::app);
			init = true;
		}
		std::time_t t = std::time(NULL);
		char timestr[100];
		strftime(timestr, 100, "[%Y-%m-%d %H:%M:%S] ", std::localtime(&t));
		return LOG_FILE << timestr;
	}

	bool ignore_pause = false;

	MessageOverlay& message_overlay() {
		static std::unique_ptr<MessageOverlay> overlay;
		assert(DisplayUi);
		if (!overlay) {
			overlay.reset(new MessageOverlay());
		}
		return *overlay;
	}

	std::string format_string(char const* fmt, va_list args) {
		char buf[4096];
	// FIXME: devkitppc r27 seems to have broken newlib
	#if __cplusplus > 199711L && !defined(GEKKO)
		int const result = vsnprintf(buf, sizeof(buf), fmt, args);
	#else
		int const result = vsprintf(buf, fmt, args);
	#endif
		assert(0 <= result && result < int(sizeof(buf)));
		return std::string(buf, result);
	}
}

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

static void WriteLog(std::string const& type, std::string const& msg, Color const& c = Color()) {
	if (!Main_Data::GetSavePath().empty()) {
		// Only write to file when project path is initialized
		// (happens after parsing the command line)
		output_time() << type << ": " << msg << std::endl;
	}

#ifdef __ANDROID__
	__android_log_print(type == "Error" ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO, "EasyRPG Player", "%s", msg.c_str());
#else
	std::cerr << type << ": " << msg << std::endl;
#endif

	if (type != "Debug") {
		if (DisplayUi) {
			message_overlay().AddMessage(msg, c);
		}
	}
}

static void HandleErrorOutput(const std::string& err) {
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();
#endif

	// Drawing directly on the screen because message_overlay is not visible
	// when faded out
	BitmapRef surface = DisplayUi->GetDisplaySurface();
	surface->FillRect(surface->GetRect(), Color(255, 0, 0, 128));

	std::string error = "Error:\n";
	error += err;

	error += "\n\nEasyRPG Player will close now.\nPress [ENTER] key to exit...";

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
}

bool Output::TakeScreenshot() {
	int index = 0;
	std::string p;
	do {
		p = FileFinder::MakePath(Main_Data::GetProjectPath(),
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
	static bool show_log = true;
	message_overlay().SetShowAll(show_log);
	show_log = !show_log;
}

void Output::Error(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Output::ErrorStr(format_string(fmt, args));
	va_end(args);
}

void Output::ErrorStr(std::string const& err) {
	WriteLog("Error", err);
	static bool recursive_call = false;
	if (!recursive_call && DisplayUi) {
		recursive_call = true;
		HandleErrorOutput(err);
		DisplayUi.reset();
	} else {
		// Fallback to Console if the display is not ready yet
		std::cout << err << std::endl;
		std::cout << std::endl;
		std::cout << "EasyRPG Player will close now.";
#if defined (GEKKO)
		// Wii stdin is non-blocking
		sleep(5);
#elif defined (EMSCRIPTEN)
		// Don't show JavaScript Window.prompt from stdin call
		std::cout << " Process finished.";
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
	Output::WarningStr(format_string(fmt, args));
	va_end(args);
}
void Output::WarningStr(std::string const& warn) {
	WriteLog("Warning", warn, Color(255, 255, 0, 255));
}

void Output::Post(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Output::PostStr(format_string(fmt, args));
	va_end(args);
}

void Output::PostStr(std::string const& msg) {
	WriteLog("Info", msg, Color(255, 255, 255, 255));
}

void Output::Debug(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Output::DebugStr(format_string(fmt, args));
	va_end(args);
}
void Output::DebugStr(std::string const& msg) {
	WriteLog("Debug", msg);
}
