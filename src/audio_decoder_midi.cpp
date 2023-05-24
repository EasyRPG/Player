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

#include <array>
#include <algorithm>
#include "audio_decoder_midi.h"
#include "midisequencer.h"
#include "output.h"

using namespace std::chrono_literals;

constexpr int AudioDecoderMidi::midi_default_tempo;

constexpr int bytes_per_sample = sizeof(int16_t) * 2;

// ~1.5 ms of MIDI message resolution
#if EP_MIDI_FREQ <= 11025
constexpr int sample_divider = 4;
#elif EP_MIDI_FREQ <= 22050
constexpr int sample_divider = 2;
#else
constexpr int sample_divider = 1;
#endif
constexpr int samples_per_play = 64 / sample_divider;

static const uint8_t midi_event_control_change = 0b1011;
static const uint8_t midi_control_volume = 7;
static const uint8_t midi_control_all_sound_off = 120;
//static const uint8_t midi_control_all_note_off = 123;
static const uint8_t midi_control_reset_all_controller = 121;

static uint32_t midimsg_make(uint8_t event_type, uint8_t channel, uint8_t value1, uint8_t value2) {
	uint32_t msg = 0;
	msg |= (((event_type << 4) & 0xF0) | (channel & 0x0F)) & 0x0000FF;
	msg |= (value1 << 8) & 0x00FF00;
	msg |= (value2 << 16) & 0xFF0000;
	return msg;
}

/*static uint32_t midimsg_all_note_off(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_all_note_off, 0);
}*/

static uint32_t midimsg_all_sound_off(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_all_sound_off, 0);
}

static uint32_t midimsg_volume(uint8_t channel, uint8_t volume) {
	return midimsg_make(midi_event_control_change, channel, midi_control_volume, volume);
}

static uint32_t midimsg_reset_all_controller(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_reset_all_controller, 0);
}

static inline uint8_t midimsg_get_event_type(uint32_t msg) {
	return (msg & 0x0000F0) >> 4;
}

static inline uint8_t midimsg_get_channel(uint32_t msg) {
	return (msg & 0x00000F);
}

static inline uint8_t midimsg_get_value1(uint32_t msg) {
	return (msg & 0x00FF00) >> 8;
}

static inline uint8_t midimsg_get_value2(uint32_t msg) {
	return (msg & 0xFF0000) >> 16;
}

AudioDecoderMidi::AudioDecoderMidi(std::unique_ptr<MidiDecoder> mididec)
	: mididec(std::move(mididec)) {
	seq = std::make_unique<midisequencer::sequencer>();
	channel_volumes.fill(127);
	music_type = "midi";
}

AudioDecoderMidi::~AudioDecoderMidi() {
	reset();
}

static int read_func(void* instance) {
	AudioDecoderMidi* midi = reinterpret_cast<AudioDecoderMidi*>(instance);

	if (midi->file_buffer_pos >= midi->file_buffer.size()) {
		return EOF;
	}

	return midi->file_buffer[midi->file_buffer_pos++];
}

bool AudioDecoderMidi::Open(Filesystem_Stream::InputStream stream) {
	Reset();
	seq->clear();

	file_buffer_pos = 0;
	file_buffer = Utils::ReadStream(stream);
	loop_count = 0;

	if (!seq->load(this, read_func)) {
		error_message = "Midi: Error reading file";
		return false;
	}
	seq->rewind();
	tempo.emplace_back(this, midi_default_tempo);
	mtime = seq->get_start_skipping_silence();
	seq->play(mtime, this);

	if (!mididec->SupportsMidiMessages()) {
		if (!mididec->Open(file_buffer)) {
			error_message = "Internal Midi: Error reading file";
			return false;
		}

		mididec->Seek(tempo.back().GetSamples(mtime), std::ios_base::beg);
	}

	return true;
}

void AudioDecoderMidi::Pause() {
	paused = true;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, 0);
		mididec->SendMidiMessage(msg);
	}
}

void AudioDecoderMidi::Resume() {
	paused = false;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
		mididec->SendMidiMessage(msg);
	}
}

int AudioDecoderMidi::GetVolume() const {
	// When handled by Midi messages fake a 100 otherwise the volume is adjusted twice

	if (!mididec->SupportsMidiMessages()) {
		return static_cast<int>(log_volume);
	}

	return 100;
}

