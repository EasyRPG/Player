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

#include "system.h"

#include <cstring>
#include <cassert>
#include <memory>
#include "audio_generic.h"
#include "output.h"

GenericAudio::GenericAudio(const Game_ConfigAudio& cfg) : AudioInterface(cfg) {
	int i = 0;
	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.id = i++;
		BGM_Channel.decoder.reset();
		BGM_Channel.instance = this;
	}
	i = 0;
	for (auto& SE_Channel : SE_Channels) {
		SE_Channel.id = i++;
		SE_Channel.decoder.reset();
		SE_Channel.instance = this;
	}
	BGM_PlayedOnceIndicator = false;
	midi_thread.reset();

	// Initialize to some arbitrary (low-quality) format to prevent crashes
	// when the inheriting class doesn't call SetFormat
	SetFormat(12345, AudioDecoder::Format::S8, 1);
}

void GenericAudio::BGM_Play(Filesystem_Stream::InputStream stream, int volume, int pitch, int fadein) {
	if (!stream) {
		Output::Warning("Couldn't play BGM {}: File not readable", stream.GetName());
		return;
	}

	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.stopped = true; //Stop all running background music
		if (!BGM_Channel.IsUsed()) {
			// If there is an unused bgm channel
			LockMutex();
			BGM_PlayedOnceIndicator = false;
			UnlockMutex();
			PlayOnChannel(BGM_Channel, std::move(stream), volume, pitch, fadein);
			return;
		}
	}
}

void GenericAudio::BGM_Pause() {
	for (auto& BGM_Channel : BGM_Channels) {
		if (BGM_Channel.IsUsed()) {
			BGM_Channel.SetPaused(true);
		}
	}
}

void GenericAudio::BGM_Resume() {
	for (auto& BGM_Channel : BGM_Channels) {
		if (BGM_Channel.IsUsed()) {
			BGM_Channel.SetPaused(false);
		}
	}
}

void GenericAudio::BGM_Stop() {
	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.Stop();
	}
	BGM_PlayedOnceIndicator = false;
	UnlockMutex();
}

bool GenericAudio::BGM_PlayedOnce() const {
	if (BGM_PlayedOnceIndicator) {
		return BGM_PlayedOnceIndicator;
	}

	LockMutex();
	// Audio Decoders set this in the Decoding thread
	for (auto& BGM_Channel : BGM_Channels) {
		if (BGM_Channel.midi_out_used) {
			BGM_PlayedOnceIndicator = midi_thread->GetMidiOut().GetLoopCount() > 0;
		}
	}
	UnlockMutex();

	return BGM_PlayedOnceIndicator;
}

bool GenericAudio::BGM_IsPlaying() const {
	for (auto& BGM_Channel : BGM_Channels) {
		if (!BGM_Channel.stopped) {
			return true;
		};
	}
	return false;
}

int GenericAudio::BGM_GetTicks() const {
	unsigned ticks = 0;
	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		int cur_ticks = BGM_Channel.GetTicks();
		if (cur_ticks >= 0) {
			ticks = static_cast<unsigned>(cur_ticks);
		}
	}
	UnlockMutex();
	return ticks;
}

void GenericAudio::BGM_Fade(int fade) {
	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.SetFade(fade);
	}
	UnlockMutex();
}

void GenericAudio::BGM_Volume(int volume) {
	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.SetVolume(volume);
	}
	UnlockMutex();
}

void GenericAudio::BGM_Pitch(int pitch) {
	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		BGM_Channel.SetPitch(pitch);
	}
	UnlockMutex();
}

std::string GenericAudio::BGM_GetType() const {
	std::string type;

	LockMutex();
	for (auto& BGM_Channel : BGM_Channels) {
		if (BGM_Channel.IsUsed()) {
			if (BGM_Channel.midi_out_used) {
				type = "midi";
				break;
			} else {
				type = BGM_Channel.decoder->GetType();
				break;
			}
		}
	}
	UnlockMutex();

	return type;
}

void GenericAudio::SE_Play(std::unique_ptr<AudioSeCache> se, int volume, int pitch) {
	if (!se) {
		Output::Warning("SE_Play: AudioSeCache data is NULL");
		return;
	}

	for (auto& SE_Channel : SE_Channels) {
		if (!SE_Channel.decoder) {
			//If there is an unused se channel
			PlayOnChannel(SE_Channel, std::move(se), volume, pitch);
			return;
		}
	}
	// FIXME Not displaying as warning because multiple games exhaust free channels available, see #1356
	Output::Debug("Couldn't play {} SE. No free channel available", se->GetName());
}

