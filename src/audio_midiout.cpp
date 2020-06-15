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
#include "audio_midiout.h"
#include "midisequencer.h"
#include "output.h"
#ifdef _WIN32
#include "platform/windows/midiout_device_win32.h"
#elif __APPLE__
#include "platform/macos/midiout_device_coreaudio.h"
#endif

class GenericMidiOut : public MidiOut, public midisequencer::output {
public:
	GenericMidiOut(MidiOutDevice* device);

	void SetFade(int begin, int end, int duration) override;
	void SetVolume(int volume) override;
	virtual int GetVolume() const override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Rewind() override;
	virtual int GetLoopCount() const override;
	virtual bool IsFinished() const override;
	virtual bool SetPitch(int pitch) override;
	virtual int GetPitch() const override;
	virtual int GetTicks() const override;
	virtual void Update(long long delta) override;
	virtual bool Open(Filesystem_Stream::InputStream stream) override;
	virtual bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;
	virtual void Reset() override;

	virtual std::string GetError() const override {
		return error_msg;
	};

	std::vector<uint8_t> file_buffer;
	size_t file_buffer_pos = 0;

private:
	void SendMessageToAllChannels(uint32_t midi_msg);
	std::unique_ptr<MidiOutDevice> device;
	std::unique_ptr<midisequencer::sequencer> seq;

	// midisequencer::output interface
	void midi_message(int, uint_least32_t message) override;
	void sysex_message(int, const void* data, std::size_t size) override;
	void meta_event(int, const void*, std::size_t) override;
	void reset() override;

	float mtime = 0.0f;
	float tempo_multiplier = 1.0f;
	bool paused = false;
	float volume = 0;
	float fade_end = 0;
	float delta_step = 0;
	int fade_steps = 0;
	float last_fade_mtime = 0.0f;

	std::string error_msg;

	// What was the mtime when the last set of volume MIDI messages were sent out
	float last_fade_msg_sent = 0.0f;
	std::array<uint8_t, 16> channel_volumes;

	int loop_count = 0;

	static constexpr int midi_default_tempo = 500000;
	struct MidiTempoData {
		MidiTempoData(const GenericMidiOut* midi, uint32_t cur_tempo, const MidiTempoData* prev = nullptr);

		uint32_t tempo = midi_default_tempo;
		float ticks_per_sec = 0.0f;
		float mtime = 0.0f;
		int ticks = 0;

		int GetTicks(float cur_mtime) const;
	};

	// Contains one entry per tempo change (latest on top)
	// When looping all entries after the loop point are dropped
	std::vector<MidiTempoData> tempo;

	void reset_tempos_after_loop();
};

static const uint8_t midi_event_control_change = 0b1011;
static const uint8_t midi_control_volume = 7;
static const uint8_t midi_control_all_sound_off = 120;
static const uint8_t midi_control_all_note_off = 123;
static const uint8_t midi_control_reset_all_controller = 121;

static uint32_t midimsg_make(uint8_t event_type, uint8_t channel, uint8_t value1, uint8_t value2) {
	uint32_t msg = 0;
	msg |= (((event_type << 4) & 0xF0) | (channel & 0x0F)) & 0x0000FF;
	msg |= (value1 << 8) & 0x00FF00;
	msg |= (value2 << 16) & 0xFF0000;
	return msg;
}

static uint32_t midimsg_all_note_off(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_all_note_off, 0);
}

static uint32_t midimsg_all_sound_off(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_all_sound_off, 0);
}

static uint32_t midimsg_volume(uint8_t channel, uint8_t volume) {
	return midimsg_make(midi_event_control_change, channel, midi_control_volume, volume);
}

static uint32_t midimsg_reset_all_controller(uint8_t channel) {
	return midimsg_make(midi_event_control_change, channel, midi_control_reset_all_controller, 0);
}

