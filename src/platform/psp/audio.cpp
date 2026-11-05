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

#include <pspaudio.h>
#include <pspkernel.h>
#include <cstdint>
#include <cstring>

#ifdef PLAYER_PSP_AUDIO_STATS
#include <cstdio>
#endif

namespace {
	constexpr int samplerate = 44100;
	constexpr int channels = 2;
	// Must be a multiple of PSP_AUDIO_SAMPLE_ALIGN (64)
	constexpr int samples_per_buf = 1024;
	constexpr int bytes_per_frame = channels * 2; // 16 bit samples
	constexpr int buf_size = samples_per_buf * bytes_per_frame;
	// How long one block lasts
	constexpr uint32_t block_us = static_cast<uint32_t>(
		static_cast<uint64_t>(samples_per_buf) * 1000000 / samplerate);

	// Mixed blocks waiting for output. The deeper the ring, the longer a mixing hiccup can be, but
	// sound effects also start later: a sound started now is heard after the ring has played.
	constexpr int ring_blocks = 4;
	// The hardware reads a buffer after sceAudioSRCOutputBlocking returned, so a buffer must not be
	// reused right away. Each submitted block gets its own buffer for a while.
	constexpr int dma_buffers = 4;

	struct alignas(64) Block {
		uint8_t data[buf_size];
	};

	Block ring[ring_blocks];
	Block dma[dma_buffers];
	SceUID ring_free = -1; // counts empty ring blocks, the mixer waits on it
	SceUID ring_full = -1; // counts mixed ring blocks, the output waits on it

	PspAudio* instance = nullptr;

#ifdef PLAYER_PSP_AUDIO_STATS
	// Diagnostic only: how long mixing one block takes compared to the time the block lasts and
	// how often the output ran out of mixed blocks. Written to a file every 64 blocks.
	struct AudioStats {
		FILE* file = nullptr;
		uint32_t blocks = 0, over_budget = 0, max_us = 0;
		uint64_t sum_us = 0;
		volatile uint32_t underruns = 0;

		void Record(uint32_t us) {
			if (!file) file = std::fopen("ms0:/PSP/GAME/EasyRPG/audio-stats.txt", "w");
			++blocks;
			sum_us += us;
			if (us > max_us) max_us = us;
			if (us > block_us) ++over_budget;
			if (blocks % 64 == 0 && file) {
				std::fprintf(file, "blocks=%u avg_us=%u max_us=%u over_budget=%u underruns=%u budget_us=%u\n",
					blocks, static_cast<unsigned>(sum_us / blocks), max_us, over_budget,
					static_cast<unsigned>(underruns), block_us);
				std::fflush(file);
			}
		}
	};
	AudioStats stats;
#endif
}

static int psp_mixer_thread(SceSize /* args */, void* /* argp */) {
	int write = 0;

	while (!instance->termStream) {
		if (sceKernelWaitSema(ring_free, 1, nullptr) < 0 || instance->termStream) {
			break;
		}

#ifdef PLAYER_PSP_AUDIO_STATS
		const uint64_t start = sceKernelGetSystemTimeWide();
#endif
		instance->LockMutex();
		instance->Decode(ring[write].data, buf_size);
		instance->UnlockMutex();
#ifdef PLAYER_PSP_AUDIO_STATS
		stats.Record(static_cast<uint32_t>(sceKernelGetSystemTimeWide() - start));
#endif

		sceKernelSignalSema(ring_full, 1);
		write = (write + 1) % ring_blocks;
	}

	sceKernelExitDeleteThread(0);
	return 0;
}

static int psp_output_thread(SceSize /* args */, void* /* argp */) {
	int res = sceAudioSRCChReserve(samples_per_buf, samplerate, channels);
	if (res < 0) {
		Output::Error("Failed to reserve PSP audio channel ({:#x})", res);
		sceKernelExitDeleteThread(0);
	}

	int read = 0;
	int dma_index = 0;
	bool started = false;

	while (!instance->termStream) {
		Block& out = dma[dma_index];

		// Wait a moment for a mixed block, if there is none play silence
		// instead so the hardware is not starved.
		SceUInt timeout = block_us / 2;
		if (sceKernelWaitSema(ring_full, 1, &timeout) == 0) {
			if (instance->termStream) {
				break;
			}
			std::memcpy(out.data, ring[read].data, buf_size);
			sceKernelSignalSema(ring_free, 1);
			read = (read + 1) % ring_blocks;
			started = true;
		} else {
			std::memset(out.data, 0, buf_size);
#ifdef PLAYER_PSP_AUDIO_STATS
			// The very first blocks are expected to be silent
			if (started) ++stats.underruns;
#endif
		}
		(void)started;

		// Make sure the hardware sees what the CPU wrote
		sceKernelDcacheWritebackRange(out.data, buf_size);
		sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, out.data);

		dma_index = (dma_index + 1) % dma_buffers;
	}

	sceAudioSRCChRelease();
	sceKernelExitDeleteThread(0);
	return 0;
}

PspAudio::PspAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	instance = this;

	// Binary semaphore used as mutex
	audio_mutex = sceKernelCreateSema("EasyRPG Audio Mutex", 0, 1, 1, nullptr);
	ring_free = sceKernelCreateSema("EasyRPG Audio Free", 0, ring_blocks, ring_blocks, nullptr);
	ring_full = sceKernelCreateSema("EasyRPG Audio Full", 0, 0, ring_blocks, nullptr);

	SetFormat(samplerate, AudioDecoder::Format::S16, channels);

	// The output thread must not be delayed by the game (main thread priority is 0x20)
	output_thread = sceKernelCreateThread("EasyRPG Audio Output", &psp_output_thread,
		0x12, 0x4000, PSP_THREAD_ATTR_USER, nullptr);
	mixer_thread = sceKernelCreateThread("EasyRPG Audio Mixer", &psp_mixer_thread,
		0x14, 0x10000, PSP_THREAD_ATTR_USER, nullptr);

	int res = sceKernelStartThread(output_thread, 0, nullptr);
	if (res == 0) {
		res = sceKernelStartThread(mixer_thread, 0, nullptr);
	}
	if (res != 0) {
		Output::Error("Failed to init audio thread ({:#x})", res);
		return;
	}
}

PspAudio::~PspAudio() {
	// Closing streaming threads: wake them up in case they wait for the other one
	termStream = true;
	sceKernelSignalSema(ring_free, 1);
	sceKernelSignalSema(ring_full, 1);
	sceKernelWaitThreadEnd(mixer_thread, nullptr);
	sceKernelWaitThreadEnd(output_thread, nullptr);

	// Deleting semaphores
	sceKernelDeleteSema(ring_free);
	sceKernelDeleteSema(ring_full);
	sceKernelDeleteSema(audio_mutex);

	instance = nullptr;
}

void PspAudio::LockMutex() const {
	sceKernelWaitSema(audio_mutex, 1, nullptr);
}

void PspAudio::UnlockMutex() const {
	sceKernelSignalSema(audio_mutex, 1);
}

#endif
