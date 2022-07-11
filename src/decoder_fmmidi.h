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

#ifndef EP_DECODER_FMMIDI_H
#define EP_DECODER_FMMIDI_H

// Headers
#include <string>
#include <memory>
#include "audio_midi.h"
#include "midisequencer.h"
#include "midisynth.h"

/**
 * Audio decoder for MIDI powered by FM MIDI
 */
class FmMidiDecoder : public MidiDecoder {
public:
	FmMidiDecoder();

	int FillBuffer(uint8_t* buffer, int length) override;

	void SendMidiMessage(uint32_t message) override;
	void SendSysExMessage(const uint8_t* data, size_t size) override;

	std::unique_ptr<midisynth::synthesizer> synth;
	std::unique_ptr<midisynth::fm_note_factory> note_factory;
	midisynth::DRUMPARAMETER p;
	void load_programs();

	std::string GetName() override {
		return "FmMidi";
	};
};

#endif
