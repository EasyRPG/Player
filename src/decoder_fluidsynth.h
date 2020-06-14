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
#include "audio_midi.h"

/**
 * Audio decoder for MIDI powered by FluidSynth
 */
class FluidSynthDecoder : public MidiDecoder {
public:
	FluidSynthDecoder();

	~FluidSynthDecoder();

	static bool Initialize(std::string& error_message);

	int FillBuffer(uint8_t* buffer, int length) override;

	void OnMidiMessage(uint32_t message) override;

	std::string GetName() override {
		return "FluidSynth";
	};

#ifdef HAVE_FLUIDSYNTH
	static fluid_settings_t* settings;
	fluid_synth_t* synth;
#endif
};

#endif
