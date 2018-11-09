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

#include "system.h"

#ifdef HAVE_WILDMIDI

// Headers
#include <cassert>
#include <stdlib.h>
#include <wildmidi_lib.h>
#include "audio_decoder.h"
#include "output.h"
#include "filefinder.h"
#include "utils.h"
#include "decoder_wildmidi.h"

#if defined(GEKKO) || defined(_3DS)
#  define WILDMIDI_FREQ 22050
#else
#  define WILDMIDI_FREQ 44100
#endif

#ifdef __ANDROID__
#include <jni.h>
#include "SDL_system.h"
#include "string.h"
#endif

/* possible options include: WM_MO_REVERB|WM_MO_ENHANCED_RESAMPLING
 * however, they cause high cpu usage, so not using them for now.
 */
#define WILDMIDI_OPTS 0

#ifdef __ANDROID__
std::string get_timidity_path_jni() {
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getTimidityPath = env->GetMethodID(cls, "getTimidityPath", "()Ljava/lang/String;");
	jstring return_string = (jstring)env->CallObjectMethod(sdl_activity, jni_getTimidityPath);

	const char *js = env->GetStringUTFChars(return_string, nullptr);

	std::string ret_str = js;

	env->ReleaseStringUTFChars(return_string, js);
	env->DeleteLocalRef(sdl_activity);
	env->DeleteLocalRef(cls);

	return ret_str;
}

#endif

static bool init = false;
static void WildMidiDecoder_deinit(void) {
	WildMidi_Shutdown();
}

