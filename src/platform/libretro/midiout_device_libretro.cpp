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

#include "midiout_device_libretro.h"
#include "libretro_ui.h"
#include "output.h"

LibretroMidiOutDevice::LibretroMidiOutDevice() {
	if (!LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_MIDI_INTERFACE, &midi_out)) {
		// error
		int err = 0;
	}

	if (!midi_out.output_enabled) {
		int err = 0;
	}
}

LibretroMidiOutDevice::~LibretroMidiOutDevice() {
}


void LibretroMidiOutDevice::SendMidiMessage(uint32_t message) {
	int event = message & 0xFF;
	int param1 = (message >> 8) & 0xFF;
	int param2 = (message >> 16) & 0xFF;

	switch (event & 0xF0) {
		case 0x80:
			midi_out.write(event, 0);
			midi_out.write(param1, 0);
			midi_out.write(0, 0);
			break;
		case 0xC0:
		case 0xD0:
			midi_out.write(event, 0);
			midi_out.write(param1, 0);
			break;
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			midi_out.write(event, 0);
			midi_out.write(param1, 0);
			midi_out.write(param2, 0);
			break;
		default:
			break;
	}
	midi_out.flush();
}

void LibretroMidiOutDevice::SendSysExMessage(const void* data, size_t size) {
	auto mdata = reinterpret_cast<const uint8_t*>(data);

	for (size_t i = 0; i < size; ++i) {
		midi_out.write(mdata[i], 0);
	}
	midi_out.flush();
}

void LibretroMidiOutDevice::SendMidiReset() {
	unsigned char gm_reset[] = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
	SendSysExMessage(gm_reset, sizeof(gm_reset));
}

std::string LibretroMidiOutDevice::GetName() {
	return "libretro Midi";
}
