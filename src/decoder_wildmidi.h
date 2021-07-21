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

#ifndef EP_DECODER_WILDMIDI_H
#define EP_DECODER_WILDMIDI_H

// Headers
#include <string>
#include <memory>
#ifdef HAVE_LIBWILDMIDI
#include <wildmidi_lib.h>
#endif
#include "audio_midi.h"

/**
 * Audio decoder for MIDI powered by WildMidi
 */
class WildMidiDecoder : public MidiDecoder {
public:
	~WildMidiDecoder();

	static bool Initialize(std::string& error_message);

	// Audio Decoder interface
	bool Open(std::vector<uint8_t>& data) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	std::string GetName() override {
		return "WildMidi";
	};

	int FillBuffer(uint8_t* buffer, int length) override;

	bool SetPitch(int) override {
		return false;
	}

	bool SupportsMidiMessages() override {
		return false;
	}

private:
#ifdef HAVE_LIBWILDMIDI
	midi* handle = nullptr;
#endif

	std::vector<uint8_t> file_buffer;
};

#endif
