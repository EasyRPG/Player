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

#define BGM_BUFSIZE 524288 // Max dimension of BGM buffer size
#define OGG_BUFSIZE 2048 // Max dimension of PCM16 decoded block by libogg

struct DecodedSound{
	bool isStereo;
	u8* audiobuf;
	u32 audiobuf_size;
	u32 samplerate;
	u16 format;
};

struct DecodedMusic{
	bool isStereo;
	u8* audiobuf;
	u32 audiobuf_size;
	u32 samplerate;
	u16 bytepersample;
	u16 format;
	FILE* handle;
	u32 audiobuf_offs;
	u64 starttick;
	u32 block_idx;
	u32 eof_idx;
	bool isPlaying;
	int fade_val;
	float vol;
	void (*updateCallback)();
	void (*closeCallback)();
};

int DecodeSound(std::string const& filename, DecodedSound* Sound);
int DecodeMusic(std::string const& filename, DecodedMusic* Sound);