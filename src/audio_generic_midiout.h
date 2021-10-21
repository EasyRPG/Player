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

#ifndef EP_AUDIO_GENERIC_MIDITHREAD_H
#define EP_AUDIO_GENERIC_MIDITHREAD_H

#include <memory>
#include "system.h"

class AudioDecoderMidi;
namespace Filesystem_Stream {
	class InputStream;
}

#ifdef HAVE_NATIVE_MIDI
#include <atomic>
#include <mutex>
#include <thread>

/**
 * This class manages a Midi thread for sending Midi messages to a Midi device
 * provided by the underlying operating system.
 * A thread is required to ensure the tight timing requirements of Midi messages
 * are fulfilled.
 */
class GenericAudioMidiOut final {
public:
	GenericAudioMidiOut();
	~GenericAudioMidiOut();

	AudioDecoderMidi& GetMidiOut();

	void LockMutex();
	void UnlockMutex();

	void UpdateMidiOut(std::chrono::microseconds delta);

	void StartThread();
	void StopThread();
	void ThreadFunction();
	bool IsInitialized() const;

	static bool IsSupported(Filesystem_Stream::InputStream& stream);
private:
	std::unique_ptr<AudioDecoderMidi> midi_out;

	std::mutex midi_mutex;
	std::thread midi_thread;
	bool thread_started = false;
	std::atomic_bool stop_thread;

	int midi_output_stuck = 0;
};

#else
// Stub implementation for systems without native midi support
class GenericAudioMidiOut final {
public:
	GenericAudioMidiOut() {}

	AudioDecoderMidi& GetMidiOut();

	void LockMutex() {}
	void UnlockMutex() {}

	void UpdateMidiOut(int) {}

	void StartThread() {};
	void StopThread() {};
	bool IsInitialized() const {
		return false;
	}

	static bool IsSupported(Filesystem_Stream::InputStream&) {
		return false;
	}

private:
	std::unique_ptr<AudioDecoderMidi> midi_out;
};

#endif
#endif