void AudioDecoderMidi::SetVolume(int new_volume) {
	// cancel any pending fades
	fade_steps = 0;

	volume = static_cast<float>(new_volume) / 100.0f;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
		mididec->SendMidiMessage(msg);
	}

	if (!mididec->SupportsMidiMessages()) {
		log_volume = AdjustVolume(volume * 100.0f);
	}
}

void AudioDecoderMidi::SetFade(int end, std::chrono::milliseconds duration) {
	fade_steps = 0;
	last_fade_mtime = 0us;

	if (duration <= 0ms) {
		SetVolume(end);
		return;
	}

	fade_volume_end = end / 100.0f;
	fade_steps = duration.count() / 100.0;
	delta_volume_step = (fade_volume_end - volume) / fade_steps;
}

bool AudioDecoderMidi::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	assert(!tempo.empty());

	if (offset == 0 && origin == std::ios_base::beg) {
		mtime = seq->rewind_to_loop()->time;
		reset_tempos_after_loop();

		// When the loop points to the end of the track keep it alive to match
		// RPG_RT behaviour.
		loops_to_end = mtime >= seq->get_total_time();

		if (!mididec->SupportsMidiMessages()) {
			mididec->Seek(tempo.back().GetSamples(loops_to_end ? seq->get_total_time() : mtime), origin);
		}

		return true;
	}

	return false;
}

bool AudioDecoderMidi::IsFinished() const {
	if (loops_to_end) {
		return false;
	}

	return seq->is_at_end();
}

void AudioDecoderMidi::Update(std::chrono::microseconds delta) {
	(void)delta; // FIXME: Why is delta unused?

	if (paused) {
		return;
	}
	if (fade_steps >= 0 && mtime - last_fade_mtime > 0.1s) {
		volume = Utils::Clamp<float>(volume + delta_volume_step, 0.0f, 1.0f);
		if (!mididec->SupportsMidiMessages()) {
			log_volume = AdjustVolume(volume * 100.0f);
		}
		for (int i = 0; i < 16; i++) {
			uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
			mididec->SendMidiMessage(msg);
		}
		last_fade_mtime = mtime;
		fade_steps -= 1;
	}
}

void AudioDecoderMidi::UpdateMidi(std::chrono::microseconds delta) {
	if (paused) {
		return;
	}

	mtime += std::chrono::microseconds(static_cast<int>(delta.count() * pitch / 100));
	Update(delta);
	seq->play(mtime, this);

	if (IsFinished() && looping) {
		mtime = seq->rewind_to_loop()->time;
		reset_tempos_after_loop();
		loop_count += 1;
	}
}

void AudioDecoderMidi::GetFormat(int& freq, AudioDecoderBase::Format& format, int& channels) const {
	mididec->GetFormat(freq, format, channels);
}

bool AudioDecoderMidi::SetFormat(int freq, AudioDecoderBase::Format format, int channels) {
	frequency = freq;
	return mididec->SetFormat(freq, format, channels);
}

bool AudioDecoderMidi::SetPitch(int pitch) {
	if (!mididec->SupportsMidiMessages()) {
		if (!mididec->SetPitch(pitch)) {
			this->pitch = 100;
			return false;
		}
	}

	this->pitch = pitch;
	return true;
}

int AudioDecoderMidi::GetTicks() const {
	assert(!tempo.empty());

	return tempo.back().GetTicks(mtime);
}

void AudioDecoderMidi::Reset() {
	// Generate a MIDI reset event so the device doesn't
	// leave notes playing or keeps any state
	reset();
}

bool AudioDecoderMidi::IsPaused() const {
	return paused;
}

