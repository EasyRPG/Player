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
	seq.reset(new midisequencer::sequencer());
	
	music_type = "midi";
	
	load_programs();
}

FmMidiDecoder::~FmMidiDecoder() {
	fclose(file);
}

int read_func(void* instance) {
	FmMidiDecoder* fmmidi = reinterpret_cast<FmMidiDecoder*>(instance);

	if (fmmidi->file_buffer_pos >= fmmidi->file_buffer.size()) {
		return EOF;
	}

	return fmmidi->file_buffer[fmmidi->file_buffer_pos++];
}

bool FmMidiDecoder::Open(FILE* file) {
	this->file = file;

	seq->clear();
	off_t old_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	file_buffer.resize(ftell(file) - old_pos);
	fseek(file, old_pos, SEEK_SET);
	size_t bytes_read = fread(file_buffer.data(), 1, file_buffer.size(), file);

	if ((bytes_read != file_buffer.size()) || (!seq->load(this, read_func))) {
		error_message = "FM Midi: Error reading file";
		return false;
	}
	seq->rewind();

	ticks_per_sec = (float)seq->get_division() / tempo * 1000000;

	return true;
}

bool FmMidiDecoder::Seek(size_t offset, Origin origin) {
	if (offset == 0 && origin == Origin::Begin) {
		mtime = seq->rewind_to_loop();
		mtime_last_tempo_change = mtime_ltc_loopstart;
		ticks_last_tempo_change = ticks_ltc_loopstart;
		if (mtime > 0.0f) {
			// Bit of a hack, prevent stuck notes
			// TODO: verify with a MIDI event stream inspector whether RPG_RT does this?
			synth->all_note_off();
			tempo = tempo_loopstart;
		}
		begin = true;

		return true;
	}

	return false;
}

bool FmMidiDecoder::IsFinished() const {
	return mtime >= seq->get_total_time();
}

void FmMidiDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& channels) const {
	freq = frequency;
	format = Format::S16;
	channels = 2;
}

bool FmMidiDecoder::SetFormat(int freq, AudioDecoder::Format format, int channels) {
	frequency = freq;

	if (channels != 2 || format != Format::S16) {
		return false;
	}

	return true;
}

bool FmMidiDecoder::SetPitch(int pitch) {
	this->pitch = 100.0f / pitch;

	return true;
}

int FmMidiDecoder::GetTicks() const {
	float delta = mtime - mtime_last_tempo_change;
	return ticks_last_tempo_change + static_cast<int>(ticks_per_sec * delta);
}

int FmMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	size_t samples = (size_t)length / sizeof(int_least16_t) / 2;

	float delta = (float)samples / (frequency * pitch);

	// FM Midi somehow returns immediately at the beginning when mtime is too small
	// This increments mtime until FM Midi is happy
	int notes = 0;
	do {
		seq->play(mtime, this);
		notes = synthesize(reinterpret_cast<int_least16_t*>(buffer), samples, frequency);
		mtime += delta;
	} while (begin && notes == 0 && !IsFinished());

	begin = false;

	return length;
}

int FmMidiDecoder::synthesize(int_least16_t * output, std::size_t samples, float rate) {
	return synth->synthesize(output, samples, rate);
}

void FmMidiDecoder::midi_message(int, uint_least32_t message) {
	// If message is a B06F loopstart control change...
	if ((message & 0xFFFF) == 0x6FB0) {
		// Save the state of these fields to restore them when looping
		tempo_loopstart = tempo;
		mtime_ltc_loopstart = mtime_last_tempo_change;
		ticks_ltc_loopstart = ticks_last_tempo_change;
	}
	synth->midi_event(message);
}

void FmMidiDecoder::sysex_message(int, const void * data, std::size_t size) {
	synth->sysex_message(data, size);
}

void FmMidiDecoder::meta_event(int event, const void * data, std::size_t size) {
	const auto* d = reinterpret_cast<const uint8_t*>(data);

	if (size == 3 && event == 0x51) {
		tempo = (static_cast<uint_least32_t>(static_cast<unsigned char>(d[0])) << 16)
				| (static_cast<unsigned char>(d[1]) << 8)
				| static_cast<unsigned char>(d[2]);
		ticks_per_sec = (float)seq->get_division() / tempo * 1000000;
		ticks_last_tempo_change = GetTicks();
		mtime_last_tempo_change = mtime;
	}
}

void FmMidiDecoder::reset() {
	synth->reset();
}

void FmMidiDecoder::load_programs() {
	// beautiful
	#include "midiprogram.h"
}

#endif