void GenericAudio::SE_Stop() {
	for (auto& SE_Channel : SE_Channels) {
		SE_Channel.stopped = true; //Stop all running sound effects
	}
}

void GenericAudio::Update() {
	// no-op, handled by the Decode function called through a thread
}

void GenericAudio::SetFormat(int frequency, AudioDecoder::Format format, int channels) {
	output_format.frequency = frequency;
	output_format.format = format;
	output_format.channels = channels;
}

bool GenericAudio::PlayOnChannel(BgmChannel& chan, Filesystem_Stream::InputStream filestream, int volume, int pitch, int fadein) {
	chan.paused = true; // Pause channel so the audio thread doesn't work on it
	chan.stopped = false; // Unstop channel so the audio thread doesn't delete it

	if (!filestream) {
		Output::Warning("BGM file not readable: {}", filestream.GetName());
		return false;
	}

	// Midiout is only supported on channel 0 because this is an exclusive resource
	if (chan.id == 0 && GenericAudioMidiOut::IsSupported(filestream)) {
		chan.decoder.reset();

		// FIXME: Try Fluidsynth and WildMidi first
		// If they work fallback to the normal AudioDecoder handler below
		// There should be a way to configure the order
		if (!MidiDecoder::CreateFluidsynth(true) && !MidiDecoder::CreateWildMidi(true)) {
			if (!midi_thread) {
				midi_thread = std::make_unique<GenericAudioMidiOut>();
				if (midi_thread->IsInitialized()) {
					midi_thread->StartThread();
				} else {
					midi_thread.reset();
				}
			}

			if (midi_thread) {
				midi_thread->LockMutex();
				auto &midi_out = midi_thread->GetMidiOut();
				if (midi_out.Open(std::move(filestream))) {
					midi_out.SetPitch(pitch);
					midi_out.SetVolume(0);
					midi_out.SetFade(volume, std::chrono::milliseconds(fadein));
					midi_out.SetLooping(true);
					midi_out.Resume();
					chan.paused = false;
					chan.midi_out_used = true;
					midi_thread->UnlockMutex();
					return true;
				}
				midi_thread->UnlockMutex();
			}
		}
	}

	if (midi_thread) {
		midi_thread->GetMidiOut().Reset();
	}

	chan.decoder = AudioDecoder::Create(filestream);
	chan.midi_out_used = false;
	if (chan.decoder && chan.decoder->Open(std::move(filestream))) {
		chan.decoder->SetPitch(pitch);
		chan.decoder->SetFormat(output_format.frequency, output_format.format, output_format.channels);
		chan.decoder->SetVolume(0);
		chan.decoder->SetFade(volume, std::chrono::milliseconds(fadein));
		chan.decoder->SetLooping(true);
		chan.paused = false; // Unpause channel -> Play it.

		return true;
	} else {
		Output::Warning("Couldn't play BGM {}. Format not supported", filestream.GetName());
	}

	return false;
}

bool GenericAudio::PlayOnChannel(SeChannel& chan, std::unique_ptr<AudioSeCache> se, int volume, int pitch) {
	chan.paused = true; // Pause channel so the audio thread doesn't work on it
	chan.stopped = false; // Unstop channel so the audio thread doesn't delete it

	chan.decoder = se->CreateSeDecoder();
	chan.decoder->SetPitch(pitch);
	chan.decoder->SetFormat(output_format.frequency, output_format.format, output_format.channels);
	chan.decoder->SetVolume(volume);
	chan.paused = false; // Unpause channel -> Play it.
	return true;
}

