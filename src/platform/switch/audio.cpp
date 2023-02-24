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

#include "audio.h"
#include "output.h"

#include <switch.h>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <malloc.h>

#define ALIGN_TO(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

namespace {
	const int samplerate = 48000;
	const int bytes_per_sample = 4;
	const int samples_per_buf = 4096;
	const int buf_size = samples_per_buf * bytes_per_sample;
	NxAudio* instance = nullptr;
}

void switch_audio_thread(void*) {
	uint8_t *buffer1 = (uint8_t*)memalign(0x1000, ALIGN_TO(buf_size, 0x1000));
	uint8_t *buffer2 = (uint8_t*)memalign(0x1000, ALIGN_TO(buf_size, 0x1000));
	uint32_t released_count;
	
	AudioOutBuffer source_buffers[2], *released_buffer;
	
	// Init audio buffers
	source_buffers[0].buffer = buffer1;
	source_buffers[1].buffer = buffer2;
	
	for (int i = 0; i < 2; i++){
		source_buffers[i].next = NULL;
		source_buffers[i].buffer_size = buf_size;
		source_buffers[i].data_size = buf_size;
		source_buffers[i].data_offset = 0;
		instance->LockMutex();
		instance->Decode((uint8_t*)source_buffers[i].buffer, buf_size);
		instance->UnlockMutex();
		audoutAppendAudioOutBuffer(&source_buffers[i]);
	}
	
	for(;;) {
		// A pretty bad way to close thread
		if (instance->termStream) {
			instance->termStream = false;
			free(buffer1);
			free(buffer2);
			return;
		}

		audoutWaitPlayFinish(&released_buffer, &released_count, UINT64_MAX);
		instance->LockMutex();
		instance->Decode((uint8_t*)released_buffer->buffer, buf_size);
		instance->UnlockMutex();
		audoutAppendAudioOutBuffer(released_buffer);

	}
}

NxAudio::NxAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	instance = this;

	audoutInitialize();
	audoutStartAudioOut();
	
	mutexInit(&audio_mutex);

	threadCreate(&audio_thread, switch_audio_thread, NULL, NULL, 0x10000, 0x2B, -2);
	
	SetFormat(samplerate, AudioDecoder::Format::S16, 2);

	if (R_FAILED(threadStart(&audio_thread))) {
		Output::Error("Failed to init audio thread.");
		return;
	}
}

NxAudio::~NxAudio() {
	// Closing streaming thread
	termStream = true;
	threadWaitForExit(&audio_thread);
	
	// Deleting thread
	threadClose(&audio_thread);
	
	// Terminating audio API
	audoutStopAudioOut();
	audoutExit();
}

void NxAudio::LockMutex() const {
	mutexLock((Mutex*)&audio_mutex);
}

void NxAudio::UnlockMutex() const {
	mutexUnlock((Mutex*)&audio_mutex);
}

#endif
