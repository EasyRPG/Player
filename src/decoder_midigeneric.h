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

#ifndef EP_DECODER_MIDI_GENERIC_H
#define EP_DECODER_MIDI_GENERIC_H

// Headers
#include "audio_decoder.h"
#include "audio_midi.h"
#include "midisequencer.h"

#if defined(GEKKO) || defined(_3DS)
#  define EP_MIDI_FREQ 22050
#else
#  define EP_MIDI_FREQ 44100
#endif

class GenericMidiDecoder : public AudioDecoder, midisequencer::output {
public:
	GenericMidiDecoder(MidiDecoder* midi_dec);

	// Audio Decoder interface
	bool Open(Filesystem_Stream::InputStream stream) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	bool SetPitch(int pitch) override;

	int GetTicks() const override;

	std::vector<uint8_t> file_buffer;
	size_t file_buffer_pos = 0;
private:
	static constexpr int midi_default_tempo = 500000;

	std::unique_ptr<MidiDecoder> mididec;

	int FillBuffer(uint8_t* buffer, int length) override;

	float mtime = 0.0f;
	float pitch = 1.0f;
	int frequency = 44100;

	struct MidiTempoData {
		MidiTempoData(const GenericMidiDecoder* midi, uint32_t cur_tempo, const MidiTempoData* prev = nullptr);

		uint32_t tempo = midi_default_tempo;
		float ticks_per_sec = 0.0f;
		float mtime = 0.0f;
		int ticks = 0;
		int samples_per_tick = 0;
		int samples = 0;

		int GetTicks(float cur_mtime) const;
		int GetSamples(float cur_mtime) const;
	};

	// Contains one entry per tempo change (latest on top)
	// When looping all entries after the loop point are dropped
	std::vector<MidiTempoData> tempo;

	// midisequencer::output interface
	void midi_message(int, uint_least32_t message) override;
	void sysex_message(int, const void* data, std::size_t size) override;
	void meta_event(int, const void*, std::size_t) override;
	void reset() override;

	std::unique_ptr<midisequencer::sequencer> seq;
};

#endif
