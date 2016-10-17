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

#include <3ds.h>
#include <cstring>

#include "audio_3ds.h"
#include "output.h"

namespace {
	const int n3ds_samples_per_buf = 2048 / 2;
	const int n3ds_bytes_per_sample = 4;
	int fill_block = 0;
}

void n3ds_audio_thread(void* userdata) {
	CtrAudio* audio = static_cast<CtrAudio*>(userdata);

	if (audio->waveBuf[fill_block].status == NDSP_WBUF_DONE) {
		memset(audio->waveBuf[fill_block].data_pcm16, '\0', n3ds_samples_per_buf * n3ds_bytes_per_sample);

		audio->LockMutex();
		audio->Decode(reinterpret_cast<uint8_t*>(
			audio->waveBuf[fill_block].data_pcm16),
			n3ds_samples_per_buf * n3ds_bytes_per_sample
		);
		DSP_FlushDataCache(audio->waveBuf[fill_block].data_pcm16,n3ds_samples_per_buf);
		ndspChnWaveBufAdd(0, &audio->waveBuf[fill_block]);
		audio->UnlockMutex();

		++fill_block;
		fill_block %= 2;
	}
}

CtrAudio::CtrAudio() :
	GenericAudio()
{
	Result res = ndspInit();

	if (res != 0){
		Output::Error("Couldn't initialize audio.\nError code: 0x%X\n", res);
	}

	LightLock_Init(&BGM_Mutex);

	audio_buffer = reinterpret_cast<uint32_t*>(linearAlloc(n3ds_samples_per_buf * n3ds_bytes_per_sample * 2));

	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	SetFormat(22050, AudioDecoder::Format::S16, 2);

	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, 22050);
	ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

	memset(waveBuf, '\0', sizeof(waveBuf));
	waveBuf[0].data_vaddr = &audio_buffer[0];
	waveBuf[0].nsamples = n3ds_samples_per_buf;
	waveBuf[0].status = NDSP_WBUF_DONE;
	waveBuf[1].data_vaddr = &audio_buffer[n3ds_samples_per_buf];
	waveBuf[1].nsamples = n3ds_samples_per_buf;
	waveBuf[1].status = NDSP_WBUF_DONE;

	ndspSetCallback(n3ds_audio_thread, this);
}

CtrAudio::~CtrAudio() {
	ndspExit();

	linearFree(audio_buffer);
}

void CtrAudio::LockMutex() const {
	LightLock_Lock(&BGM_Mutex);
}

void CtrAudio::UnlockMutex() const {
	LightLock_Unlock(&BGM_Mutex);
}

#endif
