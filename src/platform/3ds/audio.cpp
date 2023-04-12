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

#include <cstdint>
#include <cstring>

#include "audio.h"
#include "filefinder.h"
#include "game_clock.h"
#include "output.h"
#include "audio_secache.h"

//#define EP_DEBUG_CTRAUDIO
#ifdef EP_DEBUG_CTRAUDIO
using namespace std::chrono_literals;
Game_Clock::time_point dsp_tick;
#define DebugLog Output::Debug
#else
#define DebugLog(...)
#endif

namespace {
	int fill_block = 0;
	bool is_new_3ds = false;
	bool dsp_inited = false;

	struct _se {
		constexpr static int channels = 23; // 24 channels available
		ndspWaveBuf buf[channels] = {0};
	} se;
	struct _bgm {
		constexpr static int buf_size = 8192;
		constexpr static int channel = se.channels; // last, after SE channels
		constexpr static int num_bufs = 3;
		ndspWaveBuf buf[num_bufs] = {0};
		Game_Clock::time_point starttick;
		uint32_t* audio_buffer;
		std::unique_ptr<AudioDecoderBase> decoder;
	} bgm;

	LightLock audio_mutex;
	LightEvent audio_event;
	Thread audio_thread;
	bool term_stream = false;
}

bool set_channel_format(int dsp_chn, AudioDecoder::Format format, int channels, AudioDecoder::Format &out_format) {
	// false case tries to set to a close format
	out_format = format;
	bool res = true;

	if (channels < 1 || channels > 2) {
		Output::Warning("Unsupported number of channels!");
		return false;
	}

	switch (format) {
		case AudioDecoder::Format::U8:
			res = false;
			out_format = AudioDecoder::Format::S8;
		case AudioDecoder::Format::S8:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM8 : NDSP_FORMAT_STEREO_PCM8);
			break;

		case AudioDecoder::Format::U16:
		case AudioDecoder::Format::U32:
		case AudioDecoder::Format::S32:
		case AudioDecoder::Format::F32:
			res = false;
			out_format = AudioDecoder::Format::S16;
		case AudioDecoder::Format::S16:
			ndspChnSetFormat(dsp_chn, channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);
			break;

		default:
			Output::Warning("Unhandled audio channel format!");
			res = false;
	}

	return res;
}

void CtrAudio::BGM_Play(Filesystem_Stream::InputStream filestream, int volume, int pitch, int fadein) {
	if (!dsp_inited)
		return;

	StringView name = filestream.GetName();
	if (!filestream) {
		Output::Warning("Couldn't play BGM {}: File not readable", name);
		return;
	}

	LockMutex();
	bgm.decoder = AudioDecoder::Create(filestream);
	if (bgm.decoder && bgm.decoder->Open(std::move(filestream))) {
		// Fixme: music volume setting unsupported
		int frequency;
		AudioDecoder::Format format, out_format;
		int channels;

		bgm.decoder->SetPitch(is_new_3ds ? pitch : 100);
		bgm.decoder->GetFormat(frequency, format, channels);
		bgm.decoder->SetVolume(0);
		bgm.decoder->SetFade(volume, std::chrono::milliseconds(fadein));
		bgm.decoder->SetLooping(true);

		if (!set_channel_format(bgm.channel, format, channels, out_format)) {
			DebugLog("{} has unsupported format, using close format.", name);
			bgm.decoder->SetFormat(frequency, out_format, channels);
		}
		ndspChnSetRate(bgm.channel, frequency);

		const int samplesize = AudioDecoder::GetSamplesizeForFormat(out_format);
		const int nsamples = bgm.buf_size / (samplesize * channels);
		for (int i = 0; i < bgm.num_bufs; ++i) {
			bgm.buf[i].nsamples = nsamples;
		}
	} else {
		Output::Warning("Couldn't play BGM {}: Format not supported", name);
	}

	UnlockMutex();
	return;
}

void CtrAudio::BGM_Pause()  {
	if (!dsp_inited)
		return;

	ndspChnSetPaused(bgm.channel, true);
}

void CtrAudio::BGM_Resume()  {
	if (!dsp_inited)
		return;

	bgm.starttick = Game_Clock::now();
	ndspChnSetPaused(bgm.channel, false);
}

