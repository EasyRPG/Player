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

#ifndef EP_PLATFORM_PSP_AUDIO_H
#define EP_PLATFORM_PSP_AUDIO_H

#include <pspkerneltypes.h>

#include "audio_generic.h"

/**
 * PspAudio class.
 *
 * Mixes the audio with GenericAudio::Decode on a dedicated thread and sends
 * the result to the PSP's hardware resampler (44.1 kHz, 16 bit, stereo).
 */
class PspAudio : public GenericAudio {
public:
	PspAudio(const Game_ConfigAudio& cfg);
	~PspAudio();

	void LockMutex() const override;
	void UnlockMutex() const override;

	volatile bool termStream = false;

private:
	SceUID audio_mutex;
	SceUID audio_thread;
}; // class PspAudio

#endif
