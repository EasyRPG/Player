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
 * Two threads share the work:
 * - The mixer thread runs GenericAudio::Decode and fills a short ring of PCM blocks. Decoding may
 *   take longer than a block lasts now and then (e.g. when many MIDI notes play or the game
 *   holds the audio lock to open a file), the ring hides that.
 * - The output thread copies each block into one of several exclusive DMA buffers and hands it to
 *   the PSP's hardware resampler (44.1 kHz, 16 bit, stereo). It runs at a higher priority than the
 *   game so it never misses a deadline.
 */
class PspAudio : public GenericAudio {
public:
	PspAudio(const Game_ConfigAudio& cfg);
	~PspAudio();

	void LockMutex() const override;
	void UnlockMutex() const override;

	volatile bool termStream = false;

private:
	SceUID audio_mutex = -1;
	SceUID mixer_thread = -1;
	SceUID output_thread = -1;
}; // class PspAudio

#endif
