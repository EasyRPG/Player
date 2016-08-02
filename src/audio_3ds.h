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
#include <map>
#include <3ds.h>

#define SOUND_CHANNELS 22 // Number of available sounds channel

struct CtrAudio : public AudioInterface {
	CtrAudio();
	~CtrAudio();

	void BGM_Play(std::string const&, int, int, int) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override;
	unsigned BGM_GetTicks() const override;
	void BGM_Fade(int) override;
	void BGM_Volume(int) override;
	void BGM_Pitch(int) override;
	void SE_Play(std::string const&, int, int) override;
	void SE_Stop() override;
	void Update() override;

private:
	u8* audiobuffers[SOUND_CHANNELS]; // We'll use last two available channels for BGM
	uint8_t num_channels = SOUND_CHANNELS;
	ndspWaveBuf dspSounds[SOUND_CHANNELS+1]; // We need one more waveBuf for BGM purposes
	int bgm_volume; // Stubbed
	bool (*isPlayingCallback)(int);
	void (*clearCallback)(int);
	u8 last_ch; // Used only with dsp::DSP

}; // class CtrAudio