void CtrAudio::BGM_Stop() {
	if (!dsp_inited)
		return;

	LockMutex();
	bgm.decoder.reset();
	UnlockMutex();
}

bool CtrAudio::BGM_PlayedOnce() const {
	if (!bgm.decoder)
		return false;

	return bgm.decoder->GetLoopCount() > 0;
}

bool CtrAudio::BGM_IsPlaying() const {
	if (!bgm.decoder)
		return false;

	return !bgm.decoder->IsFinished();
}

int CtrAudio::BGM_GetTicks() const {
	if (!bgm.decoder)
		return 0;

	return bgm.decoder->GetTicks();
}

void CtrAudio::BGM_Fade(int fade) {
	if (!bgm.decoder)
		return;

	bgm.starttick = Game_Clock::now();
	bgm.decoder->SetFade(0, std::chrono::milliseconds(fade));
}

void CtrAudio::BGM_Volume(int volume) {
	if (!bgm.decoder)
		return;

	bgm.decoder->SetVolume(volume);
}

void CtrAudio::BGM_Pitch(int pitch) {
	if (is_new_3ds && bgm.decoder) {
		bgm.decoder->SetPitch(pitch);
	}
}

std::string CtrAudio::BGM_GetType() const {
	if (bgm.decoder) {
		return bgm.decoder->GetType();
	}
	return "";
}

void CtrAudio::SE_Play(std::unique_ptr<AudioSeCache> se_cache, int volume, int pitch) {
	if (!dsp_inited)
		return;

	if (!se_cache) {
		Output::Warning("SE_Play: cache data is invalid");
		return;
	}

	int chan = -1;
	for (int i = 0; i <= se.channels; ++i) {
		if (!ndspChnIsPlaying(i)) {
			chan = i;
			//DebugLog("Using channel {} for SE {}", chan, se_cache->GetName());
			break;
		}
	}
	if (chan == -1) {
		Output::Debug("Couldn't play SE {}: No free channel available", se_cache->GetName());
		return;
	}

	auto dec = se_cache->CreateSeDecoder();
	dec->SetPitch(pitch);

	int frequency;
	AudioDecoder::Format format, out_format;
	int channels;

	std::vector<uint8_t> dec_buf;
	std::vector<uint8_t>* out_buf = nullptr;
	AudioSeRef se_ref;
	bool use_raw_buf = false;

	dec->GetFormat(frequency, format, channels);
	if (set_channel_format(chan, format, channels, out_format)) {
		// When the DSP supports the format and the audio is not pitched the raw
		// buffer can be used directly
		use_raw_buf = pitch == 100;
	} else {
		DebugLog("{} has unsupported format, using close format.", se_cache->GetName());
		dec->SetFormat(frequency, out_format, channels);
	}
	if (use_raw_buf) {
		se_ref = se_cache->GetSeData();
		out_buf = &se_ref->buffer;
	} else {
		dec_buf = dec->DecodeAll();
		out_buf = &dec_buf;
	}

	ndspChnSetRate(chan, frequency);

	if (se.buf[chan].data_pcm16 != nullptr) {
		linearFree(se.buf[chan].data_pcm16);
	}

	size_t bsize = out_buf->size();
	size_t aligned_bsize = 8192;
	// Buffer must be correctly aligned to prevent audio glitches
	for (; ; aligned_bsize *= 2) {
		if (aligned_bsize > bsize) {
			se.buf[chan].data_pcm16 = reinterpret_cast<s16*>(linearAlloc(aligned_bsize));
			memset(se.buf[chan].data_pcm16 + bsize, '\0', aligned_bsize - bsize);
			break;
		}
	}

	const int samplesize = AudioDecoder::GetSamplesizeForFormat(out_format);
	se.buf[chan].nsamples = bsize / (samplesize * channels);

	memcpy(se.buf[chan].data_pcm16, out_buf->data(), out_buf->size());
	DSP_FlushDataCache(se.buf[chan].data_pcm16, aligned_bsize);

	float mix[12] = {0};
	mix[0] = mix[1] = volume / 100.0f * cfg.sound_volume.Get() / 100.0f;
	ndspChnSetMix(chan, mix);
	ndspChnWaveBufAdd(chan, &se.buf[chan]);
}

