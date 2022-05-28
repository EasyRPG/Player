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

#ifdef SUPPORT_AUDIO

#include <cstring>

#include "audio.h"
#include "filefinder.h"
#include "game_clock.h"
#include "output.h"
#include "audio_secache.h"

namespace {
	const int bgm_buf_size = 8192;
	const int bgm_channel = 0;
	const int se_channel_begin = 1;
	const int se_channel_end = 23;
	const int samplerate = 22050;
	int fill_block = 0;
	bool is_new_3ds;
	bool dsp_inited = false;
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

void CtrAudio::BGM_Play(Filesystem_Stream::InputStream filestream, int volume, int pitch, int fadein) {
	if (!dsp_inited)
		return;

	if (!filestream) {
		Output::Warning("Couldn't play BGM {}: File not readable", filestream.GetName());
		return;
	}

	LockMutex();
	bgm_decoder = AudioDecoder::Create(filestream);
	if (bgm_decoder && bgm_decoder->Open(std::move(filestream))) {
		int frequency;
		AudioDecoder::Format format, out_format;
		int channels;

		bgm_decoder->SetPitch(is_new_3ds ? pitch : 100);
		bgm_decoder->GetFormat(frequency, format, channels);
		bgm_decoder->SetVolume(0);
		bgm_decoder->SetFade(volume, std::chrono::milliseconds(fadein));
		bgm_decoder->SetLooping(true);

		if (!set_channel_format(bgm_channel, format, channels, out_format)) {
			bgm_decoder->SetFormat(frequency, out_format, channels);
		}
		ndspChnSetRate(bgm_channel, frequency);

		const int samplesize = AudioDecoder::GetSamplesizeForFormat(out_format);
		const int nsamples = bgm_buf_size / (samplesize * channels);
		bgm_buf[0].nsamples = nsamples;
		bgm_buf[1].nsamples = nsamples;
	} else {
		Output::Warning("Couldn't play BGM {}: Format not supported", filestream.GetName());
	}

	UnlockMutex();
	return;
}

void CtrAudio::BGM_Pause()  {
	if (!dsp_inited)
		return;

	ndspChnSetPaused(bgm_channel, true);
}

void CtrAudio::BGM_Resume()  {
	if (!dsp_inited)
		return;

	bgm_starttick = Game_Clock::now();
	ndspChnSetPaused(bgm_channel, false);
}

void CtrAudio::BGM_Stop() {
	if (!dsp_inited)
		return;

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

int CtrAudio::BGM_GetTicks() const {
	if (bgm_decoder) {
		return bgm_decoder->GetTicks();
	}

	return 0;
}

void CtrAudio::BGM_Fade(int fade) {
	if (bgm_decoder) {
		bgm_starttick = Game_Clock::now();
		bgm_decoder->SetFade(0, std::chrono::milliseconds(fade));
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

void CtrAudio::SE_Play(std::unique_ptr<AudioSeCache> se, int volume, int pitch) {
	if (!dsp_inited)
		return;

	if (!se) {
		Output::Warning("SE_Play: AudioSeCache data is NULL");
		return;
	}

	// Important!
	// When indexing se_buf use se_channel
	// In ndsp-Api functions use ndsp_channel
	int se_channel = -1;
	int ndsp_channel = -1;

	for (int i = se_channel_begin; i <= se_channel_end; ++i) {
		if (!ndspChnIsPlaying(i)) {
			ndsp_channel = i;
			se_channel = i - 1;
			break;
		}
	}

	if (se_channel == -1) {
		Output::Warning("Couldn't play SE {}: No free channel available", se->GetName());
		return;
	}

	auto dec = se->CreateSeDecoder();
	dec->SetPitch(pitch);

	int frequency;
	AudioDecoder::Format format, out_format;
	int channels;

	std::vector<uint8_t> dec_buf;
	std::vector<uint8_t>* out_buf = nullptr;
	AudioSeRef se_ref;
	bool use_raw_buf = false;

	dec->GetFormat(frequency, format, channels);
	if (set_channel_format(ndsp_channel, format, channels, out_format)) {
		// When the DSP supports the format and the audio is not pitched the raw
		// buffer can be used directly
		use_raw_buf = pitch == 100;
	}

	if (use_raw_buf) {
		se_ref = se->GetSeData();
		out_buf = &se_ref->buffer;
	} else {
		dec->SetFormat(frequency, out_format, channels);
		dec_buf = dec->DecodeAll();
		out_buf = &dec_buf;
	}

	ndspChnSetRate(ndsp_channel, frequency);

	if (se_buf[se_channel].data_pcm16 != nullptr) {
		linearFree(se_buf[se_channel].data_pcm16);
	}

	size_t bsize = out_buf->size();
	size_t aligned_bsize = 8192;
	// Buffer must be correctly aligned to prevent audio glitches
	for (; ; aligned_bsize *= 2) {
		if (aligned_bsize > bsize) {
			se_buf[se_channel].data_pcm16 = reinterpret_cast<s16*>(linearAlloc(aligned_bsize));
			memset(se_buf[se_channel].data_pcm16 + bsize, '\0', aligned_bsize - bsize);
			break;
		}
	}

	const int samplesize = AudioDecoder::GetSamplesizeForFormat(out_format);
	se_buf[se_channel].nsamples = bsize / (samplesize * channels);

	memcpy(se_buf[se_channel].data_pcm16, out_buf->data(), out_buf->size());

	DSP_FlushDataCache(se_buf[se_channel].data_pcm16, aligned_bsize);

	float mix[12] = {0};
	mix[0] = volume / 100.0f;
	mix[1] = mix[0];
	ndspChnSetMix(0, mix);

	ndspChnWaveBufAdd(ndsp_channel, &se_buf[se_channel]);
}

void CtrAudio::SE_Stop() {
	if (!dsp_inited)
		return;

	for (int i = se_channel_begin; i <= se_channel_end; ++i) {
		ndspChnWaveBufClear(i);
	}
}

void CtrAudio::Update() {
	if (bgm_decoder) {
		auto t = Game_Clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(t - bgm_starttick);
		bgm_decoder->Update(us);
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
			audio->bgm_buf[target_block].data_pcm16), bgm_buf_size
		);
		DSP_FlushDataCache(audio->bgm_buf[target_block].data_pcm16, bgm_buf_size);

		mix[0] = audio->bgm_decoder->GetVolume() / 100.0f;
		mix[1] = mix[0];
		ndspChnSetMix(bgm_channel, mix);

		ndspChnWaveBufAdd(bgm_channel, &audio->bgm_buf[target_block]);
		audio->UnlockMutex();
	}
}

CtrAudio::CtrAudio() {
	LightLock_Init(&audio_mutex);

	Result res = ndspInit();
	if (R_FAILED(res)) {
		Output::Warning("Couldn't initialize audio");
		if ((R_SUMMARY(res) == RS_NOTFOUND) && (R_MODULE(res) == RM_DSP))
			Output::Warning("This needs a dumped DSP firmware to work!");
		else
			Output::Warning("Error code: {:#X}", res);

		return;
	}

	dsp_inited = true;
	bgm_audio_buffer = reinterpret_cast<uint32_t*>(linearAlloc(bgm_buf_size * 2));
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	for (int i = 0; i <= se_channel_end; ++i) {
		ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
		ndspChnSetRate(i, samplerate);
		ndspChnSetFormat(i, NDSP_FORMAT_STEREO_PCM16);
	}

	memset(bgm_buf, '\0', sizeof(bgm_buf));
	memset(se_buf, '\0', sizeof(se_buf));
	bgm_buf[0].data_vaddr = &bgm_audio_buffer[0];
	bgm_buf[0].nsamples = bgm_buf_size / 4;
	bgm_buf[0].status = NDSP_WBUF_DONE;
	bgm_buf[1].data_vaddr = &bgm_audio_buffer[bgm_buf_size];
	bgm_buf[1].nsamples = bgm_buf_size / 4;
	bgm_buf[1].status = NDSP_WBUF_DONE;

	LightEvent_Init(&audio_event, RESET_ONESHOT);

	APT_CheckNew3DS(&is_new_3ds);

	ndspSetCallback(n3ds_dsp_callback, this);
	threadCreate(n3ds_audio_thread, this, 32768, 0x18, 1, true);
}

CtrAudio::~CtrAudio() {
	if (!dsp_inited)
		return;

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
