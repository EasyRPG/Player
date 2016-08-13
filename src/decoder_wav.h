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

#ifndef EASYRPG_AUDIO_DECODER_WAV_H
#define EASYRPG_AUDIO_DECODER_WAV_H

// Headers
#include "audio_decoder.h"
#include <string>	
#include <memory>

/**
 * Standalone basic audio decoder for WAV
 */
class WavDecoder : public AudioDecoder {
public:
	WavDecoder();

	~WavDecoder();

	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

private:
	int FillBuffer(uint8_t* buffer, int length) override;
	Format output_format;
	FILE * file_;
	bool finished;
	uint32_t samplerate;
	uint16_t nchannels;
	uint32_t audiobuf_offset;
};

#endif
