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
#ifndef AUDIOLIBRETRO2_H
#define AUDIOLIBRETRO2_H

#if defined(USE_LIBRETRO)

#include "libretro.h"
#include "audio_generic.h"

class LibretroAudio : public GenericAudio {
public:
	LibretroAudio();
	~LibretroAudio();

	void LockMutex() const override;
	void UnlockMutex() const override;

   static void EnableAudio(bool enabled);
   static void AudioThreadCallback();
   static void SetRetroAudioCallback(retro_audio_sample_batch_t cb);
}; // class LibretroAudio

#endif
#endif
