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

#ifndef _AL_AUDIO_H_
#define _AL_AUDIO_H_

// Headers
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "system.h"
#include "audio.h"

#include <map>
#include <vector>

struct ALAudio : public AudioInterface {
	ALAudio(char const *dev_name = NULL);

	void BGM_Play(std::string const &, int, int, int);
	void BGM_Pause();
	void BGM_Resume();
	void BGM_Stop();
	bool BGM_PlayedOnce();
	void BGM_Fade(int);
	void BGM_Volume(int);
	void BGM_Pitch(int);
	void BGS_Play(std::string const &, int, int, int);
	void BGS_Stop();
	void BGS_Fade(int);
	void ME_Play(std::string const &, int, int, int);
	void ME_Stop();
	void ME_Fade(int);
	void SE_Play(std::string const &, int, int);
	void SE_Stop();
	void Update();

	static char const WAVE_OUTPUT_DEVICE[];
	static char const NULL_DEVICE[];

private:
	struct source;
	struct buffer_loader;
	struct sndfile_loader;
	struct midi_loader;

	EASYRPG_SHARED_PTR<source> create_source(bool loop) const;
	EASYRPG_SHARED_PTR<buffer_loader> create_loader(source &src, std::string const &file) const;

	EASYRPG_SHARED_PTR<buffer_loader> getMusic(source &src, std::string const &file) const;
	EASYRPG_SHARED_PTR<buffer_loader> getSound(source &src, std::string const &file) const;

	EASYRPG_SHARED_PTR<ALCdevice> dev_;
	EASYRPG_SHARED_PTR<ALCcontext> ctx_;

	EASYRPG_SHARED_PTR<source> bgm_src_, bgs_src_, me_src_;

	typedef std::vector<EASYRPG_SHARED_PTR<source> > source_list;
	source_list se_src_;
};  // struct ALAudio

#endif  // _AL_AUDIO_H_
