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
#include "audio_midi.h"

/**
 * Plays MIDI through the Windows API
 */
class Win32MidiOutDevice : public MidiDecoder {
public:
	Win32MidiOutDevice();
	~Win32MidiOutDevice();

	void SendMidiMessage(uint32_t message) override;
	void SendSysExMessage(const uint8_t* data, size_t size) override;
	std::string GetName() override;
	bool IsInitialized() const;

private:
	HMIDIOUT midi_out;
};
#endif

#endif
