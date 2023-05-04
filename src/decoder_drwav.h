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

#ifndef EP_DECODER_WAV_H
#define EP_DECODER_WAV_H

#include "system.h"
#ifdef WANT_DRWAV

// Headers
#include "audio_decoder.h"
#define DR_WAV_NO_STDIO
// Use system dr_wav header if available
#if __has_include(<dr_wav.h>)
#  include <dr_wav.h>
#else
#  include "external/dr_wav.h"
#endif

/**
 * Standalone audio decoder powered by dr_wav
 */
class DrWavDecoder : public AudioDecoder {
public:
	DrWavDecoder();

	~DrWavDecoder() override;

	bool Open(Filesystem_Stream::InputStream stream) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	int GetTicks() const override;

private:
	int FillBuffer(uint8_t* buffer, int length) override;
	Filesystem_Stream::InputStream stream;
	bool finished = false;
	int decoded_samples = 0;
	bool init = false;
	drwav handle = {};
};

#endif

#endif