void GenericAudio::Decode(uint8_t* output_buffer, int buffer_length) {
	bool channel_active = false;
	float total_volume = 0;
	int samples_per_frame = buffer_length / output_format.channels / 2;

	assert(buffer_length > 0);

	if (sample_buffer.size() != (size_t)buffer_length) {
		sample_buffer.resize(buffer_length);
	}
	if (mixer_buffer.size() != (size_t)buffer_length) {
		mixer_buffer.resize(buffer_length);
	}
	scrap_buffer_size = samples_per_frame * output_format.channels * sizeof(uint32_t);
	if (scrap_buffer.size() != scrap_buffer_size) {
		scrap_buffer.resize(scrap_buffer_size);
	}
	memset(mixer_buffer.data(), '\0', mixer_buffer.size());

	for (unsigned i = 0; i < nr_of_bgm_channels + nr_of_se_channels; i++) {
		int read_bytes = 0;
		int channels = 0;
		int samplesize = 0;
		int frequency = 0;
		AudioDecoder::Format sampleformat;
		float volume;

		// Mix BGM and SE together;
		bool is_bgm_channel = i < nr_of_bgm_channels;
		bool channel_used = false;

		if (is_bgm_channel) {
			BgmChannel& currently_mixed_channel = BGM_Channels[i];
			float current_master_volume = cfg.music_volume.Get() / 100.0f;

			if (currently_mixed_channel.decoder && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					currently_mixed_channel.decoder.reset();
				} else {
					currently_mixed_channel.decoder->Update(std::chrono::microseconds(1000 * 1000 / 60));
					volume = current_master_volume * (currently_mixed_channel.decoder->GetVolume() / 100.0);
					currently_mixed_channel.decoder->GetFormat(frequency, sampleformat, channels);
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);

					total_volume += volume;

					// determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
					unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
					bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

					read_bytes = currently_mixed_channel.decoder->Decode(scrap_buffer.data(), bytes_to_read);

					if (read_bytes <= 0) {
						// An error occured when reading - the channel is faulty - discard
						currently_mixed_channel.decoder.reset();
						continue; // skip this loop run - there is nothing to mix
					}

					if (!currently_mixed_channel.stopped) {
						BGM_PlayedOnceIndicator = currently_mixed_channel.decoder->GetLoopCount() > 0;
					}

					channel_used = true;
				}
			}
		} else {
			SeChannel& currently_mixed_channel = SE_Channels[i - nr_of_bgm_channels];
			float current_master_volume = cfg.sound_volume.Get() / 100.0f;

			if (currently_mixed_channel.decoder && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					currently_mixed_channel.decoder.reset();
				} else {
					volume = current_master_volume * (currently_mixed_channel.decoder->GetVolume() / 100.0);
					currently_mixed_channel.decoder->GetFormat(frequency, sampleformat, channels);
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);

					total_volume += volume;

					// determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
					unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
					bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

					read_bytes = currently_mixed_channel.decoder->Decode(scrap_buffer.data(), bytes_to_read);

					if (read_bytes <= 0) {
						// An error occured when reading - the channel is faulty - discard
						currently_mixed_channel.decoder.reset();
						continue; // skip this loop run - there is nothing to mix
					}

					// Now decide what to do when a channel has reached its end
					if (currently_mixed_channel.decoder->IsFinished()) {
						// SE are only played once so free the se if finished
						currently_mixed_channel.decoder.reset();
					}

					channel_used = true;
				}
			}
		}

		//--------------------------------------------------------------------------------------------------------------------//
		// From here downwards the currently_mixed_channel decoder may already be freed - so don't use it below this comment. //
		//--------------------------------------------------------------------------------------------------------------------//

		if (channel_used) {
			for (unsigned ii = 0; ii < (unsigned)(read_bytes / (samplesize * channels)); ii++) {

				float vall = volume;
				float valr = vall;

				// Convert to floating point
				switch (sampleformat) {
					case AudioDecoder::Format::S8:
						vall *= (((int8_t *) scrap_buffer.data())[ii * channels] / 128.0);
						valr *= (((int8_t *) scrap_buffer.data())[ii * channels + 1] / 128.0);
						break;
					case AudioDecoder::Format::U8:
						vall *= (((uint8_t *) scrap_buffer.data())[ii * channels] / 128.0 - 1.0);
						valr *= (((uint8_t *) scrap_buffer.data())[ii * channels + 1] / 128.0 - 1.0);
						break;
					case AudioDecoder::Format::S16:
						vall *= (((int16_t *) scrap_buffer.data())[ii * channels] / 32768.0);
						valr *= (((int16_t *) scrap_buffer.data())[ii * channels + 1] / 32768.0);
						break;
					case AudioDecoder::Format::U16:
						vall *= (((uint16_t *) scrap_buffer.data())[ii * channels] / 32768.0 - 1.0);
						valr *= (((uint16_t *) scrap_buffer.data())[ii * channels + 1] / 32768.0 - 1.0);
						break;
					case AudioDecoder::Format::S32:
						vall *= (((int32_t *) scrap_buffer.data())[ii * channels] / 2147483648.0);
						valr *= (((int32_t *) scrap_buffer.data())[ii * channels + 1] / 2147483648.0);
						break;
					case AudioDecoder::Format::U32:
						vall *= (((uint32_t *) scrap_buffer.data())[ii * channels] / 2147483648.0 - 1.0);
						valr *= (((uint32_t *) scrap_buffer.data())[ii * channels + 1] / 2147483648.0 - 1.0);
						break;
					case AudioDecoder::Format::F32:
						vall *= (((float *) scrap_buffer.data())[ii * channels]);
						valr *= (((float *) scrap_buffer.data())[ii * channels + 1]);
						break;
				}

				if (!channel_active) {
					// first channel
					mixer_buffer[ii * output_format.channels] = vall;
					if (channels > 1) {
						mixer_buffer[ii * output_format.channels + 1] = valr;
					} else {
						mixer_buffer[ii * output_format.channels + 1] = mixer_buffer[ii * output_format.channels];
					}
				} else {
					mixer_buffer[ii * output_format.channels] += vall;
					if (channels > 1) {
						mixer_buffer[ii * output_format.channels + 1] += valr;
					} else {
						mixer_buffer[ii * output_format.channels + 1] = mixer_buffer[ii * output_format.channels];
					}
				}

			}
			channel_active = true;
		}
	}

	if (channel_active) {
		if (total_volume > 1.0) {
			float threshold = 0.8;
			for (unsigned i = 0; i < (unsigned)(samples_per_frame * 2); i++) {
				float sample = mixer_buffer[i];
				float sign = (sample < 0) ? -1.0 : 1.0;
				sample /= sign;
				//dynamic range compression
				if (sample > threshold) {
					sample_buffer[i] = sign * 32768.0 * (threshold + (1.0 - threshold) * (sample - threshold) / (total_volume - threshold));
				} else {
					sample_buffer[i] = sign * sample * 32768.0;
				}
			}
		} else {
			//No dynamic range compression necessary
			for (unsigned i = 0; i < (unsigned)(samples_per_frame * 2); i++) {
				sample_buffer[i] = mixer_buffer[i] * 32768.0;
			}
		}

		memcpy(output_buffer, sample_buffer.data(), buffer_length);
	} else {
		memset(output_buffer, '\0', buffer_length);
	}
}

