/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "sdl_audio.h"
#include "filefinder.h"
#include "output.h"


#ifdef _WIN32
// FIXME: A bug in sdl_mixer causes that the player is muted forever when a
// fadeout happened.
// Fade out on Vista and higher has been disabled until this is fixed.
#include "util_win.h"
#endif

///////////////////////////////////////////////////////////
SdlAudio::SdlAudio() :
	bgm_volume(0),
	bgs_channel(0),
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
	int const frequency = MIX_DEFAULT_FREQUENCY;
#endif
	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) < 0) {
		Output::Error("Couldn't initialize audio.\n%s\n", Mix_GetError());
	}
	/*int flags = MIX_INIT_MP3;
	int initted = Mix_Init(flags);
	if ((initted & flags) != flags) {
		Output::Error("Couldn't initialize audio.\n%s\n", Mix_GetError());
	}*/
}

///////////////////////////////////////////////////////////
SdlAudio::~SdlAudio() {
	Mix_CloseAudio();
}

///////////////////////////////////////////////////////////
void SdlAudio::BGM_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	bgm.reset(Mix_LoadMUS(path.c_str()), &Mix_FreeMusic);
	if (!bgm) {
		Output::Warning("Couldn't load %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
	if (!me_stopped_bgm && Mix_PlayMusic(bgm.get(), -1) == -1) {
		Output::Warning("Couldn't play %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
}

///////////////////////////////////////////////////////////
void SdlAudio::BGM_Pause() {
	// Midi pause is not supported... =.=
	Mix_PauseMusic();
}

///////////////////////////////////////////////////////////
void SdlAudio::BGM_Resume() {
	Mix_ResumeMusic();
}

///////////////////////////////////////////////////////////
void SdlAudio::BGM_Stop() {
	Mix_HaltMusic();
	me_stopped_bgm = false;
}

////////////////////////////////////////////////////////////
void SdlAudio::BGM_Fade(int fade) {
#ifdef _WIN32
	if (WindowsUtils::GetWindowsVersion() >= 6) {
		BGM_Stop();
		return;
	}
#endif
	Mix_FadeOutMusic(fade);
	me_stopped_bgm = false;
}

////////////////////////////////////////////////////////////
void SdlAudio::BGS_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	bgs.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!bgs) {
		Output::Warning("Couldn't load %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgs_channel = Mix_PlayChannel(-1, bgs.get(), -1);
	Mix_Volume(bgs_channel, volume * MIX_MAX_VOLUME / 100);
	if (bgs_channel == -1) {
		Output::Warning("Couldn't play %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
}

////////////////////////////////////////////////////////////
void SdlAudio::BGS_Stop() {
	if (Mix_Playing(bgs_channel)) {
		Mix_HaltChannel(bgs_channel);
	}
}

////////////////////////////////////////////////////////////
void SdlAudio::BGS_Fade(int fade) {
	Mix_FadeOutChannel(bgs_channel, fade);
}

/*
////////////////////////////////////////////////////////////
void me_finish(int channel) {
	if (me_channel == channel && me_stopped_bgm) {
		Mix_VolumeMusic(bgm_volume);
		Mix_FadeInMusic(bgm.get(), -1, 1000);
		me_stopped_bgm = false;
	}
}
*/

///////////////////////////////////////////////////////////
void SdlAudio::ME_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	me.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!me) {
		Output::Warning("Couldn't load %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_channel = Mix_PlayChannel(-1, me.get(), 0);
	Mix_Volume(me_channel, volume * MIX_MAX_VOLUME / 100);
	if (me_channel == -1) {
		Output::Warning("Couldn't play %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_stopped_bgm = (Mix_PlayingMusic() == 1);
	// Mix_ChannelFinished(me_finish);
}

////////////////////////////////////////////////////////////
void SdlAudio::ME_Stop() {
	if (Mix_Playing(me_channel)) {
		Mix_HaltChannel(me_channel);
	}
}

////////////////////////////////////////////////////////////
void SdlAudio::ME_Fade(int fade) {
	Mix_FadeOutChannel(me_channel, fade);
}

////////////////////////////////////////////////////////////
void SdlAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindSound(file);
	if (path.empty()) {
		Output::Warning("No such file or directory - %s", file.c_str());
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

////////////////////////////////////////////////////////////
void SdlAudio::SE_Stop() {
	for (sounds_type::iterator i = sounds.begin(); i != sounds.end(); i++) {
		if (Mix_Playing(i->first)) Mix_HaltChannel(i->first);
	}
	sounds.clear();
}

void SdlAudio::Update() {
}
