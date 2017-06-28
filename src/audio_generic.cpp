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
#include "audio_generic.h"
#include "filefinder.h"
#include "output.h"

GenericAudio::BgmChannel GenericAudio::BGM_Channels[nr_of_bgm_channels];
GenericAudio::SeChannel GenericAudio::SE_Channels[nr_of_se_channels];
bool GenericAudio::BGM_PlayedOnceIndicator;
bool GenericAudio::Muted = false;

std::vector<int16_t> GenericAudio::sample_buffer;
std::vector<uint8_t> GenericAudio::scrap_buffer;
unsigned GenericAudio::scrap_buffer_size = 0;
std::vector<float> GenericAudio::mixer_buffer;

GenericAudio::GenericAudio() {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].decoder.reset();
	}
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		SE_Channels[i].se.reset();
	}
	BGM_PlayedOnceIndicator = false;

	// Initialize to some arbitrary (low-quality) format to prevent crashes
	// when the inheriting class doesn't call SetFormat
	SetFormat(12345, AudioDecoder::Format::S8, 1);
}

GenericAudio::~GenericAudio() {
}

void GenericAudio::BGM_Play(const std::string& file, int volume, int pitch, int fadein) {
	bool bgm_set = false;
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].stopped = true; //Stop all running background music
		if (!BGM_Channels[i].decoder && !bgm_set) {
			//If there is an unused bgm channel
			bgm_set = true;
			LockMutex();
			BGM_PlayedOnceIndicator = false;
			UnlockMutex();
			PlayOnChannel(BGM_Channels[i], file, volume, pitch, fadein);
		}
	}
}

void GenericAudio::BGM_Pause() {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].paused = true;
	}
}

void GenericAudio::BGM_Resume() {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].paused = false;
	}
}

void GenericAudio::BGM_Stop() {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].stopped = true; //Stop all running background music
		LockMutex();
		if (Muted) {
			// If the audio is muted the audio thread doesn't perform the deletion (it isn't running)
			// Do the cleanup on our own.
			BGM_Channels[i].decoder.reset();
		}
		UnlockMutex();
	}
}

bool GenericAudio::BGM_PlayedOnce() const {
	return BGM_PlayedOnceIndicator;
}

bool GenericAudio::BGM_IsPlaying() const {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		if (!BGM_Channels[i].stopped) {
			return true;
		};
	}
	return false;
}

unsigned GenericAudio::BGM_GetTicks() const {
	unsigned ticks = 0;
	LockMutex();
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		if (BGM_Channels[i].decoder) {
			ticks = BGM_Channels[i].decoder->GetTicks();
		}
	}
	UnlockMutex();
	return ticks;
}

void GenericAudio::BGM_Fade(int fade) {
	LockMutex();
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		if (BGM_Channels[i].decoder) {
			BGM_Channels[i].decoder->SetFade(BGM_Channels[i].decoder->GetVolume(), 0, fade);
		}
	}
	UnlockMutex();
}

void GenericAudio::BGM_Volume(int volume) {
	LockMutex();
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		if (BGM_Channels[i].decoder) {
			BGM_Channels[i].decoder->SetVolume(volume);
		}
	}
	UnlockMutex();
}

void GenericAudio::BGM_Pitch(int pitch) {
	LockMutex();
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		if (BGM_Channels[i].decoder) {
			BGM_Channels[i].decoder->SetPitch(pitch);
		}
	}
	UnlockMutex();
}

void GenericAudio::SE_Play(std::string const &file, int volume, int pitch) {
	if (Muted) return;
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		if (!SE_Channels[i].se) {
			//If there is an unused se channel
			PlayOnChannel(SE_Channels[i], file, volume, pitch);
			return;
		}
	}

	Output::Warning("Couldn't play %s SE. No free channel available", FileFinder::GetPathInsideGamePath(file).c_str());
}

void GenericAudio::SE_Stop() {
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		SE_Channels[i].stopped = true; //Stop all running sound effects
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

bool GenericAudio::PlayOnChannel(BgmChannel& chan, const std::string& file, int volume, int pitch, int fadein) {
	chan.paused = true; // Pause channel so the audio thread doesn't work on it
	chan.stopped = false; // Unstop channel so the audio thread doesn't delete it

	FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("BGM file not readable: %s", FileFinder::GetPathInsideGamePath(file).c_str());
		return false;
	}

	chan.decoder = AudioDecoder::Create(filehandle, file);
	if (chan.decoder && chan.decoder->Open(filehandle)) {
		chan.decoder->SetPitch(pitch);
		chan.decoder->SetFormat(output_format.frequency, output_format.format, output_format.channels);
		chan.decoder->SetFade(0, volume, fadein);
		chan.decoder->SetLooping(true);
		chan.paused = false; // Unpause channel -> Play it.

		return true;
	} else {
		Output::Warning("Couldn't play BGM %s. Format not supported", FileFinder::GetPathInsideGamePath(file).c_str());
		fclose(filehandle);
	}

	return false;
}

bool GenericAudio::PlayOnChannel(SeChannel& chan, const std::string& file, int volume, int pitch) {
	chan.paused = true; // Pause channel so the audio thread doesn't work on it
	chan.stopped = false; // Unstop channel so the audio thread doesn't delete it

	std::unique_ptr<AudioSeCache> cache = AudioSeCache::Create(file);
	if (cache) {
		cache->SetPitch(pitch);
		cache->SetFormat(output_format.frequency, output_format.format, output_format.channels);

		chan.se = cache->Decode();
		chan.buffer_pos = 0;
		chan.volume = volume;
		chan.paused = false; // Unpause channel -> Play it.

		return true;
	} else {
		Output::Warning("Couldn't play SE %s. Format not supported", FileFinder::GetPathInsideGamePath(file).c_str());
	}

	return false;
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
			float current_master_volume = 1.0;

			if (currently_mixed_channel.decoder && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					currently_mixed_channel.decoder.reset();
				} else {
					currently_mixed_channel.decoder->Update(1000 / 60);
					volume = current_master_volume * (currently_mixed_channel.decoder->GetVolume() / 100.0);
					currently_mixed_channel.decoder->GetFormat(frequency, sampleformat, channels);
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);

					total_volume += volume;

					// determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
					unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
					bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

					read_bytes = currently_mixed_channel.decoder->Decode(scrap_buffer.data(), bytes_to_read);

					if (read_bytes < 0) {
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
			float current_master_volume = 1.0;

			if (currently_mixed_channel.se && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					currently_mixed_channel.se.reset();
				} else {
					volume = current_master_volume * (currently_mixed_channel.volume / 100.0);
					channels = currently_mixed_channel.se->channels;
					sampleformat = currently_mixed_channel.se->format;
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);

					total_volume += volume;

					// determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
					unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
					bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

					if (currently_mixed_channel.buffer_pos + bytes_to_read >
						currently_mixed_channel.se->buffer.size()) {
						bytes_to_read = currently_mixed_channel.se->buffer.size() - currently_mixed_channel.buffer_pos;
					}

					memcpy(scrap_buffer.data(),
						   &currently_mixed_channel.se->buffer[currently_mixed_channel.buffer_pos],
						   bytes_to_read);

					currently_mixed_channel.buffer_pos += bytes_to_read;

					read_bytes = bytes_to_read;

					// Now decide what to do when a channel has reached its end
					if (currently_mixed_channel.buffer_pos >= currently_mixed_channel.se->buffer.size()) {
						// SE are only played once so free the se if finished
						currently_mixed_channel.se.reset();
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
