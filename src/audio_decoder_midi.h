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

#ifndef EP_AUDIO_DECODER_MIDI_H
#define EP_AUDIO_DECODER_MIDI_H

#include <memory>
#include "audio_decoder_base.h"
#include "midisequencer.h"
#include "audio_midi.h"

/**
 * Manages sequencing MIDI files and emitting MIDI events
 */
class AudioDecoderMidi final : public AudioDecoderBase, public midisequencer::output {
public:
	AudioDecoderMidi(std::unique_ptr<MidiDecoder> mididec);
	~AudioDecoderMidi();

	/**
	 * Assigns a stream to the midi decoder.
	 * Open should be only called once per audio decoder instance.
	 *
	 * @return true if initializing was successful, false otherwise
	 */
	bool Open(Filesystem_Stream::InputStream stream) override;

	/**
	 * Pauses the MIDI sequencer.
	 */
	void Pause() override;

	/**
	 * Resumes the MIDI sequencer.
	 */
	void Resume() override;

	/**
	 * Gets the volume of the MIDI device.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @return current volume (from 0 - 100)
	 */
	int GetVolume() const override;

	/**
	 * Sets the volume of the MIDI devices by sending MIDI messages
	 *
	 * @param volume (from 0-100)
	 */
	void SetVolume(int volume) override;

	/**
	 * Prepares a volume fade in/out effect.
	 * To do a fade out begin must be larger then end.
	 * Call Update to do the fade.
	 *
	 * @param end End volume (from 0-100)
	 * @param duration Fade duration in ms
	 */
	void SetFade(int end, std::chrono::milliseconds duration) override;

	/**
	 * Seeks in the midi stream. The value of offset is in Midi ticks.
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	/**
	 * Determines whether the stream is finished.
	 *
	 * @return true stream ended
	 */
	bool IsFinished() const override;

	/**
	 * Updates the volume for the fade in/out effect.
	 *
	 * @param delta Time in us since the last call of this function.
	 */
	void Update(std::chrono::microseconds delta) override;

	/**
	 * Updates Midi output. Only used by Midi out devices.
	 * For Midi out devices this must be called at least once per ms.
	 *
	 * @param delta Time in us since the last call of this function.
	 */
	void UpdateMidi(std::chrono::microseconds delta) override;

	/**
	 * Retrieves the format of the Midi decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channels Filled with the amount of channels
	 */
	void GetFormat(int& frequency, AudioDecoderBase::Format& format, int& channels) const override;

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
	bool SetFormat(int frequency, AudioDecoderBase::Format format, int channels) override;

	/**
	 * Sets the pitch multiplier.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 *
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	bool SetPitch(int pitch) override;

	/**
	 * @return Position in the stream in midi ticks.
	 */
	int GetTicks() const override;

	/**
	 * Generate a MIDI reset event so the device doesn't
	 * leave notes playing or keeps any state.
	 */
	void Reset();

	/**
	 * @return Whether the MIDI playback is paused
	 */
	bool IsPaused() const;

	std::vector<uint8_t> file_buffer;
	size_t file_buffer_pos = 0;
private:
	static constexpr int midi_default_tempo = 500000;

	int FillBuffer(uint8_t* buffer, int length) override;

	void SendMessageToAllChannels(uint32_t midi_msg);

	// midisequencer::output interface
	void midi_message(int, uint_least32_t message) override;
	void sysex_message(int, const void* data, std::size_t size) override;
	void meta_event(int, const void*, std::size_t) override;
	void reset() override;
	void reset_tempos_after_loop();

	std::chrono::microseconds mtime = std::chrono::microseconds(0);
	float pitch = 1.0f;
	bool paused = false;
	float volume = 0.0f;
	float log_volume = 0.0f; // as used by RPG_RT, for Midi decoder without event support
	bool loops_to_end = false;

	int fade_steps = 0;
	float fade_volume_end = 0;
	float delta_volume_step = 0;
	std::chrono::microseconds last_fade_mtime = std::chrono::microseconds(0);

	int frequency = EP_MIDI_FREQ;

	std::array<uint8_t, 16> channel_volumes;

	struct MidiTempoData {
		MidiTempoData(const AudioDecoderMidi* midi, uint32_t cur_tempo, const MidiTempoData* prev = nullptr);

		uint32_t tempo = midi_default_tempo;
		double ticks_per_us;
		std::chrono::microseconds mtime = std::chrono::microseconds(0);
		int ticks = 0;
		int samples_per_tick = 0;
		int samples = 0;

		int GetTicks(std::chrono::microseconds cur_mtime) const;
		int GetSamples(std::chrono::microseconds cur_mtime) const;
	};

	std::unique_ptr<midisequencer::sequencer> seq;

	std::unique_ptr<MidiDecoder> mididec;

	// Contains one entry per tempo change (latest on top)
	// When looping all entries after the loop point are dropped
	std::vector<MidiTempoData> tempo;
};

#endif
