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

// Headers
#include "audio.h"
#include "system.h"
#include "baseui.h"
#include "player.h"
#include "graphics.h"

AudioInterface& Audio() {
	static EmptyAudio default_;
#ifdef SUPPORT_AUDIO
	if (!Player::no_audio_flag && DisplayUi)
		return DisplayUi->GetAudio();
#endif
	return default_;
}

void EmptyAudio::BGM_Play(std::string const&, int, int, int) {
	bgm_starttick = Player::GetFrames();
	playing = true;
}

void EmptyAudio::BGM_Stop() {
	playing = false;
}

unsigned EmptyAudio::BGM_GetTicks() const {
	if (!playing) {
		return 0;
	}

	// Time since BGM_Play was called, works for everything except MIDI
	return (Player::GetFrames() - bgm_starttick + 1) / Graphics::GetDefaultFps();
}

bool EmptyAudio::BGM_PlayedOnce() const {
	// 5 seconds, arbitrary
	return BGM_GetTicks() > (Graphics::GetDefaultFps() * 5);
}
