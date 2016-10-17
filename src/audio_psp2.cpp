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

#if defined(PSP2) && defined(SUPPORT_AUDIO)
#include "audio_psp2.h"
#include "output.h"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <vector>
#include <cstdlib>

namespace {
	const int buf_size = 8192;
	const int samplerate = 44100;
	const int bytes_per_sample = 4;
	const int samples_per_buf = buf_size / bytes_per_sample;
	Psp2Audio* instance = nullptr;
}

static int psp2_audio_thread(unsigned int, void*){
	std::vector<uint8_t> buffer;
	buffer.resize(buf_size);

	// Init audio channel
	// Seems like audio ports are thread dependant on PSVITA :/
	int audio_chn = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, samples_per_buf, samplerate, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(audio_chn, samples_per_buf, samplerate, SCE_AUDIO_OUT_MODE_STEREO);

	for(;;) {
		// A pretty bad way to close thread
		if (instance->termStream) {
			instance->termStream = false;
			sceAudioOutReleasePort(audio_chn);
			sceKernelExitDeleteThread(0);
		}

		instance->LockMutex();
		instance->Decode(buffer.data(), buf_size);
		instance->UnlockMutex();

		int res = sceAudioOutOutput(audio_chn, buffer.data());
		if (res < 0) Output::Error("An error occurred in audio thread (0x%lX)", res);
	}
}

Psp2Audio::Psp2Audio() :
	GenericAudio()
{
	instance = this;

	audio_mutex = sceKernelCreateMutex("Audio Mutex", 0, 0, NULL);

	audio_thread = sceKernelCreateThread("Audio Thread", &psp2_audio_thread, 0x10000100, 0x10000, 0, 0, NULL);

	SetFormat(samplerate, AudioDecoder::Format::S16, 2);

	int res = sceKernelStartThread(audio_thread, 0, 0);
	if (res != 0) {
		Output::Error("Failed to init audio thread (0x%x)", res);
		return;
	}
}

Psp2Audio::~Psp2Audio() {
	// Closing streaming thread
	termStream = true;
	sceKernelWaitThreadEnd(audio_thread, NULL, NULL);

	// Deleting mutex
	sceKernelDeleteMutex(audio_mutex);
}

void Psp2Audio::LockMutex() const {
	sceKernelLockMutex(audio_mutex, 1, NULL);
}

void Psp2Audio::UnlockMutex() const {
	sceKernelUnlockMutex(audio_mutex, 1);
}

#endif
