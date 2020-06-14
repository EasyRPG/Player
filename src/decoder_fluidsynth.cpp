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

fluid_settings_t* FluidSynthDecoder::settings;

FluidSynthDecoder::FluidSynthDecoder() {
	synth = new_fluid_synth(settings);

	// FIXME: Must be in Initialize but belongs to synth
	if (fluid_synth_sfload(synth, "easyrpg.soundfont", 1) == FLUID_FAILED) {
		//error_message = "Could not load soundfont.";
		return;
	}

	fluid_synth_set_interp_method(synth, -1, 7);
}

FluidSynthDecoder::~FluidSynthDecoder() {
	if (synth)
		delete_fluid_synth(synth);
}

bool FluidSynthDecoder::Initialize(std::string& error_message) {
	static bool init = false;
	static bool once = false;

	// only initialize once
	if (once)
		return init;
	once = true;

	if (!settings) {
		settings = new_fluid_settings();
		fluid_settings_setstr(settings, "player.timing-source", "sample");
		fluid_settings_setint(settings, "synth.lock-memory", 0);

		fluid_settings_setnum(settings, "synth.gain", 0.6);
		fluid_settings_setnum(settings, "synth.sample-rate", EP_MIDI_FREQ);
		fluid_settings_setint(settings, "synth.polyphony", 32);
	}

	init = true;

	return init;
}

int FluidSynthDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (fluid_synth_write_s16(synth, length / 4, buffer, 0, 2, buffer, 1, 2) == FLUID_FAILED) {
		return -1;
	}

	return length;
}

void FluidSynthDecoder::OnMidiMessage(uint32_t message) {
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
			fluid_synth_key_pressure(synth, event, param1, param2);
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
		case 0xFF:
			fluid_synth_program_reset(synth);
			break;
		default:
			break;
	}
}

#endif
