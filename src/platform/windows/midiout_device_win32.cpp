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

#ifdef _WIN32
#include "midiout_device_win32.h"
#include "output.h"

Win32MidiOutDevice::Win32MidiOutDevice() {
	// TODO: Windows MIDI Mapper was removed in Windows 8.
	// This means it's impossible to change the default ("0") MIDI device
	// without third party software. We should allow specifying the MIDI device
	// ID in a config file.
	unsigned int device_id = 0;

	MMRESULT err = midiOutOpen(&midi_out, device_id, 0, 0, CALLBACK_NULL);
	if (err != MMSYSERR_NOERROR) {
		Output::Debug("Open MIDI device {} failed: error {}", 0, err);
		midi_out = NULL;
	}
}

Win32MidiOutDevice::~Win32MidiOutDevice() {
	if (midi_out) {
		midiOutClose(midi_out);
		midi_out = NULL;
	}
}

void Win32MidiOutDevice::SendMidiMessage(uint32_t message)
{
	midiOutShortMsg(midi_out, message);
}

void Win32MidiOutDevice::SendSysExMessage(const void* data, size_t size)
{
	MIDIHDR hdr;
	hdr.dwBufferLength = size;
	hdr.dwBytesRecorded = size;
	hdr.lpData = (LPSTR) data;
	midiOutPrepareHeader(midi_out, &hdr, sizeof(hdr));
	midiOutLongMsg(midi_out, &hdr, sizeof(hdr));
}

void Win32MidiOutDevice::SendMidiReset()
{
	midiOutReset(midi_out);
}

#endif
