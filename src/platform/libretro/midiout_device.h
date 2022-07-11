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

#ifndef EP_MIDIOUT_LIBRETRO_H
#define EP_MIDIOUT_LIBRETRO_H

#include <cstdint>
#include "libretro.h"
#include "audio_midi.h"

/**
 * Plays MIDI through Libretro API
 */
class LibretroMidiOutDevice : public MidiDecoder {
public:
	LibretroMidiOutDevice();

	void SendMidiMessage(uint32_t message) override;
	void SendSysExMessage(const uint8_t* data, size_t size) override;
	std::string GetName() override;
	bool IsInitialized() const;

private:
	struct retro_midi_interface midi_out = {};
	bool works = false;
};

#endif
