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
#include <fmt/color.h>
#include <fmt/ostream.h>
#ifdef EMSCRIPTEN
#  include "platform/emscripten/interface.h"
#elif defined(__vita__)
#  include <psp2/kernel/processmgr.h>
#endif

#include "output.h"
#include "graphics.h"
#include "filefinder.h"
#include "input.h"
#include "options.h"
#include "player.h"
#include "bitmap.h"
#include "message_overlay.h"
#include "font.h"
#include "baseui.h"

// fmt 7 has renamed the namespace
#if FMT_VERSION < 70000
#  define FMT_COLOR_TYPE fmt::internal::color_type
#else
#  define FMT_COLOR_TYPE fmt::detail::color_type
#endif

using namespace std::chrono_literals;

namespace {
	constexpr const char* const log_prefix[] = {
		"Error", "Warning", "Info", "Debug"
	};
	LogLevel log_level = LogLevel::Debug;

	Filesystem_Stream::OutputStream LOG_FILE;
	bool output_recurse = false;
	bool init = false;

	std::ostream& output_time() {
		if (!init) {
			LOG_FILE = FileFinder::Save().OpenOutputStream(OUTPUT_FILENAME, std::ios_base::out | std::ios_base::app);
			init = true;
		}
		std::time_t t = std::time(nullptr);
		return LOG_FILE << Utils::FormatDate(std::localtime(&t), "[%Y-%m-%d %H:%M:%S] ");
	}

	bool ignore_pause = false;
	bool colored_log = true;

	std::vector<std::string> log_buffer;
	// pair of repeat count + message
	struct {
		int repeat = 0;
		std::string msg;
		LogLevel lvl = {};
	} last_message;

	void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
		// terminal output
		std::string prefix = Output::LogLevelToString(lvl) + ":";

		// only support colors with a "recent" fmt 6
	#if FMT_VERSION >= 60000
		if (colored_log) {
			FMT_COLOR_TYPE log_color =
				(lvl == LogLevel::Error) ? fmt::terminal_color::red :
				(lvl == LogLevel::Warning) ? fmt::terminal_color::yellow :
				(lvl == LogLevel::Debug) ? fmt::terminal_color::white :
				fmt::terminal_color::bright_white;

	#  if FMT_VERSION < 90000
			// format using temporary strings
			fmt::print(std::cerr, "{} {}\n",
				fmt::format(fmt::fg(log_color) | fmt::emphasis::bold, prefix),
				fmt::format(fmt::fg(log_color), msg));
	#  else
			// fmt 9 has styled arguments
			fmt::print(std::cerr, "{} {}\n",
				fmt::styled(prefix, fmt::fg(log_color) | fmt::emphasis::bold),
				fmt::styled(msg, fmt::fg(log_color)));
	#  endif
			return;
		}
	#endif
		std::cerr << prefix << " " << msg << std::endl;
	}

	LogCallbackFn log_cb = LogCallback;
	LogCallbackUserData log_cb_udata = nullptr;
}

std::string Output::LogLevelToString(LogLevel lvl) {
	return log_prefix[static_cast<int>(lvl)];
}

LogLevel Output::GetLogLevel() {
	return log_level;
}

void Output::SetLogLevel(LogLevel lvl) {
	log_level = lvl;
}

void Output::SetTermColor(bool colored) {
	colored_log = colored;
}

void Output::IgnorePause(bool const val) {
	ignore_pause = val;
}

void Output::SetLogCallback(LogCallbackFn fn, LogCallbackUserData userdata) {
	log_cb = fn;
	log_cb_udata = userdata;
}

