#include "audio_generic.h"
#include "filefinder.h"
#include "output.h"

#include "system.h"

#ifdef USE_SDL
#include <SDL_audio.h>

void sdl_audio_callback(void* userdata, uint8_t* stream, int length) {
	memset(stream, '\0', length);

	GenericAudio::AudioThreadCallback(static_cast<GenericAudio*>(userdata), stream, length);
}
#endif

GenericAudio::BgmChannel GenericAudio::BGM_Channels[nr_of_bgm_channels];
GenericAudio::SeChannel GenericAudio::SE_Channels[nr_of_se_channels];
bool GenericAudio::BGM_PlayedOnceIndicator;
bool GenericAudio::Muted = false;
GenericAudio::Format GenericAudio::output_format;

GenericAudio::GenericAudio() {
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].decoder.reset();
	}
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		SE_Channels[i].se.reset();
	}
	BGM_PlayedOnceIndicator = false;

#ifdef USE_SDL
	SDL_AudioSpec want = {0};
	SDL_AudioSpec have = {0};
	want.freq = 44100;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = 4096;
	want.callback = sdl_audio_callback;
	want.userdata = this;

	SDL_OpenAudio(&want, &have);

	output_format.frequency = have.freq;
	output_format.channels = have.channels;
	output_format.format = AudioDecoder::Format::S16;

	SDL_PauseAudio(0);
#endif
}

GenericAudio::~GenericAudio() {
}

void GenericAudio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	bool bgm_set = false;
	for (unsigned i = 0; i < nr_of_bgm_channels; i++) {
		BGM_Channels[i].stopped = true; //Stop all running background music
		if (!BGM_Channels[i].decoder&&!bgm_set) {
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
			//If the audio is muted the audio thread doesn't perform the deletion (it isn't running)
			//So we lock ourself in with out mutex and do the cleanup on our own.
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
			BGM_Channels[i].decoder->SetFade(BGM_Channels[i].decoder->GetVolume(),0,fade);
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

void GenericAudio::SE_Play(std::string const& file, int volume, int pitch) {
	if (Muted) return;
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		if (!SE_Channels[i].se) {
			//If there is an unused se channel
			PlayOnChannel(SE_Channels[i], file, volume, pitch);
			return;
		}
	}
}

void GenericAudio::SE_Stop() {
	for (unsigned i = 0; i < nr_of_se_channels; i++) {
		SE_Channels[i].stopped = true; //Stop all running sound effects
	}
}

void GenericAudio::Update() {
	//The update is handled in its own thread
}

bool GenericAudio::PlayOnChannel(BgmChannel& chan, std::string const& file, int volume, int pitch, int fadein) {
	chan.paused = true; //Pause channel so the audio thread doesn't work on it
	chan.stopped = false; //Unstop channel so the audio thread doesn't delete it

	FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("Audio not readable: %s", file.c_str());
		return false;
	}

	chan.decoder = AudioDecoder::Create(filehandle, file);
	if (chan.decoder && chan.decoder->Open(filehandle)) {
		chan.decoder->SetPitch(pitch);
		chan.decoder->SetFormat(output_format.frequency, output_format.format, output_format.channels);
		//chan.decoder->GetFormat(chan.samplerate, chan.sampleformat, chan.channels);
		//chan.samplesize = AudioDecoder::GetSamplesizeForFormat(chan.sampleformat);
		chan.decoder->SetFade(0,volume,fadein);
		chan.decoder->SetLooping(true);
		chan.paused = false; // Unpause channel -> Play it.
		//Output::Debug("Audio started: %s, samplerate: %u, pitch: %u", file.c_str(),chan.samplerate, pitch);
		return true;
	} else {
		Output::Debug("Audioformat of %s not supported: %s", file.c_str(),file.c_str());
		fclose(filehandle);
	}

	return false;
}


