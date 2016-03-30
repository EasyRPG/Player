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

// Headers
#include <cassert>
#include "audio_decoder.h"
#include "output.h"
#include <SDL_audio.h>

AudioDecoder::~AudioDecoder() {
}

Mpg123Decoder::Mpg123Decoder() :
	handle(nullptr, mpg123_delete)
{
	err = mpg123_init();
	if (err != MPG123_OK) {
		Output::Warning("Couldn't initialize mpg123.\n%s", mpg123_plain_strerror(err));
		return;
	}

	handle.reset(mpg123_new(nullptr, &err));
	if (!handle) {
		Output::Warning("Couldn't create mpg123 handle.\n%s", mpg123_plain_strerror(err));
		return;
	}
}

Mpg123Decoder::~Mpg123Decoder() {
}

bool Mpg123Decoder::Open(const std::string& file) {
	finished = false;

	err = mpg123_open(handle.get(), file.c_str());
	if (err != MPG123_OK) {
		Output::Warning("Couldn't open mpg123 file.\n%s", mpg123_plain_strerror(err));
		return false;
	}

	return true;
}

const std::vector<char>& Mpg123Decoder::Decode(uint8_t* stream, int length) {
	static std::vector<char> buffer;

	buffer.resize(length);

	int err;
	size_t done = 0;

	// Skip invalid frames until getting a valid one
	do err = mpg123_read(handle.get(), reinterpret_cast<unsigned char*>(buffer.data()), length, &done);
	while (done && err != MPG123_OK);
	if (err == MPG123_DONE) {
		finished = true;
		buffer.clear();
		//Mix_HookMusic(NULL, NULL);
	} else {
		//SDL_MixAudio(stream, buffer, len, SDL_MIX_MAXVOLUME);
	}

	return buffer;
}

bool Mpg123Decoder::IsFinished() const {
	return finished;
}

void Mpg123Decoder::GetFormat(int& frequency, AudioDecoder::Format & format, AudioDecoder::Channel & channels) const {
}

bool Mpg123Decoder::SetFormat(int frequency, AudioDecoder::Format fmt, AudioDecoder::Channel channels) {
	// mpg123 has a built-in pseudo-resampler, not needing SDL_ConvertAudio later
	// Remove all available conversion formats
	// Add just one format to force mpg123 pseudo-resampler work
	mpg123_format_none(handle.get());

	format = MPG123_ENC_UNSIGNED_8;

	switch (fmt) {
	case AudioDecoder::Format::U8:
		break;
	case AudioDecoder::Format::S8:
		format = MPG123_ENC_SIGNED_8;
		break;
	case AudioDecoder::Format::U16:
		format = MPG123_ENC_UNSIGNED_16;
		break;
	case AudioDecoder::Format::S16:
		format = MPG123_ENC_SIGNED_16;
		break;
	default:
		assert(false);
	}

	err = mpg123_format(handle.get(), (long)frequency, (int)channels, (int)format);

	/*mpg123_format(mp3_handle, (long)audio_rate, audio_channels, (int)mpg_format);
	if (mp3_err != MPG123_OK) {
		Output::Warning("Couldn't set mpg123 format.\n%s", mpg123_plain_strerror(mp3_err));
		return;
	}*/

	return false;
}

std::string Mpg123Decoder::GetError() const {
	return std::string(mpg123_plain_strerror(err));
}
