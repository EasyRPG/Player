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
	virtual bool BGM_PlayedOnce() const = 0;
	
	/**
	 * Reports if a music file is currently being played.
	 * 
	 * @return true when BGM is playing
	 */
	virtual bool BGM_IsPlaying() const = 0;

	/**
	 * Returns the current MIDI tick of the background music.
	 * Only useful when the BGM is a MIDI track.
	 */
	virtual unsigned BGM_GetTicks() const = 0;

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
	void BGM_Play(std::string const&, int, int, int) override;
	void BGM_Pause() override {}
	void BGM_Resume() override {}
	void BGM_Stop() override {}
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override { return false; }
	unsigned BGM_GetTicks() const override;
	void BGM_Fade(int) override {}
	void BGM_Volume(int) override {}
	void BGM_Pitch(int) override {};
	void SE_Play(std::string const&, int, int) override {}
	void SE_Stop() override {}
	void Update() override {}

	unsigned bgm_starttick = 0;
};

AudioInterface& Audio();

#endif