bool GenericAudio::PlayOnChannel(SeChannel& chan, std::string const& file, int volume, int pitch) {
	chan.paused = true; //Pause channel so the audio thread doesn't work on it
	chan.stopped = false; //Unstop channel so the audio thread doesn't delete it

	/*FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("Audio not readable: %s", file.c_str());
		return false;
	}*/

	std::unique_ptr<AudioSeCache> cache = AudioSeCache::Create(file);
	if (cache) {
		cache->SetPitch(pitch);
		cache->SetFormat(output_format.frequency, output_format.format, output_format.channels);

		chan.se = cache->Decode();
		//chan.decoder->GetFormat(chan.samplerate, chan.sampleformat, chan.channels);
		//chan.samplesize = AudioDecoder::GetSamplesizeForFormat(chan.sampleformat);
		chan.buffer_pos = 0;
		chan.volume = volume;
		chan.paused = false; // Unpause channel -> Play it.
		//Output::Debug("Audio started: %s, samplerate: %u, pitch: %u", file.c_str(),chan.samplerate, pitch);
		return true;
	} else {
		Output::Debug("Audioformat of %s not supported: %s", file.c_str(),file.c_str());
	}

	return false;
}

void GenericAudio::AudioThreadCallback(GenericAudio* audio, uint8_t* output_buffer, int buffer_length) {
	static std::vector<int16_t> sample_buffer;
	static std::vector<uint8_t> scrap_buffer;
	static unsigned scrap_buffer_size=0;
	static std::vector<float> mixer_buffer;
	bool channel_active=false;
	float total_volume=0;
	int samples_per_frame = buffer_length / output_format.channels / 2;
	int output_channels = 2;
	//if (RenderAudioFrames==0) {
	//	return;
	//}
	if (sample_buffer.empty()) {
		sample_buffer.resize(samples_per_frame*output_channels);
	}
	if (scrap_buffer.empty()) {
		scrap_buffer_size = samples_per_frame*output_channels*sizeof(uint32_t);
		scrap_buffer.resize(scrap_buffer_size);
	}
	if (mixer_buffer.empty()) {
		mixer_buffer.resize(samples_per_frame*output_channels);
	}

	memset(mixer_buffer.data(), '\0', mixer_buffer.size() * sizeof(float));

	for (unsigned i = 0; i < nr_of_bgm_channels+nr_of_se_channels; i++) {
		int read_bytes;
		int channels;
		int samplesize;
		int frequency;
		AudioDecoder::Format sampleformat;
		float volume;

		//Mix BGM and SE together;
		bool is_bgm_channel = i < nr_of_bgm_channels;
		bool channel_used = false;

		if (is_bgm_channel) {
			BgmChannel &currently_mixed_channel = BGM_Channels[i];
			float current_master_volume = 1.0;

			if (currently_mixed_channel.decoder && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					//LockMutex();
					currently_mixed_channel.decoder.reset();
					//UnlockMutex();
				} else {
					//LockMutex();
					currently_mixed_channel.decoder->Update(1000 / 60);
					volume = current_master_volume * (currently_mixed_channel.decoder->GetVolume() / 100.0);
					currently_mixed_channel.decoder->GetFormat(frequency, sampleformat, channels);
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);
					//UnlockMutex();

					if (volume <= 0) {
						//No volume -> no sound ->do nothing
					} else {
						total_volume += volume;

						//determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
						unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
						bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

						read_bytes = currently_mixed_channel.decoder->Decode(scrap_buffer.data(), bytes_to_read);

						if (read_bytes < 0) {
							//An error occured when reading - the channel is faulty - discard
							//LockMutex();
							currently_mixed_channel.decoder.reset();
							//UnlockMutex();
							continue; //skip this loop run - there is nothing to mix
						}

						if (!currently_mixed_channel.stopped) {
							BGM_PlayedOnceIndicator = currently_mixed_channel.decoder->GetLoopCount() > 0;
						}
					}

					channel_used = true;
				}
			}
		} else {
			SeChannel &currently_mixed_channel = SE_Channels[i - nr_of_bgm_channels];
			float current_master_volume = 1.0;

			if (currently_mixed_channel.se && !currently_mixed_channel.paused) {
				if (currently_mixed_channel.stopped) {
					//LockMutex();
					currently_mixed_channel.se.reset();
					//UnlockMutex();
				} else {
					//LockMutex();
					volume = current_master_volume * (currently_mixed_channel.volume / 100.0);
					channels = currently_mixed_channel.se->channels;
					sampleformat = currently_mixed_channel.se->format;
					samplesize = AudioDecoder::GetSamplesizeForFormat(sampleformat);
					//frequency = currently
					//UnlockMutex();

					if (volume <= 0) {
						//No volume -> no sound ->do nothing
					} else {
						total_volume += volume;

						//determine how much data has to be read from this channel (but cap at the bounds of the scrap buffer)
						unsigned bytes_to_read = (samplesize * channels * samples_per_frame);
						bytes_to_read = (bytes_to_read < scrap_buffer_size) ? bytes_to_read : scrap_buffer_size;

						if (currently_mixed_channel.buffer_pos + bytes_to_read > currently_mixed_channel.se->buffer.size()) {
							bytes_to_read = currently_mixed_channel.se->buffer.size() - currently_mixed_channel.buffer_pos;
						}

						memcpy(scrap_buffer.data(), &currently_mixed_channel.se->buffer.data()[currently_mixed_channel.buffer_pos], bytes_to_read);

						currently_mixed_channel.buffer_pos += bytes_to_read;

						read_bytes = bytes_to_read;

						//Now decide what to do when a channel has reached its end
						if (currently_mixed_channel.buffer_pos >= currently_mixed_channel.se->buffer.size()) {
							//SE are only played once so free the se if finished
							//LockMutex();
							currently_mixed_channel.se.reset();
							//UnlockMutex();
						}
					}
					channel_used = true;
				}
			}
		}

		//--------------------------------------------------------------------------------------------------------------------//
		// From here downwards the currently_mixed_channel decoder may already be freed - so don't use it below this comment. //
		//--------------------------------------------------------------------------------------------------------------------//

		if (channel_used) {
			for (unsigned ii = 0; ii < read_bytes / (samplesize * channels); ii++) {

				float vall = volume;
				float valr = vall;

				//Convert to floating point
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
						valr *= (((uint32_t *) scrap_buffer.data())[ii * channels + 1] / 2147483648.0 -
								 1.0);
						break;
					case AudioDecoder::Format::F32:
						vall *= (((float *) scrap_buffer.data())[ii * channels]);
						valr *= (((float *) scrap_buffer.data())[ii * channels + 1]);
						break;
				}

				if (!channel_active) {
					//first channel
					mixer_buffer.data()[ii * output_channels] = vall;
					if (channels > 1) {
						mixer_buffer.data()[ii * output_channels + 1] = valr;
					} else {
						mixer_buffer.data()[ii * output_channels + 1] = mixer_buffer.data()[ii *
																							output_channels];
					}
				} else {
					mixer_buffer.data()[ii * output_channels] += vall;
					if (channels > 1) {
						mixer_buffer.data()[ii * output_channels + 1] += valr;
					} else {
						mixer_buffer.data()[ii * output_channels + 1] = mixer_buffer.data()[ii *
																							output_channels];
					}
				}

			}
			channel_active = true;
		}
	}


	if (channel_active) {

		if (total_volume > 1.0) {
			float threshold=0.8;
			for (unsigned i=0; i < samples_per_frame*2; i++) {
				float sample=mixer_buffer.data()[i];
				float sign= (sample<0)? -1.0 : 1.0;
				sample/=sign;
				//dynamic range compression
				if (sample>threshold) {
					sample_buffer.data()[i]=sign*32768.0*(threshold+(1.0-threshold)*(sample-threshold)/(total_volume-threshold));
				} else {
					sample_buffer.data()[i]=sign*sample*32768.0;
				}
			}
		} else {
			//No dynamic range compression necessary
			for (unsigned i=0;i<samples_per_frame*2;i++) {
				sample_buffer.data()[i]=mixer_buffer.data()[i]*32768.0;
			}
		}

		//RenderAudioFrames(sample_buffer.get(),samples_per_frame);
		memcpy(output_buffer, sample_buffer.data(), buffer_length);
	}
}

bool GenericAudio::LockMutex() const {
#ifdef USE_SDL
	SDL_LockAudio();
#endif
	return true;
}

bool GenericAudio::UnlockMutex() const {
#ifdef USE_SDL
	SDL_UnlockAudio();
#endif
	return true;
}
