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

#include "audio.h"

#ifndef CTRAUDIO_H
#define CTRAUDIO_H

#ifdef _3DS
#include <3ds.h>
#include <3ds/synchronization.h>
#include <memory>

#include "audio_decoder.h"

class CtrAudio : public AudioInterface {
public:
	CtrAudio();
	~CtrAudio();

	void BGM_Play(std::string const& file, int volume, int pitch, int fadein) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override;
	unsigned BGM_GetTicks() const override;
	void BGM_Fade(int fade) override;
	void BGM_Volume(int volume) override;
	void BGM_Pitch(int pitch) override;
	void SE_Play(std::string const& file, int volume, int pitch) override;
	void SE_Stop() override;
	virtual void Update() override;

	volatile bool term_stream = false;
	
	void LockMutex() const;
	void UnlockMutex() const;
	
	ndspWaveBuf bgm_buf[2];
	std::unique_ptr<AudioDecoder> bgm_decoder;
	LightEvent audio_event;

private:
	mutable LightLock audio_mutex;
	
	ndspWaveBuf se_buf[23];
	unsigned bgm_starttick = 0;
	
	uint32_t* bgm_audio_buffer;
}; // class CtrAudio

#endif

#endif
