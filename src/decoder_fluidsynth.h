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

#ifndef EP_DECODER_FLUIDSYNTH_H
#define EP_DECODER_FLUIDSYNTH_H

// Headers
#include <string>
#include <memory>
#ifdef HAVE_FLUIDSYNTH
#include <fluidsynth.h>
#endif
#include "audio_decoder.h"
#include "midisequencer.h"

/**
 * Audio decoder for MIDI powered by FluidSynth
 */
class FluidSynthDecoder : public AudioDecoder, midisequencer::output {
public:
	FluidSynthDecoder();

	~FluidSynthDecoder();

	bool WasInited() const override;

	// Audio Decoder interface
	bool Open(Filesystem_Stream::InputStream stream) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	int GetTicks() const override;

	std::vector<uint8_t> file_buffer;
	size_t file_buffer_pos = 0;
private:
	int FillBuffer(uint8_t* buffer, int length) override;

	// midisequencer::output interface
	void midi_message(int, uint_least32_t message) override;
	void sysex_message(int, const void* data, std::size_t size) override;
	void meta_event(int, const void*, std::size_t) override;
	void reset() override;

#ifdef HAVE_FLUIDSYNTH
	fluid_settings_t *settings = NULL;
	fluid_synth_t *synth = NULL;
#endif

	std::unique_ptr<midisequencer::sequencer> seq;

	int frequency = 44100;
	bool init = false;

	float mtime = 0.0f;
	bool begin = true;

};

#endif
