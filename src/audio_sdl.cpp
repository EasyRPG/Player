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

#if defined(USE_SDL) && !defined(HAVE_SDL_MIXER) && defined(SUPPORT_AUDIO)

#include <cassert>
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_version.h>

#include "audio_sdl.h"
#include "output.h"

namespace {
#ifdef EMSCRIPTEN
	SDL_AudioDeviceID audio_dev_id = 0;
#endif
}

void sdl_audio_callback(void* userdata, uint8_t* stream, int length) {
	// no mutex locking required, SDL does this before calling

	static_cast<GenericAudio*>(userdata)->Decode(stream, length);
}

AudioDecoder::Format sdl_format_to_format(Uint16 format) {
	switch (format) {
		case AUDIO_U8:
			return AudioDecoder::Format::U8;
		case AUDIO_S8:
			return AudioDecoder::Format::S8;
		case AUDIO_U16SYS:
			return AudioDecoder::Format::U16;
		case AUDIO_S16SYS:
			return AudioDecoder::Format::S16;
#if SDL_MAJOR_VERSION > 1
		case AUDIO_S32:
			return AudioDecoder::Format::S32;
		case AUDIO_F32:
			return AudioDecoder::Format::F32;
#endif
		default:
			assert(false);
	}

	return (AudioDecoder::Format)-1;
}

SdlAudio::SdlAudio() :
	GenericAudio()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		Output::Warning("Couldn't init audio: %s", SDL_GetError());
		return;
	}

	SDL_AudioSpec want = {0};
	SDL_AudioSpec have = {0};
	want.freq = 44100;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = 4096;
	want.callback = sdl_audio_callback;
	want.userdata = this;

#ifdef EMSCRIPTEN
	audio_dev_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (audio_dev_id == 0) {
#else
	if (SDL_OpenAudio(&want, &have) < 0) {
#endif
		Output::Warning("Couldn't open audio: %s", SDL_GetError());
		return;
	}

	SetFormat(have.freq, sdl_format_to_format(have.format), have.channels);

	// Start Audio
#ifdef EMSCRIPTEN
	SDL_PauseAudioDevice(audio_dev_id, 0);
#else
	SDL_PauseAudio(0);
#endif
}

SdlAudio::~SdlAudio() {
#ifdef EMSCRIPTEN
	SDL_CloseAudioDevice(audio_dev_id);
#else
	SDL_CloseAudio();
#endif
}

void SdlAudio::LockMutex() const {
	SDL_LockAudio();
}

void SdlAudio::UnlockMutex() const {
	SDL_UnlockAudio();
}

#endif
