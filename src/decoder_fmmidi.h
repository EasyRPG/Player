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

#ifndef _EASYRPG_AUDIO_DECODER_FMMIDI_H_
#define _EASYRPG_AUDIO_DECODER_FMMIDI_H_

#ifdef HAVE_FMMIDI

// Headers
#include <string>
#include <functional>

#include <vector>
#include <memory>
#include "audio_decoder.h"
#include "midisequencer.h"
#include "midisynth.h"

class FmMidiDecoder : public AudioDecoder, midisequencer::output {
	FmMidiDecoder();

	~FmMidiDecoder();

	// Audio Decoder interface
	bool Open(const std::string& file) override;

	const std::vector<char>& Decode(uint8_t* stream, int length) override;

	bool IsFinished() const override;

	std::string GetError() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, AudioDecoder::Channel& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, AudioDecoder::Channel channels) override;

	// midisequencer::output interface
protected:
	int synthesize(int_least16_t* output, std::size_t samples, float rate) { return synth->synthesize(output, samples, rate); }
	void midi_message(int, uint_least32_t message) { synth->midi_event(message); }
	void sysex_message(int, const void* data, std::size_t size) { synth->sysex_message(data, size); }
	void meta_event(int, const void*, std::size_t) {}
	void set_mode(midisynth::system_mode_t mode) { synth->set_system_mode(mode); }
	void reset() { synth->reset(); } /* Set here because they are pure virtual */

	midisynth::synthesizer *synth;
	midisynth::fm_note_factory *note_factory;
	midisynth::DRUMPARAMETER p;
	void load_programs();
};

#endif

#endif
