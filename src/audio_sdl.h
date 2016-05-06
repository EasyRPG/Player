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

#ifndef _AUDIO_SDL_H_
#define _AUDIO_SDL_H_

#include "audio.h"
#include "audio_decoder.h"

#include <map>

#include <SDL.h>
#include <SDL_mixer.h>

struct SdlAudio : public AudioInterface {
	SdlAudio();
	~SdlAudio();

	void BGM_Play(std::string const&, int, int, int) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() override;
	unsigned BGM_GetTicks() override;
	void BGM_Fade(int) override;
	void BGM_Volume(int) override;
	void BGM_Pitch(int) override;
	void BGS_Play(std::string const&, int, int, int);
	void BGS_Pause();
	void BGS_Resume();
	void BGS_Stop();
	void BGS_Fade(int);
	void BGS_Volume(int);
	void SE_Play(std::string const&, int, int) override;
	void SE_Stop() override;
	void Update() override;

	void BGM_OnPlayedOnce();

	AudioDecoder* GetDecoder();
	SDL_AudioCVT& GetAudioCVT();
private:
	void SetupAudioDecoder(FILE* handle, int volume, int pitch, int fadein);

	std::shared_ptr<Mix_Music> bgm;
	int bgm_volume;
	unsigned bgm_starttick = 0;
	bool bgm_stop = false;
	std::shared_ptr<Mix_Chunk> bgs;
	bool bgs_playing = false;
	bool bgs_stop = false;
	bool played_once = false;

	typedef std::map<int, std::shared_ptr<Mix_Chunk> > sounds_type;
	sounds_type sounds;

	std::unique_ptr<AudioDecoder> audio_decoder;
	SDL_AudioCVT cvt;
}; // class SdlAudio

#endif // _AUDIO_SDL_H_
