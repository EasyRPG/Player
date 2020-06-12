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
#include "decoder_fluidsynth.h"


#ifdef HAVE_FLUIDSYNTH

// Headers
#include <cassert>
#include "audio_decoder.h"
#include "output.h"
#include "decoder_fluidsynth.h"

#include <fluidsynth.h>

FluidSynthDecoder::FluidSynthDecoder() {
	music_type = "midi";

	settings = new_fluid_settings();
	fluid_settings_setstr(settings, "player.timing-source", "sample");
	fluid_settings_setint(settings, "synth.lock-memory", 0);

	fluid_settings_setnum(settings, "synth.gain", 0.6);
	fluid_settings_setint(settings, "synth.polyphony", 32);

	synth = new_fluid_synth(settings);
	if (fluid_synth_sfload(synth, "easyrpg.soundfont", 1) == FLUID_FAILED) {
		error_message = "Could not load soundfont.";
		return;
	}

	fluid_synth_set_interp_method(synth, -1, 7);

	double sample_rate = 0;
	fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
	assert(sample_rate != 0);
	frequency = sample_rate;

	seq.reset(new midisequencer::sequencer());

	init = true;
}

FluidSynthDecoder::~FluidSynthDecoder() {
	if (synth)
		delete_fluid_synth(synth);
	if (settings)
		delete_fluid_settings(settings);
}

bool FluidSynthDecoder::WasInited() const {
	return init;
}

static int read_func(void* instance) {
	FluidSynthDecoder* fluid = reinterpret_cast<FluidSynthDecoder*>(instance);

	if (fluid->file_buffer_pos >= fluid->file_buffer.size()) {
		return EOF;
	}

	return fluid->file_buffer[fluid->file_buffer_pos++];
}

bool FluidSynthDecoder::Open(Filesystem_Stream::InputStream stream) {
	if (!init)
		return false;

	seq->clear();
	file_buffer.resize(stream.GetSize());
	stream.read(reinterpret_cast<char*>(file_buffer.data()), stream.GetSize());
	size_t bytes_read = stream.gcount();

	if ((bytes_read != file_buffer.size()) || (!seq->load(this, read_func))) {
		error_message = "Fluidlite: Error reading file";
		return false;
	}
	seq->rewind();

	return true;
}

bool FluidSynthDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (offset == 0 && origin == std::ios_base::beg) {
		mtime = 0.0f;
		seq->rewind();
		begin = true;

		return true;
	}

	return false;
}

bool FluidSynthDecoder::IsFinished() const {
	return mtime >= seq->get_total_time();
}

void FluidSynthDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& chans) const {
	freq = frequency;
	format = Format::S16;
	chans = 2;
}

bool FluidSynthDecoder::SetFormat(int freq, AudioDecoder::Format format, int chans) {
	if (freq != frequency || chans != 2 || format != Format::S16)
		return false;

	return true;
}

int FluidSynthDecoder::FillBuffer(uint8_t* buffer, int length) {
	if(!init)
		return 0;

	size_t samples = (size_t)length / sizeof(int_least16_t) / 2;

	float delta = (float)samples / frequency;

	// FM Midi somehow returns immediately at the beginning when mtime is too small
	// This increments mtime until FM Midi is happy
	//int notes = 0;
	//do {
		seq->play(mtime, this);
		//notes = synthesize(reinterpret_cast<int_least16_t*>(buffer), samples, frequency);
		mtime += delta;
	//} while (begin && notes == 0 && !IsFinished());

	begin = false;

	if (fluid_synth_write_s16(synth, length / 4, buffer, 0, 2, buffer, 1, 2) == FLUID_FAILED) {
		error_message = "Fluidsynth error: %s", fluid_synth_error(synth);
	}

	return length;
}

int FluidSynthDecoder::GetTicks() const {
	if (!init)
		return 0;

	// FIXME
	return 0;
}

void FluidSynthDecoder::midi_message(int, uint_least32_t message) {
	int event = message & 0xFF;
	int channel = event & 0x0F;
	int param1 = (message >> 8) & 0x7F;
	int param2 = (message >> 16) & 0x7F;

	switch (event & 0xF0){
		case 0x80:
			fluid_synth_noteoff(synth, channel, param1);
			break;
		case 0x90:
			fluid_synth_noteon(synth, channel, param1, param2);
			break;
		case 0xA0:
			//fluid_synth_key_pressure(synth, event, param1, param2);
			break;
		case 0xB0:
			fluid_synth_cc(synth, channel, param1, param2);
			break;
		case 0xC0:
			fluid_synth_program_change(synth, channel, param1);
			break;
		case 0xD0:
			fluid_synth_channel_pressure(synth, channel, param1);
			break;
		case 0xE0:
			fluid_synth_pitch_bend(synth, channel, ((param2 & 0x7F) << 7) | (param1 & 0x7F));
			break;
		case 0xFE:
			//active_sensing = 0.33f;
			break;
		case 0xFF:
			fluid_synth_program_reset(synth);
			break;
		default:
			break;
	}
}

void FluidSynthDecoder::sysex_message(int, const void *data, std::size_t size) {
	// no-op
}

void FluidSynthDecoder::meta_event(int, const void *, std::size_t) {
	// no-op
}

void FluidSynthDecoder::reset() {

}

#endif