GenericMidiOut::GenericMidiOut(MidiOutDevice* device)
	: device(device) {
	seq = std::make_unique<midisequencer::sequencer>();
	channel_volumes.fill(127);
}

std::unique_ptr<MidiOut> MidiOut::Create(Filesystem_Stream::InputStream& stream, const std::string& filename) {
	std::unique_ptr<MidiOut> midiout = nullptr;
	char magic[4] = { 0 };
	if (!stream.ReadIntoObj(magic)) {
		return nullptr;
	}
	stream.seekg(0, std::ios::beg);
	if (strncmp(magic, "MThd", 4) != 0) {
		return nullptr;
	}
#ifdef _WIN32
	std::unique_ptr<MidiOutDevice> device = std::make_unique<Win32MidiOutDevice>();
	if (!device->IsOK()) {
		return nullptr;
	}
	midiout = std::make_unique<GenericMidiOut>(device.release());
#endif
#ifdef __APPLE__
	std::unique_ptr<MidiOutDevice> device = std::make_unique<CoreAudioMidiOutDevice>();
	if (!device->IsOK()) {
		return nullptr;
	}
	midiout = std::make_unique<GenericMidiOut>(device.release());
#endif
	return midiout;
}

static int read_func(void* instance) {
	GenericMidiOut* midiout = reinterpret_cast<GenericMidiOut*>(instance);

	if (midiout->file_buffer_pos >= midiout->file_buffer.size()) {
		return EOF;
	}

	return midiout->file_buffer[midiout->file_buffer_pos++];
}

bool GenericMidiOut::Open(Filesystem_Stream::InputStream stream) {
	seq->clear();
	file_buffer = Utils::ReadStream(stream);

	if (!seq->load(this, read_func)) {
		error_msg = "Midi: Error reading file";
		return false;
	}
	seq->rewind();
	mtime = seq->get_start_skipping_silence();

	tempo.emplace_back(this, midi_default_tempo);

	return true;
}

void GenericMidiOut::SetFade(int begin, int end, int duration) {
	fade_steps = 0;
	last_fade_mtime = 0.0f;

	if (duration <= 0.0) {
		SetVolume(end);
		return;
	}

	if (begin == end) {
		SetVolume(end);
		return;
	}

	volume = begin / 100.0f;
	fade_end = end / 100.0f;
	fade_steps = duration / 100;
	delta_step = (fade_end - volume) / fade_steps;
}

void GenericMidiOut::SetVolume(int new_volume) {
	// cancel any pending fades
	fade_steps = 0;

	volume = new_volume / 100.0f;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
		device->SendMidiMessage(msg);
	}
}

int GenericMidiOut::GetVolume()const {
	if (fade_steps > 0) {
		return static_cast<int>(fade_end * 100);
	}
	return static_cast<int>(volume * 100);
}

void GenericMidiOut::Pause() {
	paused = true;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, 0);
		device->SendMidiMessage(msg);
	}
}

void GenericMidiOut::Resume() {
	paused = false;
	for (int i = 0; i < 16; i++) {
		uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
		device->SendMidiMessage(msg);
	}
}

void GenericMidiOut::Rewind() {
	seq->rewind();
}

int GenericMidiOut::GetLoopCount() const {
	return loop_count;
}

bool GenericMidiOut::IsFinished() const {
	return seq->is_at_end();
}

bool GenericMidiOut::SetPitch(int pitch) {
	tempo_multiplier = pitch / 100.0f;
	return true;
}

int GenericMidiOut::GetPitch() const {
	return static_cast<int>(tempo_multiplier * 100);
}

int GenericMidiOut::GetTicks() const {
	assert(!tempo.empty());

	return tempo.back().GetTicks(mtime);
}

