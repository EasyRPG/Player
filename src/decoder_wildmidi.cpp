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

#ifdef USE_LIBRETRO
#include "platform/libretro/libretro_ui.h"
#endif

#if defined(GEKKO) || defined(_3DS)
#  define WILDMIDI_FREQ 22050
#else
#  define WILDMIDI_FREQ 44100
#endif

#if defined(USE_SDL) && defined(__ANDROID__)
#  include <jni.h>
#  include "SDL_system.h"
#  include "string.h"
#endif

/* possible options include: WM_MO_REVERB|WM_MO_ENHANCED_RESAMPLING
 * however, they cause high cpu usage, so not using them for now.
 */
#define WILDMIDI_OPTS 0

#if defined(USE_SDL) && defined(__ANDROID__)
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
#if defined(USE_LIBRETRO)
	const char *dir = NULL;

	// Game directory
	if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir) {
		config_file = std::string(dir) + "/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}

	// Content downloader
	if (!found) {
		if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY, &dir) && dir) {
			config_file = std::string(dir) + "/wildmidi/wildmidi.cfg";
			found = FileFinder::Exists(config_file);
		}
	}
#elif defined(GEKKO)
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
#elif defined(_3DS)
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
#elif defined(__SWITCH__)
	// Only wildmidi paths, no timidity because it was never used on Switch
	config_file = "./wildmidi.cfg";
	found = FileFinder::Exists(config_file);
	if (!found) {
		config_file = "/switch/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Exists(config_file);
	}
#elif defined(__ANDROID__)
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
#elif defined(PSP2)
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

#  ifdef _WIN32
	// Probably not too useful
	if (!found) {
		config_file = "C:\\TIMIDITY\\timidity.cfg";
		found = FileFinder::Exists(config_file);
	}

	// TODO: We need some installer which creates registry keys for wildmidi

#  elif defined(__MORPHOS__) || defined(__amigaos4__) || defined(__AROS__)
	if (!found) {
		config_file = "timidity/timidity.cfg";
		found = FileFinder::Exists(config_file);
	}
#  else
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
#  endif
#endif

	// bail, if nothing found
	if (!found) {
		error_message = "WildMidi: Could not find configuration file.";
		return;
	}
	Output::Debug("WildMidi: Using {} as configuration file...", config_file);

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

	// Ugly: Parse Midi header to get Division
	// WildMidi has no Api to get Division and Tempo
	// This allows a better approximation of the Midi ticks but it is still
	// way off because the tempo information is missing
	uint8_t midi_header[14];
	fread(midi_header, 1, 14, file);

	division = midi_header[12] << 8u;
	division |= midi_header[13];

	// Wildmidi will reject such files, but just in case if they ever support it
	if (division & 0x8000u) {
		Output::Debug("WildMidi: Unsupported: Division in fps");
		division = 96;
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

	return WildMidi_GetOutput(handle, reinterpret_cast<int8_t*>(buffer), length);
}

int WildMidiDecoder::GetTicks() const {
	if (!handle) {
		return 0;
	}

	struct _WM_Info* info = WildMidi_GetInfo(handle);
	float secs = (float)info->current_sample / WILDMIDI_FREQ;

	// FIXME: tempo is an assumption, the library must internally process
	// the tempo because it can dynamically change.
	const int tempo = 500000;

	int ticks = static_cast<int>(secs / (tempo / 1000000.0 / division));

	return ticks;
}

#endif
