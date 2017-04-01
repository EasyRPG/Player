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

#if defined(_3DS) && defined(SUPPORT_AUDIO)

#include <cstring>

#include "audio_3ds.h"
#include "filefinder.h"
#include "baseui.h"
#include "output.h"
#include "audio_secache.h"

namespace {
	const int samples_per_buf = 4096 / 2;
	const int bytes_per_sample = 4;
	const int bgm_channel = 0;
	const int se_channel_begin = 1;
	const int se_channel_end = 23;
	const int samplerate = 22050;
	int fill_block = 0;
	bool is_new_3ds;
}

bool set_channel_format(int dsp_chn, AudioDecoder::Format format, int channels, AudioDecoder::Format &out_format) {
	// false case tries to set to a close format
	out_format = format;

	switch (format) {
		case AudioDecoder::Format::U8:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM8 : NDSP_FORMAT_STEREO_PCM8);
			out_format = AudioDecoder::Format::S8;
			return false;
		case AudioDecoder::Format::S8:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM8 : NDSP_FORMAT_STEREO_PCM8);
			return true;
		case AudioDecoder::Format::U16:
		case AudioDecoder::Format::U32:
		case AudioDecoder::Format::S32:
		case AudioDecoder::Format::F32:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);
			out_format = AudioDecoder::Format::S16;
			return false;
		case AudioDecoder::Format::S16:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);
			return true;
	}

	return false;
}

void CtrAudio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("Audio not readable: %s", file.c_str());
		return;
	}

	LockMutex();
	bgm_decoder = AudioDecoder::Create(filehandle, file);
	if (bgm_decoder && bgm_decoder->Open(filehandle)) {
		int frequency;
		AudioDecoder::Format format, out_format;
		int channels;

		bgm_decoder->SetPitch(is_new_3ds ? pitch : 100);
		bgm_decoder->GetFormat(frequency, format, channels);
		bgm_decoder->SetFade(0,volume,fadein);
		bgm_decoder->SetLooping(true);

		if (!set_channel_format(bgm_channel, format, channels, out_format)) {
			bgm_decoder->SetFormat(frequency, out_format, channels);
		}
		ndspChnSetRate(bgm_channel, frequency);
	} else {
		Output::Warning("Couldn't play BGM %s: Format not supported", file.c_str());
		fclose(filehandle);
	}

	UnlockMutex();
	return;
}

void CtrAudio::BGM_Pause()  {
	ndspChnSetPaused(bgm_channel, true);
}

void CtrAudio::BGM_Resume()  {
	bgm_starttick = DisplayUi->GetTicks();
	ndspChnSetPaused(bgm_channel, false);
}

void CtrAudio::BGM_Stop() {
	LockMutex();
	bgm_decoder.reset();
	UnlockMutex();
}

bool CtrAudio::BGM_PlayedOnce() const {
	if (!bgm_decoder) {
		return true;
	}

	return bgm_decoder->GetLoopCount() > 0;
}

bool CtrAudio::BGM_IsPlaying() const {
	bool res = bgm_decoder && !bgm_decoder->IsFinished();

	return res;
}

unsigned CtrAudio::BGM_GetTicks() const {
	if (bgm_decoder) {
		return bgm_decoder->GetTicks();
	}

	return 0;
}

void CtrAudio::BGM_Fade(int fade) {
	if (bgm_decoder) {
		bgm_starttick = DisplayUi->GetTicks();
		bgm_decoder->SetFade(bgm_decoder->GetVolume(),0,fade);
	}
}

void CtrAudio::BGM_Volume(int volume) {
	if (bgm_decoder) {
		bgm_decoder->SetVolume(volume);
	}
}

void CtrAudio::BGM_Pitch(int pitch) {
	if (is_new_3ds && bgm_decoder) {
		bgm_decoder->SetPitch(pitch);
	}
}

void CtrAudio::SE_Play(std::string const& file, int volume, int pitch) {
	int se_channel = -1;

	for (int i = se_channel_begin; i <= se_channel_end; ++i) {
		if (!ndspChnIsPlaying(i)) {
			se_channel = i - 1;
			break;
		}
	}

	if (se_channel == -1) {
		Output::Warning("Couldn't play %s SE: No free channel available", file.c_str());
		return;
	}

	std::unique_ptr<AudioSeCache> cache = AudioSeCache::Create(file);
	if (cache) {
		cache->SetPitch(pitch);
		cache->SetFormat(samplerate, AudioDecoder::Format::S16, 2);

		AudioSeRef se_ref = cache->Decode();

		if (se_buf[se_channel].data_pcm16 != nullptr) {
			linearFree(se_buf[se_channel].data_pcm16);
		}

		size_t bsize = se_ref->buffer.size();
		size_t aligned_bsize = 8192;
		// Buffer must be correctly aligned to prevent audio glitches
		for (; ; aligned_bsize *= 2) {
			if (aligned_bsize > bsize) {
				se_buf[se_channel].data_pcm16 = reinterpret_cast<s16*>(linearAlloc(aligned_bsize));
				memset(se_buf[se_channel].data_pcm16, '\0', aligned_bsize);
				break;
			}
		}

		se_buf[se_channel].nsamples = bsize / 4;

		memcpy(se_buf[se_channel].data_pcm16, se_ref->buffer.data(), se_ref->buffer.size());

		DSP_FlushDataCache(se_buf[se_channel].data_pcm16, aligned_bsize);

		float mix[12] = {0};
		mix[0] = volume / 100.0f;
		mix[1] = mix[0];
		ndspChnSetMix(0, mix);

		ndspChnWaveBufAdd(se_channel + 1, &se_buf[se_channel]);
	} else {
		Output::Warning("Couldn't play SE %s: Format not supported", file.c_str());
	}
}