static void WriteLog(LogLevel lvl, std::string const& msg, Color const& c = Color()) {
// skip writing log file
#ifndef EMSCRIPTEN
	std::string prefix = Output::LogLevelToString(lvl) + ": ";
	bool add_to_buffer = true;

	// Prevent recursion when the Save filesystem writes to the logfile on startup before it is ready
	if (!output_recurse) {
		output_recurse = true;
		if (FileFinder::Save()) {
			add_to_buffer = false;

			// Only write to file when save path is initialized
			// (happens after parsing the command line)
			if (!log_buffer.empty()) {
				std::vector<std::string> local_log_buffer = std::move(log_buffer);
				for (std::string& log : local_log_buffer) {
					output_time() << log << '\n';
				}
				local_log_buffer.clear();
			}

			// Every new message is written once to the file.
			// When it is repeated increment a counter until a different message appears,
			// then write the buffered message with the counter.
			if (msg == last_message.msg) {
				last_message.repeat++;
			} else {
				if (last_message.repeat > 0) {
					output_time() << Output::LogLevelToString(last_message.lvl) << ": " << last_message.msg << " [" << last_message.repeat + 1 << "x]" << std::endl;
				}
				output_time() << prefix << msg << '\n';

				last_message.repeat = 0;
				last_message.msg = msg;
				last_message.lvl = lvl;
			}
		}
		output_recurse = false;
	}

	if (add_to_buffer) {
		// buffer log messages until file system is ready
		log_buffer.push_back(prefix + msg);
	}
#endif

	// output to custom logger or terminal
	log_cb(lvl, msg, log_cb_udata);

	// output to overlay
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

	Text::Draw(*surface, 11, 11, *Font::DefaultBitmapFont(), Color(0, 0, 0, 255), error);
	Text::Draw(*surface, 10, 10, *Font::DefaultBitmapFont(), Color(255, 255, 255, 255), error);
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
	if (LOG_FILE) {
		LOG_FILE.Close();
	}

	int log_size = 1024 * 100;

	char* buf = new char[log_size];

	auto in = FileFinder::Save().OpenInputStream(OUTPUT_FILENAME, std::ios_base::in);
	if (in) {
		in.seekg(0, std::ios_base::end);
		if (in.tellg() > log_size) {
			in.seekg(-log_size, std::ios_base::end);
			// skip current incomplete line
			in.getline(buf, 1024 * 100);
			in.read(buf, 1024 * 100);
			size_t read = in.gcount();
			in.Close();

			auto out = FileFinder::Save().OpenOutputStream(OUTPUT_FILENAME, std::ios_base::out);
			if (out) {
				out.write(buf, read);
			}
		}
	}

	delete[] buf;
	init = false;
}

bool Output::TakeScreenshot() {
#ifdef EMSCRIPTEN
	Emscripten_Interface::TakeScreenshot();
	return true;
#else
	int index = 0;
	std::string p;
	do {
		p = "screenshot_" + std::to_string(index++) + ".png";
	} while(FileFinder::Save().Exists(p));
	return TakeScreenshot(p);
#endif
}

bool Output::TakeScreenshot(StringView file) {
	auto ret = FileFinder::Save().OpenOutputStream(file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

	if (ret) {
		Output::Debug("Saving Screenshot {}", file);
		return Output::TakeScreenshot(ret);
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
#if defined (PLAYER_NINTENDO) || defined(__vita__)
		// stdin is non-blocking
		Game_Clock::SleepFor(5s);
#elif defined (EMSCRIPTEN)
		// Don't show JavaScript Window.prompt from stdin call
		std::cout << " Process finished.";
#else
		std::cout << " Press any key..." << std::endl;
		std::cin.get();
#endif
	}

	// FIXME: This does not go through platform teardown code
#ifdef __vita__
	sceKernelExitProcess(EXIT_FAILURE);
#endif
	exit(EXIT_FAILURE);
}

void Output::WarningStr(std::string const& warn) {
	if (log_level < LogLevel::Warning) {
		return;
	}
	WriteLog(LogLevel::Warning, warn, Color(255, 255, 0, 255));
}

void Output::InfoStr(std::string const& msg) {
	if (log_level < LogLevel::Info) {
		return;
	}
	WriteLog(LogLevel::Info, msg, Color(255, 255, 255, 255));
}

void Output::DebugStr(std::string const& msg) {
	if (log_level < LogLevel::Debug) {
		return;
	}
	WriteLog(LogLevel::Debug, msg, Color(128, 128, 128, 255));
}
