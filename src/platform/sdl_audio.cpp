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
#include "sdl_audio.h"
#include "filefinder.h"
#include "output.h"


#ifdef _WIN32
#  include "util_win.h"
#endif

SdlAudio::SdlAudio() :
	bgm_volume(0),
	bgs_channel(0),
	bgs_playing(false),
	me_channel(0),
	me_stopped_bgm(false)
{
	if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			Output::Error("Couldn't initialize audio.\n%s\n", SDL_GetError());
		}
	}
#ifdef GEKKO
	int const frequency = 32000;
#else
	int const frequency = 44100;
#endif
	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) < 0) {
		Output::Error("Couldn't initialize audio.\n%s\n", Mix_GetError());
	}
	Mix_AllocateChannels(32); // Default is MIX_CHANNELS = 8
}

SdlAudio::~SdlAudio() {
	Mix_CloseAudio();
}

void SdlAudio::BGM_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}

	SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");
#if SDL_MIXER_MAJOR_VERSION>1
	bgm.reset(Mix_LoadMUS_RW(rw, 1), &Mix_FreeMusic);
#else
	bgm.reset(Mix_LoadMUS_RW(rw), &Mix_FreeMusic);
#endif
	if (!bgm) {
		Output::Warning("Couldn't load %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer produces noise when playing wav.
	// Workaround: Use Mix_LoadWAV
	// https://bugzilla.libsdl.org/show_bug.cgi?id=2094
	if (bgs_playing) {
		BGS_Stop();
	}
	if (Mix_GetMusicType(bgm.get()) == MUS_WAV) {
		BGM_Stop();
		BGS_Play(file, volume, 0, fadein);
		return;
	}
#endif

	BGM_Volume(volume);
	if (!me_stopped_bgm &&
#ifdef _WIN32
	    (Mix_GetMusicType(bgm.get()) == MUS_MID && WindowsUtils::GetWindowsVersion() >= 6
	     ? Mix_PlayMusic(bgm.get(), -1) : Mix_FadeInMusic(bgm.get(), -1, fadein))
#else
	     Mix_FadeInMusic(bgm.get(), -1, fadein)
#endif
	     == -1) {
		Output::Warning("Couldn't play %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
}

void SdlAudio::BGM_Pause() {
	// Midi pause is not supported... (for some systems -.-)
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (Mix_GetMusicType(bgm.get()) == MUS_WAV) {
		BGS_Pause();
		return;
	}
#endif
	Mix_PauseMusic();
}

void SdlAudio::BGM_Resume() {
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (Mix_GetMusicType(bgm.get()) == MUS_WAV) {
		BGS_Resume();
		return;
	}
#endif
	Mix_ResumeMusic();
}

void SdlAudio::BGM_Stop() {
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (Mix_GetMusicType(bgm.get()) == MUS_WAV) {
		BGS_Stop();
		return;
	}
#endif
	Mix_HaltMusic();
	me_stopped_bgm = false;
}

void SdlAudio::BGM_Volume(int volume) {
	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
}

void SdlAudio::BGM_Pitch(int /* pitch */) {
	// TODO
}

void SdlAudio::BGM_Fade(int fade) {
#ifdef _WIN32
	// FIXME: Because of design change in Vista and higher reducing Midi volume
	// alters volume of whole application and mutes it forever when restarted.
	// Fading out midi music was disabled for Windows.
	if (Mix_GetMusicType(bgm.get()) == MUS_MID &&
		WindowsUtils::GetWindowsVersion() >= 6) {
		BGM_Stop();
		return;
	}
#endif
	Mix_FadeOutMusic(fade);
	me_stopped_bgm = false;
}

void SdlAudio::BGS_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}
	
	bgs.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!bgs) {
		Output::Warning("Couldn't load %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgs_channel = Mix_FadeInChannel(-1, bgs.get(), -1, fadein);
	Mix_Volume(bgs_channel, volume * MIX_MAX_VOLUME / 100);
	if (bgs_channel == -1) {
		Output::Warning("Couldn't play %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgs_playing = true;
}

void SdlAudio::BGS_Pause() {
	if (Mix_Playing(bgs_channel)) {
		Mix_Pause(bgs_channel);
	}
}

void SdlAudio::BGS_Resume() {
	Mix_Resume(bgs_channel);
}

void SdlAudio::BGS_Stop() {
	if (Mix_Playing(bgs_channel)) {
		Mix_HaltChannel(bgs_channel);
		bgs_playing = false;
	}
}

void SdlAudio::BGS_Fade(int fade) {
	Mix_FadeOutChannel(bgs_channel, fade);
}

/*
void me_finish(int channel) {
	if (me_channel == channel && me_stopped_bgm) {
		Mix_VolumeMusic(bgm_volume);
		Mix_FadeInMusic(bgm.get(), -1, 1000);
		me_stopped_bgm = false;
	}
}
*/

void SdlAudio::ME_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}
	me.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!me) {
		Output::Warning("Couldn't load %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_channel = Mix_FadeInChannel(-1, me.get(), 0, fadein);
	Mix_Volume(me_channel, volume * MIX_MAX_VOLUME / 100);
	if (me_channel == -1) {
		Output::Warning("Couldn't play %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_stopped_bgm = (Mix_PlayingMusic() == 1);
	// Mix_ChannelFinished(me_finish);
}

void SdlAudio::ME_Stop() {
	if (Mix_Playing(me_channel)) {
		Mix_HaltChannel(me_channel);
	}
}

void SdlAudio::ME_Fade(int fade) {
	Mix_FadeOutChannel(me_channel, fade);
}

void SdlAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindSound(file);
	if (path.empty()) {
		Output::Debug("Sound not found: %s", file.c_str());
		return;
	}
	EASYRPG_SHARED_PTR<Mix_Chunk> sound(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!sound) {
		Output::Warning("Couldn't load %s SE.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	int channel = Mix_PlayChannel(-1, sound.get(), 0);
	Mix_Volume(channel, volume * MIX_MAX_VOLUME / 100);
	if (channel == -1) {
		Output::Warning("Couldn't play %s SE.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	sounds[channel] = sound;
}

void SdlAudio::SE_Stop() {
	for (sounds_type::iterator i = sounds.begin(); i != sounds.end(); ++i) {
		if (Mix_Playing(i->first)) Mix_HaltChannel(i->first);
	}
	sounds.clear();
}

void SdlAudio::Update() {
}
