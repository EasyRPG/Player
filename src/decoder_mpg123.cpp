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

#include "system.h"

#ifdef HAVE_MPG123

// Headers
#include <cassert>
#include "decoder_mpg123.h"
#include "output.h"

static ssize_t custom_read(void* io, void* buffer, size_t nbyte) {
	FILE* f = reinterpret_cast<FILE*>(io);
	return fread(buffer, 1, nbyte, f);
}

static off_t custom_seek(void* io, off_t offset, int seek_type) {
	FILE* f = reinterpret_cast<FILE*>(io);
	fseek(f, offset, seek_type);
	return ftell(f);
}

static void custom_close(void* io) {
	FILE* f = reinterpret_cast<FILE*>(io);
	fclose(f);
}

static void noop_close(void*) {}

Mpg123Decoder::Mpg123Decoder() :
	handle(nullptr, mpg123_delete)
{
	music_type = "mp3";

	err = mpg123_init();
	if (err != MPG123_OK) {
		error_message = "mpg123: " + std::string(mpg123_plain_strerror(err));
		return;
	}

	handle.reset(mpg123_new(nullptr, &err));
	mpg123_replace_reader_handle(handle.get(), custom_read, custom_seek, custom_close);

	if (!handle) {
		error_message = "mpg123: " + std::string(mpg123_plain_strerror(err));
		return;
	}

	init = true;
}

Mpg123Decoder::~Mpg123Decoder() {
}

bool Mpg123Decoder::WasInited() const {
	return init;
}

bool Mpg123Decoder::Open(FILE* file) {
	if (!init) {
		return false;
	}

	finished = false;

	err = mpg123_open_handle(handle.get(), file);
	if (err != MPG123_OK) {
		error_message = "mpg123: " + std::string(mpg123_plain_strerror(err));
		return false;
	}

	return true;
}

bool Mpg123Decoder::Seek(size_t offset, Origin origin) {
	finished = false;
	mpg123_seek_frame(handle.get(), offset, (int)origin);

	return true;
}

bool Mpg123Decoder::IsFinished() const {
	return finished;
}

static int format_to_mpg123_format(AudioDecoder::Format format) {
	switch (format) {
		case AudioDecoder::Format::U8:
			return MPG123_ENC_UNSIGNED_8;
		case AudioDecoder::Format::S8:
			return MPG123_ENC_SIGNED_8;
		case AudioDecoder::Format::U16:
			return MPG123_ENC_UNSIGNED_16;
		case AudioDecoder::Format::S16:
			return MPG123_ENC_SIGNED_16;
		case AudioDecoder::Format::U32:
			return MPG123_ENC_UNSIGNED_32;
		case AudioDecoder::Format::S32:
			return MPG123_ENC_SIGNED_32;
		case AudioDecoder::Format::F32:
			return MPG123_ENC_FLOAT_32;
		default:
			assert(false);
	}

	return -1;
}

static AudioDecoder::Format mpg123_format_to_format(int format) {
	switch (format) {
		case MPG123_ENC_UNSIGNED_8:
			return AudioDecoder::Format::U8;
		case MPG123_ENC_SIGNED_8:
			return AudioDecoder::Format::S8;
		case MPG123_ENC_UNSIGNED_16:
			return AudioDecoder::Format::U16;
		case MPG123_ENC_SIGNED_16:
			return AudioDecoder::Format::S16;
		case MPG123_ENC_UNSIGNED_32:
			return AudioDecoder::Format::U32;
		case MPG123_ENC_SIGNED_32:
			return AudioDecoder::Format::S32;
		case MPG123_ENC_FLOAT_32:
			return AudioDecoder::Format::F32;
		default:
			assert(false);
	}

	return (AudioDecoder::Format)-1;
}

void Mpg123Decoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	long freq;
	int ch;
	int fmt;

	mpg123_getformat(handle.get(), &freq, &ch, &fmt);

	frequency = (int)freq;
	channels = ch;
	format = mpg123_format_to_format(fmt);
}

bool Mpg123Decoder::SetFormat(int freq, AudioDecoder::Format fmt, int channels) {
	// mpg123 has a built-in pseudo-resampler, not needing SDL_ConvertAudio later
	// Remove all available conversion formats
	// Add just one format to force mpg123 pseudo-resampler work
	mpg123_format_none(handle.get());

	err = mpg123_format(handle.get(), (long)freq, (int)channels, (int)format_to_mpg123_format(fmt));
	if (err != MPG123_OK) {
		err = mpg123_format(handle.get(), (long)44100, (int)channels, (int)format_to_mpg123_format(fmt));
		if (err != MPG123_OK) {
			mpg123_format(handle.get(), (long)44100, (int)2, (int)MPG123_ENC_SIGNED_16);
		}

		return false;
	}

	return err == MPG123_OK;
}

bool Mpg123Decoder::IsMp3(FILE* stream) {
	Mpg123Decoder decoder;
	// Prevent stream handle destruction
	mpg123_replace_reader_handle(decoder.handle.get(), custom_read, custom_seek, noop_close);
	// Prevent skipping of too many garbage, breaks heuristic
	mpg123_param(decoder.handle.get(), MPG123_RESYNC_LIMIT, 64, 0.0);
	if (!decoder.Open(stream)) {
		return false;
	}

	unsigned char buffer[1024];
	int err = 0;
	size_t done = 0;
	int err_count = 0;
	
	// Read beginning of assumed MP3 file and count errors as an heuristic to detect MP3
	for (int i = 0; i < 10; ++i) {
		err = mpg123_read(decoder.handle.get(), buffer, 1024, &done);
		if (err != MPG123_OK) {
			err_count += 1;
		}
		if (err_count >= 3) {
			break;
		}
	}

	return err_count < 3;
}

int Mpg123Decoder::FillBuffer(uint8_t* buffer, int length) {
	int err;
	size_t done = 0;
	size_t decoded = 0;

	// Skip invalid frames until getting a valid one
	do {
		err = mpg123_read(handle.get(), reinterpret_cast<unsigned char*>(buffer), length, &done);
		decoded += done;
	} while (done && err != MPG123_OK);

	if (err == MPG123_DONE) {
		finished = true;
	}

	return (int)decoded;
}

#endif
