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

#include <cstdlib>
#include <string>
#include <vector>
#include "player.h"
#include "utils.h"
#include "output.h"

#if USE_SDL == 3 // This is needed on Windows, SDL wraps main()
#  include <SDL3/SDL.h>
#elif USE_SDL <= 2
#  include <SDL.h>
#endif
#ifdef _WIN32
#  include <windows.h>
#  include <shellapi.h>
#elif defined(__ANDROID__)
#  include <android/log.h>
#  include "platform/android/android.h"
#endif

#ifdef __ANDROID__
static void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
#  ifdef NDEBUG
	// docs say debugging logs should be disabled for release builds
	if (lvl == LogLevel::Debug || lvl == LogLevel::Info) return;
#  endif

	int prio = (lvl == LogLevel::Error) ? ANDROID_LOG_ERROR :
		(lvl == LogLevel::Warning) ? ANDROID_LOG_WARN :
		(lvl == LogLevel::Debug) ? ANDROID_LOG_DEBUG :
		ANDROID_LOG_INFO;

	__android_log_write(prio, GAME_TITLE, msg.c_str());
}
#endif

/**
 * If the main function ever needs to change, be sure to update the `main()`
 * functions of the other platforms as well.
 */
extern "C" int main(int argc, char* argv[]) {
	std::vector<std::string> args;

#if defined(_WIN32)
	// Use widestring args
	int argc_w;
	LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);
	if (argc_w > 0) {
		args.reserve(argc_w - 1);
	}
	for (int i = 0; i < argc_w; ++i) {
		args.push_back(Utils::FromWideString(argv_w[i]));
	}
	LocalFree(argv_w);
#else
	args.assign(argv, argv + argc);
#endif

#ifdef __ANDROID__
	Output::SetLogCallback(LogCallback);
#endif

#if defined(__ANDROID__)
	EpAndroid::env = (JNIEnv*)SDL_AndroidGetJNIEnv();
#endif

	bool detect_engine = args.end() != std::find(args.begin(), args.end(), "--detect-engine");

	Player::Init(std::move(args));

	if (detect_engine) {
		Player::PrintEngineInfo();
	} else {
		Player::Run();
	}

	// Close
	return Player::exit_code;
}
