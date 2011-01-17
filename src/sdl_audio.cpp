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
#include "system.h"
#if defined(USE_SDL) && defined(SUPPORT_AUDIO)
#include <map>
#include "audio.h"
#include "filefinder.h"
#include "output.h"
#include "SDL.h"
#include "SDL_mixer.h"

#ifdef _WIN32
// FIXME: A bug in sdl_mixer causes that the player is muted forever when a
// fadeout happened.
// Fade out on Vista and higher has been disabled until this is fixed.
#include "util_win.h"
#endif

////////////////////////////////////////////////////////////
namespace Audio {
	Mix_Music* bgm;
	int bgm_volume;
	Mix_Chunk* bgs;
	int bgs_channel;
	Mix_Chunk* me;
	int me_channel;
	bool me_stopped_bgm;
	std::map<int, Mix_Chunk*> sounds;
	std::map<int, Mix_Chunk*>::iterator it_sounds;
}

///////////////////////////////////////////////////////////
void Audio::Init() {
	if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			Output::Error("Couldn't initialize audio.\n%s\n", SDL_GetError());
		}
	}
#ifdef GEKKO
	int frequency = 32000;
#else
	int frequency = MIX_DEFAULT_FREQUENCY;
#endif

	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) < 0) {
		Output::Error("Couldn't initialize audio.\n%s\n", Mix_GetError());
	}
	/*int flags = MIX_INIT_MP3;
	int initted = Mix_Init(flags);
	if ((initted & flags) != flags) {
		Output::Error("Couldn't initialize audio.\n%s\n", Mix_GetError());
	}*/
	bgm = NULL;
	bgs = NULL;
	me = NULL;
	me_stopped_bgm = false;
}

///////////////////////////////////////////////////////////
void Audio::Quit() {
	std::map<int, Mix_Chunk*>::iterator i;
	for (i = sounds.begin(); i != sounds.end(); i++) {
		Mix_FreeChunk(i->second);
	}
	Mix_FreeMusic(bgm);
	Mix_FreeChunk(bgs);

	Mix_CloseAudio();
}

///////////////////////////////////////////////////////////
void Audio::BGM_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	if (bgm != NULL) Mix_FreeMusic(bgm);
	bgm = Mix_LoadMUS(path.c_str());
	if (!bgm) {
		Output::Warning("Couldn't load %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
	if (!me_stopped_bgm) {
		if (Mix_PlayMusic(bgm, -1) == -1) {
			Output::Warning("Couldn't play %s BGM.\n%s\n", file.c_str(), Mix_GetError());
			return;
		}
	}
}

///////////////////////////////////////////////////////////
void Audio::BGM_Pause() {
	// Midi pause is not supported... =.=
	Mix_PauseMusic();
}

///////////////////////////////////////////////////////////
void Audio::BGM_Resume() {
	Mix_ResumeMusic();
}

///////////////////////////////////////////////////////////
void Audio::BGM_Stop() {
	Mix_HaltMusic();
	me_stopped_bgm = false;
}

////////////////////////////////////////////////////////////
void Audio::BGM_Fade(int fade) {
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
void Audio::BGS_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	if (bgs != NULL) Mix_FreeChunk(bgs);
	bgs = Mix_LoadWAV(path.c_str());
	if (!bgs) {
		Output::Warning("Couldn't load %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	bgs_channel = Mix_PlayChannel(-1, bgs, -1);
	Mix_Volume(bgs_channel, volume * MIX_MAX_VOLUME / 100);
	if (bgs_channel == -1) {
		Output::Warning("Couldn't play %s BGS.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
}

////////////////////////////////////////////////////////////
void Audio::BGS_Stop() {
	if (Mix_Playing(bgs_channel)) Mix_HaltChannel(bgs_channel);
}

////////////////////////////////////////////////////////////
void Audio::BGS_Fade(int fade) {
	Mix_FadeOutChannel(bgs_channel, fade);
}

////////////////////////////////////////////////////////////
void me_finish(int channel) {
	if (Audio::me_channel == channel) {
		if (Audio::me_stopped_bgm) {
			Mix_VolumeMusic(Audio::bgm_volume);
			Mix_FadeInMusic(Audio::bgm, -1, 1000);
			Audio::me_stopped_bgm = false;
		}
	}
}

///////////////////////////////////////////////////////////
void Audio::ME_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	if (me != NULL) Mix_FreeChunk(bgs);
	me = Mix_LoadWAV(path.c_str());
	if (!me) {
		Output::Warning("Couldn't load %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_channel = Mix_PlayChannel(-1, me, 0);
	Mix_Volume(me_channel, volume * MIX_MAX_VOLUME / 100);
	if (me_channel == -1) {
		Output::Warning("Couldn't play %s ME.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	me_stopped_bgm = Mix_PlayingMusic() == 1;
	Mix_ChannelFinished(me_finish);
}

////////////////////////////////////////////////////////////
void Audio::ME_Stop() {
	if (Mix_Playing(me_channel)) Mix_HaltChannel(me_channel);
}

////////////////////////////////////////////////////////////
void Audio::ME_Fade(int fade) {
	Mix_FadeOutChannel(me_channel, fade);
}

////////////////////////////////////////////////////////////
void Audio::SE_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindSound(file);
	if (path == "") {
		Output::Warning("No such file or directory - %s", file.c_str());
		return;
	}
	Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
	if (!sound) {
		Output::Warning("Couldn't load %s SE.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	int channel = Mix_PlayChannel(-1, sound, 0);
	Mix_Volume(channel, volume * MIX_MAX_VOLUME / 100);
	if (channel == -1) {
		Output::Warning("Couldn't play %s SE.\n%s\n", file.c_str(), Mix_GetError());
		return;
	}
	// FIXME: Create a cache for this maybe?
	std::map<int, Mix_Chunk*>::iterator i = sounds.find(channel);
	if (i != sounds.end()) {
		Mix_FreeChunk(i->second);
	}
	sounds[channel] = sound;
}

////////////////////////////////////////////////////////////
void Audio::SE_Stop() {
	for (it_sounds = sounds.begin(); it_sounds != sounds.end(); it_sounds++) {
		if (Mix_Playing(it_sounds->first)) Mix_HaltChannel(it_sounds->first);
		Mix_FreeChunk(it_sounds->second);
	}
	sounds.clear();
}

#endif