WildMidiDecoder::WildMidiDecoder(const std::string file_name) {
	music_type = "midi";
	filename = file_name;
	std::string config_file = "";
	bool found = false;

	// only initialize once
	if (init)
		return;

	/* find the configuration file in different paths on different platforms
	 * FIXME: move this logic into some configuration class
	 */
#ifdef GEKKO
	// preferred under /data
	config_file = "usb:/data/wildmidi/wildmidi.cfg";
	found = FileFinder::Exists(config_file);
	if (!found) {
		config_file = "sd:/data/wildmidi/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// app directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// same, but legacy from SDL_mixer's timidity
	if (!found) {
		config_file = "usb:/data/timidity/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
	if (!found) {
		config_file = "sd:/data/timidity/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
	if (!found) {
		config_file = "timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
#elif _3DS
	// Only wildmidi paths, no timidity because there was never timidity used on 3DS

	// Shipped in a romfs (for CIA and newer 3dsx files)
	config_file = "romfs:/wildmidi.cfg";
	found = FileFinder::Exists(config_file);

	// preferred SD card directory
	if (!found) {
		config_file = "sdmc:/3ds/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}
#elif SWITCH
	// Only wildmidi paths, no timidity because it was never used on Switch
	config_file = "./wildmidi.cfg";
	found = FileFinder::Exists(config_file);
	if (!found) {
		config_file = "/switch/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}
#elif __ANDROID__
	// Use JNI to obtain the path
	std::string path = get_timidity_path_jni();

	config_file = path + "/wildmidi.cfg";
	found = FileFinder::Exists(config_file);

	// Support old app installs where wildmidi.cfg wasn't bundled
	// (the timidity folder is only extracted on first run)
	if (!found) {
		config_file = path + "/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
#elif PSP2
	// Only wildmidi paths, no timidity because it was never used on PSVita

	// Shipped
	config_file = "app0:/wildmidi.cfg";
	found = FileFinder::Exists(config_file);

	// Preferred global directory
	if (!found) {
		config_file = "ux0:/data/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}
#else
	// Prefer wildmidi in current directory
	config_file = "wildmidi.cfg";
	found = FileFinder::Exists(config_file);

	// wildmidi command line player default config
	if (!found) {
		config_file = "/etc/wildmidi/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// Use Timidity strategy used in SDL mixer

	// Environment variable
	if (!found) {
		const char *env = getenv("TIMIDITY_CFG");
		if (env) {
			config_file = env;
			found = FileFinder::Exists(config_file);
		}
	}

	if (!found) {
		config_file = "timidity.cfg";
		found = FileFinder::Exists(config_file);
	}

#	ifdef _WIN32
	// Probably not too useful
	if (!found) {
		config_file = "C:\\TIMIDITY\\timidity.cfg";
		found = FileFinder::Exists(config_file);
	}

	// TODO: We need some installer which creates registry keys for wildmidi

#	elif defined(__MORPHOS__) || defined(__amigaos4__) || defined(__AROS__) 
	if (!found) {
		config_file = "timidity/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
#	else
	if (!found) {
		config_file = "/etc/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}

	if (!found) {
		// Folders used in timidity code
		const std::vector<std::string> folders = {
			"/etc/timidity",
			"/usr/share/timidity",
			"/usr/local/share/timidity",
			"/usr/local/lib/timidity"
		};

		for (const std::string& s : folders) {
			config_file = s + "/timidity.cfg";
			found = FileFinder::Exists(config_file);

			if (found) {
				break;
			}

			// Some distributions have it in timidity++
			config_file = s + "++/timidity.cfg";
			found = FileFinder::Exists(config_file);

			if (found) {
				break;
			}
		}
	}
#	endif
#endif

	// bail, if nothing found
	if (!found) {
		error_message = "WildMidi: Could not find configuration file.";
		return;
	}
	Output::Debug("WildMidi: Using %s as configuration file...", config_file.c_str());

	init = (WildMidi_Init(config_file.c_str(), WILDMIDI_FREQ, WILDMIDI_OPTS) == 0);
	if (!init) {
		error_message = std::string("WildMidi_Init() failed : ") + WildMidi_GetError();
		return;
	}

#if defined(__MORPHOS__) || defined(__amigaos4__) || defined(__AROS__) 
	// the default volume is way too quiet with the SDL_mixer patches
	WildMidi_MasterVolume(127);
#endif

	// setup deinitialization
	atexit(WildMidiDecoder_deinit);
}

WildMidiDecoder::~WildMidiDecoder() {
	if (handle)
		WildMidi_Close(handle);
}

bool WildMidiDecoder::WasInited() const {
	return init;
}

bool WildMidiDecoder::Open(FILE* file) {
	if (!init)
		return false;

	// this should not happen
	if (handle) {
		WildMidi_Close(handle);
		Output::Debug("WildMidi: Previous handle was not closed.");
	}

	handle = WildMidi_Open(filename.c_str());
	if (!handle) {
		error_message = "WildMidi: Error reading file";
		return false;
	}

	fclose(file);
	return true;
}

bool WildMidiDecoder::Seek(size_t offset, Origin origin) {
	if (offset == 0 && origin == Origin::Begin) {
		if (handle) {
			unsigned long int pos = 0;
			WildMidi_FastSeek(handle, &pos);
		}
		return true;
	}

	return false;
}

bool WildMidiDecoder::IsFinished() const {
	if (!handle)
		return false;

	struct _WM_Info* midi_info = WildMidi_GetInfo(handle);

	return midi_info->current_sample >= midi_info->approx_total_samples;
}

void WildMidiDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& channels) const {
	freq = WILDMIDI_FREQ;
	format = Format::S16;
	channels = 2;
}

bool WildMidiDecoder::SetFormat(int freq, AudioDecoder::Format format, int channels) {
	if (freq != WILDMIDI_FREQ || channels != 2 || format != Format::S16)
		return false;

	return true;
}

int WildMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!handle)
		return -1;

	/* Old wildmidi (< 0.4.0) did output only in little endian and had a different API,
	 * this inverts the buffer. The used version macro exists since 0.4.0.
	 */
#ifndef LIBWILDMIDI_VERSION
	int res = WildMidi_GetOutput(handle, reinterpret_cast<char*>(buffer), length);
	if (Utils::IsBigEndian() && res > 0) {
		uint16_t* buffer_16 = reinterpret_cast<uint16_t*>(buffer);
		for (int i = 0; i < res / 2; ++i) {
			Utils::SwapByteOrder(buffer_16[i]);
		}
	}
#else
	int res = WildMidi_GetOutput(handle, reinterpret_cast<int8_t*>(buffer), length);
#endif
	return res;
}

#endif
