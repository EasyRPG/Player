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

#include "midiout_device_alsa.h"
#include "output.h"
#include "system.h"

AlsaMidiOutDevice::AlsaMidiOutDevice() {
	int status = snd_seq_open(&midi_out, "default", SND_SEQ_OPEN_DUPLEX, 0);
	if (status < 0) {
		Output::Debug("ALSA MIDI: snd_seq_open failed: {}", snd_strerror(status));
		return;
	}

	snd_seq_client_info_t* client_info;
	snd_seq_port_info_t* port_info;
	snd_seq_client_info_alloca(&client_info);
	snd_seq_port_info_alloca(&port_info);

	// TODO: This simply enumerates all devices and attempts to find a suitable device
	// Currently prefers fluidsynth or timidity
	// If they are not found connects to a different one and likely has no sound
	// There should be a way to configure this
	std::string dst_client_name;
	std::string dst_port_name;
	bool candidate_found = false;

	snd_seq_client_info_set_client(client_info, -1);
	while (snd_seq_query_next_client(midi_out, client_info) == 0) {
		const char* client_name = snd_seq_client_info_get_name(client_info);
		if (StringView(client_name) == "Midi Through") {
			continue;
		}

		int dst_client_candidate = snd_seq_client_info_get_client(client_info);
		snd_seq_port_info_set_client(port_info, dst_client_candidate);
		snd_seq_port_info_set_port(port_info, -1);

		while (snd_seq_query_next_port(midi_out, port_info) == 0) {
			unsigned int port_caps = snd_seq_port_info_get_capability(port_info);
			unsigned int port_type = snd_seq_port_info_get_type(port_info);
			const int type = SND_SEQ_PORT_TYPE_MIDI_GENERIC;
			const int cap = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;

			if ((port_type & type) == type && (port_caps & cap) == cap)	{
				// This is a suitable client
				dst_client = dst_client_candidate;
				dst_client_name = client_name;
				dst_port = snd_seq_port_info_get_port(port_info);
				dst_port_name = snd_seq_port_info_get_name(port_info);
				candidate_found = true;

				// FIXME: Hardcoded, no config scene yet
				if (StringView(client_name).starts_with("FLUID") || StringView(client_name).starts_with("TiMidity")) {
					// Perfect candidate found, stop searching
					goto done;
				}
			}
		}
	}
	done:;

	if (!candidate_found) {
		Output::Debug("ALSA MIDI: No suitable client found");
		return;
	}

	Output::Debug("ALSA MIDI: Using client {}:{}:{}", dst_client, dst_port_name, dst_port);

	status = snd_seq_create_simple_port(midi_out, "Harmony",
		SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
	if (status < 0) {
		Output::Debug("ALSA MIDI: snd_seq_create_simple_port failed: {}", snd_strerror(status));
		return;
	}

	snd_seq_set_client_name(midi_out, GAME_TITLE);

	status = snd_seq_connect_to(midi_out, 0, dst_client, dst_port);
	if (status < 0) {
		Output::Debug("ALSA MIDI: snd_seq_connect_to failed: {}", snd_strerror(status));
		return;
	}

	queue = snd_seq_alloc_named_queue(midi_out, GAME_TITLE);
	if (queue < 0) {
		Output::Debug("ALSA MIDI: snd_seq_connect_to failed: {}", snd_strerror(queue));
		return;
	}

	status = snd_seq_start_queue(midi_out, queue, nullptr);
	if (status < 0) {
		Output::Debug("ALSA MIDI: snd_seq_connect_to failed: {}", snd_strerror(status));
		return;
	}

	works = true;
}

AlsaMidiOutDevice::~AlsaMidiOutDevice() {
	if (midi_out) {
		snd_seq_close(midi_out);
		midi_out = nullptr;
	}
}

void AlsaMidiOutDevice::SendMidiMessage(uint32_t message) {
	snd_seq_event_t evt = {};
	snd_seq_ev_set_source(&evt, 0);
	evt.queue = queue;
	snd_seq_ev_set_dest(&evt, dst_client, dst_port);

	unsigned int event = message & 0xF0;
	unsigned int channel = message & 0x0F;
	unsigned int param1 = (message >> 8) & 0x7F;
	unsigned int param2 = (message >> 16) & 0x7F;

	switch (event) {
		case MidiEvent_NoteOff:
			snd_seq_ev_set_noteoff(&evt, channel, param1, param2);
			break;
		case MidiEvent_NoteOn:
			snd_seq_ev_set_noteon(&evt, channel, param1, param2);
			break;
		case MidiEvent_KeyPressure:
			snd_seq_ev_set_keypress(&evt, channel, param1, param2);
			break;
		case MidiEvent_Controller:
			snd_seq_ev_set_controller(&evt, channel, param1, param2);
			break;
		case MidiEvent_ProgramChange:
			snd_seq_ev_set_pgmchange(&evt, channel, param1);
			break;
		case MidiEvent_ChannelPressure:
			snd_seq_ev_set_chanpress(&evt, channel, param1);
			break;
		case MidiEvent_PitchBend: {
			int arg = ((param2 & 0x7F) << 7) | (param1 & 0x7F);
			// ALSA pitchbend is centered at 0 instead of 0x2000 as MIDI standard does
			snd_seq_ev_set_pitchbend(&evt, channel, arg - 0x2000);
			break;
		}
		default:
			break;
	}

	int status = snd_seq_event_output_direct(midi_out, &evt);
	if (status < 0) {
		Output::Debug("ALSA MIDI: snd_seq_event_output_direct failed: {}", snd_strerror(status));
	}
}

void AlsaMidiOutDevice::SendSysExMessage(const uint8_t* data, size_t size) {
	snd_seq_event_t evt = {};
	snd_seq_ev_set_source(&evt, 0);
	evt.queue = queue;
	snd_seq_ev_set_dest(&evt, dst_client, dst_port);

	snd_seq_ev_set_sysex(&evt, size, const_cast<void*>(reinterpret_cast<const void*>(data)));

	int status = snd_seq_event_output_direct(midi_out, &evt);
	if (status < 0) {
		Output::Debug("ALSA MIDI: SysEx snd_seq_event_output_direct failed: {}", snd_strerror(status));
	}
}

std::string AlsaMidiOutDevice::GetName() {
	return "ALSA MIDI";
}

bool AlsaMidiOutDevice::IsInitialized() const {
	return works;
}
