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

#ifndef EP_DECODER_MPG123_H
#define EP_DECODER_MPG123_H

// Headers
#include "audio_decoder.h"
#include <string>
#ifdef HAVE_LIBMPG123
#include <mpg123.h>
#endif
#include <memory>

/**
 * Audio decoder for MP3 powered by mpg123
 */
class Mpg123Decoder : public AudioDecoder {
public:
	Mpg123Decoder();

	~Mpg123Decoder();

	bool WasInited() const override;

	bool Open(Filesystem_Stream::InputStream stream) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	int GetTicks() const override;

	static bool IsMp3(Filesystem_Stream::InputStream& stream);
private:
	int FillBuffer(uint8_t* buffer, int length) override;

#ifdef HAVE_LIBMPG123
	std::unique_ptr<mpg123_handle, decltype(&mpg123_delete)> handle;
#endif
	Filesystem_Stream::InputStream stream;
	int err = 0;
	bool finished = false;

	long samplerate = 0;
};

#endif
