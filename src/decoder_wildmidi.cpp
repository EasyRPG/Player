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

#ifdef HAVE_LIBWILDMIDI

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
#  include "platform/libretro/ui.h"
#endif

/* possible options include: WM_MO_REVERB|WM_MO_ENHANCED_RESAMPLING
 * however, they cause high cpu usage, so not using them for now.
 */
#define WILDMIDI_OPTS 0

namespace {
	bool once = false;
	bool init = false;
}

static void WildMidiDecoder_deinit() {
	if (init) {
		WildMidi_Shutdown();
		init = false;
	}
}

#if LIBWILDMIDI_VERSION >= 1027 // at least 0.4.3
static void* vio_allocate_file_func(const char* filename, uint32_t* size) {
	auto stream = FileFinder::Root().OpenInputStream(filename);
	if (!stream) {
		Output::Warning("WildMidi: vio_allocate_file_func failed for {}", filename);
		return nullptr;
	}

	auto buf = Utils::ReadStream(stream);

	*size = static_cast<uint32_t>(buf.size());

	// Make buffer one byte larger, otherwise MSVC CRT detects a Heap Corruption (Wildmidi bug?)
	char* buffer = reinterpret_cast<char*>(malloc(*size + 1));
	memcpy(buffer, buf.data(), buf.size());

	return buffer;
}

static void vio_free_file_func(void* buffer) {
	free(buffer);
}

static struct _WM_VIO vio = {
	vio_allocate_file_func,
	vio_free_file_func
};
#endif

WildMidiDecoder::~WildMidiDecoder() {
	if (handle)
		WildMidi_Close(handle);
}

bool WildMidiDecoder::Initialize(std::string& error_message) {
	std::string config_file;
	bool found = false;

	// only initialize once until a new game starts
	if (once)
		return init;
	once = true;

	/* find the configuration file in different paths on different platforms
	 * FIXME: move this logic into some configuration class
	 */
#if defined(USE_LIBRETRO)
	const char *dir = NULL;

	// Game directory
	if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir) {
		config_file = std::string(dir) + "/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// Content downloader
	if (!found) {
		if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY, &dir) && dir) {
			config_file = std::string(dir) + "/wildmidi/wildmidi.cfg";
			found = FileFinder::Root().Exists(config_file);
		}
	}
#elif defined(__wii__)
	// preferred under /data
	config_file = "usb:/data/wildmidi/wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);
	if (!found) {
		config_file = "sd:/data/wildmidi/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// app directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// same, but legacy from SDL_mixer's timidity
	if (!found) {
		config_file = "usb:/data/timidity/timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
	if (!found) {
		config_file = "sd:/data/timidity/timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
	if (!found) {
		config_file = "timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#elif defined(__WIIU__)
	// preferred SD card directory
	config_file = "/vol/external01/data/easyrpg-player/wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#elif defined(__3DS__)
	// Only wildmidi paths, no timidity because there was never timidity used on 3DS

	// Shipped in a romfs (for CIA and newer 3dsx files)
	config_file = "romfs:/wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);

	// preferred SD card directory
	if (!found) {
		config_file = "sdmc:/3ds/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#elif defined(__SWITCH__)
	// Only wildmidi paths, no timidity because it was never used on Switch
	config_file = "./wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);
	if (!found) {
		config_file = "/switch/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#elif defined(__vita__)
	// Only wildmidi paths, no timidity because it was never used on PSVita

	// Shipped
	config_file = "app0:/wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);

	// Preferred global directory
	if (!found) {
		config_file = "ux0:/data/easyrpg-player/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#elif defined(__MORPHOS__)
	// Shipped with library
	config_file = "LIBS:timidity/timidity.cfg";
	found = FileFinder::Root().Exists(config_file);

	// Current directory
	if (!found) {
		config_file = "wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#else
	// Prefer wildmidi in current directory
	config_file = "wildmidi.cfg";
	found = FileFinder::Root().Exists(config_file);

	// wildmidi command line player default config
	if (!found) {
		config_file = "/etc/wildmidi/wildmidi.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// Use Timidity strategy used in SDL mixer

	// Environment variable
	if (!found) {
		const char *env = getenv("TIMIDITY_CFG");
		if (env) {
			config_file = env;
			found = FileFinder::Root().Exists(config_file);
		}
	}

	if (!found) {
		config_file = "timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

#  ifdef _WIN32
	// Probably not too useful
	if (!found) {
		config_file = "C:\\TIMIDITY\\timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}

	// TODO: We need some installer which creates registry keys for wildmidi

#  elif defined(PLAYER_AMIGA)
	if (!found) {
		config_file = "timidity/timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
	}
#  else
	if (!found) {
		config_file = "/etc/timidity.cfg";
		found = FileFinder::Root().Exists(config_file);
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
			found = FileFinder::Root().Exists(config_file);

			if (found) {
				break;
			}

			// Some distributions have it in timidity++
			config_file = s + "++/timidity.cfg";
			found = FileFinder::Root().Exists(config_file);

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
		return false;
	}
	Output::Debug("WildMidi: Using {} as configuration file...", config_file);

#if LIBWILDMIDI_VERSION >= 1027 // at least 0.4.3
	init = (WildMidi_InitVIO(&vio, config_file.c_str(), EP_MIDI_FREQ, WILDMIDI_OPTS) == 0);
#else
	init = (WildMidi_Init(config_file.c_str(), EP_MIDI_FREQ, WILDMIDI_OPTS) == 0);
#endif

	if (!init) {
		error_message = std::string("WildMidi_Init() failed : ") + WildMidi_GetError();
		return false;
	}

#if defined(PLAYER_AMIGA)
	// the default volume is way too quiet with the SDL_mixer patches
	WildMidi_MasterVolume(127);
#endif

	// setup deinitialization
	static bool atexit_once = false;
	if (!atexit_once) {
		atexit_once = true;
		atexit(WildMidiDecoder_deinit);
	}

	return true;
}

void WildMidiDecoder::ResetState() {
	once = false;
	WildMidiDecoder_deinit();
}

bool WildMidiDecoder::Open(std::vector<uint8_t>& data) {
	// this should not happen
	if (handle) {
		WildMidi_Close(handle);
		Output::Debug("WildMidi: Previous handle was not closed.");
	}

	handle = WildMidi_OpenBuffer(data.data(), data.size());

	return handle != nullptr;
}

bool WildMidiDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (origin == std::ios_base::beg) {
		if (handle) {
			unsigned long int pos = offset;
			WildMidi_FastSeek(handle, &pos);
		}
		return true;
	}

	return false;
}

int WildMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!handle)
		return -1;

	return WildMidi_GetOutput(handle, reinterpret_cast<int8_t*>(buffer), length);
}

#endif
