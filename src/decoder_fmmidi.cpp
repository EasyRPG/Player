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

#ifdef WANT_FMMIDI

// Headers
#include <cstdio>
#include <cassert>
#include "audio_decoder.h"
#include "output.h"
#include "decoder_fmmidi.h"

FmMidiDecoder::FmMidiDecoder() {
	note_factory.reset(new midisynth::fm_note_factory());
	synth.reset(new midisynth::synthesizer(note_factory.get()));

	load_programs();
}

int FmMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	size_t samples = (size_t)length / sizeof(int_least16_t) / 2;

	synth->synthesize(reinterpret_cast<int_least16_t*>(buffer), samples, static_cast<float>(frequency));

	return length;
}

void FmMidiDecoder::SendMidiMessage(uint32_t message) {
	synth->midi_event(message);
}

void FmMidiDecoder::SendSysExMessage(const uint8_t* data, std::size_t size) {
	synth->sysex_message(data, size);
}

void FmMidiDecoder::load_programs() {
	// beautiful
	#include "midiprogram.h"
}

#endif
