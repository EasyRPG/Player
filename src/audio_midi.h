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

#ifndef EP_AUDIO_MIDI_H
#define EP_AUDIO_MIDI_H

// Headers
#include "audio_decoder.h"
#include "midisequencer.h"

#if defined(GEKKO) || defined(_3DS)
#  define EP_MIDI_FREQ 22050
#else
#  define EP_MIDI_FREQ 44100
#endif

namespace AudioMidi {
	std::unique_ptr<AudioDecoder> Create(Filesystem_Stream::InputStream& stream, bool resample);
}

class MidiDecoder {
public:
	virtual ~MidiDecoder() = default;

	// Audio Decoder interface
	virtual bool Open(std::vector<uint8_t>& data) {
		// Only needed when the library does not support external sequencer
		(void)data;
		return true;
	}

	virtual bool Seek(std::streamoff offset, std::ios_base::seekdir origin) {
		// Only needed when the library does not support external sequencer
		(void)offset;
		(void)origin;
		return true;
	};

	virtual void GetFormat(int& freq, AudioDecoder::Format& format, int& channels) const {
		freq = frequency;
		format = AudioDecoder::Format::S16;
		channels = 2;
	}

	virtual bool SetFormat(int frequency, AudioDecoder::Format format, int channels) {
		if (frequency != EP_MIDI_FREQ || channels != 2 || format != AudioDecoder::Format::S16)
			return false;

		return true;
	}

	virtual bool SetPitch(int pitch) {
		// Only needed when the library does not support external sequencer
		(void)pitch;
		return true;
	}

	virtual void OnMidiMessage(uint32_t message) {
		(void)message;
	}

	virtual void OnSysExMessage(const void* data, size_t size) {
		(void)data;
		(void)size;
	}

	virtual void OnMetaEvent(int event, const void* data, size_t size) {
		(void)event;
		(void)data;
		(void)size;
	}

	virtual void OnMidiReset() {}

	virtual int FillBuffer(uint8_t* buffer, int length) = 0;

	virtual std::string GetName() = 0;

protected:
	int frequency = EP_MIDI_FREQ;
};

#endif
