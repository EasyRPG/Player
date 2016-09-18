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

#ifndef _AUDIO_AL_H_
#define _AUDIO_AL_H_

// Headers
#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenAL/al.h>
#  include <OpenAL/alc.h>
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "system.h"
#include "audio.h"

#include <map>
#include <vector>

struct ALAudio : public AudioInterface {
	ALAudio(char const *dev_name = NULL);

	void BGM_Play(std::string const &, int, int, int) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override;
	unsigned BGM_GetTicks() const override;
	void BGM_Fade(int) override;
	void BGM_Volume(int) override;
	void BGM_Pitch(int) override;
	void SE_Play(std::string const &, int, int) override;
	void SE_Stop() override;
	void Update() override;

	static char const WAVE_OUTPUT_DEVICE[];
	static char const NULL_DEVICE[];

private:
	struct source;
	struct buffer_loader;
	struct sndfile_loader;
	struct midi_loader;

	std::shared_ptr<source> create_source(bool loop) const;
	std::shared_ptr<buffer_loader> create_loader(source &src, std::string const &file) const;

	std::shared_ptr<buffer_loader> getMusic(source &src, std::string const &file) const;
	std::shared_ptr<buffer_loader> getSound(source &src, std::string const &file) const;

	std::shared_ptr<ALCdevice> dev_;
	std::shared_ptr<ALCcontext> ctx_;

	std::shared_ptr<source> bgm_src_;

	typedef std::vector<std::shared_ptr<source> > source_list;
	source_list se_src_;
};  // struct ALAudio

#endif  // _AUDIO_AL_H_
