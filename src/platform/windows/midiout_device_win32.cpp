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

static std::string get_error_str(MMRESULT res) {
	char errMsg[120];
	midiOutGetErrorTextA(res, errMsg, 120);
	return std::string(errMsg);
}

Win32MidiOutDevice::Win32MidiOutDevice() {
	// TODO: Windows MIDI Mapper was removed in Windows 8.
	// This means it's impossible to change the default ("0") MIDI device
	// without third party software. We should allow specifying the MIDI device
	// ID in a config file.
	unsigned int device_id = 0;

	MMRESULT err = midiOutOpen(&midi_out, device_id, 0, 0, CALLBACK_NULL);
	if (err != MMSYSERR_NOERROR) {
		Output::Debug("Win32 midiOutOpen {} failed: ({}) {}", 0, err, get_error_str(err));
		midi_out = nullptr;
		return;
	}
}

Win32MidiOutDevice::~Win32MidiOutDevice() {
	if (midi_out) {
		midiOutReset(midi_out);
		midiOutClose(midi_out);
		midi_out = nullptr;
	}
}

void Win32MidiOutDevice::SendMidiMessage(uint32_t message) {
	midiOutShortMsg(midi_out, message);
}

void Win32MidiOutDevice::SendSysExMessage(const uint8_t* data, size_t size) {
	MIDIHDR hdr;
	MMRESULT res;
	hdr.dwBufferLength = size;
	hdr.dwBytesRecorded = size;
	hdr.dwFlags = 0;
	hdr.lpData = (LPSTR) data;
	res = midiOutPrepareHeader(midi_out, &hdr, sizeof(hdr));
	if (res != MMSYSERR_NOERROR) {
		Output::Debug("Win32 midiOutPrepareHeader failed: ({}) {}", res, get_error_str(res));
		return;
	}
	res = midiOutLongMsg(midi_out, &hdr, sizeof(hdr));
	if (res != MMSYSERR_NOERROR) {
		Output::Debug("Win32 midiOutLongMsg failed: ({}) {}", res, get_error_str(res));
	}
	res = midiOutUnprepareHeader(midi_out, &hdr, sizeof(hdr));
	if (res != MMSYSERR_NOERROR) {
		Output::Debug("Win32 midiOutUnprepareHeader failed: ({}) {}", res, get_error_str(res));
	}
}

std::string Win32MidiOutDevice::GetName() {
	return "Windows MIDI";
}

bool Win32MidiOutDevice::IsInitialized() const {
	return midi_out != nullptr;
}

#endif
