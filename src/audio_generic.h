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

#ifndef EASYRPG_AUDIOGENERIC_H_
#define EASYRPG_AUDIOGENERIC_H_

#include "audio.h"
#include "audio_decoder.h"
#include "audio_secache.h"

struct GenericAudio : public AudioInterface {
public:
	GenericAudio();
	~GenericAudio();

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

	bool LockMutex() const;
	bool UnlockMutex() const;

	static void AudioThreadCallback(GenericAudio* audio, uint8_t* output_buffer, int buffer_length);

private:
	struct BgmChannel {
		std::unique_ptr<AudioDecoder> decoder;
		bool paused;
		bool stopped;
	};
	struct SeChannel {
		AudioSeRef se;
		size_t buffer_pos;
		int volume;
		bool paused;
		bool stopped;
		bool finished;
	};
	struct Format {
		int frequency;
		AudioDecoder::Format format;
		int channels;
	};

	static Format output_format;
	static bool PlayOnChannel(BgmChannel& chan,std::string const& file, int volume, int pitch, int fadein);
	static bool PlayOnChannel(SeChannel& chan,std::string const& file, int volume, int pitch);

	static const unsigned nr_of_se_channels=31;
	static const unsigned nr_of_bgm_channels=2;

	static BgmChannel BGM_Channels[nr_of_bgm_channels];
	static SeChannel SE_Channels[nr_of_se_channels];
	static bool BGM_PlayedOnceIndicator;
	static bool Muted;

};

#endif //EASYRPG_AUDIOGENERIC_H_