void CtrAudio::SE_Stop() {
	if (!dsp_inited)
		return;

	for (int i = 0; i <= se.channels; ++i) {
		ndspChnWaveBufClear(i);
	}
}

void CtrAudio::Update() {
	if (!bgm.decoder)
		return;

	auto t = Game_Clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(t - bgm.starttick);
	bgm.decoder->Update(us);
	bgm.starttick = t;
}

void n3ds_dsp_callback(void* userdata) {
	CtrAudio* audio = static_cast<CtrAudio*>(userdata);

	if (bgm.buf[fill_block].status == NDSP_WBUF_DONE) {
		bgm.buf[fill_block].status = NDSP_WBUF_FREE;
		LightEvent_Signal(&audio_event);
	}

#ifdef EP_DEBUG_CTRAUDIO
	auto t = Game_Clock::now();
	auto s = std::chrono::duration_cast<std::chrono::seconds>(t - dsp_tick);
	if (s > 5s) {
		dsp_tick = t;
		u32 all = ndspGetFrameCount();
		u32 dropped = ndspGetDroppedFrames();
		float percent = (float)dropped/all*100;
		if (percent > 0.0f)
			Output::Debug("DSP frames dropped: {}% ({} of {})", percent, dropped, all);
	}
#endif
}

void n3ds_audio_thread(void* userdata) {
	CtrAudio* audio = static_cast<CtrAudio*>(userdata);
	float mix[12] = {0};

	while (!term_stream) {
		LightEvent_Wait(&audio_event);

		int target_block = fill_block;
		++fill_block;
		fill_block %= bgm.num_bufs;

		audio->LockMutex();
		if (bgm.decoder) {
			bgm.decoder->Decode(
				reinterpret_cast<uint8_t*>(bgm.buf[target_block].data_pcm16),
				bgm.buf_size);
			DSP_FlushDataCache(bgm.buf[target_block].data_pcm16, bgm.buf_size);
			mix[0] = mix[1] = bgm.decoder->GetVolume() / 100.0f * audio->GetConfig().music_volume.Get() / 100.0f;
			ndspChnSetMix(bgm.channel, mix);
			ndspChnWaveBufAdd(bgm.channel, &bgm.buf[target_block]);
		} else {
			bgm.buf[target_block].status = NDSP_WBUF_DONE;
		}
		audio->UnlockMutex();
	}
	threadExit(0);
}

CtrAudio::CtrAudio(const Game_ConfigAudio& cfg) : AudioInterface(cfg) {
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
	ndspSetOutputCount(1);
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspSetClippingMode(NDSP_CLIP_NORMAL);
	for (int i = 0; i <= bgm.channel; ++i) {
		ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
	}
	dsp_inited = true;

	LightLock_Init(&audio_mutex);
	LightEvent_Init(&audio_event, RESET_ONESHOT);
	APT_CheckNew3DS(&is_new_3ds);

	bgm.audio_buffer = reinterpret_cast<uint32_t*>(linearAlloc(bgm.buf_size * bgm.num_bufs));
	for (int i = 0; i < bgm.num_bufs; ++i) {
		bgm.buf[i].data_vaddr = &bgm.audio_buffer[i * bgm.buf_size];
		bgm.buf[i].nsamples = bgm.buf_size / 4;
		bgm.buf[i].status = NDSP_WBUF_DONE;
	}

	ndspSetCallback(n3ds_dsp_callback, this);
	audio_thread = threadCreate(n3ds_audio_thread, this, 32768, 0x18, 1, true);
	if(!audio_thread)
		Output::Warning("Couldn't create bgm thread!");

#ifdef EP_DEBUG_CTRAUDIO
	dsp_tick = Game_Clock::now();
#endif
}

CtrAudio::~CtrAudio() {
	if (!dsp_inited)
		return;

	term_stream = true;
	LightEvent_Signal(&audio_event);
	threadJoin(audio_thread, U64_MAX);
	LightEvent_Clear(&audio_event);
	ndspExit();

	linearFree(bgm.audio_buffer);
	for (int i = 0; i < se.channels; ++i) {
		if (se.buf[i].data_pcm16 != nullptr) {
			linearFree(se.buf[i].data_pcm16);
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
