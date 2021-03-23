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

// Headers
#include "audio_midi.h"

#include <algorithm>
#include <memory>
#include "decoder_midigeneric.h"
#include "output.h"

constexpr int GenericMidiDecoder::midi_default_tempo;

// 1 ms of MIDI message resolution for a 44100 Hz samplerate
constexpr int samples_per_play = 512;
constexpr int bytes_per_sample = sizeof(int16_t) * 2;

GenericMidiDecoder::GenericMidiDecoder(MidiDecoder* mididec)
	: mididec(mididec) {
	assert(mididec);
	seq = std::make_unique<midisequencer::sequencer>();

	music_type = "midi";
}

GenericMidiDecoder::~GenericMidiDecoder() {
	mididec->OnMidiReset();
}

static int read_func(void* instance) {
	GenericMidiDecoder* midi = reinterpret_cast<GenericMidiDecoder*>(instance);

	if (midi->file_buffer_pos >= midi->file_buffer.size()) {
		return EOF;
	}

	return midi->file_buffer[midi->file_buffer_pos++];
}

bool GenericMidiDecoder::Open(Filesystem_Stream::InputStream stream) {
	seq->clear();
	file_buffer = Utils::ReadStream(stream);

	if (!seq->load(this, read_func)) {
		error_message = "Midi: Error reading file";
		return false;
	}
	seq->rewind();

	if (!mididec->Open(file_buffer)) {
		error_message = "Internal Midi: Error reading file";
		return false;
	}

	tempo.emplace_back(this, midi_default_tempo);

	return true;
}

bool GenericMidiDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	assert(!tempo.empty());

	if (offset == 0 && origin == std::ios_base::beg) {
		mtime = seq->rewind_to_loop();

		// When the loop points to the end of the track keep it alive to match
		// RPG_RT behaviour.
		loops_to_end = mtime >= seq->get_total_time();

		if (mtime > 0.0f) {
			// Throw away all tempo data after the loop point
			auto rit = std::find_if(tempo.rbegin(), tempo.rend(), [&](auto& t) { return t.mtime <= mtime; });
			auto it = rit.base();
			if (it != tempo.end()) {
				tempo.erase(it, tempo.end());
			}

			// Bit of a hack, prevent stuck notes
			// TODO: verify with a MIDI event stream inspector whether RPG_RT does this?
			// FIXME synth->all_note_off();
		} else {
			tempo.clear();
			tempo.emplace_back(this, midi_default_tempo);
		}

		if (mididec->GetName() == "WildMidi") {
			mididec->Seek(tempo.back().GetSamples(mtime), origin);
		} else {
			mididec->Seek(GetTicks(), origin);
		}

		return true;
	}

	return false;
}

std::streampos GenericMidiDecoder::Tell() const {
	return GetTicks();
}

bool GenericMidiDecoder::IsFinished() const {
	if (loops_to_end) {
		return false;
	}

	return mtime >= seq->get_total_time();
}

void GenericMidiDecoder::GetFormat(int &freq, AudioDecoder::Format &format, int &channels) const {
	mididec->GetFormat(freq, format, channels);
}

bool GenericMidiDecoder::SetFormat(int freq, AudioDecoder::Format format, int channels) {
	frequency = freq;
	return mididec->SetFormat(freq, format, channels);
}

bool GenericMidiDecoder::SetPitch(int pitch) {
	if (!mididec->SetPitch(pitch)) {
		this->pitch = 100.0f;
		return false;
	}

	this->pitch = static_cast<float>(pitch);

	return true;
}

int GenericMidiDecoder::GetTicks() const {
	assert(!tempo.empty());

	return tempo.back().GetTicks(mtime);
}

int GenericMidiDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (loops_to_end) {
		memset(buffer, '\0', length);
		return length;
	}

	int samples_max = length / bytes_per_sample;
	int written = 0;

	// Advance the MIDI playback in smaller steps to achieve a 1ms message resolution
	// Otherwise the MIDI sounds off because messages are processed too late.
	while (samples_max > 0) {
		// Process MIDI messages
		size_t samples = std::min(samples_per_play, samples_max);
		float delta = (float)samples / (frequency * 100.0f / pitch);
		seq->play(mtime, this);
		mtime += delta;

		// Write audio samples
		int len = samples * bytes_per_sample;
		int res = mididec->FillBuffer(buffer + written, len);
		written += res;

		if (samples < samples_per_play || res < len) {
			// Done
			break;
		}

		samples_max -= samples;
	}

	return written;
}

void GenericMidiDecoder::midi_message(int, uint_least32_t message) {
	mididec->OnMidiMessage(message);
}

void GenericMidiDecoder::sysex_message(int, const void *data, std::size_t size) {
	mididec->OnSysExMessage(data, size);
}

void GenericMidiDecoder::meta_event(int event, const void * data, std::size_t size) {
	assert(!tempo.empty());

	mididec->OnMetaEvent(event, data, size);

	const auto* d = reinterpret_cast<const uint8_t*>(data);

	if (size == 3 && event == 0x51) {
		uint32_t new_tempo = (static_cast<uint32_t>(static_cast<unsigned char>(d[0])) << 16)
				| (static_cast<unsigned char>(d[1]) << 8)
				| static_cast<unsigned char>(d[2]);
		tempo.emplace_back(this, new_tempo, &tempo.back());
	}
}

void GenericMidiDecoder::reset() {
	mididec->OnMidiReset();
}

GenericMidiDecoder::MidiTempoData::MidiTempoData(const GenericMidiDecoder* midi, uint32_t cur_tempo, const MidiTempoData* prev)
		: tempo(cur_tempo) {
	ticks_per_sec = (float)midi->seq->get_division() / tempo * 1000000;
	samples_per_tick = midi->frequency * 1 / ticks_per_sec;
	mtime = midi->mtime;
	if (prev) {
		float delta = mtime - prev->mtime;
		int ticks_since_last = static_cast<int>(ticks_per_sec * delta);
		ticks = prev->ticks + ticks_since_last;
		samples = prev->samples + ticks_since_last * samples_per_tick;
	}
}

int GenericMidiDecoder::MidiTempoData::GetTicks(float mtime_cur) const {
	float delta = mtime_cur - mtime;
	return ticks + static_cast<int>(ticks_per_sec * delta);
}

int GenericMidiDecoder::MidiTempoData::GetSamples(float mtime_cur) const {
	float delta = mtime_cur - mtime;
	int ticks_since_last = static_cast<int>(ticks_per_sec * delta);
	return samples + static_cast<int>(ticks_since_last * samples_per_tick);
}
