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
#include "audio.h"
#include "output.h"
#include "filefinder.h"

////////////////////////////////////////////////////////////
/// Global Variables
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

////////////////////////////////////////////////////////////
/// Initialize
////////////////////////////////////////////////////////////
void Audio::Init() {
	if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			Output::Error("Couldn't initialize audio.\n%s\n", SDL_GetError());
		}
	}
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
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

void Audio::Quit() {
	std::map<int, Mix_Chunk*>::iterator i;
	for (i = sounds.begin(); i != sounds.end(); i++) {
		delete i->second;
	}
}

////////////////////////////////////////////////////////////
/// BGM play
////////////////////////////////////////////////////////////
void Audio::BGM_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Error("No such file or directory - %s", file.c_str());
	}
	if (bgm != NULL) Mix_FreeMusic(bgm);
	bgm = Mix_LoadMUS(path.c_str());
	if (!bgm) {
		Output::Error("Couldn't load %s BGM.\n%s\n", file.c_str(), Mix_GetError());
	}
	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
	if (!me_stopped_bgm) {
		if (Mix_PlayMusic(bgm, -1) == -1) {
			Output::Error("Couldn't play %s BGM.\n%s\n", file.c_str(), Mix_GetError());
		}
	}
}

////////////////////////////////////////////////////////////
/// BGM stop
////////////////////////////////////////////////////////////
void Audio::BGM_Stop() {
	Mix_HaltMusic();
	me_stopped_bgm = false;
}

////////////////////////////////////////////////////////////
/// BGM fade
////////////////////////////////////////////////////////////
void Audio::BGM_Fade(int fade) {
	Mix_FadeOutMusic(fade);
	me_stopped_bgm = false;
}

////////////////////////////////////////////////////////////
/// BGS play
////////////////////////////////////////////////////////////
void Audio::BGS_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Error("No such file or directory - %s", file.c_str());
	}
	if (bgs != NULL) Mix_FreeChunk(bgs);
	bgs = Mix_LoadWAV(path.c_str());
	if (!bgs) {
		Output::Error("Couldn't load %s BGS.\n%s\n", file.c_str(), Mix_GetError());
	}
	bgs_channel = Mix_PlayChannel(-1, bgs, -1);
	Mix_Volume(bgs_channel, volume * MIX_MAX_VOLUME / 100);
	if (bgs_channel == -1) {
		Output::Error("Couldn't play %s BGS.\n%s\n", file.c_str(), Mix_GetError());
	}
}

////////////////////////////////////////////////////////////
/// BGS stop
////////////////////////////////////////////////////////////
void Audio::BGS_Stop() {
	if (Mix_Playing(bgs_channel)) Mix_HaltChannel(bgs_channel);
}

////////////////////////////////////////////////////////////
/// BGS fade
////////////////////////////////////////////////////////////
void Audio::BGS_Fade(int fade) {
	Mix_FadeOutChannel(bgs_channel, fade);
}

////////////////////////////////////////////////////////////
/// ME play
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
void Audio::ME_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		 Output::Error("No such file or directory - %s", file.c_str());
	}
	if (me != NULL) Mix_FreeChunk(bgs);
	me = Mix_LoadWAV(path.c_str());
	if (!me) {
		Output::Error("Couldn't load %s ME.\n%s\n", file.c_str(), Mix_GetError());
	}
	me_channel = Mix_PlayChannel(-1, me, 0);
	Mix_Volume(me_channel, volume * MIX_MAX_VOLUME / 100);
	if (me_channel == -1) {
		Output::Error("Couldn't play %s ME.\n%s\n", file.c_str(), Mix_GetError());
	}
	me_stopped_bgm = Mix_PlayingMusic() == 1;
	Mix_ChannelFinished(me_finish);
}

////////////////////////////////////////////////////////////
/// ME stop
////////////////////////////////////////////////////////////
void Audio::ME_Stop() {
	if (Mix_Playing(me_channel)) Mix_HaltChannel(me_channel);
}

////////////////////////////////////////////////////////////
/// ME fade
////////////////////////////////////////////////////////////
void Audio::ME_Fade(int fade) {
	Mix_FadeOutChannel(me_channel, fade);
}

////////////////////////////////////////////////////////////
/// SE play
////////////////////////////////////////////////////////////
void Audio::SE_Play(std::string file, int volume, int pitch) {
	std::string path = FileFinder::FindMusic(file);
	if (path == "") {
		Output::Error("No such file or directory - %s", file.c_str());
	}
	Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
	if (!sound) {
		Output::Error("Couldn't load %s SE.\n%s\n", file.c_str(), Mix_GetError());
	}
	int channel = Mix_PlayChannel(-1, sound, 0);
	Mix_Volume(channel, volume * MIX_MAX_VOLUME / 100);
	if (channel == -1) {
		Output::Error("Couldn't play %s SE.\n%s\n", file.c_str(), Mix_GetError());
	}
	sounds[channel] = sound;
}

////////////////////////////////////////////////////////////
/// SE stop
////////////////////////////////////////////////////////////
void Audio::SE_Stop() {
	for (it_sounds = sounds.begin(); it_sounds != sounds.end(); it_sounds++) {
		if (Mix_Playing(it_sounds->first)) Mix_HaltChannel(it_sounds->first);
		Mix_FreeChunk(it_sounds->second);
	}
	sounds.clear();
}
