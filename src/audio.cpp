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
#include "audio_midi.h"
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

void EmptyAudio::vGetConfig(Game_ConfigAudio&) const {
	// Not supported. The audio menu is disabled.
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

#if !defined(HAVE_FLUIDSYNTH) && !defined(HAVE_FLUIDLITE)
	acfg.fluidsynth_midi.SetOptionVisible(false);
	acfg.soundfont.SetOptionVisible(false);
#endif
#ifndef HAVE_LIBWILDMIDI
	acfg.wildmidi_midi.SetOptionVisible(false);
#endif
#ifndef HAVE_NATIVE_MIDI
	acfg.native_midi.SetOptionVisible(false);
#endif
#ifndef WANT_FMMIDI
	acfg.fmmidi_midi.SetOptionVisible(false);
#endif

#ifdef __ANDROID__
	// FIXME: URI encoded SAF paths are not supported
	acfg.soundfont.SetOptionVisible(false);
#endif

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

bool AudioInterface::GetFluidsynthEnabled() const {
	return cfg.fluidsynth_midi.Get();
}

void AudioInterface::SetFluidsynthEnabled(bool enable) {
	cfg.fluidsynth_midi.Set(enable);
}

bool AudioInterface::GetWildMidiEnabled() const {
	return cfg.wildmidi_midi.Get();
}

void AudioInterface::SetWildMidiEnabled(bool enable) {
	cfg.wildmidi_midi.Set(enable);
}

bool AudioInterface::GetNativeMidiEnabled() const {
	return cfg.native_midi.Get();
}

void AudioInterface::SetNativeMidiEnabled(bool enable) {
	cfg.native_midi.Set(enable);
}

std::string AudioInterface::GetFluidsynthSoundfont() const {
	return cfg.soundfont.Get();
}

void AudioInterface::SetFluidsynthSoundfont(StringView sf) {
	cfg.soundfont.Set(ToString(sf));
	MidiDecoder::ChangeFluidsynthSoundfont(sf);
}
