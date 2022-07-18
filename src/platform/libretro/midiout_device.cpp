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

#include "midiout_device.h"
#include "ui.h"
#include "output.h"

LibretroMidiOutDevice::LibretroMidiOutDevice() {
	if (!LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_MIDI_INTERFACE, &midi_out)) {
		Output::Debug("libretro: GET_MIDI_INTERFACE unsupported");
		return;
	}

	if (!midi_out.output_enabled()) {
		Output::Debug("libretro: MIDI output not enabled");
		return;
	}

	works = true;
}

void LibretroMidiOutDevice::SendMidiMessage(uint32_t message) {
	unsigned int event = message & 0xFF;
	unsigned int param1 = (message >> 8) & 0xFF;
	unsigned int param2 = (message >> 16) & 0xFF;

	switch (event & 0xF0) {
		case MidiEvent_ProgramChange:
		case MidiEvent_ChannelPressure:
			midi_out.write(event, 0);
			midi_out.write(param1, 0);
			break;
		case MidiEvent_NoteOff:
		case MidiEvent_NoteOn:
		case MidiEvent_KeyPressure:
		case MidiEvent_Controller:
		case MidiEvent_PitchBend:
			midi_out.write(event, 0);
			midi_out.write(param1, 0);
			midi_out.write(param2, 0);
			break;
		default:
			break;
	}
	midi_out.flush();
}

void LibretroMidiOutDevice::SendSysExMessage(const uint8_t* data, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		midi_out.write(data[i], 0);
	}
	midi_out.flush();
}

std::string LibretroMidiOutDevice::GetName() {
	return "libretro MIDI";
}

bool LibretroMidiOutDevice::IsInitialized() const {
	return works && midi_out.output_enabled();
}
