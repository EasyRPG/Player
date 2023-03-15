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

#include "audio_midi.h"
#include "system.h"

#if defined(HAVE_FLUIDSYNTH) && defined(HAVE_FLUIDLITE)
#error "Only HAVE_FLUIDSYNTH or HAVE_FLUIDLITE may be defined!"
#endif

#if defined(HAVE_FLUIDSYNTH)
#include <fluidsynth.h>
#elif defined(HAVE_FLUIDLITE)
#include <fluidlite.h>
#define FLUID_FAILED (-1)
#endif

/**
 * Audio decoder for MIDI powered by FluidSynth or FluidLite
 */
class FluidSynthDecoder : public MidiDecoder {
public:
	FluidSynthDecoder();
	~FluidSynthDecoder() override;

	static bool Initialize(std::string& error_message);

	/**
	 * Sets the name of the preferred soundfont.
	 * Must be called before the first MIDI is played.
	 *
	 * @param sf soundfont to check for first
	 */
	static void SetSoundfont(StringView sf);

	int FillBuffer(uint8_t* buffer, int length) override;

	void SendMidiMessage(uint32_t message) override;

	void SendSysExMessage(const uint8_t* data, size_t size) override;

	std::string GetName() override {
#if defined(HAVE_FLUIDSYNTH)
		return "FluidSynth";
#else
		return "FluidLite";
#endif
	};

private:
#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	fluid_synth_t* GetSynthInstance();

	fluid_synth_t* local_synth = nullptr;
	bool use_global_synth = false;
#endif
};

#endif