void GenericAudio::BgmChannel::Stop() {
	stopped = true;
	if (midi_out_used) {
		midi_out_used = false;
		instance->midi_thread->GetMidiOut().Reset();
		instance->midi_thread->GetMidiOut().Pause();
	} else if (decoder) {
		decoder.reset();
	}
}

void GenericAudio::BgmChannel::SetPaused(bool newPaused) {
	paused = newPaused;
	if (midi_out_used) {
		if (newPaused) {
			instance->midi_thread->GetMidiOut().Pause();
		} else {
			instance->midi_thread->GetMidiOut().Resume();
		}
	}
}

int GenericAudio::BgmChannel::GetTicks() const {
	if (midi_out_used) {
		return instance->midi_thread->GetMidiOut().GetTicks();
	} else if (decoder) {
		return decoder->GetTicks();
	}
	return -1;
}

void GenericAudio::BgmChannel::SetFade(int fade) {
	if (midi_out_used) {
		instance->midi_thread->GetMidiOut().SetFade(0, std::chrono::milliseconds(fade));
	} else if (decoder) {
		decoder->SetFade(0, std::chrono::milliseconds(fade));
	}
}

void GenericAudio::BgmChannel::SetVolume(int volume) {
	if (midi_out_used) {
		instance->midi_thread->GetMidiOut().SetVolume(volume);
	} else if (decoder) {
		decoder->SetVolume(volume);
	}
}

void GenericAudio::BgmChannel::SetPitch(int pitch) {
	if (midi_out_used) {
		instance->midi_thread->GetMidiOut().SetPitch(pitch);
	} else if (decoder) {
		decoder->SetPitch(pitch);
	}
}

bool GenericAudio::BgmChannel::IsUsed() const {
	return decoder || midi_out_used;
}
