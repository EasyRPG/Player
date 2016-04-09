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

#ifdef HAVE_FMMIDI

// Headers
#include <cassert>
#include "audio_decoder.h"
#include "output.h"
#include "decoder_fmmidi.h"

FmMidiDecoder::FmMidiDecoder() {
	note_factory.reset(new midisynth::fm_note_factory());
	synth.reset(new midisynth::synthesizer(note_factory.get()));
	seq.reset(new midisequencer::sequencer());
	load_programs();
}

FmMidiDecoder::~FmMidiDecoder() {
	fclose(file);
}

bool FmMidiDecoder::Open(FILE* file) {
	this->file = file;

	seq->clear();
	seq->load(file);
	seq->rewind();

	return true;
}

bool FmMidiDecoder::Seek(size_t offset, Origin origin) {
	return false;
}

bool FmMidiDecoder::IsFinished() const {
	return false;
}

std::string FmMidiDecoder::GetError() const {
	return std::string();
}

void FmMidiDecoder::GetFormat(int& freq, AudioDecoder::Format& format, AudioDecoder::Channel& channels) const {
	freq = frequency;
	format = Format::S16;
	channels = Channel::Stereo;
}

bool FmMidiDecoder::SetFormat(int freq, AudioDecoder::Format format, AudioDecoder::Channel channels) {
	frequency = freq;

	if (channels != Channel::Stereo || format != Format::S16) {
		return false;
	}

	return true;
}

bool FmMidiDecoder::SetPitch(int pitch) {
	this->pitch = 100.0 / pitch;

	return true;
}

int FmMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	double delta = (double)2048 / (frequency * pitch);

	seq->play(mtime, this);

	synthesize(reinterpret_cast<int_least16_t*>(buffer), (size_t)length / sizeof(int_least16_t) / 2, frequency * pitch);

	mtime += delta;

	return length;
}

int FmMidiDecoder::synthesize(int_least16_t * output, std::size_t samples, float rate) {
	return synth->synthesize(output, samples, rate);
}

void FmMidiDecoder::midi_message(int, uint_least32_t message) {
	synth->midi_event(message);
}

void FmMidiDecoder::sysex_message(int, const void * data, std::size_t size) {
	synth->sysex_message(data, size);
}

void FmMidiDecoder::meta_event(int, const void *, std::size_t) {
	// no-op
}

void FmMidiDecoder::reset() {
	synth->reset();
}

void FmMidiDecoder::load_programs() {
	// beautiful
	#include "midiprogram.h"
}

#endif
