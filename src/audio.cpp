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
#include "game_clock.h"

AudioInterface& Audio() {
	static Game_ConfigAudio cfg;
	static EmptyAudio default_(cfg);
#ifdef SUPPORT_AUDIO
	if (!Player::no_audio_flag && DisplayUi)
		return DisplayUi->GetAudio();
#endif
	return default_;
}

void EmptyAudio::BGM_Play(Filesystem_Stream::InputStream, int, int, int) {
	bgm_starttick = Player::GetFrames();
	playing = true;
}

void EmptyAudio::BGM_Stop() {
	playing = false;
}

int EmptyAudio::BGM_GetTicks() const {
	if (!playing) {
		return 0;
	}

	// Time since BGM_Play was called, works for everything except MIDI
	return (Player::GetFrames() - bgm_starttick + 1) / Game_Clock::GetTargetGameFps();
}

void EmptyAudio::vGetConfig(Game_ConfigAudio& cfg) const {
	cfg.music_volume.SetOptionVisible(false);
	cfg.sound_volume.SetOptionVisible(false);
}

bool EmptyAudio::BGM_PlayedOnce() const {
	// 5 seconds, arbitrary
	return BGM_GetTicks() > (Game_Clock::GetTargetGameFps() * 5);
}

EmptyAudio::EmptyAudio(const Game_ConfigAudio& cfg) : AudioInterface(cfg) {

}

AudioInterface::AudioInterface(const Game_ConfigAudio& cfg) : cfg(cfg) {

}

Game_ConfigAudio AudioInterface::GetConfig() const {
	auto acfg = cfg;
	acfg.Hide();
	vGetConfig(acfg);
	return acfg;
}

int AudioInterface::BGM_GetGlobalVolume() const {
	return cfg.music_volume.Get();
}

void AudioInterface::BGM_SetGlobalVolume(int volume) {
	cfg.music_volume.Set(volume);
}

int AudioInterface::SE_GetGlobalVolume() const {
	return cfg.sound_volume.Get();
}

void AudioInterface::SE_SetGlobalVolume(int volume) {
	cfg.sound_volume.Set(volume);
}