void CtrAudio::SE_Stop() {
	for (int i = se_channel_begin; i <= se_channel_end; ++i) {
		ndspChnWaveBufClear(i);
	}
}

void CtrAudio::Update() {
	if (bgm_decoder) {
		int t = DisplayUi->GetTicks();
		bgm_decoder->Update(t - bgm_starttick);
		bgm_starttick = t;
	}
}

void n3ds_dsp_callback(void* userdata) {
	CtrAudio* audio = static_cast<CtrAudio*>(userdata);

	if (audio->bgm_buf[fill_block].status == NDSP_WBUF_DONE) {
		audio->bgm_buf[fill_block].status = NDSP_WBUF_FREE;
		LightEvent_Signal(&audio->audio_event);
	}
}

void n3ds_end_audio_thread(CtrAudio* audio) {
	if (audio->term_stream) {
		audio->term_stream = false;
		threadExit(0);
	}
}

void n3ds_audio_thread(void* userdata) {
	CtrAudio* audio = static_cast<CtrAudio*>(userdata);

	float mix[12] = {0};

	for (;;) {
		n3ds_end_audio_thread(audio);

		LightEvent_Wait(&audio->audio_event);

		n3ds_end_audio_thread(audio);

		int target_block = fill_block;


		++fill_block;
		fill_block %= 2;

		audio->LockMutex();
		if (!audio->bgm_decoder) {
			audio->UnlockMutex();
			audio->bgm_buf[target_block].status = NDSP_WBUF_DONE;
			continue;
		}

		audio->bgm_decoder->Decode(reinterpret_cast<uint8_t*>(
			audio->bgm_buf[target_block].data_pcm16),
			samples_per_buf * bytes_per_sample
		);
		DSP_FlushDataCache(audio->bgm_buf[target_block].data_pcm16,samples_per_buf);

		mix[0] = audio->bgm_decoder->GetVolume() / 100.0f;
		mix[1] = mix[0];
		ndspChnSetMix(bgm_channel, mix);

		ndspChnWaveBufAdd(bgm_channel, &audio->bgm_buf[target_block]);
		audio->UnlockMutex();
	}
}

CtrAudio::CtrAudio() {
	Result res = ndspInit();

	if (res != 0){
		Output::Error("Couldn't initialize audio.\nError code: 0x%X\n", res);
	}

	LightLock_Init(&audio_mutex);

	bgm_audio_buffer = reinterpret_cast<uint32_t*>(linearAlloc(samples_per_buf * bytes_per_sample * 2));
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	for (int i = 0; i <= se_channel_end; ++i) {
		ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
		ndspChnSetRate(i, samplerate);
		ndspChnSetFormat(i, NDSP_FORMAT_STEREO_PCM16);
	}

	memset(bgm_buf, '\0', sizeof(bgm_buf));
	memset(se_buf, '\0', sizeof(se_buf));
	bgm_buf[0].data_vaddr = &bgm_audio_buffer[0];
	bgm_buf[0].nsamples = samples_per_buf;
	bgm_buf[0].status = NDSP_WBUF_DONE;
	bgm_buf[1].data_vaddr = &bgm_audio_buffer[samples_per_buf];
	bgm_buf[1].nsamples = samples_per_buf;
	bgm_buf[1].status = NDSP_WBUF_DONE;

	LightEvent_Init(&audio_event, RESET_ONESHOT);

        APT_CheckNew3DS(&is_new_3ds);

	ndspSetCallback(n3ds_dsp_callback, this);
	threadCreate(n3ds_audio_thread, this, 32768, 0x18, 1, true);
}

CtrAudio::~CtrAudio() {
	term_stream = true;

	LightEvent_Signal(&audio_event);

	while (term_stream) {}

	LightEvent_Clear(&audio_event);

	ndspExit();

	linearFree(bgm_audio_buffer);

	for (int i = 0; i < se_channel_end; ++i) {
		if (se_buf[i].data_pcm16 != nullptr) {
			linearFree(se_buf[i].data_pcm16);
		}
	}
}

void CtrAudio::LockMutex() const {
	LightLock_Lock(&audio_mutex);
}

void CtrAudio::UnlockMutex() const {
	LightLock_Unlock(&audio_mutex);
}

#endif
