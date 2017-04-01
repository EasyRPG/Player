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

#ifndef EASYRPG_AUDIO_DECODER_FMMIDI_H
#define EASYRPG_AUDIO_DECODER_FMMIDI_H

// Headers
#include <string>
#include <memory>
#include "audio_decoder.h"
#include "midisequencer.h"
#include "midisynth.h"

/**
 * Audio decoder for MIDI powered by FM MIDI
 */
class FmMidiDecoder : public AudioDecoder, midisequencer::output {
public:
	FmMidiDecoder();

	~FmMidiDecoder();

	// Audio Decoder interface
	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	bool SetPitch(int pitch) override;

	int GetTicks() const override;

	std::vector<uint8_t> file_buffer;
	size_t file_buffer_pos = 0;
private:
	int FillBuffer(uint8_t* buffer, int length) override;

	FILE* file;
	float mtime = 0.0f;
	float pitch = 1.0f;
	int frequency = 44100;
	bool begin = true;

	// midisequencer::output interface
	int synthesize(int_least16_t* output, std::size_t samples, float rate);
	void midi_message(int, uint_least32_t message) override;
	void sysex_message(int, const void* data, std::size_t size) override;
	void meta_event(int, const void*, std::size_t) override;
	void reset() override;

	std::unique_ptr<midisequencer::sequencer> seq;
	std::unique_ptr<midisynth::synthesizer> synth;
	std::unique_ptr<midisynth::fm_note_factory> note_factory;
	midisynth::DRUMPARAMETER p;
	void load_programs();
};

#endif
