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

#define WILDMIDI_FREQ 44100
/* possible options include: WM_MO_REVERB|WM_MO_ENHANCED_RESAMPLING
 * however, they cause high cpu usage, so not using them for now.
 */
#define WILDMIDI_OPTS 0

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
#else
	// TODO
	config_file = "wildmidi.cfg";
	found = FileFinder::Exists(config_file);
#endif

	// bail, if nothing found
	if (!found) {
		error_message = "WildMidi: Could not find configuration file.";
		return;
	}

	Output::Debug("WildMidi: Using %s as configuration file...", config_file.c_str());

	init = (WildMidi_Init(config_file.c_str(), WILDMIDI_FREQ, WILDMIDI_OPTS) == 0);
	if (!init) {
		error_message = "Could not initialize libWildMidi";
		return;
	}

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

bool WildMidiDecoder::SetPitch(int pitch) {
	if (pitch != 100)
		return false;

	return true;
}

int WildMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!handle)
		return -1;

	int res = WildMidi_GetOutput(handle, reinterpret_cast<char*>(buffer), length);

	/* Old wildmidi (< 0.4.0) did output only in little endian, this inverts the buffer.
	 * The used version macro exists since 0.4.0
	 */
#ifndef LIBWILDMIDI_VERSION
	if (Utils::IsBigEndian() && res > 0) {
		uint16_t* buffer_16 = reinterpret_cast<uint16_t*>(buffer);
		for (int i = 0; i < res / 2; ++i) {
			Utils::SwapByteOrder(buffer_16[i]);
		}
	}
#endif
	return res;
}

#endif
