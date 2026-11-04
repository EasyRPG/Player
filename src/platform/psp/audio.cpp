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
#include <vector>

namespace {
	constexpr int samplerate = 44100;
	constexpr int channels = 2;
	// Must be a multiple of PSP_AUDIO_SAMPLE_ALIGN (64)
	constexpr int samples_per_buf = 1024;
	constexpr int bytes_per_frame = channels * 2; // 16 bit samples
	constexpr int buf_size = samples_per_buf * bytes_per_frame;

	PspAudio* instance = nullptr;
}

static int psp_audio_thread(SceSize /* args */, void* /* argp */) {
	std::vector<uint8_t> buffer(buf_size);

	int res = sceAudioSRCChReserve(samples_per_buf, samplerate, channels);
	if (res < 0) {
		Output::Error("Failed to reserve PSP audio channel ({:#x})", res);
		sceKernelExitDeleteThread(0);
	}

	for (;;) {
		// A pretty bad way to close thread
		if (instance->termStream) {
			instance->termStream = false;
			sceAudioSRCChRelease();
			sceKernelExitDeleteThread(0);
		}

		instance->LockMutex();
		instance->Decode(buffer.data(), buf_size);
		instance->UnlockMutex();

		res = sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, buffer.data());
		if (res < 0) Output::Error("An error occurred in audio thread ({:#x})", res);
	}
}

PspAudio::PspAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	instance = this;

	// Binary semaphore used as mutex
	audio_mutex = sceKernelCreateSema("EasyRPG Audio Mutex", 0, 1, 1, nullptr);

	SetFormat(samplerate, AudioDecoder::Format::S16, channels);

	audio_thread = sceKernelCreateThread("EasyRPG Audio Thread", &psp_audio_thread,
		0x12, 0x10000, PSP_THREAD_ATTR_USER, nullptr);

	int res = sceKernelStartThread(audio_thread, 0, nullptr);
	if (res != 0) {
		Output::Error("Failed to init audio thread ({:#x})", res);
		return;
	}
}

PspAudio::~PspAudio() {
	// Closing streaming thread
	termStream = true;
	sceKernelWaitThreadEnd(audio_thread, nullptr);

	// Deleting mutex
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
