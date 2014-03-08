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
#include <boost/noncopyable.hpp>

/**
 * Base Audio class.
 */
struct AudioInterface : boost::noncopyable {
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
	 */
	virtual void BGM_Play(std::string const& file, int volume, int pitch) = 0;

	/**
	 * Stops the currently playing background music.
	 */
	virtual void BGM_Stop() = 0;

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
	 * Plays a background sound.
	 *
	 * @param file file to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 */
	virtual void BGS_Play(std::string const& file, int volume, int pitch) = 0;

	/**
	 * Stops the currently playing background sound.
	 */
	virtual void BGS_Stop() = 0;

	/**
	 * Does a fade out of the background sound.
	 *
	 * @param fade fade out time.
	 */
	virtual void BGS_Fade(int fade) = 0;

	/**
	 * Plays a music effect.
	 *
	 * @param file file to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 */
	virtual void ME_Play(std::string const& file, int volume, int pitch) = 0;

	/**
	 * Stops the currently playing music effect.
	 */
	virtual void ME_Stop() = 0;

	/**
	 * Does a fade out of the music effect.
	 *
	 * @param fade Fade out time
	 */
	virtual void ME_Fade(int fade) = 0;

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
	void BGM_Play(std::string const&, int, int) {}
	void BGM_Pause() {}
	void BGM_Resume() {}
	void BGM_Stop() {}
	void BGM_Fade(int) {}
	void BGM_Volume(int) {}
	void BGM_Pitch(int) {};
	void BGS_Play(std::string const&, int, int) {}
	void BGS_Stop() {}
	void BGS_Fade(int) {}
	void ME_Play(std::string const&, int, int) {}
	void ME_Stop() {}
	void ME_Fade(int) {}
	void SE_Play(std::string const&, int, int) {}
	void SE_Stop() {}
	void Update() {}
};

AudioInterface& Audio();

#endif
