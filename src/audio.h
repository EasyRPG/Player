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

#ifndef _EASYRPG_PLAYER_AUDIO_H_
#define _EASYRPG_PLAYER_AUDIO_H_

// Headers
#include <string>

/**
 * Base Audio class.
 */
struct AudioInterface {
	virtual ~AudioInterface() {}

	/**
	 * Update audio. Must be called each frame.
	 */
	virtual void Update() = 0;

	/**
	 * Plays a background music.
	 *
	 * @param file file to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 * @param fadein fadein.
	 */
	virtual void BGM_Play(std::string const& file, int volume, int pitch, int fadein) = 0;

	/**
	 * Stops the currently playing background music.
	 */
	virtual void BGM_Stop() = 0;

	/**
	 * Returns whether the background music has played at least once.
	 */
	virtual bool BGM_PlayedOnce() = 0;

	/**
	 * TODO
	 */
	virtual unsigned BGM_GetTicks() = 0;

	/**
	 * Does a fade out of the background music.
	 *
	 * @param fade fade out time.
	 */
	virtual void BGM_Fade(int fade) = 0;

	/**
	 * Pauses the currently playing background music.
	 */
	virtual void BGM_Pause() = 0;

	/**
	 * Resumes the currently playing background music.
	 */
	virtual void BGM_Resume() = 0;

	/**
	 * Adjusts the volume of the currently playing background music.
	 *
	 * @param volume volume
	 */
	virtual void BGM_Volume(int volume) = 0;

	/**
	 * Adjusts the pitch of the currently playing background music.
	 *
	 * @param pitch pitch
	 */
	virtual void BGM_Pitch(int pitch) = 0;

	/**
	 * Plays a sound effect.
	 *
	 * @param file file to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 */
	virtual void SE_Play(std::string const& file, int volume, int pitch) = 0;

	/**
	 * Stops the currently playing sound effect.
	 */
	virtual void SE_Stop() = 0;
};

struct EmptyAudio : public AudioInterface {
	void BGM_Play(std::string const&, int, int, int);
	void BGM_Pause() {}
	void BGM_Resume() {}
	void BGM_Stop() {}
	bool BGM_PlayedOnce();
	unsigned BGM_GetTicks();
	void BGM_Fade(int) {}
	void BGM_Volume(int) {}
	void BGM_Pitch(int) {};
	void SE_Play(std::string const&, int, int) {}
	void SE_Stop() {}
	void Update() {}

	unsigned bgm_starttick = 0;
};

AudioInterface& Audio();

#endif