int AudioDecoderMidi::FillBuffer(uint8_t* buffer, int length) {
	if (loops_to_end) {
		memset(buffer, '\0', length);
		return length;
	}

	if (!mididec->SupportsMidiMessages()) {
		// Fast path for WildMidi as it does not care about messages
		float delta = (float)(length / bytes_per_sample) / (frequency * 100.0f / pitch);
		mtime += std::chrono::microseconds(static_cast<int>(delta * 1'000'000));
		seq->play(mtime, this);
		return mididec->FillBuffer(buffer, length);
	}

	int samples_max = length / bytes_per_sample;
	int written = 0;

	// Advance the MIDI playback in smaller steps to achieve a good message resolution
	// Otherwise the MIDI sounds off because messages are processed too late.
	while (samples_max > 0) {
		// Process MIDI messages
		size_t samples = std::min(samples_per_play, samples_max);
		float delta = (float)samples / (frequency * 100.0f / pitch);
		mtime += std::chrono::microseconds(static_cast<int>(delta * 1'000'000));
		seq->play(mtime, this);

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

void AudioDecoderMidi::SendMessageToAllChannels(uint32_t midi_msg) {
	for (int channel = 0; channel < 16; channel++) {
		midi_msg &= ~(0xFu);
		midi_msg |= (channel & 0x0F);
		mididec->SendMidiMessage(midi_msg);
	}
}

void AudioDecoderMidi::midi_message(int, uint_least32_t message) {
	uint8_t event_type = midimsg_get_event_type(message);
	uint8_t channel = midimsg_get_channel(message);
	uint8_t value1 = midimsg_get_value1(message);
	uint8_t value2 = midimsg_get_value2(message);

	if (event_type == midi_event_control_change && value1 == midi_control_volume) {
		// Adjust channel volume
		channel_volumes[channel] = value2;
		// Send the modified volume to midiout
		message = midimsg_volume(channel, static_cast<uint8_t>(value2 * volume));
	}
	mididec->SendMidiMessage(message);
}

void AudioDecoderMidi::sysex_message(int, const void* data, std::size_t size) {
	mididec->SendSysExMessage(reinterpret_cast<const uint8_t*>(data), size);
}

void AudioDecoderMidi::meta_event(int event, const void* data, std::size_t size) {
	// Meta events are never sent over MIDI ports.
	assert(!tempo.empty());
	const auto* d = reinterpret_cast<const uint8_t*>(data);
	if (size == 3 && event == 0x51) {
		uint32_t new_tempo = (static_cast<uint32_t>(static_cast<unsigned char>(d[0])) << 16)
			| (static_cast<unsigned char>(d[1]) << 8)
			| static_cast<unsigned char>(d[2]);
		tempo.emplace_back(this, new_tempo, &tempo.back());
	}
}

void AudioDecoderMidi::reset() {
	// MIDI reset event
	SendMessageToAllChannels(midimsg_all_sound_off(0));
	SendMessageToAllChannels(midimsg_reset_all_controller(0));

	// GM system on (resets most parameters)
	const unsigned char gm_reset[] = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
	mididec->SendSysExMessage(gm_reset, sizeof(gm_reset));
}

void AudioDecoderMidi::reset_tempos_after_loop() {
	if (mtime > 0us) {
		// Throw away all tempo data after the loop point
		auto rit = std::find_if(tempo.rbegin(), tempo.rend(), [&](auto& t) { return t.mtime <= mtime; });
		auto it = rit.base();
		if (it != tempo.end()) {
			tempo.erase(it, tempo.end());
		}
	} else {
		tempo.clear();
		tempo.emplace_back(this, midi_default_tempo);
	}
}

AudioDecoderMidi::MidiTempoData::MidiTempoData(const AudioDecoderMidi* midi, uint32_t cur_tempo, const MidiTempoData* prev)
	: tempo(cur_tempo) {
	ticks_per_us = (float)midi->seq->get_division() / tempo;
	samples_per_tick = midi->frequency * 1 / (ticks_per_us * 1000000);
	mtime = midi->mtime;
	if (prev) {
		std::chrono::microseconds delta = mtime - prev->mtime;
		int ticks_since_last = static_cast<int>(ticks_per_us * delta.count());
		ticks = prev->ticks + ticks_since_last;
		samples = prev->samples + ticks_since_last * samples_per_tick;
	}
}

int AudioDecoderMidi::MidiTempoData::GetTicks(std::chrono::microseconds mtime_cur) const {
	std::chrono::microseconds delta = mtime_cur - mtime;
	return ticks + static_cast<int>(ticks_per_us * delta.count());
}

int AudioDecoderMidi::MidiTempoData::GetSamples(std::chrono::microseconds mtime_cur) const {
	std::chrono::microseconds delta = mtime_cur - mtime;
	int ticks_since_last = static_cast<int>(ticks_per_us * delta.count());
	return samples + static_cast<int>(ticks_since_last * samples_per_tick);
}
