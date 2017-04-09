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

#if defined(USE_LIBRETRO) && defined(SUPPORT_AUDIO)
#include "audio_libretro.h"
#include "output.h"
#include "graphics.h"

#include <vector>
#include <cstdlib>
#include <stddef.h>
#include <rthreads/rthreads.h>

retro_audio_sample_batch_t RenderAudioFrames=0;

static const unsigned AUDIO_SAMPLERATE = 44100.0;

namespace {
   unsigned samples_per_frame = 0;
   bool enable_audio = false;
	LibretroAudio* instance = nullptr;
  std::vector<uint8_t> buffer;
  slock_t* mutex=NULL;
}

void LibretroAudio::AudioThreadCallback()
{
   if (!enable_audio)
      return;

   instance->LockMutex();
   instance->Decode(buffer.data(), samples_per_frame * 2 * 2);
   instance->UnlockMutex();

   RenderAudioFrames((const int16_t*)buffer.data(), samples_per_frame);
}

void LibretroAudio::EnableAudio(bool enabled)
{
   enable_audio = enabled;
}

LibretroAudio::LibretroAudio() :
	GenericAudio()
{
	instance = this;
    
  mutex = slock_new();
    
  buffer.resize(8192);

	SetFormat(44100, AudioDecoder::Format::S16, 2);

   samples_per_frame = AUDIO_SAMPLERATE / Graphics::GetDefaultFps();
}

LibretroAudio::~LibretroAudio() {
	// clean up resources here
}

void LibretroAudio::LockMutex() const {
	slock_lock(mutex);
}

void LibretroAudio::UnlockMutex() const {
	slock_unlock(mutex);
}

void LibretroAudio::SetRetroAudioCallback(retro_audio_sample_batch_t cb){
	RenderAudioFrames=cb;
}

#endif
