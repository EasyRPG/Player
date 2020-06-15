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

#ifndef EP_AUDIO_MIDIOUT_DEVICE_H
#define EP_AUDIO_MIDIOUT_DEVICE_H

#include <cstdint>
#include <cstddef>

 /**
  * Manages sequencing MIDI files and emitting MIDI events to a MIDI out
  * device.
  */
class MidiOutDevice {
public:
	virtual ~MidiOutDevice() = default;

	virtual void Pause() {}

	virtual void SetVolume(int volume) {
		(void)volume;
	}

	virtual void SendMidiMessage(uint32_t message) {
		(void)message;
	}

	virtual void SendSysExMessage(const void* data, size_t size) {
		(void)data;
		(void)size;
	}

	virtual void SendMidiReset() {}

	virtual bool IsOK() {
		return true;
	}
};

#endif
