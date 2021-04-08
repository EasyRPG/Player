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

#include <cassert>
#include <chrono>
#include "audio_generic_midiout.h"
#include "audio_decoder_midi.h"
#include "filesystem_stream.h"
#include "game_clock.h"

#ifdef _WIN32
#include "platform/windows/midiout_device_win32.h"
#elif __APPLE__
#include "platform/macos/midiout_device_coreaudio.h"
#endif

using namespace std::chrono_literals;


GenericAudioMidiOut::GenericAudioMidiOut() {
	stop_thread.store(false);

#ifdef _WIN32
	auto dec = std::make_unique<Win32MidiOutDevice>();
	midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
	/*FIXME if (!device->IsOK()) {
		return nullptr;
	}*/
#endif
#ifdef __APPLE__
	auto dec = std::make_unique<CoreAudioMidiOutDevice>();
	midi_out = std::make_unique<AudioDecoderMidi>(std::move(dec));
	/*if (!device->IsOK()) {
		return nullptr;
	}*/
#endif
}

GenericAudioMidiOut::~GenericAudioMidiOut() {
	if (thread_started) {
		StopThread();
	}
}

AudioDecoderMidi& GenericAudioMidiOut::GetMidiOut() {
	assert(midi_out);
	return *midi_out;
}

void GenericAudioMidiOut::LockMutex() {
	midi_mutex.lock();
}

void GenericAudioMidiOut::UnlockMutex() {
	midi_mutex.unlock();
}

void GenericAudioMidiOut::UpdateMidiOut(int delta) {
	LockMutex();
	assert(midi_out);
	midi_out->Update(delta);
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
	Game_Clock::time_point start_ticks = Game_Clock::now();
	while (!stop_thread) {
		auto ticks = Game_Clock::now();

		auto us = std::chrono::duration_cast<std::chrono::milliseconds>(ticks - start_ticks);
		UpdateMidiOut(us.count());

		Game_Clock::SleepFor(1ms);
		start_ticks = ticks;
	}
}

bool GenericAudioMidiOut::IsSupported(Filesystem_Stream::InputStream& stream) {
	char magic[4] = { 0 };
	if (!stream.ReadIntoObj(magic)) {
		return false;
	}
	stream.seekg(0, std::ios::beg);
	return strncmp(magic, "MThd", 4) == 0;
}
