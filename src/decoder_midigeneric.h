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

/**
 * GenericMidiDecoder wraps a MidiDecoder and uses a FmMidi Sequencer for
 * timing and message processing.
 */
class GenericMidiDecoder : public AudioDecoder, midisequencer::output {
public:
	/**
	 * @param midi_dec MidiDecoder to wrap
	 */
	explicit GenericMidiDecoder(MidiDecoder* midi_dec);

	~GenericMidiDecoder() override;

	/**
	 * Assigns a stream to the midi decoder.
	 * Open should be only called once per audio decoder instance.
	 *
	 * @return true if initializing was successful, false otherwise
	 */
	bool Open(Filesystem_Stream::InputStream stream) override;

	/**
	 * Seeks in the midi stream. The value of offset is in Midi ticks.
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	/**
	 * @return Position in the stream in midi ticks.
	 */
	std::streampos Tell() const override;

	/**
	 * Determines whether the stream is finished.
	 *
	 * @return true stream ended
	 */
	bool IsFinished() const override;

	/**
	 * Retrieves the format of the Midi decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channels Filled with the amount of channels
	 */
	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	/**
	 * Requests a preferred format from the audio decoder. Not all decoders
	 * support everything and it's recommended to use the audio hardware
	 * for audio processing.
	 * When false is returned use GetFormat to get the real format of the
	 * output data.
	 *
	 * @param frequency Audio frequency
	 * @param format Audio format
	 * @param channels Number of channels
	 * @return true when all settings were set, otherwise false (use GetFormat)
	 */
	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	/**
	 * Sets the pitch multiplier.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 * Not all audio decoders support this. Using the audio hardware is
	 * recommended.
	 *
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	bool SetPitch(int pitch) override;

	/**
	 * @return Position in the stream in midi ticks.
	 */
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
	bool loops_to_end = false;

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
