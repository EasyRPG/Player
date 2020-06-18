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
#include <thread>
#include <chrono>

#include "graphics.h"

#ifdef GEKKO
#  include <unistd.h>
#  include <gccore.h>
#  include <sys/iosupport.h>
#elif defined(__SWITCH__)
#  include <unistd.h>
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
#include "font.h"
#include "baseui.h"

using namespace std::chrono_literals;

namespace {
	enum class LogLevel {
		Error,
		Warning,
		Info,
		Debug
	};
	constexpr const char* const log_prefix[4] = {
		"Error: ",
		"Warning: ",
		"Info: ",
		"Debug: "
	};

	static const char* GetLogPrefix(LogLevel lvl) {
		return log_prefix[static_cast<int>(lvl)];
	}

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

	std::vector<std::string> log_buffer;
	// pair of repeat count + message
	struct {
		int repeat = 0;
		std::string msg;
		LogLevel lvl = {};
	} last_message;

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
		NULL, NULL, NULL
	};
#endif

}

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

static void WriteLog(LogLevel lvl, std::string const& msg, Color const& c = Color()) {
	const char* prefix = GetLogPrefix(lvl);
	// Skip logging to file in the browser
#ifndef EMSCRIPTEN
	if (!Main_Data::GetSavePath().empty()) {
		// Only write to file when project path is initialized
		// (happens after parsing the command line)
		for (std::string& log : log_buffer) {
			output_time() << log << std::endl;
		}
		log_buffer.clear();

		// Every new message is written once to the file.
		// When it is repeated increment a counter until a different message appears,
		// then write the buffered message with the counter.
		if (msg == last_message.msg) {
			last_message.repeat++;
		} else {
			if (last_message.repeat > 0) {
				output_time() << GetLogPrefix(last_message.lvl) << last_message.msg << " [" << last_message.repeat + 1 << "x]" << std::endl;
				output_time() << prefix << msg << std::endl;
			} else {
				output_time() << prefix << msg << std::endl;
			}
			last_message.repeat = 0;
			last_message.msg = msg;
			last_message.lvl = lvl;
		}
	} else {
		// buffer log messages until file system is ready
		log_buffer.push_back(prefix + msg);
	}
#endif

#ifdef __ANDROID__
	__android_log_print(lvl == LogLevel::Error ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO, "EasyRPG Player", "%s", msg.c_str());
#else
	std::cerr << prefix << msg << std::endl;
#endif

	if (lvl != LogLevel::Debug && lvl != LogLevel::Error) {
		Graphics::GetMessageOverlay().AddMessage(msg, c);
	}
}

static void HandleErrorOutput(const std::string& err) {
	// Drawing directly on the screen because message_overlay is not visible
	// when faded out
	BitmapRef surface = DisplayUi->GetDisplaySurface();
	surface->FillRect(surface->GetRect(), Color(255, 0, 0, 128));

	std::string error = "Error:\n";
	error += err;

	error += "\n\nEasyRPG Player will close now.\nPress [ENTER] key to exit...";

	Text::Draw(*surface, 11, 11, *Font::Default(), Color(0, 0, 0, 255), error);
	Text::Draw(*surface, 10, 10, *Font::Default(), Color(255, 255, 255, 255), error);
	DisplayUi->UpdateDisplay();

	if (ignore_pause) { return; }

	Input::ResetKeys();
	while (!Input::IsAnyPressed()) {
#if !defined(USE_LIBRETRO)
		Game_Clock::SleepFor(1ms);
#endif
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
								 + std::to_string(index++)
								 + ".png");
	} while(FileFinder::Exists(p));
	return TakeScreenshot(p);
}

bool Output::TakeScreenshot(std::string const& file) {
	std::shared_ptr<std::fstream> ret =
		FileFinder::openUTF8(file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

	if (ret) {
		Output::Debug("Saving Screenshot {}", file);
		return Output::TakeScreenshot(*ret);
	}
	return false;
}

bool Output::TakeScreenshot(std::ostream& os) {
	return DisplayUi->GetDisplaySurface()->WritePNG(os);
}

void Output::ToggleLog() {
	static bool show_log = true;
	Graphics::GetMessageOverlay().SetShowAll(show_log);
	show_log = !show_log;
}

void Output::ErrorStr(std::string const& err) {
	WriteLog(LogLevel::Error, err);
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
#if defined (GEKKO) || defined(__SWITCH__)
		// stdin is non-blocking
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

void Output::WarningStr(std::string const& warn) {
	WriteLog(LogLevel::Warning, warn, Color(255, 255, 0, 255));
}

void Output::InfoStr(std::string const& msg) {
	WriteLog(LogLevel::Info, msg, Color(255, 255, 255, 255));
}

void Output::DebugStr(std::string const& msg) {
	WriteLog(LogLevel::Debug, msg, Color(128, 128, 128, 255));
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
