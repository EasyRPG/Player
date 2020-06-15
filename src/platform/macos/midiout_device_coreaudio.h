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

#ifndef EP_MIDIOUT_COREAUDIO_H
#define EP_MIDIOUT_COREAUDIO_H

#ifdef __APPLE__
#include "audio_midiout_device.h"
#include <AudioToolbox/AudioToolbox.h>

/**
 * Plays MIDI through the Apple's AudioToolbox
 */
class CoreAudioMidiOutDevice : public MidiOutDevice {
public:
	CoreAudioMidiOutDevice();
	~CoreAudioMidiOutDevice();

	void SendMidiMessage(uint32_t message) override;
	void SendSysExMessage(const void* data, size_t size) override;
	void SendMidiReset() override;

	bool IsOK() override {
		return graph != NULL;
	}

private:
	AudioUnit midi_out;
    AUGraph graph;
};

#endif

#endif
