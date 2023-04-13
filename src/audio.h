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

#ifndef EP_AUDIO_H
#define EP_AUDIO_H

// Headers
#include <string>
#include "filesystem_stream.h"
#include "audio_secache.h"
#include "game_config.h"

/**
 * Base Audio class.
 */
struct AudioInterface {
	explicit AudioInterface(const Game_ConfigAudio& cfg);

	virtual ~AudioInterface() = default;

	/**
 	 * @return current audio options.
 	 */
	Game_ConfigAudio GetConfig() const;

	virtual void vGetConfig(Game_ConfigAudio& cfg) const = 0;

	/**
	 * Update audio. Must be called each frame.
	 */
	virtual void Update() = 0;

	/**
	 * Plays a background music.
	 *
	 * @param stream file to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 * @param fadein fadein.
	 */
	virtual void BGM_Play(Filesystem_Stream::InputStream stream, int volume, int pitch, int fadein) = 0;

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
	virtual int BGM_GetTicks() const = 0;

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
	 * @return Type of music being played.
	 */
	virtual std::string BGM_GetType() const = 0;

	/**
	 * Plays a sound effect.
	 *
	 * @param se se to play.
	 * @param volume volume.
	 * @param pitch pitch.
	 */
	virtual void SE_Play(std::unique_ptr<AudioSeCache> se, int volume, int pitch) = 0;

	/**
	 * Stops the currently playing sound effect.
	 */
	virtual void SE_Stop() = 0;

	int BGM_GetGlobalVolume() const;
	void BGM_SetGlobalVolume(int volume);

	int SE_GetGlobalVolume() const;
	void SE_SetGlobalVolume(int volume);

protected:
	Game_ConfigAudio cfg;
};

struct EmptyAudio : public AudioInterface {
public:
	explicit EmptyAudio(const Game_ConfigAudio& cfg);
	void BGM_Play(Filesystem_Stream::InputStream, int, int, int) override;
	void BGM_Pause() override {}
	void BGM_Resume() override {}
	void BGM_Stop() override;
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override { return false; }
	int BGM_GetTicks() const override;
	void BGM_Fade(int) override {}
	void BGM_Volume(int) override {}
	void BGM_Pitch(int) override {};
	std::string BGM_GetType() const override { return {}; };
	void SE_Play(std::unique_ptr<AudioSeCache>, int, int) override {}
	void SE_Stop() override {}
	void Update() override {}
	void vGetConfig(Game_ConfigAudio& cfg) const override;

private:
	unsigned bgm_starttick = 0;

	bool playing = false;
};

AudioInterface& Audio();

#endif