void GenericMidiOut::Update(long long delta) {
	if (paused) {
		return;
	}
	if (fade_steps >= 0 && mtime - last_fade_mtime > 0.1f) {
		volume = std::max(0.0f, std::min(1.0f, volume + delta_step));
		for (int i = 0; i < 16; i++) {
			uint32_t msg = midimsg_volume(i, static_cast<uint8_t>(channel_volumes[i] * volume));
			device->SendMidiMessage(msg);
		}
		last_fade_mtime = mtime;
		fade_steps -= 1;
	}

	seq->play(mtime, this);
	mtime = mtime + ((delta / 1000000.0f) * tempo_multiplier);

	if (IsFinished() && looping) {
		mtime = seq->rewind_to_loop();
		reset_tempos_after_loop();
	}
}

bool GenericMidiOut::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (offset == 0 && origin == std::ios_base::beg) {
		seq->rewind();
		mtime = 0.0f;
		reset_tempos_after_loop();
		SendMessageToAllChannels(midimsg_all_note_off(0));
		return true;
	}
	return false;
}

void GenericMidiOut::Reset() {
	// Generate a MIDI reset event so the device doesn't
	// leave notes playing or keeps any state
	reset();
}

void GenericMidiOut::SendMessageToAllChannels(uint32_t midi_msg) {
	for (int channel = 0; channel < 16; channel++) {
		uint8_t event_type = (midi_msg & 0x0000F0) >> 4;
		midi_msg |= (((event_type << 4) & 0xF0) | (channel & 0x0F)) & 0x0000FF;
		device->SendMidiMessage(midi_msg);
	}
}

void GenericMidiOut::midi_message(int, uint_least32_t message) {
	uint8_t event_type = (message & 0x0000F0) >> 4;
	uint8_t channel = (message & 0x00000F);
	uint8_t value1 = (message & 0x00FF00) >> 8;
	uint8_t value2 = (message & 0xFF0000) >> 16;

	if (event_type == midi_event_control_change && value1 == midi_control_volume) {
		// Adjust channel volume
		channel_volumes[channel] = value2;
		// Send the modified volume to midiout
		message = midimsg_volume(channel, static_cast<uint8_t>(value2 * volume));
	}
	device->SendMidiMessage(message);
}

void GenericMidiOut::sysex_message(int, const void* data, std::size_t size) {
	device->SendSysExMessage(data, size);
}

void GenericMidiOut::meta_event(int event, const void* data, std::size_t size) {
	// Meta events are never sent over MIDI ports.
	const auto* d = reinterpret_cast<const uint8_t*>(data);
	if (size == 3 && event == 0x51) {
		uint32_t new_tempo = (static_cast<uint32_t>(static_cast<unsigned char>(d[0])) << 16)
			| (static_cast<unsigned char>(d[1]) << 8)
			| static_cast<unsigned char>(d[2]);
		tempo.emplace_back(this, new_tempo, &tempo.back());
	}
}

void GenericMidiOut::reset() {
	// MIDI reset event
	SendMessageToAllChannels(midimsg_all_sound_off(0));
	SendMessageToAllChannels(midimsg_reset_all_controller(0));
	device->SendMidiReset();
}

void GenericMidiOut::reset_tempos_after_loop() {
	if (mtime > 0.0f) {
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

// TODO: Solve the copy-paste job between this and GenericMidiDecoder
GenericMidiOut::MidiTempoData::MidiTempoData(const GenericMidiOut* midi, uint32_t cur_tempo, const MidiTempoData* prev)
	: tempo(cur_tempo) {
	ticks_per_sec = (float)midi->seq->get_division() / tempo * 1000000;
	mtime = midi->mtime;
	if (prev) {
		float delta = mtime - prev->mtime;
		int ticks_since_last = static_cast<int>(ticks_per_sec * delta);
		ticks = prev->ticks + ticks_since_last;
	}
}

int GenericMidiOut::MidiTempoData::GetTicks(float mtime_cur) const {
	float delta = mtime_cur - mtime;
	return ticks + static_cast<int>(ticks_per_sec * delta);
}
