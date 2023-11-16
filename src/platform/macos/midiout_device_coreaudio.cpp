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

#ifdef __APPLE__
#include "midiout_device_coreaudio.h"
#include "output.h"

CoreAudioMidiOutDevice::CoreAudioMidiOutDevice() {
	OSStatus status = NewAUGraph(&graph);
	if (status != noErr) {
		Output::Debug("macOS Midi: NewAUGraph failed: {}", status);
		return;
	}
	AudioComponentDescription synthDesc = {
		.componentType = kAudioUnitType_MusicDevice,
		.componentSubType = kAudioUnitSubType_DLSSynth,
		.componentManufacturer = kAudioUnitManufacturer_Apple,
		.componentFlags = 0,
		.componentFlagsMask = 0
	};
	AUNode synthNode;
	status = AUGraphAddNode(graph, &synthDesc, &synthNode);

	AudioComponentDescription limiterDesc = {
		.componentType = kAudioUnitType_Effect,
		.componentSubType = kAudioUnitSubType_PeakLimiter,
		.componentManufacturer = kAudioUnitManufacturer_Apple,
		.componentFlags = 0,
		.componentFlagsMask = 0
	};

	AUNode limiterNode;
	status = AUGraphAddNode(graph, &limiterDesc, &limiterNode);

	AudioComponentDescription outputDesc = {
		.componentType = kAudioUnitType_Output,
		.componentSubType = kAudioUnitSubType_DefaultOutput,
		.componentManufacturer = kAudioUnitManufacturer_Apple,
		.componentFlags = 0,
		.componentFlagsMask = 0
	};

	AUNode soundOutNode;
	status = AUGraphAddNode(graph, &outputDesc, &soundOutNode);

	status = AUGraphConnectNodeInput(graph, synthNode, 0, limiterNode, 0);

	status = AUGraphConnectNodeInput(graph, limiterNode, 0, soundOutNode, 0);

	status = AUGraphOpen(graph);

	status = AUGraphNodeInfo(graph, synthNode, nil, &midi_out);

	status = AUGraphInitialize(graph);

	status = AUGraphStart(graph);

	if (status != noErr) {
		Output::Debug("macOS Midi: AUGraphStart failed: {}", status);
		return;
	}

	works = true;
}

CoreAudioMidiOutDevice::~CoreAudioMidiOutDevice() {
	if (graph) {
		DisposeAUGraph(graph);
	}
}

void CoreAudioMidiOutDevice::SendMidiMessage(uint32_t message) {
	uint8_t status = (message & 0x0000FF);
	uint8_t value1 = (message & 0x00FF00) >> 8;
	uint8_t value2 = (message & 0xFF0000) >> 16;
	MusicDeviceMIDIEvent(midi_out, status, value1, value2, 0);
}

void CoreAudioMidiOutDevice::SendSysExMessage(const uint8_t* data, size_t size) {
	MusicDeviceSysEx(midi_out, (const UInt8*) data, (UInt32) size);
}

std::string CoreAudioMidiOutDevice::GetName() {
	return "CoreAudio MIDI";
}

bool CoreAudioMidiOutDevice::NeedsSoftReset() {
	return true;
}

bool CoreAudioMidiOutDevice::IsInitialized() const {
	return works;
}

#endif
