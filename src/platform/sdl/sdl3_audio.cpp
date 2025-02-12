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

#include "SDL3/SDL_audio.h"
#include "system.h"

#ifdef SUPPORT_AUDIO

#include <cassert>
#include <cstdint>
#include <chrono>
#include <SDL3/SDL.h>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

#include "sdl3_audio.h"
#include "output.h"

using namespace std::chrono_literals;

namespace {
	SDL_AudioStream* audio_stream = nullptr;
}

void sdl_audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
	// no mutex locking required, SDL does this before calling

	if (additional_amount > 0) {
		static std::vector<uint8_t> buffer;
		buffer.resize(total_amount);

		static_cast<GenericAudio*>(userdata)->Decode(buffer.data(), additional_amount);
		SDL_PutAudioStreamData(stream, buffer.data(), additional_amount);
	}
}

AudioDecoder::Format sdl_format_to_format(Uint16 format) {
	switch (format) {
		case SDL_AUDIO_U8:
			return AudioDecoder::Format::U8;
		case SDL_AUDIO_S8:
			return AudioDecoder::Format::S8;
		case SDL_AUDIO_S16:
			return AudioDecoder::Format::S16;
		case SDL_AUDIO_S32:
			return AudioDecoder::Format::S32;
		case SDL_AUDIO_F32:
			return AudioDecoder::Format::F32;
		default:
			Output::Warning("Couldn't find GenericAudio format for {:#x}", format);
			assert(false);
	}

	return (AudioDecoder::Format)-1;
}

Sdl3Audio::Sdl3Audio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		Output::Warning("Couldn't init audio: {}", SDL_GetError());
		return;
	}

#ifdef EMSCRIPTEN
	// Get preferred sample rate from Browser (-> OS)
	const int frequency = EM_ASM_INT_V({
		var context;
		try {
			context = new AudioContext();
		} catch (e) {
			context = new webkitAudioContext();
		}
		return context.sampleRate;
	});
#else
	const int frequency = 44100;
#endif

	const SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, frequency };
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, sdl_audio_callback, this);

	if (!audio_stream) {
		Output::Warning("Couldn't open audio: {}", SDL_GetError());
		return;
	}

	SetFormat(frequency, sdl_format_to_format(SDL_AUDIO_S16), 2);

	// Start Audio
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audio_stream));
}

Sdl3Audio::~Sdl3Audio() {
	SDL_DestroyAudioStream(audio_stream);
}

void Sdl3Audio::LockMutex() const {
	if (!audio_stream) {
		return;
	}

	SDL_LockAudioStream(audio_stream);
}

void Sdl3Audio::UnlockMutex() const {
	if (!audio_stream) {
		return;
	}

	SDL_UnlockAudioStream(audio_stream);
}

#endif
