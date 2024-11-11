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

#include "audio_generic_midiout.h"
#include "audio_decoder_midi.h"
#include <cassert>

#ifdef HAVE_NATIVE_MIDI
#include <chrono>
#include "filesystem_stream.h"
#include "game_clock.h"
#include "output.h"

#ifdef USE_LIBRETRO
#include "platform/libretro/midiout_device.h"
#endif

#ifdef HAVE_ALSA
#include "platform/linux/midiout_device_alsa.h"
#elif _WIN32
#include "platform/windows/midiout_device_win32.h"
#elif __APPLE__
#include "platform/macos/midiout_device_coreaudio.h"
#endif

using namespace std::chrono_literals;

static struct {
	bool libretro = true;
	bool alsa = true;
	bool win32 = true;
	bool coreaudio = true;
	std::string status;
} works;

GenericAudioMidiOut::GenericAudioMidiOut() {
	stop_thread.store(false);

#ifdef USE_LIBRETRO
	std::string libretro_status;
	if (works.libretro) {
		auto dec = std::make_unique<LibretroMidiOutDevice>(libretro_status);
		if (dec->IsInitialized()) {
			midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
		} else {
			works.libretro = false;
			Output::Debug(libretro_status);
		}
	}

	if (midi_out) {
		return;
	}
#endif

#ifdef HAVE_ALSA
	if (works.alsa) {
		auto dec = std::make_unique<AlsaMidiOutDevice>(works.status);
		if (dec->IsInitialized()) {
			midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
		} else {
			works.alsa = false;
			Output::Debug(works.status);
		}
	}
#elif _WIN32
	if (works.win32) {
		auto dec = std::make_unique<Win32MidiOutDevice>(works.status);
		if (dec->IsInitialized()) {
			midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
		} else {
			works.win32 = false;
			Output::Debug(works.status);
		}
	}
#elif __APPLE__
	if (works.coreaudio) {
		auto dec = std::make_unique<CoreAudioMidiOutDevice>(works.status);
		if (dec->IsInitialized()) {
			midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
		} else {
			works.coreaudio = false;
			Output::Debug(works.status);
		}
	}
#endif

#ifdef USE_LIBRETRO
	works.status = libretro_status + ". " + works.status;
#endif
}

GenericAudioMidiOut::~GenericAudioMidiOut() {
	if (thread_started) {
		GetMidiOut().Reset();
		StopThread();
	}
}

void GenericAudioMidiOut::LockMutex() {
	midi_mutex.lock();
}

void GenericAudioMidiOut::UnlockMutex() {
	midi_mutex.unlock();
}

void GenericAudioMidiOut::UpdateMidiOut(std::chrono::microseconds delta) {
	LockMutex();
	assert(midi_out);

	midi_out->UpdateMidi(delta);
	UnlockMutex();
}

void GenericAudioMidiOut::StartThread() {
	assert(!thread_started);
	thread_started = true;
	midi_thread = std::thread(&GenericAudioMidiOut::ThreadFunction, this);
}

void GenericAudioMidiOut::StopThread() {
	stop_thread.store(true);
	midi_thread.join();
}

void GenericAudioMidiOut::ThreadFunction() {
	// The libretro clock is not updating often enough for good MIDI timing but
	// all platforms that support Native Midi also have a working high precision clock
	using clock = std::chrono::steady_clock;

	auto start_ticks = clock::now();
	while (!stop_thread) {
		auto ticks = clock::now();

		auto us = std::chrono::duration_cast<std::chrono::microseconds>(ticks - start_ticks);
		UpdateMidiOut(us);

		std::this_thread::sleep_for(1ms);

		start_ticks = ticks;
	}
}

bool GenericAudioMidiOut::IsInitialized(std::string& status_message) const {
	status_message = works.status;
	return midi_out != nullptr;
}

bool GenericAudioMidiOut::IsSupported(Filesystem_Stream::InputStream& stream) {
	char magic[4] = { 0 };
	if (!stream.ReadIntoObj(magic)) {
		return false;
	}
	stream.seekg(0, std::ios::beg);
	return strncmp(magic, "MThd", 4) == 0;
}
#endif

AudioDecoderMidi& GenericAudioMidiOut::GetMidiOut() {
	assert(midi_out);
	return *midi_out;
}
