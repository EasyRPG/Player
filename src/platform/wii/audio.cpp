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

#ifdef SUPPORT_AUDIO

#include <ogcsys.h>
#include <aesndlib.h>
#include "audio.h"
#include "output.h"

namespace {
	constexpr int NUMBUFS = 2;
	constexpr int SNDBUFFERSIZE = 5760;
	constexpr int THREADPRIORITY = 80;
	AESNDPB *voice = nullptr;
	uint8_t buffer[NUMBUFS][SNDBUFFERSIZE];
	int cur_buf = 0;
	constexpr int AUDIOSTACK = 16384*2;
	lwpq_t audioqueue;
	lwp_t athread = LWP_THREAD_NULL;
	uint8_t astack[AUDIOSTACK];
	bool stopaudio = false;
	mutex_t audio_mutex;
	WiiAudio* instance = nullptr;
}

static void VoiceStreamCallback(AESNDPB *pb, u32 state) {
	if (stopaudio || state != VOICE_STATE_STREAM) return;

	// play current buffer, switch buffers, then let thread decode more
	instance->LockMutex();
	AESND_SetVoiceBuffer(pb, buffer[cur_buf], SNDBUFFERSIZE);
	cur_buf = (cur_buf + 1) % NUMBUFS;
	LWP_ThreadSignal(audioqueue);
	instance->UnlockMutex();
}

static void *AudioThread (void *) {
	while (!stopaudio) {
		// clear old data
		memset(buffer[cur_buf], 0, SNDBUFFERSIZE);

		instance->LockMutex();
		instance->Decode(buffer[cur_buf], SNDBUFFERSIZE);
		instance->UnlockMutex();

		// make sure data is in main memory
		DCFlushRange(buffer[cur_buf], SNDBUFFERSIZE);

		LWP_ThreadSleep(audioqueue);
	}
	return nullptr;
}

WiiAudio::WiiAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	instance = this;

	// setup DSP
	AESND_Init();
	voice = AESND_AllocateVoice(VoiceStreamCallback);
	if (voice == nullptr) {
		Output::Warning("Couldn't open audio! Failed to allocate voice!");
		return;
	}

	int freq = 44100;
	SetFormat(freq, AudioDecoder::Format::S16, 2);
	AESND_SetVoiceFormat(voice, VOICE_STEREO16);
	AESND_SetVoiceFrequency(voice, freq);
	AESND_SetVoiceStream(voice, true);
	AESND_SetVoiceStop(voice, false);

	// setup thread and queue
	LWP_MutexInit(&audio_mutex, 0);
	LWP_InitQueue(&audioqueue);
	LWP_CreateThread(&athread, AudioThread, nullptr, astack, AUDIOSTACK, THREADPRIORITY);
	stopaudio = false;
}

WiiAudio::~WiiAudio() {
	// stop DSP
	AESND_SetVoiceStop(voice, true);
	AESND_FreeVoice(voice);
	voice = nullptr;
	AESND_Reset();

	// stop and clear queue/thread
	stopaudio = true;
	LWP_ThreadSignal(audioqueue);
	LWP_JoinThread(athread, nullptr);
	LWP_CloseQueue(audioqueue);
	athread = LWP_THREAD_NULL;
	LWP_MutexDestroy(audio_mutex);
}

void WiiAudio::LockMutex() const {
	LWP_MutexLock(audio_mutex);
}

void WiiAudio::UnlockMutex() const {
	LWP_MutexUnlock(audio_mutex);
}

#endif
