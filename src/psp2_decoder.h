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

#ifndef _EASYRPG_3DS_DECODER_H_
#define _EASYRPG_3DS_DECODER_H_

#include <string>
#include "audio_decoder.h"

#define BGM_BUFSIZE 8192 // Max dimension of BGM/SFX buffer size
#define OGG_BUFSIZE 2048 // Max dimension of PCM16 decoded block by libogg

struct DecodedSound{
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	uint32_t audiobuf_size;
	uint32_t samplerate;
	uint32_t orig_samplerate;
	FILE* handle;
	bool isNewTrack;
	bool endedOnce;
	bool isPlaying;
	bool isStereo;
	int fade_val;
	int vol;
	void (*updateCallback)(DecodedSound*);
	void (*closeCallback)(DecodedSound*);
	uint8_t id;
};

struct DecodedMusic{
	bool isStereo;
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	uint32_t audiobuf_size;
	uint32_t samplerate;
	uint32_t orig_samplerate;
	FILE* handle;
	bool isNewTrack;
	bool endedOnce;
	bool isPlaying;
	int fade_val;
	int vol;
	void (*updateCallback)();
	void (*closeCallback)();
};

int DecodeSound(std::string const& filename, DecodedSound* Sound);
int DecodeMusic(std::string const& filename, DecodedMusic* Sound);
extern std::unique_ptr<AudioDecoder> audio_decoder;

#endif
