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

#ifdef GEKKO
#  include <unistd.h>
#  include <gccore.h>
#  include <sys/iosupport.h>
#elif defined(__ANDROID__)
#  include <android/log.h>
#elif defined(EMSCRIPTEN)
#  include <emscripten.h>
#endif

#include "filefinder.h"
#include "input.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include "main_data.h"
#include "message_overlay.h"
#include "utils.h"

namespace {
	std::ofstream LOG_FILE;
	bool init = false;
	
	std::ostream& output_time() {
		if (!init) {
			LOG_FILE.open(FileFinder::MakePath(Main_Data::GetSavePath(), OUTPUT_FILENAME).c_str(), std::ios_base::out | std::ios_base::app);
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
#if __cplusplus > 199711L || defined(_MSC_VER)
		int const result = vsnprintf(buf, sizeof(buf), fmt, args);
#else
#  warning Using (probably insecure) `vsprintf` function!
		int const result = vsprintf(buf, fmt, args);
		if (result > int(sizeof(buf))) {
			assert(false);
			exit(-1);
		}
#endif
		if (result < 0) {
			return std::string();
		}

		return std::string(buf, static_cast<unsigned int>(result) < sizeof(buf) ? result : sizeof(buf));
	}

	std::vector<std::string> log_buffer;

#ifdef GEKKO
	/* USBGecko Debugging on Wii */
	bool usbgecko = false;
	mutex_t usbgecko_mutex = 0;

	static ssize_t __usbgecko_write(struct _reent * /* r */, void* /* fd */, const char *ptr, size_t len) {
		uint32_t level;

		if (!ptr || !len || !usbgecko)
			return 0;

		LWP_MutexLock(usbgecko_mutex);
		level = IRQ_Disable();
		usb_sendbuffer(1, ptr, len);
		IRQ_Restore(level);
		LWP_MutexUnlock(usbgecko_mutex);

		return len;
	}

	const devoptab_t dotab_geckoout = {
		"stdout", 0, NULL, NULL, __usbgecko_write, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL
	};
#endif

}

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

static void WriteLog(std::string const& type, std::string const& msg, Color const& c = Color()) {
// Skip logging to file in the browser
#ifndef EMSCRIPTEN
	if (!Main_Data::GetSavePath().empty()) {
		// Only write to file when project path is initialized
		// (happens after parsing the command line)
		for (std::string& log : log_buffer) {
			output_time() << log << std::endl;
		}
		log_buffer.clear();

		output_time() << type << ": " << msg << std::endl;
	} else {
		// buffer log messages until file system is ready
		log_buffer.push_back(type + ": " + msg);
	}
#endif

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
	// Do not execute any game logic after an error happened
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

void Output::Quit() {
	if (LOG_FILE.is_open()) {
		LOG_FILE.close();
	}

	int log_size = 1024 * 100;

	char* buf = new char[log_size];

	std::ifstream in;
	in.open(FileFinder::MakePath(Main_Data::GetSavePath(), OUTPUT_FILENAME).c_str());
	if (!in.bad()) {
		in.seekg(0, std::ios_base::end);
		if (in.tellg() > log_size) {
			in.seekg(-log_size, std::ios_base::end);
			// skip current incomplete line
			in.getline(buf, 1024 * 100);
			in.read(buf, 1024 * 100);
			size_t read = in.gcount();
			in.close();

			std::ofstream out;
			out.open(FileFinder::MakePath(Main_Data::GetSavePath(), OUTPUT_FILENAME).c_str());
			out.write(buf, read);
			out.close();
		}
	}

	delete[] buf;
}

bool Output::TakeScreenshot() {
	int index = 0;
	std::string p;
	do {
		p = FileFinder::MakePath(Main_Data::GetSavePath(),
								 "screenshot_"
								 + Utils::ToString(index++)
								 + ".png");
	} while(FileFinder::Exists(p));
	return TakeScreenshot(p);
}

bool Output::TakeScreenshot(std::string const& file) {
	std::shared_ptr<std::fstream> ret =
		FileFinder::openUTF8(file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

	if(ret) {
		Output::Debug("Saving Screenshot %s", file.c_str());
		return Output::TakeScreenshot(*ret);
	}
	return false;
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

#ifdef GEKKO
extern const devoptab_t dotab_stdnull;

void Output::WiiSetConsole() {
	LWP_MutexInit(&usbgecko_mutex, false);
	usbgecko = usb_isgeckoalive(1);

	if (usbgecko) {
		devoptab_list[STD_OUT] = &dotab_geckoout;
		devoptab_list[STD_ERR] = &dotab_geckoout;
	} else {
		devoptab_list[STD_OUT] = &dotab_stdnull;
		devoptab_list[STD_ERR] = &dotab_stdnull;
	}
}
#endif
