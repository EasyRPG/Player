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
#ifndef AUDIOPSP_H
#define AUDIOPSP_H
#include "audio.h"
#include <map>

#define SOUND_CHANNELS 22

struct Psp2Audio : public AudioInterface {
	Psp2Audio();
	~Psp2Audio();

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
	uint8_t num_channels = SOUND_CHANNELS;
	int bgm_volume; // Stubbed
	bool (*isPlayingCallback)(int);
	void (*clearCallback)(int);

}; // class Psp2Audio
#endif