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

#ifndef EP_MIDIOUT_WIN32_H
#define EP_MIDIOUT_WIN32_H

#ifdef _WIN32

 // Headers
#include <string>
#include <windows.h>
#include <mmsystem.h>
#include "audio_midiout_device.h"

/**
 * Plays MIDI through the Windows API
 */
class Win32MidiOutDevice : public MidiOutDevice {
public:
	Win32MidiOutDevice();
	~Win32MidiOutDevice();

	void SendMidiMessage(uint32_t message) override;
	void SendSysExMessage(const void* data, size_t size) override;
	void SendMidiReset() override;

private:
	HMIDIOUT midi_out;
};
#endif

#endif
