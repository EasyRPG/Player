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

#ifndef _AUDIO_H_
#define _AUDIO_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <map>
#include "SDL.h"
#include "SDL_mixer.h"

////////////////////////////////////////////////////////////
/// Audio namespace
////////////////////////////////////////////////////////////
namespace Audio {
	void Init();
	void Quit();
	void BGM_Play(std::string file, int volume, int pitch);
	void BGM_Stop();
	void BGM_Fade(int fade);
	void BGS_Play(std::string file, int volume, int pitch);
	void BGS_Stop();
	void BGS_Fade(int fade);
	void ME_Play(std::string file, int volume, int pitch);
	void ME_Stop();
	void ME_Fade(int fade);
	void SE_Play(std::string file, int volume, int pitch);
	void SE_Stop();
	
	extern Mix_Music* bgm;
	extern int bgm_volume;
	extern Mix_Chunk* bgs;
	extern int bgs_channel;
	extern Mix_Chunk* me;
	extern int me_channel;
	extern bool me_stopped_bgm;
	extern std::map<int, Mix_Chunk*> sounds;
	extern std::map<int, Mix_Chunk*>::iterator it_sounds;
}

#endif
